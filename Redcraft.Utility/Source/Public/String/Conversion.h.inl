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

#define LEFT_BRACE  LITERAL(T, '{')
#define RIGHT_BRACE LITERAL(T, '}')

#define ESCAPE_LEFT_BRACE  TStringView(LITERAL(T, "<[{"))
#define ESCAPE_RIGHT_BRACE TStringView(LITERAL(T, "}]>"))

NAMESPACE_PRIVATE_BEGIN

template <CCharType T, bool bIsFormat>
struct TStringHelper
{
	static FORCEINLINE bool FormatObject(auto& Result, TStringView<T> Fmt, auto& Object) requires (bIsFormat)
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

	static FORCEINLINE bool ParseObject(TStringView<T>& View, TStringView<T> Fmt, auto& Object) requires (!bIsFormat)
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

	static FORCEINLINE size_t Do(auto& Result, TStringView<T> Fmt, auto ArgsTuple)
	{
		size_t FormattedObjectNum = 0;

		size_t ArgsIndex = 0;

		auto ParseFormat = [&FormattedObjectNum, &ArgsIndex, ArgsTuple](auto& Self, auto& String, TStringView<T>& Fmt) -> bool
		{
			bool bIsFullyFormatted = true;

			while (!Fmt.IsEmpty())
			{
				if (Fmt.StartsWith(ESCAPE_LEFT_BRACE))
				{
					Fmt.RemovePrefix(ESCAPE_LEFT_BRACE.Num());

					if constexpr (!bIsFormat)
					{
						if (!String.StartsWith(LEFT_BRACE)) return false;

						String.RemovePrefix(1);
					}
					else String += LEFT_BRACE;

					continue;
				}

				if (Fmt.StartsWith(ESCAPE_RIGHT_BRACE))
				{
					Fmt.RemovePrefix(ESCAPE_RIGHT_BRACE.Num());

					if constexpr (!bIsFormat)
					{
						if (!String.StartsWith(RIGHT_BRACE)) return false;

						String.RemovePrefix(1);
					}
					else String += RIGHT_BRACE;

					continue;
				}

				if (Fmt.StartsWith(LEFT_BRACE))
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
							PlaceholderBegin = Fmt.FindFirstOf(LEFT_BRACE, PlaceholderBegin + 1);

							if (PlaceholderBegin == INDEX_NONE) break;

							if (Fmt.First(PlaceholderBegin + 1).EndsWith(ESCAPE_LEFT_BRACE))
							{
								++PlaceholderBegin;
							}
							else break;
						}

						while (true)
						{
							PlaceholderEnd = Fmt.FindFirstOf(RIGHT_BRACE, PlaceholderEnd + 1);

							if (PlaceholderEnd == INDEX_NONE) break;

							if (Fmt.Substr(PlaceholderEnd).StartsWith(ESCAPE_RIGHT_BRACE))
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

						else bIsSuccessful = TStringHelper<T, true>::Do(FormattedSubfmt, Subfmt, ArgsTuple);

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
							if (!PlaceholderIndex.IsNumeric())
							{
								checkf(false, TEXT("Invalid placeholder index."));

								if constexpr (bIsFormat)
								{
									String += LEFT_BRACE;
									String += Subfmt;
									String += RIGHT_BRACE;

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

								if constexpr (bIsFormat) return TStringHelper::FormatObject(String, Subfmt, Object);

								else return TStringHelper::ParseObject(String, Subfmt, Object);
							},
							Index
						);
					}

					if (!bIsSuccessful)
					{
						if constexpr (bIsFormat)
						{
							String += LEFT_BRACE;
							String += Subfmt;
							String += RIGHT_BRACE;

							bIsFullyFormatted = false;
						}
						else return false;
					}
					else ++FormattedObjectNum;

					continue;
				}

				check_code({ if (Fmt.StartsWith(RIGHT_BRACE)) check_no_entry(); });

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
TString<T, Allocator> TString<T, Allocator>::Format(TStringView<ElementType> Fmt, const Ts&... Args)
{
	// The Unreal Engine says that the starting buffer size catches 99.97% of printf calls.
	constexpr size_t ReserveBufferSize = 512;

	TString Result;

	Result.Reserve(ReserveBufferSize);

	NAMESPACE_PRIVATE::TStringHelper<ElementType, true>::Do(Result, Fmt, ForwardAsTuple(Args...));

	return Result;
}

template <CCharType T>
template <typename ... Ts>
size_t TStringView<T>::Parse(TStringView Fmt, Ts&... Args) const
{
	TStringView View = *this;

	return NAMESPACE_PRIVATE::TStringHelper<ElementType, false>::Do(View, Fmt, ForwardAsTuple(Args...));
}

#undef LEFT_BRACE
#undef RIGHT_BRACE

#undef ESCAPE_LEFT_BRACE
#undef ESCAPE_RIGHT_BRACE

template <CCharType T>
constexpr bool TStringView<T>::ToBool() const
{
	if    (StartsWith(LITERAL(T, '1'))
		|| StartsWith(LITERAL(T, "true"))
		|| StartsWith(LITERAL(T, "True"))
		|| StartsWith(LITERAL(T, "TRUE")))
	{
		return true;
	}

	if    (StartsWith(LITERAL(T, '0'))
		|| StartsWith(LITERAL(T, "false"))
		|| StartsWith(LITERAL(T, "False"))
		|| StartsWith(LITERAL(T, "FALSE")))
	{
		return false;
	}

	return ToInt() != 0;
}

template <CCharType T>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToInt(unsigned Base) const
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	bool bNegative = false;

	TStringView View = *this;

	if constexpr (CSigned<U>)
	{
		if (View.StartsWith(LITERAL(ElementType, '-')))
		{
			bNegative = true;
			View.RemovePrefix(1);
		}
	}

	using UnsignedU = TMakeUnsigned<U>;

	constexpr UnsignedU UnsignedMaximum = static_cast<UnsignedU>(-1);

	constexpr U SignedMaximum = static_cast<U>(UnsignedMaximum >> 1);
	constexpr U SignedMinimum =  -static_cast<U>(SignedMaximum) - 1;

	UnsignedU LastValue = 0;
	UnsignedU     Value = 0;

	for (ElementType Char : View)
	{
		auto Digit = TChar<ElementType>::ToDigit(Char);

		if (Digit >= Base) break;

		LastValue = Value;

		Value = static_cast<UnsignedU>(LastValue * Base + Digit);

		if (Value < LastValue)
		{
			if constexpr (CSigned<U>)
			{
				return bNegative ? SignedMinimum : SignedMaximum;
			}
			else return UnsignedMaximum;
		}
	}

	if constexpr (CSigned<U>)
	{
		if (!bNegative && Value >= static_cast<UnsignedU>(SignedMaximum)) return SignedMaximum;
		if ( bNegative && Value >= static_cast<UnsignedU>(SignedMinimum)) return SignedMinimum;

		if (bNegative) Value = static_cast<UnsignedU>(-Value);
	}

	return static_cast<U>(Value);
}

template <CCharType T>
template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
constexpr U TStringView<T>::ToFloat(bool bFixed, bool bScientific) const
{
	NAMESPACE_STD::chars_format Format;

	if      ( bFixed &&  bScientific) Format = NAMESPACE_STD::chars_format::general;
	else if ( bFixed && !bScientific) Format = NAMESPACE_STD::chars_format::fixed;
	else if (!bFixed &&  bScientific) Format = NAMESPACE_STD::chars_format::scientific;
	else                              Format = NAMESPACE_STD::chars_format::hex;

	U Result;

	auto Iter = this->Begin();

	bool bNegativeMantissa = false;
	bool bNegativeExponent = false;

	do
	{
		if (Iter == this->End()) break;

		if (*Iter == LITERAL(ElementType, '-'))
		{
			bNegativeMantissa = true;
			++Iter;
		}

		auto DecimalPoint = this->End();
		auto NonZeroBegin = this->End();

		while (Iter != this->End())
		{
			if (DecimalPoint == this->End() && *Iter == LITERAL(ElementType, '.'))
			{
				DecimalPoint = Iter;
			}
			else if (TChar<ElementType>::IsDigit(*Iter, Format == NAMESPACE_STD::chars_format::hex ? 16 : 10))
			{
				if (NonZeroBegin == this->End() && *Iter != LITERAL(ElementType, '0'))
				{
					NonZeroBegin = Iter;
				}
			}
			else break;

			++Iter;
		}

		if (DecimalPoint == this->End()) DecimalPoint = Iter;

		bNegativeExponent = DecimalPoint < NonZeroBegin;

		if (Iter == this->End()) break;

		bool bHasExponent = false;

		if (Format == NAMESPACE_STD::chars_format::general || Format == NAMESPACE_STD::chars_format::scientific)
		{
			if (*Iter == LITERAL(ElementType, 'e') || *Iter == LITERAL(ElementType, 'E'))
			{
				bHasExponent = true;
				++Iter;
			}
		}
		else if (Format == NAMESPACE_STD::chars_format::hex)
		{
			if (*Iter == LITERAL(ElementType, 'p') || *Iter == LITERAL(ElementType, 'P'))
			{
				bHasExponent = true;
				++Iter;
			}
		}

		if (Iter == this->End() || !bHasExponent) break;

		if (*Iter == LITERAL(ElementType, '+')) ++Iter;
		if (*Iter == LITERAL(ElementType, '-')) { bNegativeExponent = true; ++Iter; }

		auto ExponentBegin = Iter;

		while (Iter != this->End() && TChar<ElementType>::IsDigit(*Iter, 10)) ++Iter;

		auto ExponentEnd = Iter;

		if (NonZeroBegin == this->End()) break;

		auto Exponent = TStringView(ExponentBegin, ExponentEnd).ToInt();

		if (bNegativeExponent) Exponent = -Exponent;

		Exponent += static_cast<int>(DecimalPoint - NonZeroBegin);

		bNegativeExponent = Exponent < 0;
	}
	while (false);

	NAMESPACE_STD::from_chars_result ConvertResult;

	if constexpr (!CSameAs<ElementType, char>)
	{
		TArray<char, TInlineAllocator<64>> Buffer(this->Begin(), Iter);

		ConvertResult = NAMESPACE_STD::from_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Result, Format);
	}
	else ConvertResult = NAMESPACE_STD::from_chars(ToAddress(this->Begin()), ToAddress(this->End()), Result, Format);

	if (ConvertResult.ec == NAMESPACE_STD::errc::result_out_of_range)
	{
		if (!bNegativeMantissa && !bNegativeExponent) return  NAMESPACE_STD::numeric_limits<U>::infinity();
		if ( bNegativeMantissa && !bNegativeExponent) return -NAMESPACE_STD::numeric_limits<U>::infinity();
		if (!bNegativeMantissa &&  bNegativeExponent) return static_cast<U>( 0.0);
		                                              return static_cast<U>(-0.0);
	}

	if (ConvertResult.ec == NAMESPACE_STD::errc::invalid_argument) return NAMESPACE_STD::numeric_limits<U>::quiet_NaN();

	return Result;
}

template <CCharType T, CAllocator<T> Allocator>
void TString<T, Allocator>::AppendBool(bool Value)
{
	if (Value) Append(LITERAL(ElementType, "True"));
	else       Append(LITERAL(ElementType, "False"));
}

template <CCharType T, CAllocator<T> Allocator>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendInt(U Value, unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	static_assert(TChar<ElementType>::IsASCII());

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

	constexpr size_t BufferSize = sizeof(UnsignedU) * 8 + (CSigned<U> ? 1 : 0);

	ElementType Buffer[BufferSize];

	ElementType* Iter = Buffer + BufferSize;

	switch (Base)
	{
	case 0x02: do { *--Iter = static_cast<ElementType>('0' + (Unsigned & 0b00001)); Unsigned >>= 1; } while (Unsigned != 0); break;
	case 0x04: do { *--Iter = static_cast<ElementType>('0' + (Unsigned & 0b00011)); Unsigned >>= 2; } while (Unsigned != 0); break;
	case 0x08: do { *--Iter = static_cast<ElementType>('0' + (Unsigned & 0b00111)); Unsigned >>= 3; } while (Unsigned != 0); break;
	case 0x10: do { *--Iter =   TChar<ElementType>::FromDigit(Unsigned & 0b01111);  Unsigned >>= 4; } while (Unsigned != 0); break;
	case 0X20: do { *--Iter =   TChar<ElementType>::FromDigit(Unsigned & 0b11111);  Unsigned >>= 5; } while (Unsigned != 0); break;

	case 3:
	case 5:
	case 6:
	case 7:
	case 9:
	case 10: do { *--Iter = static_cast<ElementType>('0' + Unsigned % Base); Unsigned = static_cast<UnsignedU>(Unsigned / Base); } while (Unsigned != 0); break;
	default: do { *--Iter =  TChar<ElementType>::FromDigit(Unsigned % Base); Unsigned = static_cast<UnsignedU>(Unsigned / Base); } while (Unsigned != 0); break;
	}

	if constexpr (CSigned<U>) if (bNegative) *--Iter = LITERAL(T, '-');

	Append(Iter, Buffer + BufferSize);
}

NAMESPACE_PRIVATE_BEGIN

template <CCharType T, size_t Overload>
struct TStringFloatSerializer
{
	static FORCEINLINE void Do(auto& Result, auto Value, bool bFixed, bool bScientific, unsigned Precision)
	{
		NAMESPACE_STD::chars_format Format;

		if constexpr (Overload >= 1)
		{
			if      ( bFixed &&  bScientific) Format = NAMESPACE_STD::chars_format::general;
			else if ( bFixed && !bScientific) Format = NAMESPACE_STD::chars_format::fixed;
			else if (!bFixed &&  bScientific) Format = NAMESPACE_STD::chars_format::scientific;
			else                              Format = NAMESPACE_STD::chars_format::hex;
		}

		constexpr size_t StartingBufferSize = 64;

		TArray<char, TInlineAllocator<StartingBufferSize>> Buffer(StartingBufferSize / 2);

		NAMESPACE_STD::to_chars_result ConvertResult;

		do
		{
			Buffer.SetNum(Buffer.Num() * 2);

			if      constexpr (Overload == 0) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value);
			else if constexpr (Overload == 1) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value, Format);
			else                              ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Value, Format, Precision);
		}
		while (ConvertResult.ec == NAMESPACE_STD::errc::value_too_large);

		Buffer.SetNum(ConvertResult.ptr - Buffer.GetData());

		const bool bNegative = Buffer[0] == '-';

		const char* Iter = Buffer.GetData() + (bNegative ? 1 : 0);

		if (*Iter == 'i')
		{
			if (bNegative) Result.Append(LITERAL(T, "-Infinity"));
			else           Result.Append(LITERAL(T,  "Infinity"));
			return;
		}

		if (*Iter == 'n')
		{
			if (bNegative) Result.Append(LITERAL(T, "-NaN"));
			else           Result.Append(LITERAL(T,  "NaN"));
			return;
		}

		unsigned Base;

		if constexpr (Overload == 0) Base = 10;
		else Base = Format == NAMESPACE_STD::chars_format::hex ? 16 : 10;

		for (char& Char : Buffer)
		{
			const auto Digit = FChar::ToDigit(Char);
			if (Digit < Base) Char = FChar::FromDigit(Digit);
		}

		Result.Append(Buffer.Begin(), Buffer.End());
	}
};

NAMESPACE_PRIVATE_END

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value)
{
	NAMESPACE_PRIVATE::TStringFloatSerializer<ElementType, 0>::Do(*this, Value, false, false, 0);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific)
{
	NAMESPACE_PRIVATE::TStringFloatSerializer<ElementType, 1>::Do(*this, Value, bFixed, bScientific, 0);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific, unsigned Precision)
{
	NAMESPACE_PRIVATE::TStringFloatSerializer<ElementType, 2>::Do(*this, Value, bFixed, bScientific, Precision);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
