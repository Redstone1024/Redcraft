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

// The conversion tool uses a string to describe the object format.

// NOTE: These functions are used to format an object to a string and parse a string to an object.
// If the user-defined overloads a function with the 'Fmt' parameter, fill-and-align needs to be handled.
// The formatting function should produce a string that can be parsed by the parsing function, if the parsing function exists.

// NOTE: These functions are recommended for debug programs.

NAMESPACE_PRIVATE_BEGIN

// In private, conversion tools use structured parameters to describe the object format.
// The structured parameter is an object with specific public members:
//
// - DigitStyle:  A signed integer that represents the letter case of the first part or the digit part.
//	              Less than 0 for lowercase, greater than 0 for uppercase, 0 for default or any in parsing.
//	              It is valid for boolean, integer and floating-point values.
//
// - OtherStyle:  A signed integer that represents the letter case of the other part.
//	              Less than 0 for lowercase, greater than 0 for uppercase, 0 for default or any in parsing.
//	              It is valid for boolean, integer and floating-point values.
//
// - bSign:       A boolean that represents whether to show the sign of the number if it is positive.
//	              It is valid for integer and floating-point values.
//
// - bPrefix:     A boolean that represents whether to show the prefix of the number.
//	              Legal only when base is binary octal decimal and hexadecimal
//	              For parsing, together with the following parameters, it also determines whether to automatically detect the base.
//	              It is valid for integer and floating-point values.
//
// - Base:        A unsigned integer that represents the base of the number, between [2, 36].
//	              However, when parsed and prefixed, 0 is allowed to indicate auto-detection.
//	              It is valid for integer values.
//
// - bFixed:      A boolean that represents whether to use the decimal fixed-point format.
// - bScientific: A boolean that represents whether to use the decimal scientific format.
//	              These two parameters together determine the format of the floating-point value.
//	              When both are false, represents the hex scientific is format.
//	              However, when parsed and prefixed, any values allows auto-detection hex scientific format.
//	              It is valid for floating-point values.
//
// - Precision:   A unsigned integer that represents the number of digits after the decimal point.
//	              For parsing, it is used to determine the maximum number of digits after the decimal point.
//	              It is valid for floating-point values.

template <CCharType T>
struct TStringObjectFormatter
{
	static FORCEINLINE bool Do(auto& Result, auto& Object, auto Param)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (!CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Unsafe formatting for a variable that is non-const."));

			return false;
		}

		// Parse format string if parameter is TStringView, otherwise use the structured parameters directly.
		if constexpr (requires { { Param.Fmt } -> CConvertibleTo<TStringView<T>>; })
		{
			TStringView<T> Fmt = Param.Fmt;

			checkf(Fmt.IsEmpty(), TEXT("Formatted parsing of arithmetic types not implemented."));

			// Format the boolean value by format string.
			if constexpr (CSameAs<U, bool>)
			{
				return TStringObjectFormatter::Do(Result, Object, Invalid);
			}

			// Format the integer value by format string.
			else if constexpr (CIntegral<U> && !CSameAs<U, bool>)
			{
				return TStringObjectFormatter::Do(Result, Object, Invalid);
			}

			// Format the floating-point value by format string.
			else if constexpr (CFloatingPoint<U>)
			{
				return TStringObjectFormatter::Do(Result, Object, Invalid);
			}
		}
		else
		{
			// Format the boolean value by structured parameters.
			if constexpr (CSameAs<U, bool>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				if constexpr (bHasDigitStyle || bHasOtherStyle)
				{
					Result.Reserve(Result.Num() + 5);

					bool bDigitLowercase = false; if constexpr (bHasDigitStyle) bDigitLowercase = Param.DigitStyle <  0;
					bool bOtherLowercase = true;  if constexpr (bHasOtherStyle) bOtherLowercase = Param.OtherStyle <= 0;

					if (bDigitLowercase)
					{
						if (Object) Result += LITERAL(T, 't');
						else        Result += LITERAL(T, 'f');
					}
					else
					{
						if (Object) Result += LITERAL(T, 'T');
						else        Result += LITERAL(T, 'F');
					}

					if (bOtherLowercase)
					{
						if (Object) Result += LITERAL(T, "RUE");
						else        Result += LITERAL(T, "ALSE");
					}
					else
					{
						if (Object) Result += LITERAL(T, "rue");
						else        Result += LITERAL(T, "alse");
					}

					return true;
				}

				if (Object) Result += LITERAL(T, "True");
				else        Result += LITERAL(T, "False");
				return true;
			}

			// Format the integer value by structured parameters.
			else if constexpr (CIntegral<U> && !CSameAs<U, bool>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				constexpr bool bHasSign   = requires { { Param.bSign   } -> CBooleanTestable; };
				constexpr bool bHasPrefix = requires { { Param.bPrefix } -> CBooleanTestable; };
				constexpr bool bHasBase   = requires { { Param.Base    } -> CConvertibleTo<unsigned>; };

				static_assert(TChar<T>::IsASCII());

				// If the value should be formatted with prefix, the value must be binary, octal, decimal or hexadecimal.
				if constexpr (bHasPrefix && bHasBase) if (Param.bPrefix)
				{
					if (Param.Base != 2 && Param.Base != 8 && Param.Base != 10 && Param.Base != 16)
					{
						checkf(false, TEXT("Prefix is only supported for binary, octal, decimal and hexadecimal value."));

						return false;
					}
				}

				using UnsignedU = TMakeUnsigned<U>;

				UnsignedU Unsigned = static_cast<UnsignedU>(Object);

				bool bNegative = false;

				if constexpr (CSigned<U>)
				{
					if (Object < 0)
					{
						bNegative = true;

						Unsigned = static_cast<UnsignedU>(-Unsigned);
					}
				}

				constexpr size_t BufferSize = sizeof(UnsignedU) * 8 + 4;

				T Buffer[BufferSize];

				T* Iter = Buffer + BufferSize;

				// Reverse append the digits to the buffer.
				if constexpr (bHasBase)
				{
					checkf(Param.Base >= 2 && Param.Base <= 36, TEXT("Illegal base."));

					if constexpr (bHasDigitStyle)
					{
						const bool bLowercase = Param.DigitStyle < 0;

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
				if constexpr (bHasPrefix && bHasBase) if (Param.bPrefix && Param.Base != 10)
				{
					bool bOtherLowercase = true; if constexpr (bHasOtherStyle) bOtherLowercase = Param.OtherStyle <= 0;

					const T PrefixBin = bOtherLowercase ? LITERAL(T, 'b') : LITERAL(T, 'B');
					const T PrefixHex = bOtherLowercase ? LITERAL(T, 'x') : LITERAL(T, 'X');

					if (Param.Base ==  2) { *--Iter = PrefixBin; *--Iter = LITERAL(T, '0'); }
					if (Param.Base ==  8) { if (Object != 0)     *--Iter = LITERAL(T, '0'); }
					if (Param.Base == 16) { *--Iter = PrefixHex; *--Iter = LITERAL(T, '0'); }
				}

				// Append the negative sign to the buffer.
				if constexpr (CSigned<U>) if (bNegative) *--Iter = LITERAL(T, '-');

				// Append the positive sign to the buffer.
				if constexpr (bHasSign) if (!bNegative && Param.bSign) *--Iter = LITERAL(T, '+');

				Result.Append(Iter, Buffer + BufferSize);

				return true;
			}

			// Format the floating-point value by structured parameters.
			else if constexpr (CFloatingPoint<U>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				constexpr bool bHasSign      = requires { { Param.bSign     } -> CBooleanTestable; };
				constexpr bool bHasPrefix    = requires { { Param.bPrefix   } -> CBooleanTestable; };
				constexpr bool bHasPrecision = requires { { Param.Precision } -> CConvertibleTo<unsigned>; };

				constexpr bool bHasFormat =
					requires
					{
						{ Param.bFixed      } -> CBooleanTestable;
						{ Param.bScientific } -> CBooleanTestable;
					};

				NAMESPACE_STD::chars_format Format = NAMESPACE_STD::chars_format::general;

				if constexpr (bHasFormat) if ( Param.bFixed && !Param.bScientific) Format = NAMESPACE_STD::chars_format::fixed;
				if constexpr (bHasFormat) if (!Param.bFixed &&  Param.bScientific) Format = NAMESPACE_STD::chars_format::scientific;
				if constexpr (bHasFormat) if (!Param.bFixed && !Param.bScientific) Format = NAMESPACE_STD::chars_format::hex;

				constexpr size_t StartingBufferSize = 64;

				// Create a buffer with a starting size.
				TArray<char, TInlineAllocator<StartingBufferSize>> Buffer(StartingBufferSize / 2);

				// Formatting strings using the standard library until successful
				NAMESPACE_STD::to_chars_result ConvertResult;

				do
				{
					Buffer.SetNum(Buffer.Num() * 2);

					if      constexpr (bHasPrecision) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object, Format, Param.Precision);
					else if constexpr (bHasFormat)    ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object, Format);
					else                              ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object);
				}
				while (ConvertResult.ec == NAMESPACE_STD::errc::value_too_large);

				// Set the buffer size to the number of characters written.
				Buffer.SetNum(ConvertResult.ptr - Buffer.GetData());

				const bool bNegative = Buffer[0] == '-';

				const char* Iter = Buffer.GetData() + (bNegative ? 1 : 0);

				bool bDigitLowercase = false; if constexpr (bHasDigitStyle) bDigitLowercase = Param.DigitStyle <  0;
				bool bOtherLowercase = true;  if constexpr (bHasOtherStyle) bOtherLowercase = Param.OtherStyle <= 0;

				// Handle the infinity values.
				if (*Iter == 'i')
				{
					Result.Reserve(Result.Num() + 9);

					if (bNegative) Result.Append(LITERAL(T, "-"));

					// Append the positive sign to the buffer.
					else if constexpr (bHasSign) if (Param.bSign) Result.Append(LITERAL(T, "+"));

					if constexpr (bHasDigitStyle || bHasOtherStyle)
					{
						if (bDigitLowercase) Result += LITERAL(T, 'i');
						else                 Result += LITERAL(T, 'I');

						if (bOtherLowercase) Result += LITERAL(T, "nfinity");
						else                 Result += LITERAL(T, "NFINITY");

						return true;
					}

					Result += LITERAL(T, "Infinity");
					return true;
				}

				// Handle the NaN values.
				if (*Iter == 'n')
				{
					Result.Reserve(Result.Num() + 4);

					if (bNegative) Result.Append(LITERAL(T, "-"));

					// Append the positive sign to the buffer.
					else if constexpr (bHasSign) if (Param.bSign) Result.Append(LITERAL(T, "+"));

					if constexpr (bHasDigitStyle || bHasOtherStyle)
					{
						if (bDigitLowercase) Result += LITERAL(T, 'n');
						else                 Result += LITERAL(T, 'N');

						if (bOtherLowercase) Result += LITERAL(T, "a");
						else                 Result += LITERAL(T, "A");

						if (bDigitLowercase) Result += LITERAL(T, 'n');
						else                 Result += LITERAL(T, 'N');

						return true;
					}

					Result += LITERAL(T, "NaN");
					return true;
				}

				Result.Reserve(Result.Num() + Buffer.Num() + 4);

				// Append the positive sign to the buffer.
				if constexpr (bHasSign) if (!bNegative && Param.bSign) Result.Append(LITERAL(T, "+"));

				// Handle the prefix.
				if constexpr (bHasPrefix) if (Param.bPrefix)
				{
					if (Format == NAMESPACE_STD::chars_format::hex)
					{
						if (bOtherLowercase) Result += LITERAL(T, "0x");
						else                 Result += LITERAL(T, "0X");
					}
				}

				// Handle the lowercase or uppercase characters.
				if constexpr (bHasFormat || bHasDigitStyle || bHasOtherStyle)
				{
					const unsigned Base = Format == NAMESPACE_STD::chars_format::hex ? 16 : 10;

					if (Base == 16 && !bDigitLowercase)
					{
						for (char& Char : Buffer) if (FChar::ToDigit(Char) <  Base) Char = FChar::ToUpper(Char);
					}

					if (!bOtherLowercase)
					{
						for (char& Char : Buffer) if (FChar::ToDigit(Char) >= Base) Char = FChar::ToUpper(Char);
					}
				}

				Result.Append(Buffer.Begin(), Buffer.End());

				return true;
			}
		}

		return false;
	}
};

template <CCharType T>
struct TStringObjectParser
{
	static FORCEINLINE bool Do(auto& View, auto& Object, auto Param)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Cannot assign to a variable that is const."));

			return false;
		}

		if (View.IsEmpty()) return false;

		// Parse format string if parameter is TStringView, otherwise use the structured parameters directly.
		if constexpr (requires { { Param.Fmt } -> CConvertibleTo<TStringView<T>>; })
		{
			TStringView<T> Fmt = Param.Fmt;

			checkf(Fmt.IsEmpty(), TEXT("Formatted parsing of arithmetic types not implemented."));

			View.TrimStart();

			// Format the boolean value by format string.
			if constexpr (CSameAs<U, bool>)
			{
				return TStringObjectParser::Do(View, Object, Invalid);
			}

			// Format the integer value by format string.
			else if constexpr (CIntegral<U> && !CSameAs<U, bool>)
			{
				return TStringObjectParser::Do(View, Object, Invalid);
			}

			// Format the floating-point value by format string.
			else if constexpr (CFloatingPoint<U>)
			{
				return TStringObjectParser::Do(View, Object, Invalid);
			}
		}
		else
		{
			// Parse the boolean value by structured parameters.
			if constexpr (CSameAs<U, bool>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				if (View.Num() < 4) return false;

				if constexpr (bHasDigitStyle || bHasOtherStyle)
				{
					TOptional<bool> Result;

					signed DigitStyle = 0; if constexpr (bHasDigitStyle) DigitStyle = Param.DigitStyle;
					signed OtherStyle = 0; if constexpr (bHasOtherStyle) OtherStyle = Param.OtherStyle;

					if (DigitStyle <= 0 && OtherStyle <= 0)
					{
						if (View.StartsWith(LITERAL(T, "true")))  Result = true;
						if (View.StartsWith(LITERAL(T, "false"))) Result = false;
					}

					if (DigitStyle >= 0 && OtherStyle <= 0)
					{
						if (View.StartsWith(LITERAL(T, "True")))  Result = true;
						if (View.StartsWith(LITERAL(T, "False"))) Result = false;
					}

					if (DigitStyle <= 0 && OtherStyle >= 0)
					{
						if (View.StartsWith(LITERAL(T, "tRUE")))  Result = true;
						if (View.StartsWith(LITERAL(T, "fALSE"))) Result = false;
					}

					if (DigitStyle >= 0 && OtherStyle >= 0)
					{
						if (View.StartsWith(LITERAL(T, "TRUE")))  Result = true;
						if (View.StartsWith(LITERAL(T, "FALSE"))) Result = false;
					}

					if (Result.IsValid())
					{
						View.RemovePrefix(*Result == true ? 4 : 5);
						Object = *Result;
						return true;
					}

					return false;
				}

				if    (View.StartsWith(LITERAL(T, "true"))
					|| View.StartsWith(LITERAL(T, "True"))
					|| View.StartsWith(LITERAL(T, "tRUE"))
					|| View.StartsWith(LITERAL(T, "TRUE")))
				{
					View.RemovePrefix(4);
					Object = true;
					return true;
				}

				if    (View.StartsWith(LITERAL(T, "false"))
					|| View.StartsWith(LITERAL(T, "False"))
					|| View.StartsWith(LITERAL(T, "fALSE"))
					|| View.StartsWith(LITERAL(T, "FALSE")))
				{
					View.RemovePrefix(5);
					Object = false;
					return true;
				}

				return false;
			}

			// Parse the integer value by structured parameters.
			else if constexpr (CIntegral<U> && !CSameAs<U, bool>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				constexpr bool bHasSign   = requires { { Param.bSign   } -> CBooleanTestable; };
				constexpr bool bHasPrefix = requires { { Param.bPrefix } -> CBooleanTestable; };
				constexpr bool bHasBase   = requires { { Param.Base    } -> CConvertibleTo<unsigned>; };

				static_assert(TChar<T>::IsASCII());

				// Create a temporary view to avoid modifying the original view.
				TStringView<T> TrimmedView = View;

				bool bNegative = false;

				// Handle optional negative sign.
				if constexpr (CSigned<U>)
				{
					if (TrimmedView.StartsWith(LITERAL(T, '-')))
					{
						bNegative = true;
						TrimmedView.RemovePrefix(1);
					}
				}

				// Handle optional positive sign.
				if constexpr (bHasSign) if (!bNegative && Param.bSign) if (TrimmedView.StartsWith(LITERAL(T, '+'))) TrimmedView.RemovePrefix(1);

				unsigned Base; if constexpr (bHasBase) Base = Param.Base; else Base = 10;

				// Handle optional prefix.
				if constexpr (bHasPrefix) if (Param.bPrefix)
				{
					signed OtherStyle = 0; if constexpr (bHasOtherStyle) OtherStyle = Param.OtherStyle;

					// Auto detect base.
					if (Base == 0)
					{
						if (TrimmedView.Num() >= 2 && TrimmedView.Front() == LITERAL(T, '0'))
						{
							if    ((OtherStyle <= 0 && TrimmedView[1] == LITERAL(T, 'x'))
								|| (OtherStyle >= 0 && TrimmedView[1] == LITERAL(T, 'X')))
							{
								Base = 16;
								TrimmedView.RemovePrefix(2);
							}
							else if ((OtherStyle <= 0 && TrimmedView[1] == LITERAL(T, 'b'))
								  || (OtherStyle >= 0 && TrimmedView[1] == LITERAL(T, 'B')))
							{
								Base = 2;
								TrimmedView.RemovePrefix(2);
							}
							else if (TChar<T>::IsDigit(TrimmedView.Front(), 8)) Base = 8;
						}

						if (Base == 0) Base = 10;
					}

					// Handle prefixes with known base.
					else if (Base == 2 || Base == 8 || Base == 10 || Base == 16)
					{
						bool bNeedRemove = false;

						bNeedRemove |= OtherStyle <= 0 && Base ==  2 && TrimmedView.StartsWith(LITERAL(T, "0b"));
						bNeedRemove |= OtherStyle >= 0 && Base ==  2 && TrimmedView.StartsWith(LITERAL(T, "0B"));

						bNeedRemove |= OtherStyle <= 0 && Base == 16 && TrimmedView.StartsWith(LITERAL(T, "0x"));
						bNeedRemove |= OtherStyle >= 0 && Base == 16 && TrimmedView.StartsWith(LITERAL(T, "0X"));

						if (bNeedRemove) TrimmedView.RemovePrefix(2);
					}

					// Illegal base for prefix.
					else checkf(false, TEXT("Prefix is only supported for binary, octal, decimal and hexadecimal value."));
				}

				checkf(Base >= 2 && Base <= 36, TEXT("Illegal base."));

				auto ToDigit = [=](T Char) -> unsigned
				{
					if constexpr (bHasDigitStyle) if (Param.DigitStyle != 0)
					{
						return TChar<T>::ToDigit(Char, Param.DigitStyle < 0);
					}

					return TChar<T>::ToDigit(Char);
				};

				using UnsignedU = TMakeUnsigned<U>;

				// The limit value that can be stored in an unsigned integer.
				constexpr UnsignedU UnsignedMaximum = static_cast<UnsignedU>(-1);

				// The limit value that can be stored in a signed integer.
				constexpr U SignedMaximum = static_cast<U>(UnsignedMaximum >> 1);
				constexpr U SignedMinimum =  -static_cast<U>(SignedMaximum) - 1;

				UnsignedU LastValue = 0;
				UnsignedU Unsigned  = 0;

				if (TrimmedView.IsEmpty()) return false;

				unsigned Digit;

				Digit = ToDigit(TrimmedView.Front());

				// The first character must be a digit.
				if (Digit >= Base) return false;

				TrimmedView.RemovePrefix(1);

				Unsigned = static_cast<UnsignedU>(Digit);

				while (!TrimmedView.IsEmpty())
				{
					Digit = ToDigit(TrimmedView.Front());

					if (Digit >= Base) break;

					TrimmedView.RemovePrefix(1);

					LastValue = Unsigned;

					Unsigned = static_cast<UnsignedU>(LastValue * Base + Digit);

					if (Unsigned < LastValue) return false;
				}

				View = TrimmedView;

				if constexpr (CSigned<U>)
				{
					// Handle overflow.
					if (!bNegative && Unsigned >= static_cast<UnsignedU>(SignedMaximum)) return false;
					if ( bNegative && Unsigned >= static_cast<UnsignedU>(SignedMinimum)) return false;

					// Handle negative sign.
					if (bNegative) Unsigned = static_cast<UnsignedU>(-Unsigned);
				}

				Object = static_cast<U>(Unsigned);
				return true;
			}

			// Format the floating-point value by structured parameters.
			else if constexpr (CFloatingPoint<U>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<signed>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<signed>; };

				constexpr bool bHasSign      = requires { { Param.bSign     } -> CBooleanTestable; };
				constexpr bool bHasPrefix    = requires { { Param.bPrefix   } -> CBooleanTestable; };
				constexpr bool bHasPrecision = requires { { Param.Precision } -> CConvertibleTo<unsigned>; };

				constexpr bool bHasFormat =
					requires
					{
						{ Param.bFixed      } -> CBooleanTestable;
						{ Param.bScientific } -> CBooleanTestable;
					};

				NAMESPACE_STD::chars_format Format = NAMESPACE_STD::chars_format::general;

				if constexpr (bHasFormat) if ( Param.bFixed && !Param.bScientific) Format = NAMESPACE_STD::chars_format::fixed;
				if constexpr (bHasFormat) if (!Param.bFixed &&  Param.bScientific) Format = NAMESPACE_STD::chars_format::scientific;
				if constexpr (bHasFormat) if (!Param.bFixed && !Param.bScientific) Format = NAMESPACE_STD::chars_format::hex;

				// Create a temporary view to avoid modifying the original view.
				TStringView<T> TrimmedView = View;

				bool bNegative = false;

				if (TrimmedView.StartsWith(LITERAL(T, '-')))
				{
					bNegative = true;
					TrimmedView.RemovePrefix(1);
				}
				else if constexpr (bHasSign) if (Param.bSign) if (TrimmedView.StartsWith(LITERAL(T, '+'))) TrimmedView.RemovePrefix(1);

				signed DigitStyle = 0; if constexpr (bHasDigitStyle) DigitStyle = Param.DigitStyle;
				signed OtherStyle = 0; if constexpr (bHasOtherStyle) OtherStyle = Param.OtherStyle;

				// Handle the infinity and NaN values.
				{
					const U Infinity = bNegative ? -std::numeric_limits<U>::infinity()  : std::numeric_limits<U>::infinity();
					const U NaN      = bNegative ? -std::numeric_limits<U>::quiet_NaN() : std::numeric_limits<U>::quiet_NaN();

					if constexpr (bHasDigitStyle || bHasOtherStyle)
					{
						TOptional<U> Result;

						if (DigitStyle <= 0 && OtherStyle <= 0)
						{
							if (TrimmedView.StartsWith(LITERAL(T, "infinity"))) Result = Infinity;
							if (TrimmedView.StartsWith(LITERAL(T, "nan")))      Result = NaN;
						}

						if (DigitStyle >= 0 && OtherStyle <= 0)
						{
							if (TrimmedView.StartsWith(LITERAL(T, "Infinity"))) Result = Infinity;
							if (TrimmedView.StartsWith(LITERAL(T, "NaN")))      Result = NaN;
						}

						if (DigitStyle <= 0 && OtherStyle >= 0)
						{
							if (TrimmedView.StartsWith(LITERAL(T, "iNFINITY"))) Result = Infinity;
							if (TrimmedView.StartsWith(LITERAL(T, "nAn")))      Result = NaN;
						}

						if (DigitStyle >= 0 && OtherStyle >= 0)
						{
							if (TrimmedView.StartsWith(LITERAL(T, "INFINITY"))) Result = Infinity;
							if (TrimmedView.StartsWith(LITERAL(T, "NAN")))      Result = NaN;
						}

						if (Result.IsValid())
						{
							TrimmedView.RemovePrefix(NAMESPACE_STD::isnan(*Result) ? 3 : 8);
							Object = *Result;
							return true;
						}

						return false;
					}

					if    (TrimmedView.StartsWith(LITERAL(T, "infinity"))
						|| TrimmedView.StartsWith(LITERAL(T, "Infinity"))
						|| TrimmedView.StartsWith(LITERAL(T, "iNFINITY"))
						|| TrimmedView.StartsWith(LITERAL(T, "INFINITY")))
					{
						TrimmedView.RemovePrefix(8);
						Object = Infinity;
						return true;
					}

					if    (TrimmedView.StartsWith(LITERAL(T, "nan"))
						|| TrimmedView.StartsWith(LITERAL(T, "NaN"))
						|| TrimmedView.StartsWith(LITERAL(T, "nAn"))
						|| TrimmedView.StartsWith(LITERAL(T, "NAN")))
					{
						TrimmedView.RemovePrefix(3);
						Object = NaN;
						return true;
					}
				}

				bool bHex = Format == NAMESPACE_STD::chars_format::hex;

				// Handle the prefix.
				if constexpr (bHasPrefix) if (Param.bPrefix)
				{
					bool bNeedRemove = false;

					bNeedRemove |= OtherStyle <= 0 && TrimmedView.StartsWith(LITERAL(T, "0x"));
					bNeedRemove |= OtherStyle >= 0 && TrimmedView.StartsWith(LITERAL(T, "0X"));

					if (bNeedRemove)
					{
						bHex = true;
						TrimmedView.RemovePrefix(2);
						Format = NAMESPACE_STD::chars_format::hex;
					}
				}

				auto Iter = TrimmedView.Begin();

				do
				{
					auto IsDigit = [=](T Char, unsigned Base) -> unsigned
					{
						if constexpr (bHasDigitStyle) if (Param.DigitStyle != 0)
						{
							return TChar<T>::ToDigit(Char, Param.DigitStyle < 0) < Base;
						}

						return TChar<T>::ToDigit(Char) < Base;
					};

					// Handle the number before the decimal point.
					while (Iter != View.End() && IsDigit(*Iter, bHex ? 16 : 10)) ++Iter;

					// Handle the decimal point.
					if (Iter != View.End() && *Iter == LITERAL(T, '.')) ++Iter;

					// Handle the number after the decimal point.
					if constexpr (bHasPrecision)
					{
						for (unsigned Index = 0; Index < Param.Precision; ++Index)
						{
							if (Iter != View.End() && IsDigit(*Iter, bHex ? 16 : 10)) ++Iter;
						}
					}
					else while (Iter != View.End() && IsDigit(*Iter, bHex ? 16 : 10)) ++Iter;

					const bool bScientific = static_cast<bool>(Format & NAMESPACE_STD::chars_format::scientific);

					// Handle the scientific notation.
					if (Iter != View.End())
					{
						bool bNeedRemove = false;

						bNeedRemove |= OtherStyle <= 0 && bHex && *Iter == LITERAL(T, 'p');
						bNeedRemove |= OtherStyle >= 0 && bHex && *Iter == LITERAL(T, 'P');

						bNeedRemove |= OtherStyle <= 0 && bScientific && *Iter == LITERAL(T, 'e');
						bNeedRemove |= OtherStyle >= 0 && bScientific && *Iter == LITERAL(T, 'E');

						if (bNeedRemove) ++Iter;
					}

					// Handle the sign of the exponent.
					if (Iter != View.End() && *Iter == LITERAL(T, '+')) ++Iter;
					if (Iter != View.End() && *Iter == LITERAL(T, '-')) ++Iter;

					// Handle the number of the exponent.
					while (Iter != View.End() && IsDigit(*Iter, 10)) ++Iter;
				}
				while (false);

				U Result;

				NAMESPACE_STD::from_chars_result ConvertResult;

				TString<char, TInlineAllocator<64>> Buffer;

				if (bNegative) Buffer += '-';

				Buffer.Append(TrimmedView.Begin(), Iter);

				ConvertResult = NAMESPACE_STD::from_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Result, Format);

				if (ConvertResult.ec == NAMESPACE_STD::errc::result_out_of_range) return false;
				if (ConvertResult.ec == NAMESPACE_STD::errc::invalid_argument)    return false;

				size_t Num = ConvertResult.ptr - Buffer.GetData();

				check(Num != 0);

				if (bNegative) Num -= 1;

				View = TrimmedView.RemovePrefix(Num);

				Object = Result;
				return true;
			}
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

								struct { TStringView<T> Fmt; } Param = { Subfmt };

								if constexpr (bIsFormat) return TStringObjectFormatter<T>::Do(String, Object, Param);

								else return TStringObjectParser<T>::Do(String, Object, Param);
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
	NAMESPACE_PRIVATE::TStringObjectFormatter<ElementType>::Do(*this, AsConst(Value), Invalid);
}

template <CCharType T, CAllocator<T> Allocator>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendInt(U Value, unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	struct { unsigned Base; } Param = { Base };

	NAMESPACE_PRIVATE::TStringObjectFormatter<ElementType>::Do(*this, AsConst(Value), Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value)
{
	NAMESPACE_PRIVATE::TStringObjectFormatter<ElementType>::Do(*this, AsConst(Value), Invalid);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific)
{
	struct { bool bFixed; bool bScientific; } Param = { bFixed, bScientific };

	NAMESPACE_PRIVATE::TStringObjectFormatter<ElementType>::Do(*this, AsConst(Value), Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific, unsigned Precision)
{
	struct { bool bFixed; bool bScientific; unsigned Precision; } Param = { bFixed, bScientific, Precision };

	NAMESPACE_PRIVATE::TStringObjectFormatter<ElementType>::Do(*this, AsConst(Value), Param);
}

template <CCharType T>
constexpr bool TStringView<T>::ToBoolAndTrim()
{
	bool Value = false;

	if (!NAMESPACE_PRIVATE::TStringObjectParser<ElementType>::Do(*this, Value, Invalid))
	{
		if (int IntValue; NAMESPACE_PRIVATE::TStringObjectParser<ElementType>::Do(*this, IntValue, Invalid))
		{
			Value = IntValue != 0;
		}
	}

	return Value;
}

template <CCharType T>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToIntAndTrim(unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	U Value = 0;

	struct { unsigned Base; } Param = { Base };

	NAMESPACE_PRIVATE::TStringObjectParser<ElementType>::Do(*this, Value, Param);

	return Value;
}

template <CCharType T>
template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToFloatAndTrim(bool bFixed, bool bScientific)
{
	U Value = NAMESPACE_STD::numeric_limits<U>::quiet_NaN();

	struct { bool bFixed; bool bScientific; } Param = { bFixed, bScientific };

	NAMESPACE_PRIVATE::TStringObjectParser<ElementType>::Do(*this, Value, Param);

	return Value;
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
