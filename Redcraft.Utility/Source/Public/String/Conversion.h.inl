#pragma once

// NOTE: This file is not intended to be included directly, it is included by 'String/String.h'.

#include "Templates/Tuple.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

#include <cmath>
#include <limits>

#pragma warning(push)
#pragma warning(disable : 4146)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// NOTE: These functions are used to format an object to a string and parse a string to an object.
// If the user-defined overloads a function with the 'Fmt' parameter, fill-and-align needs to be handled.
// The formatting function should produce a string that can be parsed by the parsing function, if the parsing function exists.

// NOTE: These functions are recommended for debug programs.

#define LEFT_BRACE  LITERAL(T, '{')
#define RIGHT_BRACE LITERAL(T, '}')

#define ESCAPE_LEFT_BRACE  TStringView(LITERAL(T, "#{"))
#define ESCAPE_RIGHT_BRACE TStringView(LITERAL(T, "}#"))

NAMESPACE_PRIVATE_BEGIN

template <CCharType T, bool bIsFormat>
struct TStringHelper
{
	FORCEINLINE static bool FormatObject(auto& Result, TStringView<T> Fmt, auto& Object) requires (bIsFormat)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (!CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Unsafe formatting for a variable that is non-const."));

			return false;
		}

		return false;
	}

	FORCEINLINE static bool ParseObject(TStringView<T>& View, TStringView<T> Fmt, auto& Object) requires (!bIsFormat)
	{
		using U = TRemoveCVRef<decltype(Object)>;

		if constexpr (CConst<TRemoveReference<decltype(Object)>>)
		{
			checkf(false, TEXT("Cannot assign to a variable that is const."));

			return false;
		}

		return false;
	}

	FORCEINLINE static size_t Do(auto& Result, TStringView<T> Fmt, auto ArgsTuple)
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
							if (PlaceholderIndex.FindFirstNotOf(LITERAL(T, "0123456789")) != INDEX_NONE)
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

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
