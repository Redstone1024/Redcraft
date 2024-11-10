#pragma once

#include "CoreTypes.h"
#include "Templates/Optional.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

#include <locale>
#include <climits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CCharType = CSameAs<T, char> || CSameAs<T, wchar> || CSameAs<T, u8char> || CSameAs<T, u16char> || CSameAs<T, u32char>;

NAMESPACE_PRIVATE_BEGIN

template <CCharType>
struct TLiteral;

template <>
struct TLiteral<char>
{
	NODISCARD FORCEINLINE static constexpr       char  Select(const char  X, const wchar , const u8char , const u16char , const u32char ) { return X; }
	NODISCARD FORCEINLINE static constexpr const char* Select(const char* X, const wchar*, const u8char*, const u16char*, const u32char*) { return X; }
};

template <>
struct TLiteral<wchar>
{
	NODISCARD FORCEINLINE static constexpr       wchar  Select(const char , const wchar  X, const u8char , const u16char , const u32char ) { return X; }
	NODISCARD FORCEINLINE static constexpr const wchar* Select(const char*, const wchar* X, const u8char*, const u16char*, const u32char*) { return X; }
};

template <>
struct TLiteral<u8char>
{
	NODISCARD FORCEINLINE static constexpr       u8char  Select(const char , const wchar , const u8char  X, const u16char , const u32char ) { return X; }
	NODISCARD FORCEINLINE static constexpr const u8char* Select(const char*, const wchar*, const u8char* X, const u16char*, const u32char*) { return X; }
};

template <>
struct TLiteral<u16char>
{
	NODISCARD FORCEINLINE static constexpr       u16char  Select(const char , const wchar , const u8char , const u16char  X, const u32char ) { return X; }
	NODISCARD FORCEINLINE static constexpr const u16char* Select(const char*, const wchar*, const u8char*, const u16char* X, const u32char*) { return X; }
};

template <>
struct TLiteral<u32char>
{
	NODISCARD FORCEINLINE static constexpr       u32char  Select(const char , const wchar , const u8char , const u16char , const u32char  X) { return X; }
	NODISCARD FORCEINLINE static constexpr const u32char* Select(const char*, const wchar*, const u8char*, const u16char*, const u32char* X) { return X; }
};

NAMESPACE_PRIVATE_END

/** Templated literal struct to allow selection of string literals based on the character type provided, and not on compiler switches. */
#define LITERAL(CharType, StringLiteral) NAMESPACE_PRIVATE::TLiteral<CharType>::Select(TEXT(StringLiteral), WTEXT(StringLiteral), U8TEXT(StringLiteral), U16TEXT(StringLiteral), U32TEXT(StringLiteral))

static_assert(CUnsigned<u8char>,      "TChar assumes u8char is an unsigned integer");
static_assert(CUnsigned<u16char>,     "TChar assumes u16char is an unsigned integer");
static_assert(CUnsigned<u32char>,     "TChar assumes u32char is an unsigned integer");
static_assert(CUnsigned<unicodechar>, "TChar assumes unicodechar is an unsigned integer");

/** Set of utility functions operating on a single character. Implemented based on user-preferred locale and ISO 30112 "i18n". */
template <CCharType T>
struct TChar
{
	using CharType = T;

	/** The maximum number of code units required to represent a single character. if unknown, guess 1. */
	static constexpr size_t MaxCodeUnitLength =
		CSameAs<CharType, char>    ? MB_LEN_MAX :
		CSameAs<CharType, wchar>   ?
			PLATFORM_WINDOWS ? 2 :
			PLATFORM_LINUX   ? 1 : 1 :
		CSameAs<CharType, u8char>  ? 4 :
		CSameAs<CharType, u16char> ? 2 :
		CSameAs<CharType, u32char> ? 1 : 1;

	/** Whether the character type is fixed-length. */
	static constexpr bool bIsFixedLength = MaxCodeUnitLength == 1;

	NODISCARD FORCEINLINE static constexpr bool IsValid(CharType InChar)
	{
		if constexpr (CSameAs<CharType, u8char>)
		{
			if ((InChar & 0b10000000) == 0b00000000) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char> || CSameAs<CharType, u32char>)
		{
			if (InChar >= 0xD800 && InChar <= 0xDBFF) return false;
			if (InChar >= 0xDC00 && InChar <= 0xDFFF) return false;

			return InChar <= 0x10FFFF;
		}

		// Windows uses UTF-16 encoding for wchar.
		else if constexpr (PLATFORM_WINDOWS && (CSameAs<CharType, wchar>))
		{
			return TChar::IsValid(static_cast<u16char>(InChar));
		}

		// Linux uses UTF-32 encoding for wchar.
		else if constexpr (PLATFORM_LINUX && (CSameAs<CharType, wchar>))
		{
			return TChar::IsValid(static_cast<u32char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsNonch(CharType InChar)
	{
		if constexpr (CSameAs<CharType, u8char>)
		{
			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			if (InChar >= U16TEXT('\uFDD0') && InChar <= U16TEXT('\uFDEF')) return true;

			if (InChar == U16TEXT('\uFFFE')) return true;
			if (InChar == U16TEXT('\uFFFF')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			if (InChar >= U32TEXT('\uFDD0') && InChar <= U32TEXT('\uFDEF')) return true;

			if ((InChar & 0x0000FFFE) == 0x0000FFFE) return TChar::IsValid(InChar);

			return false;
		}

		// Windows uses UTF-16 encoding for wchar.
		else if constexpr (PLATFORM_WINDOWS && (CSameAs<CharType, wchar>))
		{
			return TChar::IsNonch(static_cast<u16char>(InChar));
		}

		// Linux uses UTF-32 encoding for wchar.
		else if constexpr (PLATFORM_LINUX && (CSameAs<CharType, wchar>))
		{
			return TChar::IsNonch(static_cast<u32char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsASCII(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			constexpr bool ASCIICompatible = []() -> bool
			{
				constexpr char ASCIITable[] =
					TEXT("\u0000\u0001\u0002\u0003\u0004\u0005\u0006")
					TEXT("\a\b\t\n\v\f\r")
					TEXT("\u000E\u000F")
					TEXT("\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017")
					TEXT("\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F")
					TEXT(" !\"#$%&'()*+,-./0123456789:;<=>?")
					TEXT("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_")
					TEXT("`abcdefghijklmnopqrstuvwxyz{|}~\u007F");

				for (size_t Index = 0; Index <= 0x7F; ++Index)
				{
					if (ASCIITable[Index] != static_cast<char>(Index)) return false;
				}

				return true;
			}
			();

			return ASCIICompatible && 0x00 <= InChar && InChar <= 0x7F;
		}

		else if constexpr (CSameAs<CharType, wchar>)
		{
			constexpr bool ASCIICompatible = []() -> bool
			{
				constexpr wchar ASCIITable[] =
					WTEXT("\u0000\u0001\u0002\u0003\u0004\u0005\u0006")
					WTEXT("\a\b\t\n\v\f\r")
					WTEXT("\u000E\u000F")
					WTEXT("\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017")
					WTEXT("\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F")
					WTEXT(" !\"#$%&'()*+,-./0123456789:;<=>?")
					WTEXT("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_")
					WTEXT("`abcdefghijklmnopqrstuvwxyz{|}~\u007F");

				for (size_t Index = 0; Index <= 0x7F; ++Index)
				{
					if (ASCIITable[Index] != static_cast<wchar>(Index)) return false;
				}

				return true;
			}
			();

			return ASCIICompatible && 0x00 <= InChar && InChar <= 0x7F;
		}

		else if constexpr (CSameAs<CharType, u8char> || CSameAs<CharType, u16char> || CSameAs<CharType, u32char> || CSameAs<CharType, unicodechar>)
		{
			return InChar <= 0x7F;
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsAlnum(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isalnum(InChar, Loc);
		}
		else
		{
			return TChar::IsAlpha(InChar) || TChar::IsDigit(InChar);
		}
	}

	NODISCARD FORCEINLINE static constexpr bool IsAlpha(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isalpha(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0041>..<U005A>;<U0061>..<U007A>;
			 */
			if ((InChar >= U8TEXT('\u0041') && InChar <= U8TEXT('\u005A')) ||
				(InChar >= U8TEXT('\u0061') && InChar <= U8TEXT('\u007A')))
				return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char> || CSameAs<CharType, u32char>)
		{
			checkf(InChar <= LITERAL(CharType, '\u007F'), TEXT("TChar::IsAlpha() only supports basic latin block."));

			if (InChar > LITERAL(CharType, '\u007F')) return false;

			return TChar<u8char>::IsAlpha(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsLower(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::islower(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0061>..<U007A>;
			 */
			if (InChar >= U8TEXT('\u0061') && InChar <= U8TEXT('\u007A')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::IsLower() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return TChar<u8char>::IsLower(static_cast<u8char>(InChar));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::IsLower() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return TChar<u8char>::IsLower(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsUpper(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isupper(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0041>..<U005A>;
			 */
			if (InChar >= U8TEXT('\u0041') && InChar <= U8TEXT('\u005A')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::IsUpper() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return TChar<u8char>::IsUpper(static_cast<u8char>(InChar));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::IsUpper() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return TChar<u8char>::IsUpper(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsDigit(CharType InChar)
	{
		/* <U0030>..<U0039>; */
		return (InChar >= LITERAL(CharType, '0') && InChar <= LITERAL(CharType, '9'));
	}

	NODISCARD FORCEINLINE static constexpr bool IsDigit(CharType InChar, unsigned Base)
	{
		checkf(Base >= 2 && Base <= 36, TEXT("Base must be in the range [2, 36]."));

		/* <U0030>..<U0039>;<U0041>..<U0046>;<U0061>..<U0066>; */
		return
			(InChar >= LITERAL(CharType, '0') && InChar < LITERAL(CharType, '0') + static_cast<signed>(Base)     ) ||
			(InChar >= LITERAL(CharType, 'a') && InChar < LITERAL(CharType, 'a') + static_cast<signed>(Base) - 10) ||
			(InChar >= LITERAL(CharType, 'A') && InChar < LITERAL(CharType, 'A') + static_cast<signed>(Base) - 10);
	}

	NODISCARD FORCEINLINE static constexpr bool IsCntrl(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::iscntrl(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/* <U0000>..<U001F>;<U007F>; */
			return (InChar >= U8TEXT('\u0000') && InChar <= U8TEXT('\u001F')) || InChar == U8TEXT('\u007F');
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			/* <U0000>..<U001F>;<U007F>..<U009F>;<U2028>;<U2029>; */
			return
				(InChar >= U16TEXT('\u0000') && InChar <= U16TEXT('\u001F')) ||
				(InChar >= U16TEXT('\u007F') && InChar <= U16TEXT('\u009F')) ||
				(InChar == U16TEXT('\u2028') || InChar == U16TEXT('\u2029'));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			/* <U0000>..<U001F>;<U007F>..<U009F>;<U2028>;<U2029>; */
			return
				(InChar >= U32TEXT('\u0000') && InChar <= U32TEXT('\u001F')) ||
				(InChar >= U32TEXT('\u007F') && InChar <= U32TEXT('\u009F')) ||
				(InChar == U32TEXT('\u2028') || InChar == U32TEXT('\u2029'));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsGraph(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isgraph(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0021>..<U007E>;
			 */
			if (InChar >= U8TEXT('\u0021') && InChar <= U8TEXT('\u007E')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::IsGraph() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return TChar<u8char>::IsGraph(static_cast<u8char>(InChar));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::IsGraph() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return TChar<u8char>::IsGraph(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsSpace(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isspace(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * ISO/IEC 6429
			 * <U0009>..<U000D>;
			 */
			if (InChar >= U8TEXT('\u0009') && InChar <= U8TEXT('\u000D')) return true;

			/*
			 * BASIC LATIN
			 * <U0020>;
			 */
			if (InChar == U8TEXT('\u0020')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			/*
			 * ISO/IEC 6429
			 * <U0009>..<U000D>;
			 */
			if (InChar >= U16TEXT('\u0009') && InChar <= U16TEXT('\u000D')) return true;

			/*
			 * BASIC LATIN
			 * <U0020>;
			 */
			if (InChar == U16TEXT('\u0020')) return true;

			/*
			 * OGHAM
			 * <U1680>;
			 */
			if (InChar == U16TEXT('\u1680')) return true;

			/*
			 * MONGOL
			 * <U180E>;
			 */
			if (InChar == U16TEXT('\u180E')) return true;

			/*
			 * GENERAL PUNCTUATION
			 * <U2000>..<U2006>;<U2008>..<U200A>;<U2028>;<U2029>;<U205F>;
			 */
			if ((InChar >= U16TEXT('\u2000') && InChar <= U16TEXT('\u2006')) ||
				(InChar >= U16TEXT('\u2008') && InChar <= U16TEXT('\u200A')) ||
				(InChar == U16TEXT('\u2028') || InChar == U16TEXT('\u2029')) ||
				(InChar == U16TEXT('\u205F')))
				return true;

			/*
			 * CJK SYMBOLS AND PUNCTUATION, HIRAGANA
			 * <U3000>;
			 */
			if (InChar == U16TEXT('\u3000')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			/*
			 * ISO/IEC 6429
			 * <U0009>..<U000D>;
			 */
			if (InChar >= U32TEXT('\u0009') && InChar <= U32TEXT('\u000D')) return true;

			/*
			 * BASIC LATIN
			 * <U0020>;
			 */
			if (InChar == U32TEXT('\u0020')) return true;

			/*
			 * OGHAM
			 * <U1680>;
			 */
			if (InChar == U32TEXT('\u1680')) return true;

			/*
			 * MONGOL
			 * <U180E>;
			 */
			if (InChar == U32TEXT('\u180E')) return true;

			/*
			 * GENERAL PUNCTUATION
			 * <U2000>..<U2006>;<U2008>..<U200A>;<U2028>;<U2029>;<U205F>;
			 */
			if ((InChar >= U32TEXT('\u2000') && InChar <= U32TEXT('\u2006')) ||
				(InChar >= U32TEXT('\u2008') && InChar <= U32TEXT('\u200A')) ||
				(InChar == U32TEXT('\u2028') || InChar == U32TEXT('\u2029')) ||
				(InChar == U32TEXT('\u205F')))
				return true;

			/*
			 * CJK SYMBOLS AND PUNCTUATION, HIRAGANA
			 * <U3000>;
			 */
			if (InChar == U32TEXT('\u3000')) return true;

			return false;
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsBlank(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isblank(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/* <U0009>;<U0020>; */
			return InChar == U8TEXT('\u0009') || InChar == U8TEXT('\u0020');
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			/* <U0009>;<U0020>;<U1680>;<U180E>;<U2000>..<U2006>;<U2008>..<U200A>;<U205F>;<U3000>; */
			return
				(InChar >= U16TEXT('\u2000') && InChar <= U16TEXT('\u2006')) ||
				(InChar == U16TEXT('\u0009') || InChar == U16TEXT('\u0020')) ||
				(InChar == U16TEXT('\u1680') || InChar == U16TEXT('\u180E')) ||
				(InChar == U16TEXT('\u2008') || InChar == U16TEXT('\u200A')) ||
				(InChar == U16TEXT('\u205F') || InChar == U16TEXT('\u3000'));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			/* <U0009>;<U0020>;<U1680>;<U180E>;<U2000>..<U2006>;<U2008>..<U200A>;<U205F>;<U3000>; */
			return
				(InChar >= U32TEXT('\u2000') && InChar <= U32TEXT('\u2006')) ||
				(InChar == U32TEXT('\u0009') || InChar == U32TEXT('\u0020')) ||
				(InChar == U32TEXT('\u1680') || InChar == U32TEXT('\u180E')) ||
				(InChar == U32TEXT('\u2008') || InChar == U32TEXT('\u200A')) ||
				(InChar == U32TEXT('\u205F') || InChar == U32TEXT('\u3000'));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsPrint(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::isprint(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0020>..<U007E>;
			 */
			if (InChar >= U8TEXT('\u0020') && InChar <= U8TEXT('\u007E')) return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::IsPrint() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return TChar<u8char>::IsPrint(static_cast<u8char>(InChar));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::IsPrint() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return TChar<u8char>::IsPrint(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr bool IsPunct(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return NAMESPACE_STD::ispunct(InChar, Loc);
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * <U0021>..<U002F>;<U003A>..<U0040>;<U005B>..<U0060>;<U007B>..<U007E>;
			 */
			if ((InChar >= U8TEXT('\u0021') && InChar <= U8TEXT('\u002F')) ||
				(InChar >= U8TEXT('\u003A') && InChar <= U8TEXT('\u0040')) ||
				(InChar >= U8TEXT('\u005B') && InChar <= U8TEXT('\u0060')) ||
				(InChar >= U8TEXT('\u007B') && InChar <= U8TEXT('\u007E')))
				return true;

			return false;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::IsPunct() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return TChar<u8char>::IsPunct(static_cast<u8char>(InChar));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::IsPunct() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return TChar<u8char>::IsPunct(static_cast<u8char>(InChar));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return false;
	}

	NODISCARD FORCEINLINE static constexpr CharType ToLower(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return static_cast<CharType>(NAMESPACE_STD::tolower(InChar, Loc));
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * (<U0041>,<U0061>);(<U0042>,<U0062>);(<U0043>,<U0063>);(<U0044>,<U0064>);
			 * (<U0045>,<U0065>);(<U0046>,<U0066>);(<U0047>,<U0067>);(<U0048>,<U0068>);
			 * (<U0049>,<U0069>);(<U004A>,<U006A>);(<U004B>,<U006B>);(<U004C>,<U006C>);
			 * (<U004D>,<U006D>);(<U004E>,<U006E>);(<U004F>,<U006F>);(<U0050>,<U0070>);
			 * (<U0051>,<U0071>);(<U0052>,<U0072>);(<U0053>,<U0073>);(<U0054>,<U0074>);
			 * (<U0055>,<U0075>);(<U0056>,<U0076>);(<U0057>,<U0077>);(<U0058>,<U0078>);
			 * (<U0059>,<U0079>);(<U005A>,<U007A>);
			 */
			if (InChar >= U8TEXT('\u0041') && InChar <= U8TEXT('\u005A')) return InChar + U8TEXT('\u0020');

			return InChar;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::ToLower() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return static_cast<u16char>(TChar<u8char>::ToLower(static_cast<u8char>(InChar)));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::ToLower() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return static_cast<u16char>(TChar<u8char>::ToLower(static_cast<u8char>(InChar)));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return InChar;
	}

	NODISCARD FORCEINLINE static constexpr CharType ToUpper(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char> || CSameAs<CharType, wchar>)
		{
			NAMESPACE_STD::locale Loc("");
			return static_cast<CharType>(NAMESPACE_STD::toupper(InChar, Loc));
		}

		else if constexpr (CSameAs<CharType, u8char>)
		{
			/*
			 * BASIC LATIN
			 * (<U0061>,<U0041>);(<U0062>,<U0042>);(<U0063>,<U0043>);(<U0064>,<U0044>);/
			 * (<U0065>,<U0045>);(<U0066>,<U0046>);(<U0067>,<U0047>);(<U0068>,<U0048>);/
			 * (<U0069>,<U0049>);(<U006A>,<U004A>);(<U006B>,<U004B>);(<U006C>,<U004C>);/
			 * (<U006D>,<U004D>);(<U006E>,<U004E>);(<U006F>,<U004F>);(<U0070>,<U0050>);/
			 * (<U0071>,<U0051>);(<U0072>,<U0052>);(<U0073>,<U0053>);(<U0074>,<U0054>);/
			 * (<U0075>,<U0055>);(<U0076>,<U0056>);(<U0077>,<U0057>);(<U0078>,<U0058>);/
			 * (<U0079>,<U0059>);(<U007A>,<U005A>);
			 */
			if (InChar >= U8TEXT('\u0061') && InChar <= U8TEXT('\u007A')) return InChar - U8TEXT('\u0020');

			return InChar;
		}

		else if constexpr (CSameAs<CharType, u16char>)
		{
			checkf(InChar <= U16TEXT('\u007F'), TEXT("TChar::ToUpper() only supports basic latin block."));

			if (InChar > U16TEXT('\u007F')) return false;

			return static_cast<u16char>(TChar<u8char>::ToUpper(static_cast<u8char>(InChar)));
		}

		else if constexpr (CSameAs<CharType, u32char>)
		{
			checkf(InChar <= U32TEXT('\u007F'), TEXT("TChar::ToUpper() only supports basic latin block."));

			if (InChar > U32TEXT('\u007F')) return false;

			return static_cast<u16char>(TChar<u8char>::ToUpper(static_cast<u8char>(InChar)));
		}

		else static_assert(sizeof(CharType) == -1, "Unsupported character type");

		return InChar;
	}

	NODISCARD FORCEINLINE static constexpr TOptional<unsigned> ToDigit(CharType InChar)
	{
		switch (InChar)
		{
		case LITERAL(CharType, '0'): return 0;
		case LITERAL(CharType, '1'): return 1;
		case LITERAL(CharType, '2'): return 2;
		case LITERAL(CharType, '3'): return 3;
		case LITERAL(CharType, '4'): return 4;
		case LITERAL(CharType, '5'): return 5;
		case LITERAL(CharType, '6'): return 6;
		case LITERAL(CharType, '7'): return 7;
		case LITERAL(CharType, '8'): return 8;
		case LITERAL(CharType, '9'): return 9;
		case LITERAL(CharType, 'a'): return 10;
		case LITERAL(CharType, 'b'): return 11;
		case LITERAL(CharType, 'c'): return 12;
		case LITERAL(CharType, 'd'): return 13;
		case LITERAL(CharType, 'e'): return 14;
		case LITERAL(CharType, 'f'): return 15;
		case LITERAL(CharType, 'g'): return 16;
		case LITERAL(CharType, 'h'): return 17;
		case LITERAL(CharType, 'i'): return 18;
		case LITERAL(CharType, 'j'): return 19;
		case LITERAL(CharType, 'k'): return 20;
		case LITERAL(CharType, 'l'): return 21;
		case LITERAL(CharType, 'm'): return 22;
		case LITERAL(CharType, 'n'): return 23;
		case LITERAL(CharType, 'o'): return 24;
		case LITERAL(CharType, 'p'): return 25;
		case LITERAL(CharType, 'q'): return 26;
		case LITERAL(CharType, 'r'): return 27;
		case LITERAL(CharType, 's'): return 28;
		case LITERAL(CharType, 't'): return 29;
		case LITERAL(CharType, 'u'): return 30;
		case LITERAL(CharType, 'v'): return 31;
		case LITERAL(CharType, 'w'): return 32;
		case LITERAL(CharType, 'x'): return 33;
		case LITERAL(CharType, 'y'): return 34;
		case LITERAL(CharType, 'z'): return 35;
		case LITERAL(CharType, 'A'): return 10;
		case LITERAL(CharType, 'B'): return 11;
		case LITERAL(CharType, 'C'): return 12;
		case LITERAL(CharType, 'D'): return 13;
		case LITERAL(CharType, 'E'): return 14;
		case LITERAL(CharType, 'F'): return 15;
		case LITERAL(CharType, 'G'): return 16;
		case LITERAL(CharType, 'H'): return 17;
		case LITERAL(CharType, 'I'): return 18;
		case LITERAL(CharType, 'J'): return 19;
		case LITERAL(CharType, 'K'): return 20;
		case LITERAL(CharType, 'L'): return 21;
		case LITERAL(CharType, 'M'): return 22;
		case LITERAL(CharType, 'N'): return 23;
		case LITERAL(CharType, 'O'): return 24;
		case LITERAL(CharType, 'P'): return 25;
		case LITERAL(CharType, 'Q'): return 26;
		case LITERAL(CharType, 'R'): return 27;
		case LITERAL(CharType, 'S'): return 28;
		case LITERAL(CharType, 'T'): return 29;
		case LITERAL(CharType, 'U'): return 30;
		case LITERAL(CharType, 'V'): return 31;
		case LITERAL(CharType, 'W'): return 32;
		case LITERAL(CharType, 'X'): return 33;
		case LITERAL(CharType, 'Y'): return 34;
		case LITERAL(CharType, 'Z'): return 35;
		default: return Invalid;
		}
	}

	NODISCARD FORCEINLINE static constexpr TOptional<CharType> FromDigit(int InDigit)
	{
		if (InDigit < 0 || InDigit > 35) return Invalid;

		return LITERAL(CharType, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")[InDigit];
	}

};

using FChar        = TChar<char>;
using FWChar       = TChar<wchar>;
using FU8Char      = TChar<u8char>;
using FU16Char     = TChar<u16char>;
using FU32Char     = TChar<u32char>;
using FUnicodeChar = TChar<unicodechar>;

static_assert(FUnicodeChar::bIsFixedLength);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
