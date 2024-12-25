#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Ranges/Utility.h"
#include "Numerics/Limits.h"
#include "Algorithms/Basic.h"
#include "Memory/Allocators.h"
#include "Memory/Address.h"
#include "Containers/Array.h"
#include "Strings/Char.h"
#include "Miscellaneous/AssertionMacros.h"

#include <charconv>

#pragma warning(push)
#pragma warning(disable : 4146)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename R>
concept CStringRange = CInputRange<R> && CCharType<TRangeElement<R>>;

template <typename I>
concept CStringIterator = CInputIterator<I> && CCharType<TIteratorElement<I>>;

NAMESPACE_BEGIN(Algorithms)

/**
 * Parses a boolean value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 *
 * - "True"  become true.
 * - "False" become false.
 *
 * @param Range - The range of characters to parse.
 * @param Value - The boolean value to parse.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringRange R>
constexpr bool Parse(R&& Range, bool& Value)
{
	using FCharTraits = TChar<TRangeElement<R>>;

	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	bool Result;

	// Ignore leading spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter == Sent) return false;

	// Parse the true value.
	if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 't') || *Iter == LITERAL(TRangeElement<R>, 'T')))
	{
		++Iter;

		Result = true;

		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'r') || *Iter == LITERAL(TRangeElement<R>, 'R'))) ++Iter; else return false;
		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'u') || *Iter == LITERAL(TRangeElement<R>, 'U'))) ++Iter; else return false;
		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'e') || *Iter == LITERAL(TRangeElement<R>, 'E'))) ++Iter; else return false;
	}

	// Parse the false value.
	else if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'f') || *Iter == LITERAL(TRangeElement<R>, 'F')))
	{
		++Iter;

		Result = false;

		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'a') || *Iter == LITERAL(TRangeElement<R>, 'A'))) ++Iter; else return false;
		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'l') || *Iter == LITERAL(TRangeElement<R>, 'L'))) ++Iter; else return false;
		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 's') || *Iter == LITERAL(TRangeElement<R>, 'S'))) ++Iter; else return false;
		if (Iter != Sent && (*Iter == LITERAL(TRangeElement<R>, 'e') || *Iter == LITERAL(TRangeElement<R>, 'E'))) ++Iter; else return false;
	}

	else return false;

	// Ignore trailing spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter != Sent) return false;

	Value = Result;

	return true;
}

/**
 * Parses a boolean value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 *
 * - "True"  become true.
 * - "False" become false.
 *
 * @param First - The iterator of the range.
 * @param Last  - The sentinel of the range.
 * @param Value - The boolean value to parse.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringIterator I, CSentinelFor<I> S>
FORCEINLINE constexpr bool Parse(I First, S Last, bool& Value)
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Parse(Ranges::View(MoveTemp(First), Last), Value);
}

/**
 * Parses an integral value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 * If the ingeter value is unsigned, the negative sign causes the parsing to fail.
 * Allow parsing base prefixes: "0x" for hexadecimal, "0b" for binary, and "0" for octal.
 *
 * @param Range - The range of characters to parse.
 * @param Value - The integral value to parse.
 * @param Base  - The base of the number, between [2, 36], or 0 for auto-detect.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringRange R, CIntegral T> requires (!CConst<T> && !CVolatile<T> && !CSameAs<T, bool>)
constexpr bool Parse(R&& Range, T& Value, uint Base = 0)
{
	using FCharTraits = TChar<TRangeElement<R>>;

	checkf(Base == 0 || (Base >= 2 && Base <= 36), TEXT("Illegal base. Please check the Base."));

	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	// Ignore leading spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter == Sent) return false;

	bool bNegative = false;

	// Parse the negative sign.
	if constexpr (CSigned<T>)
	{
		if (*Iter == LITERAL(TRangeElement<R>, '-'))
		{
			bNegative = true;
			++Iter;
		}
	}

	// Parse the positive sign.
	if (!bNegative && *Iter == LITERAL(TRangeElement<R>, '+')) ++Iter;

	// Auto-detect the base.
	if (Base == 0)
	{
		if (Iter == Sent) return false;

		if (*Iter == LITERAL(TRangeElement<R>, '0'))
		{
			++Iter;

			// Return zero if the string has only one zero.
			if (Iter == Sent || FCharTraits::IsSpace(*Iter))
			{
				while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

				if (Iter != Sent) return false;

				Value = 0;

				return true;
			}

			if (*Iter == LITERAL(TRangeElement<R>, 'x') || *Iter == LITERAL(TRangeElement<R>, 'X'))
			{
				Base = 16;
				++Iter;
			}

			else if (*Iter == LITERAL(TRangeElement<R>, 'b') || *Iter == LITERAL(TRangeElement<R>, 'B'))
			{
				Base = 2;
				++Iter;
			}

			else if (FCharTraits::IsDigit(*Iter, 8)) Base = 8;

			else return false;
		}

		else Base = 10;
	}

	// Parse the base prefix.
	else if (Base == 2 || Base == 16)
	{
		if (Iter == Sent) return false;

		if (*Iter == LITERAL(TRangeElement<R>, '0'))
		{
			++Iter;

			// Return zero if the string has only one zero.
			if (Iter == Sent || FCharTraits::IsSpace(*Iter))
			{
				while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

				if (Iter != Sent) return false;

				Value = 0;

				return true;
			}

			if (Base == 16 && (*Iter == LITERAL(TRangeElement<R>, 'x') || *Iter == LITERAL(TRangeElement<R>, 'X'))) ++Iter;
			if (Base ==  2 && (*Iter == LITERAL(TRangeElement<R>, 'b') || *Iter == LITERAL(TRangeElement<R>, 'B'))) ++Iter;
		}
	}

	if (Iter == Sent) return false;

	check(Base >= 2 && Base <= 36);

	if (!FCharTraits::IsDigit(*Iter, Base)) return false;

	using FUnsignedT = TMakeUnsigned<T>;

	FUnsignedT LastValue = 0;
	FUnsignedT Unsigned  = 0;

	do
	{
		uint Digit = FCharTraits::ToDigit(*Iter);

		// Break if the char is not a digit.
		if (Digit >= Base) break;

		++Iter;

		LastValue = Unsigned;

		Unsigned = LastValue * Base + Digit;

		// Fail if the value is overflowed.
		if (Unsigned < LastValue) return false;
	}
	while (Iter != Sent);

	// Ignore trailing spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter != Sent) return false;

	if constexpr (CSigned<T>)
	{
		// Fail if the value is overflowed.
		if (!bNegative && Unsigned >= static_cast<FUnsignedT>(TNumericLimits<T>::Max())) return false;
		if ( bNegative && Unsigned >= static_cast<FUnsignedT>(TNumericLimits<T>::Min())) return false;

		// Reverse if the value is negative.
		if (bNegative) Unsigned = -Unsigned;
	}

	Value = Unsigned;

	return true;
}

/**
 * Parses an integral value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 * If the ingeter value is unsigned, the negative sign causes the parsing to fail.
 * Allow parsing base prefixes: "0x" for hexadecimal, "0b" for binary, and "0" for octal.
 *
 * @param First - The iterator of the range.
 * @param Last  - The sentinel of the range.
 * @param Value - The integral value to parse.
 * @param Base  - The base of the number, between [2, 36], or 0 for auto-detect.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringIterator I, CSentinelFor<I> S, CIntegral T> requires (!CConst<T> && !CVolatile<T> && !CSameAs<T, bool>)
FORCEINLINE constexpr bool Parse(I First, S Last, T& Value, uint Base = 0)
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Parse(Ranges::View(MoveTemp(First), Last), Value, Base);
}

/**
 * Parses a floating-point value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 * Automatically detect formats if multiple formats are allowed.
 * Allow parsing base prefixes: "0x" for hexadecimal.
 *
 * @param Range       - The range of characters to parse.
 * @param Value       - The floating-point value to parse.
 * @param bFixed      - Allow parsing fixed-point values.
 * @param bScientific - Allow parsing scientific notation values.
 * @param bHex        - Allow parsing hex floating-point values.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringRange R, CFloatingPoint T> requires (!CConst<T> && !CVolatile<T>)
constexpr bool Parse(R&& Range, T& Value, bool bFixed = true, bool bScientific = true, bool bHex = true)
{
	if (!bFixed && !bScientific && !bHex) return false;

	using FCharTraits = TChar<TRangeElement<R>>;

	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	// Ignore leading spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter == Sent) return false;

	bool bNegative = false;

	// Parse the negative sign.
	if (*Iter == LITERAL(TRangeElement<R>, '-'))
	{
		bNegative = true;
		++Iter;
	}

	// Parse the positive sign.
	else if (*Iter == LITERAL(TRangeElement<R>, '+')) ++Iter;

	if (Iter == Sent) return false;

	// Fail if the string has multiple signs.
	if (*Iter == LITERAL(TRangeElement<R>, '-')) return false;
	if (*Iter == LITERAL(TRangeElement<R>, '+')) return false;

	NAMESPACE_STD::chars_format Format = NAMESPACE_STD::chars_format::general;

	if      ( bFixed && !bScientific) Format = NAMESPACE_STD::chars_format::fixed;
	else if (!bFixed &&  bScientific) Format = NAMESPACE_STD::chars_format::scientific;
	else if (!bFixed && !bScientific) Format = NAMESPACE_STD::chars_format::hex;

	// Auto-detect the hex format.
	if (bHex)
	{
		if (*Iter == LITERAL(TRangeElement<R>, '0'))
		{
			++Iter;

			// Return zero if the string has only one zero.
			if (Iter == Sent || FCharTraits::IsSpace(*Iter))
			{
				while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

				if (Iter != Sent) return false;

				Value = static_cast<T>(bNegative ? -0.0 : 0.0);

				return true;
			}

			if (*Iter == LITERAL(TRangeElement<R>, 'x') || *Iter == LITERAL(TRangeElement<R>, 'X'))
			{
				Format = NAMESPACE_STD::chars_format::hex;
				++Iter;
			}
		}
	}

	if (Iter == Sent) return false;

	T Result;

	// Copy to a buffer if the range is not contiguous.
	if constexpr (!CContiguousRange<R> || !CSameAs<TRangeElement<R>, char>)
	{
		TArray<char, TInlineAllocator<64>> Buffer;

		for (; Iter != Sent; ++Iter)
		{
			auto Char = *Iter;

			// Ignore trailing spaces.
			if (FCharTraits::IsSpace(Char)) break;

			// Assert that floating-point values must be represented by ASCII.
			if (FCharTraits::IsASCII(Char)) Buffer.PushBack(static_cast<char>(Char));

			else return false;
		}

		const char* First = Buffer.GetData();
		const char* Last  = Buffer.GetData() + Buffer.Num();

		NAMESPACE_STD::from_chars_result ConvertResult = NAMESPACE_STD::from_chars(First, Last, Result, Format);

		if (ConvertResult.ec == NAMESPACE_STD::errc::result_out_of_range) return false;
		if (ConvertResult.ec == NAMESPACE_STD::errc::invalid_argument)    return false;

		// Assert that the buffer is fully parsed.
		if (ConvertResult.ptr != Last) return false;
	}

	else
	{
		const char* First = ToAddress(Iter);
		const char* Last  = ToAddress(Iter) + Algorithms::Distance(Iter, Sent);

		NAMESPACE_STD::from_chars_result ConvertResult = NAMESPACE_STD::from_chars(First, Last, Result, Format);

		if (ConvertResult.ec == NAMESPACE_STD::errc::result_out_of_range) return false;
		if (ConvertResult.ec == NAMESPACE_STD::errc::invalid_argument)    return false;

		// Move the iterator to the end of the parsed value.
		Algorithms::Advance(Iter, ConvertResult.ptr - First);
	}

	// Ignore trailing spaces.
	while (Iter != Sent && FCharTraits::IsSpace(*Iter)) ++Iter;

	if (Iter != Sent) return false;

	Value = bNegative ? -Result : Result;

	return true;
}

/**
 * Parses a floating-point value from the given string range.
 * Ignore leading and trailing spaces and case-insensitive.
 * Automatically detect formats if multiple formats are allowed.
 * Allow parsing base prefixes: "0x" for hexadecimal.
 *
 * @param First       - The iterator of the range.
 * @param Last        - The sentinel of the range.
 * @param Value       - The floating-point value to parse.
 * @param bFixed      - Allow parsing fixed-point values.
 * @param bScientific - Allow parsing scientific notation values.
 * @param bHex        - Allow parsing hex floating-point values.
 *
 * @return true if the value is successfully parsed, false otherwise.
 */
template <CStringIterator I, CSentinelFor<I> S, CFloatingPoint T> requires (!CConst<T> && !CVolatile<T>)
FORCEINLINE constexpr bool Parse(I First, S Last, T& Value, bool bFixed = true, bool bScientific = true, bool bHex = true)
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Parse(Ranges::View(MoveTemp(First), Last), Value, bFixed, bScientific, bHex);
}

NAMESPACE_END(Algorithms)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
