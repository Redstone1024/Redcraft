#pragma once

// NOTE: This file is not intended to be included directly, it is included by 'String/String.h'.

#include "Templates/Tuple.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

#include <cmath>
#include <limits>
#include <charconv>

#pragma warning(push)
#pragma warning(disable : 4146 4244)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// NOTE: These functions are used to format an object to a string and parse a string to an object.
// If the user-defined overloads a function with the 'Fmt' parameter, fill-and-align needs to be handled.
// The formatting function should produce a string that can be parsed by the parsing function, if the parsing function exists.

// NOTE: These functions are recommended for debug programs.

NAMESPACE_PRIVATE_BEGIN

enum class EStringCase : uint8
{
	Unspecified, // Match any case.
	Generic,     // Upper first letter, lower the rest. Upper digits, lower the rest.
	Bizarre,     // Lower first letter, upper the rest. Lower digits, upper the rest.
	Lowercase,   // Lowercase all letters.
	Uppercase,   // Uppercase all letters.
};

// The overload parameter is used to indicate whether a parameter must be considered to optimize the function.

struct FStringFormatParameter
{
	bool bSign = false;

	EStringCase StringCase = EStringCase::Unspecified;

	bool bPrefix = false;

	unsigned Base = 10;

	bool bFixed      = true;
	bool bScientific = true;

	unsigned Precision = 6;

	FORCEINLINE bool IsDigitLowercase() const { return StringCase == EStringCase::Lowercase || StringCase == EStringCase::Bizarre; }

	FORCEINLINE bool IsOtherLowercase() const { return StringCase == EStringCase::Lowercase || StringCase == EStringCase::Generic || StringCase == EStringCase::Unspecified; }

	FORCEINLINE bool IsBin() const { return Base == 2; }

	FORCEINLINE bool IsOct() const { return Base == 8; }

	FORCEINLINE bool IsHex() const { return Base == 16 || (!bFixed && !bScientific); }

	FORCEINLINE NAMESPACE_STD::chars_format ToSTDFormat() const
	{
		if ( bFixed && !bScientific) return NAMESPACE_STD::chars_format::fixed;
		if (!bFixed &&  bScientific) return NAMESPACE_STD::chars_format::scientific;
		if (!bFixed && !bScientific) return NAMESPACE_STD::chars_format::hex;

		return NAMESPACE_STD::chars_format::general;
	}
};

// Overload 0b1: With string case parameter and disable integer parsing.

template <CCharType T, size_t Overload> struct TStringBooleanFormatter;
template <CCharType T, size_t Overload> struct TStringBooleanParser;

// Overload 0b0001: With show positive sign parameter.
// Overload 0b0010: With string case parameter.
// Overload 0b0100: With prefix parameter.
// Overload 0b1000: With base parameter.

template <CCharType T, size_t Overload> struct TStringIntegerFormatter;
template <CCharType T, size_t Overload> struct TStringIntegerParser;

// Overload 0b00001: With show positive sign parameter.
// Overload 0b00010: With string case parameter.
// Overload 0b00100: With prefix parameter.
// Overload 0b01000: With fixed or scientific parameter.
// Overload 0b10000: With precision parameter.

template <CCharType T, size_t Overload> struct TStringFloatingPointFormatter;
template <CCharType T, size_t Overload> struct TStringFloatingPointParser;

template <CCharType T, size_t Overload>
struct TStringBooleanFormatter
{
	static_assert(Overload <= 0b1, "Invalid overload.");

	static FORCEINLINE bool Do(auto& Result, bool Value, FStringFormatParameter Param = { })
	{
		if constexpr (Overload & 0b1)
		{
			switch (Param.StringCase)
			{
			case EStringCase::Generic:
				if (Value) Result += LITERAL(T, "True");
				else       Result += LITERAL(T, "False");
				return true;
			case EStringCase::Bizarre:
				if (Value) Result += LITERAL(T, "tRUE");
				else       Result += LITERAL(T, "fALSE");
				return true;
			case EStringCase::Lowercase:
				if (Value) Result += LITERAL(T, "true");
				else       Result += LITERAL(T, "false");
				return true;
			case EStringCase::Uppercase:
				if (Value) Result += LITERAL(T, "TRUE");
				else       Result += LITERAL(T, "FALSE");
				return true;
			default: break;
			}
		}

		if (Value) Result += LITERAL(T, "True");
		else       Result += LITERAL(T, "False");
		return true;
	}
};

template <CCharType T, size_t Overload>
struct TStringBooleanParser
{
	static_assert(Overload <= 0b0, "Invalid overload.");

	static FORCEINLINE bool Do(auto& View, bool& Value, FStringFormatParameter Param = { })
	{
		if (View.IsEmpty()) return false;

		if constexpr (Overload & 0b1)
		{
			TOptional<bool> Result;

			switch (Param.StringCase)
			{
			case EStringCase::Generic:
				if (View.StartsWith(LITERAL(T, "True")))  Result = true;
				if (View.StartsWith(LITERAL(T, "False"))) Result = false;
				break;
			case EStringCase::Bizarre:
				if (View.StartsWith(LITERAL(T, "tRUE")))  Result = true;
				if (View.StartsWith(LITERAL(T, "fALSE"))) Result = false;
				break;
			case EStringCase::Lowercase:
				if (View.StartsWith(LITERAL(T, "true")))  Result = true;
				if (View.StartsWith(LITERAL(T, "false"))) Result = false;
				break;
			case EStringCase::Uppercase:
				if (View.StartsWith(LITERAL(T, "TRUE")))  Result = true;
				if (View.StartsWith(LITERAL(T, "FALSE"))) Result = false;
				break;
			default: break;
			}

			if (Result.IsValid())
			{
				View.RemovePrefix(*Result == true ? 4 : 5);
				Value = *Result;
				return true;
			}
		}

		if (View.Front() == LITERAL(T, '1'))
		{
			View.RemovePrefix(1);
			Value = true;
			return true;
		}

		if (View.Front() == LITERAL(T, '0'))
		{
			View.RemovePrefix(1);
			Value = false;
			return true;
		}

		if    (View.StartsWith(LITERAL(T, "true"))
			|| View.StartsWith(LITERAL(T, "True"))
			|| View.StartsWith(LITERAL(T, "TRUE")))
		{
			View.RemovePrefix(4);
			Value = true;
			return true;
		}

		if    (View.StartsWith(LITERAL(T, "false"))
			|| View.StartsWith(LITERAL(T, "False"))
			|| View.StartsWith(LITERAL(T, "FALSE")))
		{
			View.RemovePrefix(5);
			Value = false;
			return true;
		}

		if (int IntValue; TStringIntegerParser<T, 0b0000>::Do(View, IntValue)) {
			Value = IntValue != 0;
			return true;
		}

		return false;
	}
};

template <CCharType T, size_t Overload>
struct TStringIntegerFormatter
{
	static_assert(Overload <= 0b1111, "Invalid overload.");

	static FORCEINLINE bool Do(auto& Result, auto Value, FStringFormatParameter Param = { })
	{
		static_assert(TChar<T>::IsASCII());

		using U = TRemoveCVRef<decltype(Value)>;

		// If the value should be formatted with prefix, the value must be binary, octal or hexadecimal.
		if constexpr (Overload & 0b0100) if (Param.bPrefix && !(Param.IsBin() || Param.IsOct() || Param.IsHex()))
		{
			checkf(false, TEXT("Prefix is only supported for binary, octal and hexadecimal value."));

			return false;
		}

		using UnsignedU = TMakeUnsigned<U>;

		UnsignedU Unsigned = static_cast<UnsignedU>(Value);

		bool bNegative = false;

		if constexpr (CSigned<U>)
		{
			if (Value < 0)
			{
				bNegative = true;

				Unsigned = static_cast<UnsignedU>(-Unsigned);
			}
		}

		constexpr size_t BufferSize = sizeof(UnsignedU) * 8 + 4;

		T Buffer[BufferSize];

		T* Iter = Buffer + BufferSize;

		// Reverse append the digits to the buffer.
		if constexpr (Overload & 0b1000)
		{
			if constexpr (Overload & 0b0010)
			{
				const bool bLowercase = Param.IsDigitLowercase();

				switch (Param.Base)
				{
				case 0x02: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00001));             Unsigned >>= 1; } while (Unsigned != 0); break;
				case 0x04: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00011));             Unsigned >>= 2; } while (Unsigned != 0); break;
				case 0x08: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00111));             Unsigned >>= 3; } while (Unsigned != 0); break;
				case 0x10: do { *--Iter =   TChar<T>::FromDigit(Unsigned & 0b01111, bLowercase);  Unsigned >>= 4; } while (Unsigned != 0); break;
				case 0X20: do { *--Iter =   TChar<T>::FromDigit(Unsigned & 0b11111, bLowercase);  Unsigned >>= 5; } while (Unsigned != 0); break;

				case 3:
				case 5:
				case 6:
				case 7:
				case 9:
				case 10: do { *--Iter = static_cast<T>('0' + Unsigned % Param.Base);             Unsigned = static_cast<UnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
				default: do { *--Iter =  TChar<T>::FromDigit(Unsigned % Param.Base, bLowercase); Unsigned = static_cast<UnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
				}
			}
			else
			{
				switch (Param.Base)
				{
				case 0x02: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00001)); Unsigned >>= 1; } while (Unsigned != 0); break;
				case 0x04: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00011)); Unsigned >>= 2; } while (Unsigned != 0); break;
				case 0x08: do { *--Iter = static_cast<T>('0' + (Unsigned & 0b00111)); Unsigned >>= 3; } while (Unsigned != 0); break;
				case 0x10: do { *--Iter =   TChar<T>::FromDigit(Unsigned & 0b01111);  Unsigned >>= 4; } while (Unsigned != 0); break;
				case 0X20: do { *--Iter =   TChar<T>::FromDigit(Unsigned & 0b11111);  Unsigned >>= 5; } while (Unsigned != 0); break;

				case 3:
				case 5:
				case 6:
				case 7:
				case 9:
				case 10: do { *--Iter = static_cast<T>('0' + Unsigned % Param.Base); Unsigned = static_cast<UnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
				default: do { *--Iter =  TChar<T>::FromDigit(Unsigned % Param.Base); Unsigned = static_cast<UnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
				}
			}
		}
		else do { *--Iter = static_cast<T>('0' + Unsigned % 10); Unsigned = static_cast<UnsignedU>(Unsigned / 10); } while (Unsigned != 0);

		// Append the prefix to the buffer.
		if constexpr (Overload & 0b1100) if (Param.bPrefix)
		{
			const T PrefixBin = Param.IsOtherLowercase() ? LITERAL(T, 'b') : LITERAL(T, 'B');
			const T PrefixHex = Param.IsOtherLowercase() ? LITERAL(T, 'x') : LITERAL(T, 'X');

			if (Param.IsBin()) { *--Iter = PrefixBin; *--Iter = LITERAL(T, '0'); }
			if (Param.IsOct()) { if (Value != 0)      *--Iter = LITERAL(T, '0'); }
			if (Param.IsHex()) { *--Iter = PrefixHex; *--Iter = LITERAL(T, '0'); }
		}

		// Append the negative sign to the buffer.
		if constexpr (CSigned<U>) if (bNegative) *--Iter = LITERAL(T, '-');

		// Append the positive sign to the buffer.
		if constexpr (Overload & 0b0001) if (!bNegative && Param.bSign) *--Iter = LITERAL(T, '+');

		Result.Append(Iter, Buffer + BufferSize);

		return true;
	}
};

template <CCharType T, size_t Overload>
struct TStringIntegerParser
{
	static_assert(Overload <= 0b1111, "Invalid overload.");

	static FORCEINLINE bool Do(auto& View, auto& Value, FStringFormatParameter Param = { })
	{
		static_assert(TChar<T>::IsASCII());

		static_assert(!CConst<decltype(Value)> && !CVolatile<decltype(Value)>);

		using U = TRemoveCVRef<decltype(Value)>;

		// Create a temporary view to avoid modifying the original view.
		TStringView<T> Temp = View;

		bool bNegative = false;

		// Handle optional negative sign.
		if constexpr (CSigned<U>)
		{
			if (Temp.StartsWith(LITERAL(T, '-')))
			{
				bNegative = true;
				Temp.RemovePrefix(1);
			}
		}

		// Handle optional positive sign.
		if constexpr (Overload & 0b0001) if (!bNegative && Param.bSign) if (Temp.StartsWith(LITERAL(T, '+'))) Temp.RemovePrefix(1);

		// Handle optional prefix.
		if constexpr (Overload & 0b1100) if (Param.bPrefix)
		{
			// Auto detect base.
			if (Param.Base == 0)
			{
				if (Temp.Num() >= 2 && Temp.Front() == LITERAL(T, '0'))
				{
					if (Temp[1] == LITERAL(T, 'x') || Temp[1] == LITERAL(T, 'X'))
					{
						Param.Base = 16;
						Temp.RemovePrefix(2);
					}
					else if (Temp[1] == LITERAL(T, 'b') || Temp[1] == LITERAL(T, 'B'))
					{
						Param.Base = 2;
						Temp.RemovePrefix(2);
					}
					else if (TChar<T>::IsDigit(Temp.Front(), 8))
					{
						Param.Base = 8;
						Temp.RemovePrefix(1);
					}
				}

				if (Param.Base == 0) Param.Base = 10;
			}
			else
			{
				checkf(Param.IsBin() || Param.IsOct() || Param.IsHex(), TEXT("Prefix is only supported for binary, octal and hexadecimal value."));

				if constexpr (Overload & 0b0010) if (Param.StringCase != EStringCase::Unspecified)
				{
					if (Param.IsOtherLowercase())
					{
						if (Param.IsBin() && Temp.StartsWith(LITERAL(T, "0b"))) Temp.RemovePrefix(2);
						if (Param.IsHex() && Temp.StartsWith(LITERAL(T, "0x"))) Temp.RemovePrefix(2);
					}
					else
					{
						if (Param.IsBin() && Temp.StartsWith(LITERAL(T, "0B"))) Temp.RemovePrefix(2);
						if (Param.IsHex() && Temp.StartsWith(LITERAL(T, "0X"))) Temp.RemovePrefix(2);
					}
				}
				else
				{
					if (Param.IsBin() && Temp.StartsWith(LITERAL(T, "0b") || Temp.StartsWith(LITERAL(T, "0B")))) Temp.RemovePrefix(2);
					if (Param.IsHex() && Temp.StartsWith(LITERAL(T, "0x") || Temp.StartsWith(LITERAL(T, "0X")))) Temp.RemovePrefix(2);
				}

			}
		}

		check(Param.Base >= 2 && Param.Base <= 36);

		using UnsignedU = TMakeUnsigned<U>;

		// The limit value that can be stored in an unsigned integer.
		constexpr UnsignedU UnsignedMaximum = static_cast<UnsignedU>(-1);

		// The limit value that can be stored in a signed integer.
		constexpr U SignedMaximum = static_cast<U>(UnsignedMaximum >> 1);
		constexpr U SignedMinimum =  -static_cast<U>(SignedMaximum) - 1;

		UnsignedU LastValue = 0;
		UnsignedU Unsigned  = 0;

		bool bOverflow = false;

		if (Temp.IsEmpty()) return false;

		unsigned Digit;

		if constexpr (Overload & 0b0010)
		{
			Digit = TChar<T>::ToDigit(Temp.Front(), Param.IsDigitLowercase());
		}
		else Digit = TChar<T>::ToDigit(Temp.Front());

		// The first character must be a digit.
		if (Digit >= Param.Base) return false;

		Temp.RemovePrefix(1);

		Unsigned = static_cast<UnsignedU>(Digit);

		while (!Temp.IsEmpty())
		{
			if constexpr (Overload & 0b0010)
			{
				Digit = TChar<T>::ToDigit(Temp.Front(), Param.IsDigitLowercase());
			}
			else Digit = TChar<T>::ToDigit(Temp.Front());

			if (Digit >= Param.Base) break;

			Temp.RemovePrefix(1);

			LastValue = Unsigned;

			Unsigned = static_cast<UnsignedU>(LastValue * Param.Base + Digit);

			if (Unsigned < LastValue) bOverflow = true;
		}

		View = Temp;

		// Handle overflow.
		if (bOverflow)
		{
			if constexpr (CSigned<U>)
			{
				Value = bNegative ? SignedMinimum : SignedMaximum;
			}
			else Value = UnsignedMaximum;

			return true;
		}

		if constexpr (CSigned<U>)
		{
			// Handle overflow.
			if (!bNegative && Unsigned >= static_cast<UnsignedU>(SignedMaximum)) { Value = SignedMaximum; return true; }
			if ( bNegative && Unsigned >= static_cast<UnsignedU>(SignedMinimum)) { Value = SignedMinimum; return true; }

			// Handle negative sign.
			if (bNegative) Unsigned = static_cast<UnsignedU>(-Unsigned);
		}

		Value = static_cast<U>(Unsigned);
		return true;
	}
};

template <CCharType T, size_t Overload>
struct TStringFloatingPointFormatter
{
	static_assert(Overload <= 0b11111, "Invalid overload.");

	static FORCEINLINE bool Do(auto& Result, auto Value, FStringFormatParameter Param = { })
	{
		constexpr size_t StartingBufferSize = 64;

		// Create a buffer with a starting size.
		TArray<char, TInlineAllocator<StartingBufferSize>> Buffer(StartingBufferSize / 2);

		// Formatting strings using the standard library until successful
		NAMESPACE_STD::to_chars_result ConvertResult;

		do
		{
			Buffer.SetNum(Buffer.Num() * 2);

			if      constexpr (Overload & 0b10000) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value, Param.ToSTDFormat(), Param.Precision);
			else if constexpr (Overload & 0b01000) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value, Param.ToSTDFormat());
			else                                   ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value);
		}
		while (ConvertResult.ec == NAMESPACE_STD::errc::value_too_large);

		// Set the buffer size to the number of characters written.
		Buffer.SetNum(ConvertResult.ptr - Buffer.GetData());

		const bool bNegative = Buffer[0] == '-';

		const char* Iter = Buffer.GetData() + (bNegative ? 1 : 0);

		// Append the positive sign to the buffer.
		if constexpr (Overload & 0b00001) if (!bNegative && Param.bSign) Result.Append(LITERAL(T, "+"));

		// Handle the infinity values.
		if (*Iter == 'i')
		{
			if constexpr (Overload & 0b0010)
			{
				switch (Param.StringCase)
				{
				case EStringCase::Generic:
					if (bNegative) Result += LITERAL(T, "-Infinity");
					else           Result += LITERAL(T,  "Infinity");
					return true;
				case EStringCase::Bizarre:
					if (bNegative) Result += LITERAL(T, "-iNFINITY");
					else           Result += LITERAL(T,  "iNFINITY");
					return true;
				case EStringCase::Lowercase:
					if (bNegative) Result += LITERAL(T, "-infinity");
					else           Result += LITERAL(T,  "infinity");
					return true;
				case EStringCase::Uppercase:
					if (bNegative) Result += LITERAL(T, "-INFINITY");
					else           Result += LITERAL(T,  "INFINITY");
					return true;
				default: break;
				}
			}

			if (bNegative) Result.Append(LITERAL(T, "-Infinity"));
			else           Result.Append(LITERAL(T,  "Infinity"));
			return true;
		}

		// Handle the NaN values.
		if (*Iter == 'n')
		{
			if constexpr (Overload & 0b0010)
			{
				switch (Param.StringCase)
				{
				case EStringCase::Generic:
					if (bNegative) Result += LITERAL(T, "-NaN");
					else           Result += LITERAL(T,  "NaN");
					return true;
				case EStringCase::Bizarre:
					if (bNegative) Result += LITERAL(T, "-nAn");
					else           Result += LITERAL(T,  "nAn");
					return true;
				case EStringCase::Lowercase:
					if (bNegative) Result += LITERAL(T, "-nan");
					else           Result += LITERAL(T,  "nan");
					return true;
				case EStringCase::Uppercase:
					if (bNegative) Result += LITERAL(T, "-NAN");
					else           Result += LITERAL(T,  "NAN");
					return true;
				default: break;
				}
			}

			if (bNegative) Result.Append(LITERAL(T, "-NaN"));
			else           Result.Append(LITERAL(T,  "NaN"));
			return true;
		}

		// Handle the lowercase or uppercase characters.
		for (char& Char : Buffer)
		{
			if (FChar::ToDigit(Char) < (Param.IsHex() ? 16u : 10u))
			{
				Char = Param.IsDigitLowercase() ? FChar::ToLower(Char) : FChar::ToUpper(Char);
			}
			else Char = Param.IsOtherLowercase() ? FChar::ToLower(Char) : FChar::ToUpper(Char);
		}

		Result.Append(Buffer.Begin(), Buffer.End());

		return true;
	}
};

template <CCharType T, size_t Overload>
struct TStringFloatingPointParser
{
	static_assert(Overload <= 0b11111, "Invalid overload.");

	static FORCEINLINE bool Do(auto& View, auto& Value, FStringFormatParameter Param = { })
	{
		// @TODO: Implement the parsing function without the standard library.

		static_assert(!CConst<decltype(Value)> && !CVolatile<decltype(Value)>);

		using U = TRemoveCVRef<decltype(Value)>;

		U Result;

		auto Iter = View.Begin();

		bool bNegativeMantissa = false;
		bool bNegativeExponent = false;

		do
		{
			if (Iter == View.End()) break;

			if (*Iter == LITERAL(T, '-'))
			{
				bNegativeMantissa = true;
				++Iter;
			}

			auto DecimalPoint = View.End();
			auto NonZeroBegin = View.End();

			while (Iter != View.End())
			{
				if (DecimalPoint == View.End() && *Iter == LITERAL(T, '.'))
				{
					DecimalPoint = Iter;
				}
				else if (TChar<T>::IsDigit(*Iter, Param.IsHex() ? 16 : 10))
				{
					if (NonZeroBegin == View.End() && *Iter != LITERAL(T, '0'))
					{
						NonZeroBegin = Iter;
					}
				}
				else break;

				++Iter;
			}

			if (DecimalPoint == View.End()) DecimalPoint = Iter;

			bNegativeExponent = DecimalPoint < NonZeroBegin;

			if (Iter == View.End()) break;

			bool bHasExponent = false;

			if (Param.bScientific)
			{
				if (*Iter == LITERAL(T, 'e') || *Iter == LITERAL(T, 'E'))
				{
					bHasExponent = true;
					++Iter;
				}
			}
			else if (Param.IsHex())
			{
				if (*Iter == LITERAL(T, 'p') || *Iter == LITERAL(T, 'P'))
				{
					bHasExponent = true;
					++Iter;
				}
			}

			if (Iter == View.End() || !bHasExponent) break;

			if (*Iter == LITERAL(T, '+')) ++Iter;
			if (*Iter == LITERAL(T, '-')) { bNegativeExponent = true; ++Iter; }

			auto ExponentBegin = Iter;

			while (Iter != View.End() && TChar<T>::IsDigit(*Iter, 10)) ++Iter;

			auto ExponentEnd = Iter;

			if (NonZeroBegin == View.End()) break;

			auto Exponent = TStringView(ExponentBegin, ExponentEnd).ToInt();

			if (bNegativeExponent) Exponent = -Exponent;

			Exponent += static_cast<int>(DecimalPoint - NonZeroBegin);

			bNegativeExponent = Exponent < 0;
		}
		while (false);

		NAMESPACE_STD::from_chars_result ConvertResult;

		if constexpr (!CSameAs<T, char>)
		{
			TArray<char, TInlineAllocator<64>> Buffer(View.Begin(), Iter);

			ConvertResult = NAMESPACE_STD::from_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Result, Param.ToSTDFormat());
		}
		else ConvertResult = NAMESPACE_STD::from_chars(ToAddress(View.Begin()), ToAddress(View.End()), Result, Param.ToSTDFormat());

		View.RemovePrefix(Iter - View.Begin());

		if (ConvertResult.ec == NAMESPACE_STD::errc::result_out_of_range)
		{
			if      (!bNegativeMantissa && !bNegativeExponent) Value =  NAMESPACE_STD::numeric_limits<U>::infinity();
			else if ( bNegativeMantissa && !bNegativeExponent) Value = -NAMESPACE_STD::numeric_limits<U>::infinity();
			else if (!bNegativeMantissa &&  bNegativeExponent) Value = static_cast<U>( 0.0);
			else                                               Value = static_cast<U>(-0.0);

			return true;
		}

		if (ConvertResult.ec == NAMESPACE_STD::errc::invalid_argument) return false;

		Value = Result;
		return true;
	}
};

template <CCharType T>
struct TStringObjectFormatter
{
	static FORCEINLINE bool Do(auto& Result, TStringView<T> Fmt, auto& Object)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (!CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Unsafe formatting for a variable that is non-const."));

			return false;
		}

		else if (Fmt.IsEmpty())
		{
			if constexpr (CArithmetic<U>)
			{
				constexpr const T* DigitToChar = LITERAL(T, "9876543210123456789");
				constexpr size_t   ZeroIndex   = 9;

				if constexpr (CSameAs<U, bool>)
				{
					Result += Object ? LITERAL(T, "True") : LITERAL(T, "False");

					return true;
				}

				else if constexpr (CIntegral<U>)
				{
					U Value = Object;

					const bool bNegative = Object < 0;

					constexpr size_t BufferSize = 32;

					T Buffer[BufferSize];

					size_t Index = BufferSize;

					do Buffer[--Index] = DigitToChar[ZeroIndex + Value % 10]; while (Value /= 10);

					if (bNegative) Buffer[--Index] = LITERAL(T, '-');

					const T* Begin = Buffer + Index;
					const T* End   = Buffer + BufferSize;

					Result.Append(Begin, End);

					return true;
				}

				else if constexpr (CFloatingPoint<U>)
				{
					if (NAMESPACE_STD::isinf(Object) && !NAMESPACE_STD::signbit(Object)) { Result += LITERAL(T,  "Infinity"); return true; }
					if (NAMESPACE_STD::isinf(Object) &&  NAMESPACE_STD::signbit(Object)) { Result += LITERAL(T, "-Infinity"); return true; }

					if (NAMESPACE_STD::isnan(Object) && !NAMESPACE_STD::signbit(Object)) { Result += LITERAL(T,  "NaN"); return true; }
					if (NAMESPACE_STD::isnan(Object) &&  NAMESPACE_STD::signbit(Object)) { Result += LITERAL(T, "-NaN"); return true; }

					U Value = NAMESPACE_STD::round(Object * static_cast<U>(1e6));

					const bool bNegative = NAMESPACE_STD::signbit(Object);

					TString<T, TInlineAllocator<32>> Buffer;

					for (size_t Index = 0; Index <= 6 || static_cast<signed>(Value) != 0; ++Index)
					{
						Buffer += DigitToChar[ZeroIndex + static_cast<signed>(NAMESPACE_STD::fmod(Value, 10))];

						if (Index == 5) Buffer += LITERAL(T, '.');

						Value /= 10;
					}

					if (bNegative) Buffer += LITERAL(T, '-');

					Result.Append(Buffer.RBegin(), Buffer.REnd());

					return true;
				}

				else static_assert(sizeof(U) == -1, "Unsupported arithmetic type");
			}
		}

		return false;
	}
};

template <CCharType T>
struct TStringObjectParser
{
	static FORCEINLINE bool Do(TStringView<T>& View, TStringView<T> Fmt, auto& Object)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Cannot assign to a variable that is const."));

			return false;
		}

		else if constexpr (CArithmetic<U>)
		{
			checkf(Fmt.IsEmpty(), TEXT("Formatted parsing of arithmetic types not implemented."));

			// Skip leading white spaces.
			while (!View.IsEmpty() && TChar<T>::IsSpace(View.Front())) View.RemovePrefix(1);

			if (View.IsEmpty()) return false;

			bool bNegative = false;

			// Handle optional sign.
			if (View.Front() == LITERAL(T, '+'))
			{
				View.RemovePrefix(1);
			}
			else if (View.Front() == LITERAL(T, '-'))
			{
				bNegative = true;
				View.RemovePrefix(1);
			}

			// Handle boolean conversion.
			else if constexpr (CSameAs<U, bool>)
			{
				bool bIsTrue  = false;
				bool bIsFalse = false;

				bIsTrue  |= View.StartsWith(LITERAL(T, "true"))  || View.StartsWith(LITERAL(T, "True"))  || View.StartsWith(LITERAL(T, "TRUE"));
				bIsFalse |= View.StartsWith(LITERAL(T, "false")) || View.StartsWith(LITERAL(T, "False")) || View.StartsWith(LITERAL(T, "FALSE"));

				if (bIsTrue)  { View.RemovePrefix(4); Object = true;  return true; }
				if (bIsFalse) { View.RemovePrefix(5); Object = false; return true; }
			}

			// Handle floating-point conversion.
			if constexpr (CFloatingPoint<U>)
			{
				bool bIsInfinity = false;
				bool bIsNaN      = false;

				bIsInfinity |= View.StartsWith(LITERAL(T, "infinity")) || View.StartsWith(LITERAL(T, "Infinity")) || View.StartsWith(LITERAL(T, "INFINITY"));
				bIsNaN      |= View.StartsWith(LITERAL(T, "nan"))      || View.StartsWith(LITERAL(T, "NaN"))      || View.StartsWith(LITERAL(T, "NAN"));

				if (bIsInfinity) { View.RemovePrefix(8); Object = bNegative ? -NAMESPACE_STD::numeric_limits<U>::infinity()  : NAMESPACE_STD::numeric_limits<U>::infinity();  return true; }
				if (bIsNaN)      { View.RemovePrefix(3); Object = bNegative ? -NAMESPACE_STD::numeric_limits<U>::quiet_NaN() : NAMESPACE_STD::numeric_limits<U>::quiet_NaN(); return true; }
			}

			unsigned Base = 0;

			// Auto detect base.
			{
				if (View.Num() >= 2 && View.Front() == LITERAL(T, '0'))
				{
					if (View[1] == LITERAL(T, 'x') || View[1] == LITERAL(T, 'X'))
					{
						Base = 16;
						View.RemovePrefix(2);
					}
					else if (View[1] == LITERAL(T, 'b') || View[1] == LITERAL(T, 'B'))
					{
						Base = 2;
						View.RemovePrefix(2);
					}
					else if (TChar<T>::IsDigit(View.Front(), 8))
					{
						Base = 8;
						View.RemovePrefix(1);
					}
					else Base = 10;
				}
				else Base = 10;
			}

			// Parse the number.
			auto ToNumber = [&View]<typename NumberType>(TInPlaceType<NumberType>, unsigned Base, NumberType Init = static_cast<NumberType>(0)) -> NumberType
			{
				NumberType Result = Init;

				while (!View.IsEmpty())
				{
					auto Digit = TChar<T>::ToDigit(View.Front());

					if (Digit >= Base) break;

					Result = Result * static_cast<NumberType>(Base) + static_cast<NumberType>(Digit);

					View.RemovePrefix(1);
				}

				return Result;
			};

			// Handle integral conversion.
			if constexpr (CIntegral<U>)
			{
				using UnsignedU = TMakeUnsigned<TConditional<!CSameAs<U, bool>, U, int>>;

				if (View.IsEmpty()) return false;

				// The integral number must start with a digit.
				if (!TChar<T>::IsDigit(View.Front(), Base)) return false;

				// Parse the integral number.
				UnsignedU Number = ToNumber(InPlaceType<UnsignedU>, Base);

				Object = static_cast<U>(bNegative ? -Number : Number);

				return true;
			}

			// Handle floating-point conversion.
			else if constexpr (CFloatingPoint<U>)
			{
				if (View.IsEmpty()) return false;

				// The floating-point number must start with a digit or a dot.
				if (!(TChar<T>::IsDigit(View.Front(), Base) || View.Front() == LITERAL(T, '.'))) return false;

				size_t IntegralBeginNum = View.Num();

				// Parse the integral number.
				Object = ToNumber(InPlaceType<U>, Base);

				size_t IntegralLength = IntegralBeginNum - View.Num();

				// Parse the fractional number.
				if (!View.IsEmpty() && View.Front() == LITERAL(T, '.'))
				{
					View.RemovePrefix(1);

					U InvBase = 1 / static_cast<U>(Base);

					size_t FractionBeginNum = View.Num();

					Object = ToNumber(InPlaceType<U>, Base, Object);

					size_t FractionLength = FractionBeginNum - View.Num();

					Object *= NAMESPACE_STD::pow(InvBase, static_cast<U>(FractionLength));
				}
				else if (IntegralLength == 0) return false;

				// For floating point numbers apply the symbols directly
				Object = static_cast<U>(bNegative ? -Object : Object);

				if (View.IsEmpty()) return true;

				if (Base != 10 && Base != 16) return true;

				bool bHasExponent = false;

				bHasExponent |= Base == 10 && View.Front() == LITERAL(T, 'e');
				bHasExponent |= Base == 10 && View.Front() == LITERAL(T, 'E');
				bHasExponent |= Base == 16 && View.Front() == LITERAL(T, 'p');
				bHasExponent |= Base == 16 && View.Front() == LITERAL(T, 'P');

				if (!bHasExponent) return true;

				View.RemovePrefix(1);

				if (View.IsEmpty()) return false;

				// Parse the exponent number.
				{
					bool bNegativeExponent = false;

					if (View.Front() == LITERAL(T, '+'))
					{
						View.RemovePrefix(1);
					}
					else if (View.Front() == LITERAL(T, '-'))
					{
						bNegativeExponent = true;
						View.RemovePrefix(1);
					}

					// The exponent number must start with a digit.
					if (!TChar<T>::IsDigit(View.Front())) return false;

					U Exponent = ToNumber(InPlaceType<U>, 10);

					Exponent = bNegativeExponent ? -Exponent : Exponent;

					Object *= static_cast<U>(NAMESPACE_STD::pow(static_cast<U>(Base == 16 ? 2 : 10), Exponent));
				}

				return true;
			}

			else static_assert(sizeof(U) == -1, "Unsupported arithmetic type");

			return false;
		}

		return false;
	}
};

template <CCharType T, bool bIsFormat>
struct TStringFormatOrParseHelper
{
	static constexpr T LeftBrace  = LITERAL(T, '{');
	static constexpr T RightBrace = LITERAL(T, '}');

	static inline const TStringView EscapeLeftBrace  = LITERAL(T, "<[{");
	static inline const TStringView EscapeRightBrace = LITERAL(T, "}]>");

	static FORCEINLINE size_t Do(auto& Result, TStringView<T> Fmt, auto ArgsTuple)
	{
		size_t FormattedObjectNum = 0;

		size_t ArgsIndex = 0;

		auto ParseFormat = [&FormattedObjectNum, &ArgsIndex, ArgsTuple](auto& Self, auto& String, TStringView<T>& Fmt) -> bool
		{
			bool bIsFullyFormatted = true;

			while (!Fmt.IsEmpty())
			{
				if (Fmt.StartsWith(EscapeLeftBrace))
				{
					Fmt.RemovePrefix(EscapeLeftBrace.Num());

					if constexpr (!bIsFormat)
					{
						if (!String.StartsWith(LeftBrace)) return false;

						String.RemovePrefix(1);
					}
					else String += LeftBrace;

					continue;
				}

				if (Fmt.StartsWith(EscapeRightBrace))
				{
					Fmt.RemovePrefix(EscapeRightBrace.Num());

					if constexpr (!bIsFormat)
					{
						if (!String.StartsWith(RightBrace)) return false;

						String.RemovePrefix(1);
					}
					else String += RightBrace;

					continue;
				}

				if (Fmt.StartsWith(LeftBrace))
				{
					Fmt.RemovePrefix(1);

					int SubplaceholderNum = -1;

					size_t PlaceholderBegin = -1;
					size_t PlaceholderEnd   = -1;

					// Find the end of the placeholder.
					do
					{
						while (true)
						{
							PlaceholderBegin = Fmt.FindFirstOf(LeftBrace, PlaceholderBegin + 1);

							if (PlaceholderBegin == INDEX_NONE) break;

							if (Fmt.First(PlaceholderBegin + 1).EndsWith(EscapeLeftBrace))
							{
								++PlaceholderBegin;
							}
							else break;
						}

						while (true)
						{
							PlaceholderEnd = Fmt.FindFirstOf(RightBrace, PlaceholderEnd + 1);

							if (PlaceholderEnd == INDEX_NONE) break;

							if (Fmt.Substr(PlaceholderEnd).StartsWith(EscapeRightBrace))
							{
								++PlaceholderEnd;
							}
							else break;
						}

						if (PlaceholderEnd == INDEX_NONE)
						{
							checkf(false, TEXT("Unmatched '{' in format string."));

							if constexpr (bIsFormat) String += Fmt;

							Fmt = LITERAL(T, "");

							return false;
						}

						++SubplaceholderNum;
					}
					while (PlaceholderBegin != INDEX_NONE && PlaceholderBegin < PlaceholderEnd);

					TStringView Subfmt = Fmt.First(PlaceholderEnd);

					Fmt.RemovePrefix(PlaceholderEnd + 1);

					bool bIsSuccessful = true;

					// The subformat string size are usually smaller than 16.
					TString<T, TInlineAllocator<16>> FormattedSubfmt;

					// Recursively format the subformat string.
					if (SubplaceholderNum > 0)
					{
						if constexpr (bIsFormat) bIsSuccessful = Self(Self, FormattedSubfmt, Subfmt);

						else bIsSuccessful = TStringFormatOrParseHelper<T, true>::Do(FormattedSubfmt, Subfmt, ArgsTuple);

						Subfmt = FormattedSubfmt;
					}

					if (bIsSuccessful)
					{
						// Find the placeholder index delimiter.
						size_t IndexLength = Subfmt.FindFirstOf(LITERAL(T, ':'));

						if (IndexLength == INDEX_NONE) IndexLength = Subfmt.Num();

						TStringView PlaceholderIndex  = Subfmt.First(IndexLength);
						TStringView PlaceholderSubfmt = IndexLength != Subfmt.Num() ? Subfmt.Substr(IndexLength + 1) : LITERAL(T, "");

						size_t Index;

						if (IndexLength != 0)
						{
							if (!PlaceholderIndex.IsInteger(10, false))
							{
								checkf(false, TEXT("Invalid placeholder index."));

								if constexpr (bIsFormat)
								{
									String += LeftBrace;
									String += Subfmt;
									String += RightBrace;

									bIsFullyFormatted = false;
								}
								else return false;

								continue;
							}

							verify(PlaceholderIndex.Parse(LITERAL(T, "{}"), Index) == 1);
						}
						else Index = ArgsIndex++;

						checkf(Index < ArgsTuple.Num(), TEXT("Argument not found."));

						bIsSuccessful = ArgsTuple.Visit(
							[&String, Subfmt = PlaceholderSubfmt](auto& Object) mutable
							{
								if (Subfmt.StartsWith(LITERAL(T, ':'))) Subfmt.RemovePrefix(1);

								if constexpr (bIsFormat) return TStringObjectFormatter<T>::Do(String, Subfmt, Object);

								else return TStringObjectParser<T>::Do(String, Subfmt, Object);
							},
							Index
						);
					}

					if (!bIsSuccessful)
					{
						if constexpr (bIsFormat)
						{
							String += LeftBrace;
							String += Subfmt;
							String += RightBrace;

							bIsFullyFormatted = false;
						}
						else return false;
					}
					else ++FormattedObjectNum;

					continue;
				}

				check_code({ if (Fmt.StartsWith(RightBrace)) check_no_entry(); });

				if constexpr (!bIsFormat)
				{
					if (TChar<T>::IsSpace(Fmt.Front()))
					{
						Fmt.RemovePrefix(1);

						while (TChar<T>::IsSpace(String.Front()))
						{
							String.RemovePrefix(1);
						}

						continue;
					}

					if (!String.StartsWith(Fmt.Front())) return false;

					String.RemovePrefix(1);
				}
				else String += Fmt.Front();

				Fmt.RemovePrefix(1);
			}

			return bIsFullyFormatted;
		};

		bool bIsSuccessful = ParseFormat(ParseFormat, Result, Fmt);

		if constexpr (bIsFormat) return bIsSuccessful;

		return FormattedObjectNum;
	}
};

NAMESPACE_PRIVATE_END

template <CCharType T, CAllocator<T> Allocator>
template <typename ... Ts>
void TString<T, Allocator>::AppendFormat(TStringView<ElementType> Fmt, const Ts&... Args)
{
	// The Unreal Engine says that the starting buffer size catches 99.97% of printf calls.
	constexpr size_t ReserveBufferSize = 512;

	TString<T, TInlineAllocator<ReserveBufferSize>> Result;

	NAMESPACE_PRIVATE::TStringFormatOrParseHelper<ElementType, true>::Do(Result, Fmt, ForwardAsTuple(Args...));

	Append(Result.Begin(), Result.End());
}

template <CCharType T>
template <typename ... Ts>
size_t TStringView<T>::ParseAndTrim(TStringView Fmt, Ts&... Args)
{
	return NAMESPACE_PRIVATE::TStringFormatOrParseHelper<ElementType, false>::Do(*this, Fmt, ForwardAsTuple(Args...));
}

template <CCharType T, CAllocator<T> Allocator>
void TString<T, Allocator>::AppendBool(bool Value)
{
	NAMESPACE_PRIVATE::TStringBooleanFormatter<ElementType, 0b0>::Do(*this, Value);
}

template <CCharType T, CAllocator<T> Allocator>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendInt(U Value, unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	NAMESPACE_PRIVATE::FStringFormatParameter Param;

	Param.Base = Base;

	NAMESPACE_PRIVATE::TStringIntegerFormatter<ElementType, 0b1000>::Do(*this, Value, Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value)
{
	NAMESPACE_PRIVATE::TStringFloatingPointFormatter<ElementType, 0b0>::Do(*this, Value);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific)
{
	NAMESPACE_PRIVATE::FStringFormatParameter Param;

	Param.bFixed      = bFixed;
	Param.bScientific = bScientific;

	NAMESPACE_PRIVATE::TStringFloatingPointFormatter<ElementType, 0b01000>::Do(*this, Value, Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific, unsigned Precision)
{
	NAMESPACE_PRIVATE::FStringFormatParameter Param;

	Param.bFixed      = bFixed;
	Param.bScientific = bScientific;
	Param.Precision   = Precision;

	NAMESPACE_PRIVATE::TStringFloatingPointFormatter<ElementType, 0b11000>::Do(*this, Value, Param);
}

template <CCharType T>
constexpr bool TStringView<T>::ToBoolAndTrim()
{
	bool Value = false;

	NAMESPACE_PRIVATE::TStringBooleanParser<ElementType, 0b0>::Do(*this, Value);

	return Value;
}

template <CCharType T>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToIntAndTrim(unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	U Value = 0;

	NAMESPACE_PRIVATE::FStringFormatParameter Param;

	Param.Base = Base;

	NAMESPACE_PRIVATE::TStringIntegerParser<ElementType, 0b100>::Do(*this, Value, Param);

	return Value;
}

template <CCharType T>
template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToFloatAndTrim(bool bFixed, bool bScientific)
{
	U Value = NAMESPACE_STD::numeric_limits<U>::quiet_NaN();

	NAMESPACE_PRIVATE::FStringFormatParameter Param;

	Param.bFixed      = bFixed;
	Param.bScientific = bScientific;

	NAMESPACE_PRIVATE::TStringFloatingPointParser<ElementType, 0b01000>::Do(*this, Value, Param);

	return Value;
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
