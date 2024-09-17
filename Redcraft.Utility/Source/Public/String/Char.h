#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

#include <cctype>
#include <cwctype>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CCharType = CSameAs<T, char> || CSameAs<T, wchar_t> || CSameAs<T, char8_t> || CSameAs<T, char16_t> || CSameAs<T, char32_t>;

NAMESPACE_PRIVATE_BEGIN

template <CCharType>
struct TLiteral;

template <>
struct TLiteral<char>
{
	FORCEINLINE static constexpr const char  Select(const char  X, const wchar_t , const char8_t , const char16_t , const char32_t ) { return X; }
	FORCEINLINE static constexpr const char* Select(const char* X, const wchar_t*, const char8_t*, const char16_t*, const char32_t*) { return X; }
};

template <>
struct TLiteral<wchar_t>
{
	FORCEINLINE static constexpr const wchar_t  Select(const char , const wchar_t  X, const char8_t , const char16_t , const char32_t ) { return X; }
	FORCEINLINE static constexpr const wchar_t* Select(const char*, const wchar_t* X, const char8_t*, const char16_t*, const char32_t*) { return X; }
};

template <>
struct TLiteral<char8_t>
{
	FORCEINLINE static constexpr const char8_t  Select(const char , const wchar_t , const char8_t  X, const char16_t , const char32_t ) { return X; }
	FORCEINLINE static constexpr const char8_t* Select(const char*, const wchar_t*, const char8_t* X, const char16_t*, const char32_t*) { return X; }
};

template <>
struct TLiteral<char16_t>
{
	FORCEINLINE static constexpr const char16_t  Select(const char , const wchar_t , const char8_t , const char16_t  X, const char32_t ) { return X; }
	FORCEINLINE static constexpr const char16_t* Select(const char*, const wchar_t*, const char8_t*, const char16_t* X, const char32_t*) { return X; }
};

template <>
struct TLiteral<char32_t>
{
	FORCEINLINE static constexpr const char32_t  Select(const char , const wchar_t , const char8_t , const char16_t , const char32_t  X) { return X; }
	FORCEINLINE static constexpr const char32_t* Select(const char*, const wchar_t*, const char8_t*, const char16_t*, const char32_t* X) { return X; }
};

NAMESPACE_PRIVATE_END

/** Templated literal struct to allow selection of string literals based on the character type provided, and not on compiler switches. */
#define LITERAL(CharType, StringLiteral) NAMESPACE_PRIVATE::TLiteral<CharType>::Select(StringLiteral, WTEXT(StringLiteral), U8TEXT(StringLiteral), U16TEXT(StringLiteral), U32TEXT(StringLiteral))

/** Set of utility functions operating on a single character. Implemented based on ISO 30112 "i18n" */
template <CCharType T>
struct TChar
{
	using CharType = T;

	inline static constexpr CharType NONE = CharType(-1);

	FORCEINLINE static constexpr bool IsAlnum(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isalnum(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswalnum(InChar);
		}
		else
		{
			return IsAlpha(InChar) || IsDigit(InChar);
		}
	}

	FORCEINLINE static constexpr bool IsAlpha(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isalpha(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswalpha(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
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
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsLower(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::islower(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswlower(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/*
			 * BASIC LATIN
			 * <U0061>..<U007A>;
			 */
			if (InChar >= U8TEXT('\u0061') && InChar <= U8TEXT('\u007A')) return true;

			return false;
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsUpper(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isupper(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswupper(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/*
			 * BASIC LATIN
			 * <U0041>..<U005A>;
			 */
			if (InChar >= U8TEXT('\u0041') && InChar <= U8TEXT('\u005A')) return true;

			return false;
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}
	
	FORCEINLINE static constexpr bool IsDigit(CharType InChar)
	{
		/* <U0030>..<U0039>; */
		return (InChar >= LITERAL(CharType, '0') && InChar <= LITERAL(CharType, '9'));
	}

	FORCEINLINE static constexpr bool IsDigit(CharType InChar, int Base)
	{
		checkf(Base >= 2 && Base <= 36, TEXT("Base must be in the range [2, 36]."));

		/* <U0030>..<U0039>;<U0041>..<U0046>;<U0061>..<U0066>; */
		return
			(InChar >= LITERAL(CharType, '0') && InChar < LITERAL(CharType, '0') + Base     ) ||
			(InChar >= LITERAL(CharType, 'a') && InChar < LITERAL(CharType, 'a') + Base - 10) ||
			(InChar >= LITERAL(CharType, 'A') && InChar < LITERAL(CharType, 'A') + Base - 10);
	}

	FORCEINLINE static constexpr bool IsCntrl(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::iscntrl(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswcntrl(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/* <U0000>..<U001F>;<U007F>; */
			return (InChar >= U8TEXT('\u0000') && InChar <= U8TEXT('\u001F')) || InChar == U8TEXT('\u007F');
		}
		else if constexpr (CSameAs<CharType, char16_t>)
		{
			/* <U0000>..<U001F>;<U007F>..<U009F>;<U2028>;<U2029>; */
			return
				(InChar >= U16TEXT('\u0000') && InChar <= U16TEXT('\u001F')) ||
				(InChar >= U16TEXT('\u007F') && InChar <= U16TEXT('\u009F')) ||
				(InChar == U16TEXT('\u2028') || InChar == U16TEXT('\u2029'));
		}
		else if constexpr (CSameAs<CharType, char32_t>)
		{
			/* <U0000>..<U001F>;<U007F>..<U009F>;<U2028>;<U2029>; */
			return
				(InChar >= U32TEXT('\u0000') && InChar <= U32TEXT('\u001F')) ||
				(InChar >= U32TEXT('\u007F') && InChar <= U32TEXT('\u009F')) ||
				(InChar == U32TEXT('\u2028') || InChar == U32TEXT('\u2029'));
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsGraph(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isgraph(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswgraph(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/*
			 * BASIC LATIN
			 * <U0021>..<U007E>;
			 */
			if (InChar >= U8TEXT('\u0021') && InChar <= U8TEXT('\u007E')) return true;

			return false;
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsSpace(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isspace(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswspace(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
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
		else if constexpr (CSameAs<CharType, char16_t>)
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
		else if constexpr (CSameAs<CharType, char32_t>)
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
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsBlank(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isblank(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswblank(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/* <U0009>;<U0020>; */
			return InChar == U8TEXT('\u0009') || InChar == U8TEXT('\u0020');
		}
		else if constexpr (CSameAs<CharType, char16_t>)
		{
			/* <U0009>;<U0020>;<U1680>;<U180E>;<U2000>..<U2006>;<U2008>..<U200A>;<U205F>;<U3000>; */
			return
				(InChar >= U16TEXT('\u2000') && InChar <= U16TEXT('\u2006')) ||
				(InChar == U16TEXT('\u0009') || InChar == U16TEXT('\u0020')) ||
				(InChar == U16TEXT('\u1680') || InChar == U16TEXT('\u180E')) ||
				(InChar == U16TEXT('\u2008') || InChar == U16TEXT('\u200A')) ||
				(InChar == U16TEXT('\u205F') || InChar == U16TEXT('\u3000'));
		}
		else if constexpr (CSameAs<CharType, char32_t>)
		{
			/* <U0009>;<U0020>;<U1680>;<U180E>;<U2000>..<U2006>;<U2008>..<U200A>;<U205F>;<U3000>; */
			return
				(InChar >= U32TEXT('\u2000') && InChar <= U32TEXT('\u2006')) ||
				(InChar == U32TEXT('\u0009') || InChar == U32TEXT('\u0020')) ||
				(InChar == U32TEXT('\u1680') || InChar == U32TEXT('\u180E')) ||
				(InChar == U32TEXT('\u2008') || InChar == U32TEXT('\u200A')) ||
				(InChar == U32TEXT('\u205F') || InChar == U32TEXT('\u3000'));
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsPrint(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::isprint(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswprint(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
		{
			/*
			 * BASIC LATIN
			 * <U0020>..<U007E>;
			 */
			if (InChar >= U8TEXT('\u0020') && InChar <= U8TEXT('\u007E')) return true;

			return false;
		}
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr bool IsPunct(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return NAMESPACE_STD::ispunct(static_cast<unsigned char>(InChar));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return NAMESPACE_STD::iswpunct(InChar);
		}
		else if constexpr (CSameAs<CharType, char8_t>)
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
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr CharType ToLower(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return static_cast<CharType>(NAMESPACE_STD::tolower(static_cast<unsigned char>(InChar)));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return static_cast<CharType>(NAMESPACE_STD::towlower(InChar));
		}
		else if constexpr (CSameAs<CharType, char8_t>)
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
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr CharType ToUpper(CharType InChar)
	{
		if constexpr (CSameAs<CharType, char>)
		{
			return static_cast<CharType>(NAMESPACE_STD::toupper(static_cast<unsigned char>(InChar)));
		}
		else if constexpr (CSameAs<CharType, wchar_t>)
		{
			return static_cast<CharType>(NAMESPACE_STD::towupper(InChar));
		}
		else if constexpr (CSameAs<CharType, char8_t>)
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
		else
		{
			static_assert(sizeof(CharType) == -1, "Unsupported character type");
		}
	}

	FORCEINLINE static constexpr int ToDigit(CharType InChar)
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
		default: return -1;
		}
	}

	FORCEINLINE static constexpr CharType FromDigit(int InDigit)
	{
		if (InDigit < 0 || InDigit >= 36) return NONE;

		return LITERAL(CharType, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")[InDigit];
	}
};

using FChar    = TChar<char>;
using FWChar   = TChar<wchar_t>;
using FU8Char  = TChar<char8_t>;
using FU16Char = TChar<char16_t>;
using FU32Char = TChar<char32_t>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
