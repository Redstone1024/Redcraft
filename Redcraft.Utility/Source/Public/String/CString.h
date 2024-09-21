#pragma once

#include "CoreTypes.h"
#include "String/Char.h"
#include "Memory/Memory.h"
#include "Templates/Invoke.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

#include <cstring>
#include <cwchar>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#pragma warning(push)
#pragma warning(disable : 4996)

/** Explicit instructions to ignore buffer size, but may lead to buffer overflow attacks. */
constexpr size_t IGNORE_SIZE = -1;

/** Determines search direction for string operations. */
enum class ESearchDirection
{
	/** Search from the start, moving forward through the string. */
	FromStart,

	/** Search from the end, moving backward through the string. */
	FromEnd,
};

/** Set of utility functions operating on C-style null-terminated byte strings. */
template <CCharType T>
struct TCString
{
	using CharType = T;

	/** Copies one string to another. The size is used only for buffer safety and will not append null characters to the destination. */
	FORCEINLINE static CharType* Copy(CharType* Destination, size_t DestinationSize, const CharType* Source, size_t SourceSize)
	{
		checkf(Destination && Source, "Read access violation. Destination and source must not be nullptr.");

		checkf(DestinationSize != 0 && SourceSize != 0, "Illegal buffer size. DestinationSize and SourceSize must not be zero.");

		if (DestinationSize == IGNORE_SIZE && SourceSize == IGNORE_SIZE)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strcpy(Destination, Source);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcscpy(Destination, Source);
			}
		}

		size_t SourceLength = TCString::Length(Source, SourceSize);

		if (DestinationSize != IGNORE_SIZE && DestinationSize < SourceLength + 1)
		{
			return nullptr;
		}

		Memory::Memcpy(Destination, Source, SourceLength * sizeof(CharType));

		Destination[SourceLength] = LITERAL(CharType, '\0');

		return Destination;
	}

	/** Concatenates two strings. The size is used only for buffer safety and will not append null characters to the destination. */
	FORCEINLINE static CharType* Cat(CharType* Destination, size_t DestinationSize, const CharType* Source, size_t SourceSize)
	{
		checkf(Destination && Source, "Read access violation. Destination and source must not be nullptr.");

		checkf(DestinationSize != 0 && SourceSize != 0, "Illegal buffer size. DestinationSize and SourceSize must not be zero.");

		if (DestinationSize == IGNORE_SIZE && SourceSize == IGNORE_SIZE)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strcat(Destination, Source);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcscat(Destination, Source);
			}
		}

		size_t DestinationLength = TCString::Length(Destination, DestinationSize);

		CharType* Result = Copy(Destination + DestinationLength, DestinationSize - DestinationLength, Source, SourceSize);

		return Result ? Destination : nullptr;
	}

	/** @return The length of a given string. The maximum length is the buffer size. */
	NODISCARD FORCEINLINE static size_t Length(const CharType* InString, size_t SourceSize)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(SourceSize != 0, "Illegal buffer size. SourceSize must not be zero.");

		if (SourceSize == IGNORE_SIZE)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strlen(InString);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcslen(InString);
			}
		}

		size_t Result = 0;

		while (*InString++ != LITERAL(CharType, '\0') && SourceSize--)
		{
			++Result;
		}

		return Result;
	}

	/** Compares two strings. The size is used only for buffer safety not for comparison. */
	NODISCARD FORCEINLINE static strong_ordering Compare(const CharType* LHS, size_t LHSSize, const CharType* RHS, size_t RHSSize)
	{
		checkf(LHS && RHS, "Read access violation. LHS and RHS must not be nullptr.");

		checkf(LHSSize != 0 && RHSSize != 0, "Illegal buffer size. LHSSize and RHSSize must not be zero.");

		if (LHSSize == IGNORE_SIZE && RHSSize == IGNORE_SIZE)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strcmp(LHS, RHS) <=> 0;
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcscmp(LHS, RHS) <=> 0;
			}
		}

		while (LHSSize-- && RHSSize--)
		{
			if (*LHS != *RHS)
			{
				return *LHS <=> *RHS;
			}

			if (*LHS++ == LITERAL(CharType, '\0') || *RHS++ == LITERAL(CharType, '\0')) break;
		}

		return strong_ordering::equal;
	}

	/** Finds the first or last occurrence of a character that satisfies the predicate. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	template <CPredicate<CharType> F>
	NODISCARD FORCEINLINE static const CharType* Find(const CharType* InString, size_t BufferSize, F&& InPredicate, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		if (SearchDirection == ESearchDirection::FromStart)
		{
			while (BufferSize--)
			{
				if (InvokeResult<bool>(Forward<F>(InPredicate), *InString))
				{
					return InString;
				}

				if (*InString++ == LITERAL(CharType, '\0')) break;
			}
		}
		else
		{
			size_t Index = TCString::Length(InString, BufferSize);

			if (Index == BufferSize) --Index;

			while (true)
			{
				if (InvokeResult<bool>(Forward<F>(InPredicate), InString[Index]))
				{
					return InString + Index;
				}

				if (!Index--) break;
			}
		}

		return nullptr;
	}

	/** Finds the first or last occurrence of a character that satisfies the predicate. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	template <CPredicate<CharType> F>
	NODISCARD FORCEINLINE static       CharType* Find(      CharType* InString, size_t BufferSize, F&& InPredicate, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::Find(const_cast<const CharType*>(InString), BufferSize, Forward<F>(InPredicate), SearchDirection));
	}

	/** Finds the first or last occurrence of a character. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindChar(const CharType* InString, size_t BufferSize, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		if (BufferSize == IGNORE_SIZE)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return SearchDirection == ESearchDirection::FromStart ? NAMESPACE_STD::strchr(InString, Character) : NAMESPACE_STD::strrchr(InString, Character);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return SearchDirection == ESearchDirection::FromStart ? NAMESPACE_STD::wcschr(InString, Character) : NAMESPACE_STD::wcsrchr(InString, Character);
			}
		}

		return TCString::Find(InString, BufferSize, [Character](CharType C) { return C == Character; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindChar(      CharType* InString, size_t BufferSize, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindChar(const_cast<const CharType*>(InString), BufferSize, Character, SearchDirection));
	}
	
	/** Finds the first or last occurrence of a character in a charset. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindChar(const CharType* InString, size_t BufferSize, const CharType* Charset, size_t CharsetSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, "Read access violation. InString and Charset must not be nullptr.");

		checkf(BufferSize != 0 && CharsetSize != 0, "Illegal buffer size. BufferSize and CharsetSize must not be zero.");

		if (BufferSize == IGNORE_SIZE && CharsetSize == IGNORE_SIZE && SearchDirection == ESearchDirection::FromStart)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strpbrk(InString, Charset);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcspbrk(InString, Charset);
			}
		}

		return TCString::Find
		(
			InString, BufferSize,
			[Charset, CharsetSize](CharType C)
			{
				const CharType* Result = TCString::FindChar(Charset, CharsetSize, C);
				return Result != nullptr && *Result != LITERAL(CharType, '\0');
			},
			SearchDirection
		);
	}

	/** Finds the first or last occurrence of a character in a charset. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindChar(      CharType* InString, size_t BufferSize, const CharType* Charset, size_t CharsetSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, "Read access violation. InString and Charset must not be nullptr.");

		checkf(BufferSize != 0 && CharsetSize != 0, "Illegal buffer size. BufferSize and CharsetSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindChar(const_cast<const CharType*>(InString), BufferSize, Charset, CharsetSize, SearchDirection));
	}

	/** Finds the first or last occurrence of a character that is not the given character. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindNotChar(const CharType* InString, size_t BufferSize, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		if (Character == LITERAL(CharType, '\0') && SearchDirection == ESearchDirection::FromStart)
		{
			return *InString != LITERAL(CharType, '\0') ? InString : nullptr;
		}

		if (BufferSize == IGNORE_SIZE && SearchDirection == ESearchDirection::FromStart)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				const CharType Charset[] = { Character, LITERAL(CharType, '\0') };
				return InString + NAMESPACE_STD::strspn(InString, Charset);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				const CharType Charset[] = { Character, LITERAL(CharType, '\0') };
				return InString + NAMESPACE_STD::wcsspn(InString, Charset);
			}
		}

		return TCString::Find(InString, BufferSize, [Character](CharType C) { return C != Character; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character that is not the given character. The terminating null character is considered to be a part of the string. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindNotChar(      CharType* InString, size_t BufferSize, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, "Read access violation. InString must not be nullptr.");

		checkf(BufferSize != 0, "Illegal buffer size. BufferSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindNotChar(const_cast<const CharType*>(InString), BufferSize, Character, SearchDirection));
	}

	/** Finds the first or last occurrence of a character that is not in the given charset. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindNotChar(const CharType* InString, size_t BufferSize, const CharType* Charset, size_t CharsetSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, "Read access violation. InString and Charset must not be nullptr.");

		checkf(BufferSize != 0 && CharsetSize != 0, "Illegal buffer size. BufferSize and CharsetSize must not be zero.");

		if (BufferSize == IGNORE_SIZE && CharsetSize == IGNORE_SIZE && SearchDirection == ESearchDirection::FromStart)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				size_t Index = NAMESPACE_STD::strspn(InString, Charset);
				return InString[Index] != LITERAL(CharType, '\0') ? InString + Index : nullptr;
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				size_t Index = NAMESPACE_STD::wcsspn(InString, Charset);
				return InString[Index] != LITERAL(CharType, '\0') ? InString + Index : nullptr;
			}
		}
		
		return TCString::Find(InString, BufferSize, [Charset, CharsetSize](CharType C) { return TCString::FindChar(Charset, CharsetSize, C) == nullptr; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character that is not in the given charset. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindNotChar(      CharType* InString, size_t BufferSize, const CharType* Charset, size_t CharsetSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, "Read access violation. InString and Charset must not be nullptr.");

		checkf(BufferSize != 0 && CharsetSize != 0, "Illegal buffer size. BufferSize and CharsetSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindNotChar(const_cast<const CharType*>(InString), BufferSize, Charset, CharsetSize, SearchDirection));
	}

	/** Finds the first or last occurrence of a substring. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindString(const CharType* InString, size_t BufferSize, const CharType* Substring, size_t SubstringSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Substring, "Read access violation. InString and Substring must not be nullptr.");

		checkf(BufferSize != 0 && SubstringSize != 0, "Illegal buffer size. BufferSize and SubstringSize must not be zero.");

		if (*Substring == LITERAL(CharType, '\0'))
		{
			return SearchDirection == ESearchDirection::FromStart ? InString : InString + TCString::Length(InString, BufferSize);
		}

		if (BufferSize == IGNORE_SIZE && SubstringSize == IGNORE_SIZE && SearchDirection == ESearchDirection::FromStart)
		{
			if constexpr (CSameAs<CharType, char>)
			{
				return NAMESPACE_STD::strstr(InString, Substring);
			}
			else if constexpr (CSameAs<CharType, wchar_t>)
			{
				return NAMESPACE_STD::wcsstr(InString, Substring);
			}
		}

		size_t StringLength    = TCString::Length(InString,  BufferSize);
		size_t SubstringLength = TCString::Length(Substring, SubstringSize);

		if (StringLength < SubstringLength)
		{
			return nullptr;
		}

		if (SearchDirection == ESearchDirection::FromStart)
		{
			for (size_t Index = 0; Index < StringLength - SubstringLength; ++Index)
			{
				if (TCString::Compare(InString + Index, SubstringLength, Substring, SubstringLength) == 0)
				{
					return InString + Index;
				}
			}
		}
		else
		{
			for (size_t Index = StringLength - SubstringLength; Index > 0; --Index)
			{
				if (TCString::Compare(InString + Index, SubstringLength, Substring, SubstringLength) == 0)
				{
					return InString + Index;
				}
			}
		}

		return nullptr;
	}

	/** Finds the first or last occurrence of a substring. The size is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindString(      CharType* InString, size_t BufferSize, const CharType* Substring, size_t SubstringSize, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Substring, "Read access violation. InString and Substring must not be nullptr.");

		checkf(BufferSize != 0 && SubstringSize != 0, "Illegal buffer size. BufferSize and SubstringSize must not be zero.");

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindString(const_cast<const CharType*>(InString), BufferSize, Substring, SubstringSize, SearchDirection));
	}

};

using FCString    = TCString<char>;
using FWCString   = TCString<wchar_t>;
using FU8CString  = TCString<char8_t>;
using FU16CString = TCString<char16_t>;
using FU32CString = TCString<char32_t>;

#pragma warning(pop)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
