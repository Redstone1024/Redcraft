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

// TODO: Refactor the conversion tool by more elegant way.

// The conversion tool uses a string to describe the object format.
//
// The format string consists of the following parts:
//
// - A pair of braces:    The object placeholder.
// - A escaped brace:     The brace is formatted or parsed as-is
// - A general character: The character is formatted or parsed as-is.
// - A space character:   The character is formatted as-is or all leading space characters are consumed when parsing.
//
// About the object placeholder:
//
// Use the ':' character to separate the different layers of object placeholders, for a normal object he has only two layers,
// for a string or a character he may have three layers to represent the format of the escape character,
// for a container he may have many layers to represent the format of the elements.
//
// The first level is the object index.
// The other levels are the object format, which is used to format or parse the object.
//
// The object format contains a common optional fill-and-align consisting of the following parts:
//
// i.   A fill character:   The character is used to fill width of the object. It is optional.
//	                        It should be representable as a single unicode otherwise it is undefined behavior.
// ii.  A alignment option: The character is used to indicate the direction of alignment. It is optional if it does not create ambiguity.
//	                        '<' for left, '>' for right, '^' for center. If cannot absolute centering, offset to the left.
// iii. A width number:     The number is used to specify the width of the object.
//	                        It should be a decimal number without any sign.
//
// The width is limits the minimum number of characters in formatting and the maximum number of characters in parsing.
// The fill character is treated as a space character in parsing.
//
// After the fill-and-align, the object format contains type-specific options.
//
// Specially, only strings and characters that agree with the main character type are considered string values and character values.
//
// For string values:
//
// 1. The type indicators part:
//
//	- none: Indicates the as-is formatting.
//	- 'S':  Indicates uppercase formatting if case indicators is '!', otherwise as-is formatting.
//	- 's':  Indicates lowercase formatting if case indicators is '!', otherwise as-is formatting.
//
// 2. The case indicators part:
//
//	- none: Indicates the as-is formatting.
//	- '!':  Indicates the case as the type indicators case.
//
// 3. The escape indicators part:
//
//	- none: Indicates the as-is formatting.
//	- '?':  Indicates the escape formatting.
//
// For character values:
//
// 1. The type indicators part:
//
//	- none:               Indicates the as-is formatting.
//	- 'C':                Indicates uppercase formatting if case indicators is '!', otherwise as-is formatting.
//	- 'c':                Indicates lowercase formatting if case indicators is '!', otherwise as-is formatting.
//	- 's' or 'S':         Indicates that characters should be treated as strings.
//		                  See the string values section for additional formatting.
//	- 'B', 'D', 'O', 'X': Indicates that characters should be treated as integer values.
//		                  See the integer values section for additional formatting.
//
// 2. The case indicators part:
//
//	- none: Indicates the as-is formatting.
//	- '!':  Indicates the case as the type indicators case.
//
// 3. The escape indicators part:
//
//	- none: Indicates the as-is formatting.
//	- '?':  Indicates the escape formatting.
//
// For boolean values:
//
// 1. The type indicators part:
//
//	- none or 'S':        Indicates that boolean value should be treated as string 'True' or 'False'.
//		                  See the string values section for additional formatting.
//	- 'C':                Indicates that boolean value should be treated as character 'T' or 'F'.
//		                  See the character values section for additional formatting.
//	- 'B', 'D', 'O', 'X': Indicates that boolean value should be treated as integer 1 or 0.
//		                  See the integer values section for additional formatting.
//
// For integer values:
//
// 1. The positive indicators part:
//
//	- none or '-': Indicates hide the sign of the positive number.
//	- '+':         Indicates show the '+' of the positive number.
//	- ' ':         Indicates show the ' ' of the positive number.
//
// 2. The prefix indicators part:
//
//	- none: Indicates hide the prefix of the number.
//	- '#':  Indicates show the prefix of the number. Indicates auto-detect the base in parsing.
//
// 3. The '0' padded width indicators part:
//
//	- none: Indicates that the '0' padded width is 0.
//	- '0N': Indicates that the '0' padded width is N.
//
// 4. The base indicators part:
//
//	- none or '_0': Indicates decimal in formatting. Indicates auto-detect the base in parsing.
//	- '_N':         Indicates that the base is N, between [2, 36].
//
// 5. The type indicators part:
//
//	- none or 'D': Indicates decimal formatting.     Same as '_10I'.
//	- 'B':         Indicates binary formatting.      Same as '_2I'.
//	- 'O':         Indicates octal formatting.       Same as '_8I'.
//	- 'X':         Indicates hexadecimal formatting. Same as '_16I'.
//	- 'I':         Indicates specified formatting by base indicators.
//
// For floating-point values:
//
// 1. The positive indicators part:
//
//	- none or '-': Indicates hide the sign of the positive number.
//	- '+':         Indicates show the '+' of the positive number.
//	- ' ':         Indicates show the ' ' of the positive number.
//
// 2. The prefix indicators part:
//
//	- none: Indicates hide the prefix of the number.
//	- '#':  Indicates show the prefix of the number. Indicates auto-detect the hex scientific in parsing.
//
// 3. The precision indicators part:
//
//	- none: Indicates six decimal for fixed-point in formatting. Indicates auto-detect the precision in parsing.
//	- '.N': Indicates that the precision is N, It should be a decimal number without any sign.
//
// 4. The type indicators part:
//
//	- none or 'F': Indicates fixed-point formatting.
//	- 'G':         Indicates general formatting.
//	- 'E':         Indicates scientific formatting.
//	- 'A':         Indicates hex scientific formatting.
//
// For pointer values:
//
// 1. The type indicators part:
//
//	- none or 'P': The pointer value is formatted as hexadecimal with prefix and fill-and-align.
//		           Same as '#X'. The default width depends on the platform.
//
// For tuple values:
//
// 1. The type indicators part:
//
//	- none: Indicates general formatting. Same as 'T(_, _)'.
//	- 'M':  Indicates map formatting.     Same as 'T_: _'.
//	- 'N':  Indicates none formatting.    Same as 'T__'.
//	- 'T':  Indicates user-defined formatting.
//
// 2. The user-defined part:
//
//	i.   A begin string:     Indicates the begin string of the tuple. Cannot contain '_' or ':' character.
//	ii.  '_':                Indicates a placeholder.
//	iii. A separator string: Indicates the separator string of the tuple. Cannot contain '_' character.
//	iv.  '_':                Indicates a placeholder.
//	v.   An end string:      Indicates the end string of the tuple. Cannot contain '_' or ':' character.
//
// For container values:
//
// 1. The type indicators part:
//
//	- none: Indicates general formatting. Same as 'T[_, _]'.
//	- 'N':  Indicates none formatting.    Same as 'T__'.
//	- 'T':  Indicates user-defined formatting.
//
// 2. The user-defined part:
//
//	i.   A begin string:     Indicates the begin string of the container. Cannot contain '_' or ':' character.
//	ii.  '_':                Indicates a placeholder.
//	iii. A separator string: Indicates the separator string of the container. Cannot contain '_' character.
//	iv.  '_':                Indicates a placeholder.
//	v.   An end string:      Indicates the end string of the container. Cannot contain '_' or ':' character.
//
// For the type indicator part of the boolean, integer, and floating-point values,
// The case of letter indicates the case of the first letter or number part,
// and other parts can also be uppercase by appended the '!' mark.
//
// Specially, the case of letters is ignored by default in parsing,
// and can be forced to match the required case by appending the '=' mark.
//
// Tuples of pointers and containers cannot be parsed.
//
// Examples:
//
// - '{:}':    Parse the integer value in decimal without positive sign.
// - '{:+D}':  Parse the integer value in decimal with optional positive sign.
// - '{:+#I}': Parse the integer value in any formatting.
// - '{:}':    Parse the floating-point value in fixed-point without positive sign.
// - '{:+F}':  Parse the floating-point value in fixed-point with optional positive sign.
// - '{:+#G}': Parse the floating-point value in any formatting.

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
// - Padding:     A unsigned integer that represents the '0' padded width of the number.
//	              It is valid for integer values.
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
// - Precision:   A signed integer that represents the number of digits after the decimal point. Negative value means ignore.
//	              For parsing, it is used to determine the maximum number of digits after the decimal point.
//	              It is valid for floating-point values.

template <CCharType T>
struct TStringObjectFormatter
{
	static bool Do(auto& Result, auto& Object, auto Param)
	{
		// ReSharper disable once CppInconsistentNaming
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

			// Parse the fill-and-align part and reserve the space for the result.
			auto ParseFillAndAlign = [&Result, &Fmt]
			{
				TStringView<T> FillCharacter = LITERAL(T, " ");

				T AlignmentOption = CIntegral<U> || CFloatingPoint<U> ? LITERAL(T, '>') : LITERAL(T, '<');

				size_t AlignmentWidth = 0;

				// Parse the fill-and-align part of the object format.
				if (!Fmt.IsEmpty())
				{
					size_t Index = Fmt.FindFirstOf(LITERAL(T, "123456789"));

					if (Index != INDEX_NONE)
					{
						// Create a temporary view to avoid modifying the original view.
						TStringView<T> TrimmedFmt = Fmt;

						TStringView<T> FillAndAlign = TrimmedFmt.First(Index);

						TrimmedFmt.RemovePrefix(Index);

						TStringView<T> View = TrimmedFmt.Substr(0, TrimmedFmt.FindFirstNotOf(LITERAL(T, "0123456789")));

						TrimmedFmt.RemovePrefix(View.Num());

						size_t PossibleWidth = View.template ToInt<size_t>();

						bool bIsValid = true;

						if (!FillAndAlign.IsEmpty())
						{
							if      (FillAndAlign.Back() == LITERAL(T, '<')) { FillAndAlign.RemoveSuffix(1); AlignmentOption = LITERAL(T, '<'); }
							else if (FillAndAlign.Back() == LITERAL(T, '>')) { FillAndAlign.RemoveSuffix(1); AlignmentOption = LITERAL(T, '>'); }
							else if (FillAndAlign.Back() == LITERAL(T, '^')) { FillAndAlign.RemoveSuffix(1); AlignmentOption = LITERAL(T, '^'); }
							else
							{
								if (FillAndAlign.Num() != 1)
								{
									// If the string contains ASCII then it must not be represented as a single unicode.
									for (T Char : FillAndAlign) if (TChar<T>::IsASCII(Char)) bIsValid = false;
								}
								else if (FillAndAlign.Front() == LITERAL(T, '.')) bIsValid = false; // Ambiguously with the precision indicator.
								else if (FillAndAlign.Front() == LITERAL(T, '_')) bIsValid = false; // Ambiguously with the base indicator.
							}
						}

						if (bIsValid)
						{
							if (!FillAndAlign.IsEmpty()) FillCharacter = FillAndAlign;

							AlignmentWidth = PossibleWidth;

							Fmt = TrimmedFmt;
						}
					}
				}

				Result.Reserve(Result.Num() + AlignmentWidth * FillCharacter.Num());

				return MakeTuple(FillCharacter, AlignmentOption, AlignmentWidth, Result.Num());
			};

			// Apply the fill-and-align part to the result.
			auto ApplyFillAndAlign = [&Result](auto FillAndAlign)
			{
				auto [FillCharacter, AlignmentOption, AlignmentWidth, OriginalNum] = FillAndAlign;

				const size_t AppendedNum = Result.Num() - OriginalNum;

				if (AlignmentWidth > AppendedNum)
				{
					size_t LeftWidth  = 0;
					size_t RightWidth = 0;

					switch (AlignmentOption)
					{
					case LITERAL(T, '<'): RightWidth = AlignmentWidth - AppendedNum; break;
					case LITERAL(T, '>'): LeftWidth  = AlignmentWidth - AppendedNum; break;
					case LITERAL(T, '^'):
						{
							LeftWidth = (AlignmentWidth - AppendedNum) / 2;
							RightWidth = AlignmentWidth - AppendedNum - LeftWidth;
							break;
						}
					default: check_no_entry();
					}

					if (LeftWidth != 0)
					{
						Result.SetNum(Result.Num() + LeftWidth * FillCharacter.Num(), false);

						for (size_t Index = 0; Index != AppendedNum; ++Index)
						{
							Result[Result.Num() - Index - 1] = Result[OriginalNum + AppendedNum - Index - 1];
						}

						for (size_t Index = 0; Index != LeftWidth * FillCharacter.Num(); ++Index)
						{
							Result[OriginalNum + Index] = FillCharacter[Index % FillCharacter.Num()];
						}
					}

					if (RightWidth != 0)
					{
						for (size_t Index = 0; Index < RightWidth; ++Index)
						{
							Result += FillCharacter;
						}
					}
				}
			};

			// Format the string value by format string.
			if constexpr (requires { TStringView(Object); })
			{
				auto FillAndAlign = ParseFillAndAlign();

				bool bNeedToCase      = false;
				bool bStringLowercase = false;
				bool bNeedToEscape    = false;
				bool bEscapeLowercase = false;

				if      (Fmt.StartsWith(LITERAL(T, 'S'))) { bStringLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 's'))) { bStringLowercase = true;  Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '!'))) { bNeedToCase   = true; Fmt.RemovePrefix(1); }
				if (Fmt.StartsWith(LITERAL(T, '?'))) { bNeedToEscape = true; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);

				if (bNeedToEscape && Fmt.StartsWith(LITERAL(T, ':')))
				{
					Fmt.RemovePrefix(1);

					if      (Fmt.StartsWith(LITERAL(T, 'X'))) { bEscapeLowercase = false; Fmt.RemovePrefix(1); }
					else if (Fmt.StartsWith(LITERAL(T, 'x'))) { bEscapeLowercase = true;  Fmt.RemovePrefix(1); }

					if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);
				}

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				TStringView<T> String = Object;

				if (bNeedToEscape) Result += LITERAL(T, '\"');

				if (bNeedToCase || bNeedToEscape)
				{
					for (T Char : String)
					{
						if (bNeedToCase)
						{
							if (bStringLowercase) Char = TChar<T>::ToLower(Char);
							else                  Char = TChar<T>::ToUpper(Char);
						}

						if (bNeedToEscape)
						{
							switch (Char)
							{
							case LITERAL(T, '\"'): Result += LITERAL(T, "\\\""); break;
							case LITERAL(T, '\\'): Result += LITERAL(T, "\\\\"); break;
							case LITERAL(T, '\a'): Result += LITERAL(T, "\\a");  break;
							case LITERAL(T, '\b'): Result += LITERAL(T, "\\b");  break;
							case LITERAL(T, '\f'): Result += LITERAL(T, "\\f");  break;
							case LITERAL(T, '\n'): Result += LITERAL(T, "\\n");  break;
							case LITERAL(T, '\r'): Result += LITERAL(T, "\\r");  break;
							case LITERAL(T, '\t'): Result += LITERAL(T, "\\t");  break;
							case LITERAL(T, '\v'): Result += LITERAL(T, "\\v");  break;
							default:
								{
									if (!TChar<T>::IsASCII(Char) || !TChar<T>::IsPrint(Char))
									{
										Result += LITERAL(T, "\\x");

										const TMakeUnsigned<T> IntValue = static_cast<TMakeUnsigned<T>>(Char);

										struct { int DigitStyle; unsigned Padding; unsigned Base; } DigitParam = { bEscapeLowercase ? -1 : 1, sizeof(T) * 2, 16};

										verify(TStringObjectFormatter::Do(Result, IntValue, DigitParam));
									}
									else Result += Char;
								}
							}
						}
						else Result += Char;
					}
				}
				else Result += String;

				if (bNeedToEscape) Result += LITERAL(T, '\"');

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the character value by format string.
			else if constexpr (CCharType<U>)
			{
				if (Fmt.FindFirstOf(LITERAL(T, "Ss")) != INDEX_NONE)
				{
					const TStringView<T> StringValue(&Object, 1);

					return TStringObjectFormatter::Do(Result, StringValue, Param);
				}

				if (Fmt.FindFirstOf(LITERAL(T, "BbDdOoXxIi")) != INDEX_NONE)
				{
					const TMakeUnsigned<T> IntValue = static_cast<TMakeUnsigned<T>>(Object);

					return TStringObjectFormatter::Do(Result, IntValue, Param);
				}

				auto FillAndAlign = ParseFillAndAlign();

				bool bNeedToCase      = false;
				bool bStringLowercase = false;
				bool bNeedToEscape    = false;
				bool bEscapeLowercase = false;

				if      (Fmt.StartsWith(LITERAL(T, 'C'))) { bStringLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'c'))) { bStringLowercase = true;  Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '!'))) { bNeedToCase   = true; Fmt.RemovePrefix(1); }
				if (Fmt.StartsWith(LITERAL(T, '?'))) { bNeedToEscape = true; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);

				if (bNeedToEscape && Fmt.StartsWith(LITERAL(T, ':')))
				{
					Fmt.RemovePrefix(1);

					if      (Fmt.StartsWith(LITERAL(T, 'X'))) { bEscapeLowercase = false; Fmt.RemovePrefix(1); }
					else if (Fmt.StartsWith(LITERAL(T, 'x'))) { bEscapeLowercase = true;  Fmt.RemovePrefix(1); }

					if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);
				}

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				T Char = Object;

				if (bNeedToEscape) Result += LITERAL(T, '\'');

				if (bNeedToCase || bNeedToEscape)
				{
					if (bNeedToCase)
					{
						if (bStringLowercase) Char = TChar<T>::ToLower(Char);
						else                  Char = TChar<T>::ToUpper(Char);
					}

					if (bNeedToEscape)
					{
						switch (Char)
						{
						case LITERAL(T, '\''): Result += LITERAL(T, "\\\'"); break;
						case LITERAL(T, '\\'): Result += LITERAL(T, "\\\\"); break;
						case LITERAL(T, '\a'): Result += LITERAL(T, "\\a");  break;
						case LITERAL(T, '\b'): Result += LITERAL(T, "\\b");  break;
						case LITERAL(T, '\f'): Result += LITERAL(T, "\\f");  break;
						case LITERAL(T, '\n'): Result += LITERAL(T, "\\n");  break;
						case LITERAL(T, '\r'): Result += LITERAL(T, "\\r");  break;
						case LITERAL(T, '\t'): Result += LITERAL(T, "\\t");  break;
						case LITERAL(T, '\v'): Result += LITERAL(T, "\\v");  break;
						default:
							{
								if (!TChar<T>::IsASCII(Char) || !TChar<T>::IsPrint(Char))
								{
									Result += LITERAL(T, "\\x");

									const TMakeUnsigned<T> IntValue = static_cast<TMakeUnsigned<T>>(Char);

									struct { int DigitStyle; unsigned Padding; unsigned Base; } DigitParam = { bEscapeLowercase ? -1 : 1, sizeof(T) * 2, 16 };

									verify(TStringObjectFormatter::Do(Result, IntValue, DigitParam));
								}
								else Result += Char;
							}
						}
					}
					else Result += Char;
				}
				else Result += Char;

				if (bNeedToEscape) Result += LITERAL(T, '\'');

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the boolean value by format string.
			else if constexpr (CSameAs<U, bool>)
			{
				if (Fmt.IsEmpty()) return TStringObjectFormatter::Do(Result, Object, Invalid);

				if (Fmt.FindFirstOf(LITERAL(T, 'S')) != INDEX_NONE)
				{
					const TStringView<T> StringValue = Object ? LITERAL(T, "True") : LITERAL(T, "False");

					return TStringObjectFormatter::Do(Result, StringValue, Param);
				}

				if (Fmt.FindFirstOf(LITERAL(T, 's')) != INDEX_NONE)
				{
					const TStringView<T> StringValue = Object ? LITERAL(T, "true") : LITERAL(T, "false");

					return TStringObjectFormatter::Do(Result, StringValue, Param);
				}

				if (Fmt.FindFirstOf(LITERAL(T, 'C')) != INDEX_NONE)
				{
					const T CharacterValue = Object ? LITERAL(T, 'T') : LITERAL(T, 'F');

					return TStringObjectFormatter::Do(Result, CharacterValue, Param);
				}

				if (Fmt.FindFirstOf(LITERAL(T, 'c')) != INDEX_NONE)
				{
					const T CharacterValue = Object ? LITERAL(T, 't') : LITERAL(T, 'f');

					return TStringObjectFormatter::Do(Result, CharacterValue, Param);
				}

				if (Fmt.FindFirstOf(LITERAL(T, "BbDdOoXxIi")) != INDEX_NONE)
				{
					const int IntValue = Object ? 1 : 0;

					return TStringObjectFormatter::Do(Result, IntValue, Param);
				}

				checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
				return false;
			}

			// Format the integer value by format string.
			else if constexpr (CIntegral<U> && !CSameAs<U, bool>)
			{
				if (Fmt.IsEmpty()) return TStringObjectFormatter::Do(Result, Object, Invalid);

				auto FillAndAlign = ParseFillAndAlign();

				T PositiveIndicator = LITERAL(T, '-');

				bool bPrefix  = false;

				unsigned Padding = 0;

				bool bHasBase = false;

				unsigned Base = 10;

				bool bDigitLowercase = false;
				bool bOtherLowercase = true;

				if      (Fmt.StartsWith(LITERAL(T, '-'))) { PositiveIndicator = LITERAL(T, '-'); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, '+'))) { PositiveIndicator = LITERAL(T, '+'); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, ' '))) { PositiveIndicator = LITERAL(T, ' '); Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '#'))) { bPrefix = true; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '0')) && Fmt.Num() > 1 && TChar<T>::IsDigit(Fmt[1]) && Fmt[1] != LITERAL(T, '0'))
				{
					Fmt.RemovePrefix(1);

					TStringView<T> View = Fmt.Substr(0, Fmt.FindFirstNotOf(LITERAL(T, "0123456789")));

					Fmt.RemovePrefix(View.Num());

					Padding = View.template ToInt<uint>();
				}

				if (Fmt.StartsWith(LITERAL(T, '_')) && Fmt.Num() > 1 && TChar<T>::IsDigit(Fmt[1]))
				{
					Fmt.RemovePrefix(1);

					bHasBase = true;

					TStringView<T> View = Fmt.Substr(0, Fmt.FindFirstNotOf(LITERAL(T, "0123456789")));

					Fmt.RemovePrefix(View.Num());

					Base = View.template ToInt<uint>();
				}

				if      (             Fmt.StartsWith(LITERAL(T, 'I'))) {            bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (             Fmt.StartsWith(LITERAL(T, 'i'))) {            bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'D'))) { Base = 10; bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'd'))) { Base = 10; bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'B'))) { Base =  2; bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'b'))) { Base =  2; bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'O'))) { Base =  8; bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'o'))) { Base =  8; bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'X'))) { Base = 16; bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (!bHasBase && Fmt.StartsWith(LITERAL(T, 'x'))) { Base = 16; bDigitLowercase = true;  Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '!'))) { bOtherLowercase = false; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				struct { int DigitStyle; int OtherStyle; T PositiveSign; bool bPrefix; unsigned Padding; unsigned Base; } IntParam =
				{
					bDigitLowercase ? -1 : 1,
					bOtherLowercase ? -1 : 1,
					PositiveIndicator,
					bPrefix,
					Padding,
					Base == 0 ? 10 : Base,
				};

				verify(TStringObjectFormatter::Do(Result, Object, IntParam));

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the floating-point value by format string.
			else if constexpr (CFloatingPoint<U>)
			{
				if (Fmt.IsEmpty())
				{
					struct { bool bFixed; bool bScientific; unsigned Precision; } FloatParam = { true, false, 6 };

					return TStringObjectFormatter::Do(Result, Object, FloatParam);
				}

				auto FillAndAlign = ParseFillAndAlign();

				T PositiveIndicator = LITERAL(T, '-');

				bool bPrefix = false;

				int Precision = -1;

				bool bDigitLowercase = false;
				bool bOtherLowercase = true;

				bool bFixed      = true;
				bool bScientific = false;

				if      (Fmt.StartsWith(LITERAL(T, '-'))) { PositiveIndicator = LITERAL(T, '-'); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, '+'))) { PositiveIndicator = LITERAL(T, '+'); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, ' '))) { PositiveIndicator = LITERAL(T, ' '); Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '#'))) { bPrefix = true; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '.')) && Fmt.Num() > 1 && TChar<T>::IsDigit(Fmt[1]))
				{
					Fmt.RemovePrefix(1);

					TStringView<T> View = Fmt.Substr(0, Fmt.FindFirstNotOf(LITERAL(T, "0123456789")));

					Fmt.RemovePrefix(View.Num());

					Precision = View.template ToInt<uint>();
				}

				if      (Fmt.StartsWith(LITERAL(T, 'F'))) { bFixed = true; bScientific = false;  bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'f'))) { bFixed = true; bScientific = false;  bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'G'))) { bFixed = true;  bScientific = true;  bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'g'))) { bFixed = true;  bScientific = true;  bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'E'))) { bFixed = false; bScientific = true;  bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'e'))) { bFixed = false; bScientific = true;  bDigitLowercase = true;  Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'A'))) { bFixed = false; bScientific = false; bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'a'))) { bFixed = false; bScientific = false; bDigitLowercase = true;  Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '!'))) { bOtherLowercase = false; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				if (Precision == -1 && bFixed && !bScientific) Precision = 6;

				struct { bool bFixed; bool bScientific; int Precision; int DigitStyle; int OtherStyle; T PositiveSign; bool bPrefix; } FloatParam =
				{
					bFixed,
					bScientific,
					Precision,
					bDigitLowercase ? -1 : 1,
					bOtherLowercase ? -1 : 1,
					PositiveIndicator,
					bPrefix,
				};

				verify(TStringObjectFormatter::Do(Result, Object, FloatParam));

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the pointer value by format string.
			else if constexpr (CNullPointer<U> || TPointerTraits<U>::bIsPointer)
			{
				void* Ptr = nullptr; if constexpr (!CNullPointer<U>) Ptr = ToAddress(Object);

				auto FillAndAlign = ParseFillAndAlign();

				bool bDigitLowercase = false;
				bool bOtherLowercase = true;

				if      (Fmt.StartsWith(LITERAL(T, 'P'))) { bDigitLowercase = false; Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'p'))) { bDigitLowercase = true;  Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '!'))) { bOtherLowercase = false; Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, '='))) Fmt.RemovePrefix(1);

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				const uintptr IntValue = reinterpret_cast<uintptr>(Ptr);

				struct { int DigitStyle; int OtherStyle; bool bPrefix; unsigned Padding; unsigned Base; } IntParam =
				{
					bDigitLowercase ? -1 : 1,
					bOtherLowercase ? -1 : 1,
					true,
					sizeof(uintptr) * 2,
					16,
				};

				verify(TStringObjectFormatter::Do(Result, IntValue, IntParam));

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the tuple value by format string.
			else if constexpr (CTTuple<U>)
			{
				auto FillAndAlign = ParseFillAndAlign();

				TStringView<T> Begin     = LITERAL(T, "(");
				TStringView<T> Separator = LITERAL(T, ", ");
				TStringView<T> End       = LITERAL(T, ")");

				if (Fmt.StartsWith(LITERAL(T, 'T')) || Fmt.StartsWith(LITERAL(T, 't')))
				{
					Fmt.RemovePrefix(1);

					const size_t PlaceholderA = Fmt.FindFirstOf(LITERAL(T, '_'));
					const size_t PlaceholderB = Fmt.FindFirstOf(LITERAL(T, '_'), PlaceholderA + 1);

					if (PlaceholderA == INDEX_NONE || PlaceholderB == INDEX_NONE || PlaceholderA == PlaceholderB)
					{
						checkf(false, TEXT("Illegal format string. Expect placeholders."));
						return false;
					}

					size_t UserDefinedEnd = Fmt.FindFirstOf(LITERAL(T, ':'), PlaceholderB + 1);

					if (UserDefinedEnd == INDEX_NONE) UserDefinedEnd = Fmt.Num();

					Begin     = Fmt.First(PlaceholderA);
					Separator = Fmt.Substr(PlaceholderA + 1, PlaceholderB   - PlaceholderA - 1);
					End       = Fmt.Substr(PlaceholderB + 1, UserDefinedEnd - PlaceholderB - 1);

					Fmt.RemovePrefix(UserDefinedEnd);
				}
				else if (Fmt.StartsWith(LITERAL(T, 'M'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, ": "); End = LITERAL(T, ""); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'm'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, ": "); End = LITERAL(T, ""); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'N'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, "");   End = LITERAL(T, ""); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'n'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, "");   End = LITERAL(T, ""); Fmt.RemovePrefix(1); }

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				if (Object.IsEmpty())
				{
					Result += Begin;
					Result += End;

					ApplyFillAndAlign(FillAndAlign);

					return true;
				}

				TString<T, TInlineAllocator<64>> Buffer;

				struct { TStringView<T> Fmt; } Empty = { LITERAL(T, "") };

				bool bIsSuccessful = TStringObjectFormatter::Do(Buffer, Object.template GetValue<0>(), Empty);

				bIsSuccessful = [=, &Object, &Buffer]<size_t... Indices>(TIndexSequence<Indices...>) -> bool
				{
					return (bIsSuccessful && ... && (Buffer += Separator, TStringObjectFormatter::Do(Buffer, Object.template GetValue<Indices + 1>(), Empty)));
				}
				(TMakeIndexSequence<Object.Num() - 1>());

				if (!bIsSuccessful)
				{
					checkf(false, TEXT("Failed to fully format tuple value."));
					return false;
				}

				Result += Begin;
				Result += Buffer;
				Result += End;

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			// Format the container value by format string.
			else if constexpr (requires { Ranges::Begin(Object); Ranges::End(Object); })
			{
				auto FillAndAlign = ParseFillAndAlign();

				TStringView<T> Begin     = LITERAL(T, "[");
				TStringView<T> Separator = LITERAL(T, ", ");
				TStringView<T> End       = LITERAL(T, "]");

				TStringView<T> Subfmt = LITERAL(T, "");

				if (Fmt.StartsWith(LITERAL(T, 'T')) || Fmt.StartsWith(LITERAL(T, 't')))
				{
					Fmt.RemovePrefix(1);

					const size_t PlaceholderA = Fmt.FindFirstOf(LITERAL(T, '_'));
					const size_t PlaceholderB = Fmt.FindFirstOf(LITERAL(T, '_'), PlaceholderA + 1);

					if (PlaceholderA == INDEX_NONE || PlaceholderB == INDEX_NONE || PlaceholderA == PlaceholderB)
					{
						checkf(false, TEXT("Illegal format string. Expect placeholders."));
						return false;
					}

					size_t UserDefinedEnd = Fmt.FindFirstOf(LITERAL(T, ':'), PlaceholderB + 1);

					if (UserDefinedEnd == INDEX_NONE) UserDefinedEnd = Fmt.Num();

					Begin     = Fmt.First(PlaceholderA);
					Separator = Fmt.Substr(PlaceholderA + 1, PlaceholderB   - PlaceholderA - 1);
					End       = Fmt.Substr(PlaceholderB + 1, UserDefinedEnd - PlaceholderB - 1);

					Fmt.RemovePrefix(UserDefinedEnd);
				}
				else if (Fmt.StartsWith(LITERAL(T, 'N'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, ""); End = LITERAL(T, ""); Fmt.RemovePrefix(1); }
				else if (Fmt.StartsWith(LITERAL(T, 'n'))) { Begin = LITERAL(T, ""); Separator = LITERAL(T, ""); End = LITERAL(T, ""); Fmt.RemovePrefix(1); }

				if (Fmt.StartsWith(LITERAL(T, ':')))
				{
					Fmt.RemovePrefix(1);

					Subfmt = Fmt;

					Fmt = LITERAL(T, "");
				}

				if (!Fmt.IsEmpty())
				{
					checkf(false, TEXT("Illegal format string. Redundant unknown characters."));
					return false;
				}

				if (Ranges::Begin(Object) == Ranges::End(Object))
				{
					Result += Begin;
					Result += End;

					ApplyFillAndAlign(FillAndAlign);

					return true;
				}

				TString<T, TInlineAllocator<64>> Buffer;

				struct { TStringView<T> Fmt; } ElementParam = { Subfmt };

				// It is assumed that if the first element is successfully formatted, all elements will succeed.
				bool bIsSuccessful = TStringObjectFormatter::Do(Buffer, *Ranges::Begin(Object), ElementParam);

				if (!bIsSuccessful)
				{
					checkf(false, TEXT("Failed to fully format container value."));
					return false;
				}

				Result += Begin;
				Result += Buffer;

				auto Sentinel = Ranges::End(Object);

				for (auto Iter = ++Ranges::Begin(Object); Iter != Sentinel; ++Iter)
				{
					Result += Separator;

					verify(TStringObjectFormatter::Do(Result, *Iter, ElementParam));
				}

				Result += End;

				ApplyFillAndAlign(FillAndAlign);

				return true;
			}

			else static_assert(sizeof(U) == -1, "Unsupported object type.");
		}
		else
		{
			// Format the boolean value by structured parameters.
			if constexpr (CSameAs<U, bool>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<int>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<int>; };

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
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<int>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<int>; };

				constexpr bool bHasSign   = requires { { Param.PositiveSign } -> CConvertibleTo<T>; };

				constexpr bool bHasPrefix = requires { { Param.bPrefix } -> CBooleanTestable; };
				constexpr bool bHasBase   = requires { { Param.Base    } -> CConvertibleTo<unsigned>; };

				constexpr bool bHasPadding = requires { { Param.Padding } -> CConvertibleTo<unsigned>; };

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

				using FUnsignedU = TMakeUnsigned<U>;

				FUnsignedU Unsigned = static_cast<FUnsignedU>(Object);

				bool bNegative = false;

				if constexpr (CSigned<U>)
				{
					if (Object < 0)
					{
						bNegative = true;

						Unsigned = static_cast<FUnsignedU>(-Unsigned);
					}
				}

				constexpr size_t BufferSize = sizeof(FUnsignedU) * 8 + 4;

				T Buffer[BufferSize];

				T* DigitEnd = Buffer + BufferSize;

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
						case 10: do { *--Iter = static_cast<T>('0' + Unsigned % Param.Base);             Unsigned = static_cast<FUnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
						default: do { *--Iter =  TChar<T>::FromDigit(Unsigned % Param.Base, bLowercase); Unsigned = static_cast<FUnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
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
						case 10: do { *--Iter = static_cast<T>('0' + Unsigned % Param.Base); Unsigned = static_cast<FUnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
						default: do { *--Iter =  TChar<T>::FromDigit(Unsigned % Param.Base); Unsigned = static_cast<FUnsignedU>(Unsigned / Param.Base); } while (Unsigned != 0); break;
						}
					}
				}
				else do { *--Iter = static_cast<T>('0' + Unsigned % 10); Unsigned = static_cast<FUnsignedU>(Unsigned / 10); } while (Unsigned != 0);

				T* DigitBegin = Iter;

				// Handle the width parameter.
				if constexpr (bHasPadding) if (Param.Padding > DigitEnd - DigitBegin)
				{
					const size_t Padding = Param.Padding - (DigitEnd - DigitBegin);

					if (Param.Padding < sizeof(FUnsignedU) * 8) for (size_t Index = 0; Index != Padding; ++Index) *--Iter = LITERAL(T, '0');
				}

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
				if constexpr (bHasSign) if (!bNegative && Param.PositiveSign != LITERAL(T, '-')) *--Iter = Param.PositiveSign;

				// Handle the width parameter.
				if constexpr (bHasPadding) if (Param.Padding > DigitEnd - DigitBegin)
				{
					const size_t Padding = Param.Padding - (DigitEnd - DigitBegin);

					if (Param.Padding > sizeof(FUnsignedU) * 8)
					{
						Result.Reserve(Result.Num() + (DigitBegin - Iter) + Param.Padding);

						Result.Append(Iter, DigitBegin);

						for (size_t Index = 0; Index != Padding; ++Index) Result += LITERAL(T, '0');

						Result.Append(DigitBegin, DigitEnd);

						return true;
					}
				}

				Result.Append(Iter, DigitEnd);

				return true;
			}

			// Format the floating-point value by structured parameters.
			else if constexpr (CFloatingPoint<U>)
			{
				constexpr bool bHasDigitStyle = requires { { Param.DigitStyle } -> CConvertibleTo<int>; };
				constexpr bool bHasOtherStyle = requires { { Param.OtherStyle } -> CConvertibleTo<int>; };

				constexpr bool bHasSign = requires { { Param.PositiveSign } -> CConvertibleTo<T>; };

				constexpr bool bHasPrefix    = requires { { Param.bPrefix   } -> CBooleanTestable; };
				constexpr bool bHasPrecision = requires { { Param.Precision } -> CConvertibleTo<int>; };

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

					if constexpr (bHasPrecision)
					{
						if (Param.Precision >= 0) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object, Format, Param.Precision);
						else                      ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object, Format);
					}
					else if constexpr (bHasFormat) ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object, Format);
					else                           ConvertResult = NAMESPACE_STD::to_chars(ToAddress(Buffer.Begin()), ToAddress(Buffer.End()), Object);
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
					else if constexpr (bHasSign) if (Param.PositiveSign != LITERAL(T, '-')) Result += Param.PositiveSign;

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
					else if constexpr (bHasSign) if (Param.PositiveSign != LITERAL(T, '-')) Result += Param.PositiveSign;

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
				if constexpr (bHasSign) if (Param.PositiveSign != LITERAL(T, '-')) Result += Param.PositiveSign;

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

			else static_assert(sizeof(U) == -1, "Unsupported object type.");
		}

		checkf(false, TEXT("Unsupported type for formatting."));

		return false;
	}
};

template <CCharType T, bool bIsFormat>
struct TStringFormatOrParseHelper
{
	static constexpr T LeftBrace  = LITERAL(T, '{');
	static constexpr T RightBrace = LITERAL(T, '}');

	static inline const TStringView EscapeLeftBrace  = LITERAL(T, "[{");
	static inline const TStringView EscapeRightBrace = LITERAL(T, "}]");

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
								++SubplaceholderNum;
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
								++SubplaceholderNum;
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
							if (!PlaceholderIndex.template IsInteger<size_t>(10))
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
							else Index = PlaceholderIndex.template ToInt<size_t>();
						}
						else Index = ArgsIndex++;

						checkf(Index < ArgsTuple.Num(), TEXT("Argument not found."));

						bIsSuccessful = ArgsTuple.Visit(
							[&String, Subfmt = PlaceholderSubfmt](auto& Object) mutable
							{
								struct { TStringView<T> Fmt; } Param = { Subfmt };

								if constexpr (bIsFormat) return TStringObjectFormatter<T>::Do(String, Object, Param);

								else static_assert(bIsFormat, "Parsing is not supported.");
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
void TString<T, Allocator>::AppendFormat(TStringView<FElementType> Fmt, const Ts&... Args)
{
	// The Unreal Engine says that the starting buffer size catches 99.97% of printf calls.
	constexpr size_t ReserveBufferSize = 512;

	TString<T, TInlineAllocator<ReserveBufferSize>> Result;

	NAMESPACE_PRIVATE::TStringFormatOrParseHelper<FElementType, true>::Do(Result, Fmt, ForwardAsTuple(Args...));

	Append(Result.Begin(), Result.End());
}

template <CCharType T, CAllocator<T> Allocator>
void TString<T, Allocator>::AppendBool(bool Value)
{
	NAMESPACE_PRIVATE::TStringObjectFormatter<FElementType>::Do(*this, AsConst(Value), Invalid);
}

template <CCharType T, CAllocator<T> Allocator>
template <CIntegral U> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendInt(U Value, unsigned Base)
{
	checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

	struct { unsigned Base; } Param = { Base };

	NAMESPACE_PRIVATE::TStringObjectFormatter<FElementType>::Do(*this, AsConst(Value), Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value)
{
	NAMESPACE_PRIVATE::TStringObjectFormatter<FElementType>::Do(*this, AsConst(Value), Invalid);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific)
{
	struct { bool bFixed; bool bScientific; } Param = { bFixed, bScientific };

	NAMESPACE_PRIVATE::TStringObjectFormatter<FElementType>::Do(*this, AsConst(Value), Param);
}

template <CCharType T, CAllocator<T> Allocator> template <CFloatingPoint U> requires (!CConst<U> && !CVolatile<U>)
void TString<T, Allocator>::AppendFloat(U Value, bool bFixed, bool bScientific, unsigned Precision)
{
	struct { bool bFixed; bool bScientific; unsigned Precision; } Param = { bFixed, bScientific, Precision };

	NAMESPACE_PRIVATE::TStringObjectFormatter<FElementType>::Do(*this, AsConst(Value), Param);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
