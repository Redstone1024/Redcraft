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

	/** Copies one string to another. The end sentinel is used only for buffer safety and will not append null characters to the destination. */
	FORCEINLINE static CharType* Copy(CharType* Destination, const CharType* DestinationEnd, const CharType* Source, const CharType* SourceEnd)
	{
		checkf(Destination && Source, TEXT("Read access violation. Destination and source must not be nullptr."));

		if (DestinationEnd == nullptr && SourceEnd == nullptr)
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

		size_t SourceLength = TCString::Length(Source, SourceEnd);

		if (DestinationEnd != nullptr && Destination + SourceLength + 1 > DestinationEnd)
		{
			return nullptr;
		}

		Memory::Memcpy(Destination, Source, SourceLength * sizeof(CharType));

		Destination[SourceLength] = LITERAL(CharType, '\0');

		return Destination;
	}

	/** Concatenates two strings. The end sentinel is used only for buffer safety and will not append null characters to the destination. */
	FORCEINLINE static CharType* Cat(CharType* Destination, const CharType* DestinationEnd, const CharType* Source, const CharType* SourceEnd)
	{
		checkf(Destination && Source, TEXT("Read access violation. Destination and source must not be nullptr."));

		if (DestinationEnd == nullptr && SourceEnd == nullptr)
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

		size_t DestinationLength = TCString::Length(Destination, DestinationEnd);

		CharType* Result = Copy(Destination + DestinationLength, DestinationEnd, Source, SourceEnd);

		return Result ? Destination : nullptr;
	}

	/** @return The length of a given string. The maximum length is the buffer size. */
	NODISCARD FORCEINLINE static size_t Length(const CharType* InString, const CharType* End)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		if (End == nullptr)
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

		while (*InString != LITERAL(CharType, '\0') && InString != End)
		{
			++Result;
			++InString;
		}

		return Result;
	}

	/** Compares two strings. The end sentinel is used only for buffer safety not for comparison. */
	NODISCARD FORCEINLINE static strong_ordering Compare(const CharType* LHS, const CharType* LHSEnd, const CharType* RHS, const CharType* RHSEnd)
	{
		checkf(LHS && RHS, TEXT("Read access violation. LHS and RHS must not be nullptr."));

		if (LHSEnd == nullptr && RHSEnd == nullptr)
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

		while (LHS != LHSEnd && RHS != RHSEnd)
		{
			if (*LHS != *RHS)
			{
				return *LHS <=> *RHS;
			}

			if (*LHS == LITERAL(CharType, '\0') && *RHS == LITERAL(CharType, '\0'))
			{
				return strong_ordering::equal;
			}

			++LHS;
			++RHS;
		}

		if (LHS != LHSEnd && RHS == RHSEnd)
		{
			return *LHS <=> LITERAL(CharType, '\0');
		}
		else if (LHS == LHSEnd && RHS != RHSEnd)
		{
			return LITERAL(CharType, '\0') <=> *RHS;
		}

		return strong_ordering::equal;
	}

	/** Finds the first or last occurrence of a character that satisfies the predicate. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	template <CPredicate<CharType> F>
	NODISCARD FORCEINLINE static const CharType* Find(const CharType* InString, const CharType* End, F&& InPredicate, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		if (SearchDirection == ESearchDirection::FromStart)
		{
			while (InString != End)
			{
				if (InvokeResult<bool>(Forward<F>(InPredicate), *InString))
				{
					return InString;
				}

				if (*InString == LITERAL(CharType, '\0')) break;

				++InString;
			}
		}
		else
		{
			size_t Index = TCString::Length(InString, End);

			const CharType* Iter = InString + Index;

			if (Iter == End) --Iter;

			while (Iter != InString - 1)
			{
				if (InvokeResult<bool>(Forward<F>(InPredicate), *Iter))
				{
					return Iter;
				}

				--Iter;
			}
		}

		return nullptr;
	}

	/** Finds the first or last occurrence of a character that satisfies the predicate. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	template <CPredicate<CharType> F>
	NODISCARD FORCEINLINE static       CharType* Find(      CharType* InString, const CharType* End, F&& InPredicate, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::Find(const_cast<const CharType*>(InString), End, Forward<F>(InPredicate), SearchDirection));
	}

	/** Finds the first or last occurrence of a character. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindChar(const CharType* InString, const CharType* End, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		if (End == nullptr)
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

		return TCString::Find(InString, End, [Character](CharType C) { return C == Character; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindChar(      CharType* InString, const CharType* End, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindChar(const_cast<const CharType*>(InString), End, Character, SearchDirection));
	}
	
	/** Finds the first or last occurrence of a character in a charset. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindChar(const CharType* InString, const CharType* End, const CharType* Charset, const CharType* CharsetEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, TEXT("Read access violation. InString and Charset must not be nullptr."));

		if (End == nullptr && CharsetEnd == nullptr && SearchDirection == ESearchDirection::FromStart)
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
			InString, End,
			[Charset, CharsetEnd](CharType C)
			{
				const CharType* Result = TCString::FindChar(Charset, CharsetEnd, C);
				return Result != nullptr && *Result != LITERAL(CharType, '\0');
			},
			SearchDirection
		);
	}

	/** Finds the first or last occurrence of a character in a charset. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindChar(      CharType* InString, const CharType* End, const CharType* Charset, const CharType* CharsetEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, TEXT("Read access violation. InString and Charset must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindChar(const_cast<const CharType*>(InString), End, Charset, CharsetEnd, SearchDirection));
	}

	/** Finds the first or last occurrence of a character that is not the given character. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindNotChar(const CharType* InString, const CharType* End, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		if (InString == End) return nullptr;

		if (Character == LITERAL(CharType, '\0') && SearchDirection == ESearchDirection::FromStart)
		{
			return *InString != LITERAL(CharType, '\0') ? InString : nullptr;
		}

		if (End == nullptr && SearchDirection == ESearchDirection::FromStart)
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

		return TCString::Find(InString, End, [Character](CharType C) { return C != Character; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character that is not the given character. The terminating null character is considered to be a part of the string. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindNotChar(      CharType* InString, const CharType* End, CharType Character, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString, TEXT("Read access violation. InString must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindNotChar(const_cast<const CharType*>(InString), End, Character, SearchDirection));
	}

	/** Finds the first or last occurrence of a character that is not in the given charset. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static const CharType* FindNotChar(const CharType* InString, const CharType* End, const CharType* Charset, const CharType* CharsetEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, TEXT("Read access violation. InString and Charset must not be nullptr."));

		if (End == nullptr && CharsetEnd == nullptr && SearchDirection == ESearchDirection::FromStart)
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
		
		return TCString::Find(InString, End, [Charset, CharsetEnd](CharType C) { return TCString::FindChar(Charset, CharsetEnd, C) == nullptr; }, SearchDirection);
	}

	/** Finds the first or last occurrence of a character that is not in the given charset. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindNotChar(      CharType* InString, const CharType* End, const CharType* Charset, const CharType* CharsetEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Charset, TEXT("Read access violation. InString and Charset must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindNotChar(const_cast<const CharType*>(InString), End, Charset, CharsetEnd, SearchDirection));
	}

	/** Finds the first or last occurrence of a substring. The end sentinel is used only for buffer safety. */
	NODISCARD             static const CharType* FindString(const CharType* InString, const CharType* End, const CharType* Substring, const CharType* SubstringEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Substring, TEXT("Read access violation. InString and Substring must not be nullptr."));

		if (InString == End) return nullptr;

		if (Substring == SubstringEnd || *Substring == LITERAL(CharType, '\0'))
		{
			if (SearchDirection == ESearchDirection::FromStart) return InString;
			else
			{
				const CharType* Iter = InString + TCString::Length(InString, End);
				if (Iter == End) --Iter;
				return Iter;
			}
		}

		if (End == nullptr && SubstringEnd == nullptr && SearchDirection == ESearchDirection::FromStart)
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

		size_t StringLength    = TCString::Length(InString,  End);
		size_t SubstringLength = TCString::Length(Substring, SubstringEnd);

		if (StringLength < SubstringLength)
		{
			return nullptr;
		}

		if (SearchDirection == ESearchDirection::FromStart)
		{
			for (size_t Index = 0; Index < StringLength - SubstringLength; ++Index)
			{
				if (TCString::Compare(InString + Index, InString + Index + SubstringLength, Substring, Substring + SubstringLength) == 0)
				{
					return InString + Index;
				}
			}
		}
		else
		{
			for (size_t Index = StringLength - SubstringLength; Index > 0; --Index)
			{
				if (TCString::Compare(InString + Index, InString + Index + SubstringLength, Substring, Substring + SubstringLength) == 0)
				{
					return InString + Index;
				}
			}
		}

		return nullptr;
	}

	/** Finds the first or last occurrence of a substring. The end sentinel is used only for buffer safety. */
	NODISCARD FORCEINLINE static       CharType* FindString(      CharType* InString, const CharType* End, const CharType* Substring, const CharType* SubstringEnd, ESearchDirection SearchDirection = ESearchDirection::FromStart)
	{
		checkf(InString && Substring, TEXT("Read access violation. InString and Substring must not be nullptr."));

		check_no_recursion();

		return const_cast<CharType*>(TCString::FindString(const_cast<const CharType*>(InString), End, Substring, SubstringEnd, SearchDirection));
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
