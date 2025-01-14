#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Ranges/Utility.h"
#include "Ranges/View.h"
#include "Algorithms/Basic.h"
#include "Containers/StaticArray.h"
#include "Containers/Array.h"
#include "Numerics/Bit.h"
#include "Numerics/Math.h"
#include "Strings/Char.h"
#include "Miscellaneous/AssertionMacros.h"

#include <charconv>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** A concept specifies a type is a format description string context. */
template <typename T, typename CharType = char>
concept CFormatStringContext = CInputRange<T> && CCharType<CharType> && CSameAs<TRangeElement<T>, CharType>
	&& requires(T& Context, TRangeIterator<T> Iter, size_t Index)
	{
		/** Set the iterator of the context. */
		Context.AdvanceTo(MoveTemp(Iter));

		/** @return The next automatic index. */
		{ Context.GetNextIndex() } -> CSameAs<size_t>;

		/** @return true if the manual index is valid. */
		{ Context.CheckIndex(Index) } -> CBooleanTestable;
	};

/** A concept specifies a type is a format output context. */
template <typename T, typename CharType = char>
concept CFormatObjectContext = COutputRange<T, CharType> && CCharType<CharType>
	&& requires(T& Context, TRangeIterator<T> Iter, size_t Index)
	{
		/** Set the iterator of the context. */
		Context.AdvanceTo(MoveTemp(Iter));

		/** Visit the format argument by index, and the argument is always like const&. */
		{ Context.Visit([](const auto&) { return 0; }, Index) } -> CIntegral;

		/** Visit the format argument by index, and the argument is always like const&. */
		{ Context.template Visit<int>([](const auto&) { return 0; }, Index) } -> CIntegral;
	};

/**
 * A template class that defines the formatting rules for a specific type.
 *
 * @tparam T        - The type of object being formatted.
 * @tparam CharType - The character type of the formatting target.
 */
template <typename T, CCharType CharType = char> requires (CSameAs<TRemoveCVRef<T>, T>)
class TFormatter
{
public:

	static_assert(sizeof(T) == -1, "The type is not formattable.");

	FORCEINLINE constexpr TFormatter()                             = delete;
	FORCEINLINE constexpr TFormatter(const TFormatter&)            = delete;
	FORCEINLINE constexpr TFormatter(TFormatter&&)                 = delete;
	FORCEINLINE constexpr TFormatter& operator=(const TFormatter&) = delete;
	FORCEINLINE constexpr TFormatter& operator=(TFormatter&&)      = delete;

	/**
	 * Parses the format description string from the context.
	 * Assert that the format description string is valid.
	 *
	 * @return The iterator that points to the first unmatched character.
	 */
	template <CFormatStringContext<CharType> CTX>
	FORCEINLINE constexpr TRangeIterator<CTX> Parse(CTX& Context);

	/**
	 * Formats the object and writes the result to the context.
	 * Do not assert that the output range is always large enough, and return directly if it is insufficient.
	 * Specify, unlike visiting arguments from the context which is always like const&,
	 * the object argument is a forwarding reference.
	 *
	 * @return The iterator that points to the next position of the output.
	 */
	template <typename U, CFormatObjectContext<CharType> CTX> requires (CSameAs<TRemoveCVRef<T>, U>)
	FORCEINLINE constexpr TRangeIterator<CTX> Format(U&& Object, CTX& Context) const;

};

NAMESPACE_PRIVATE_BEGIN

template <typename I, typename S, typename... Ts>
class TFormatStringContext
{
public:

	FORCEINLINE constexpr TFormatStringContext(I InFirst, S InLast) : First(MoveTemp(InFirst)), Last(InLast), AutomaticIndex(0) { }

	NODISCARD FORCEINLINE constexpr I Begin()       requires (!CCopyable<I>) { return MoveTemp(First); }
	NODISCARD FORCEINLINE constexpr I Begin() const requires ( CCopyable<I>) { return          First;  }

	NODISCARD FORCEINLINE constexpr S End() const { return Last; }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedSentinelFor<S, I>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

	FORCEINLINE constexpr void AdvanceTo(I Iter) { First = MoveTemp(Iter); }

	NODISCARD FORCEINLINE constexpr size_t GetNextIndex()
	{
		bool bIsValid = AutomaticIndex < sizeof...(Ts) && AutomaticIndex != INDEX_NONE;

		checkf(bIsValid, TEXT("Illegal automatic indexing. Already entered manual indexing mode."));

		if (!bIsValid) return INDEX_NONE;

		return AutomaticIndex++;
	}

	NODISCARD FORCEINLINE constexpr bool CheckIndex(size_t Index)
	{
		bool bIsValid = AutomaticIndex == 0 || AutomaticIndex == INDEX_NONE;

		checkf(bIsValid, TEXT("Illegal manual indexing. Already entered automatic indexing mode."));

		if (!bIsValid) return false;

		AutomaticIndex = INDEX_NONE;

		return Index < sizeof...(Ts);
	}

private:

	NO_UNIQUE_ADDRESS I First;
	NO_UNIQUE_ADDRESS S Last;

	size_t AutomaticIndex = 0;
};

static_assert(CFormatStringContext<TFormatStringContext<IInputIterator<char>, ISentinelFor<IInputIterator<char>>>>);

template <typename I, typename S, typename... Ts>
class TFormatObjectContext
{
public:

	FORCEINLINE constexpr TFormatObjectContext(I InFirst, S InLast, Ts&... Args) : First(MoveTemp(InFirst)), Last(InLast), ArgsTuple(Args...) { }

	NODISCARD FORCEINLINE constexpr I Begin()       requires (!CCopyable<I>) { return MoveTemp(First); }
	NODISCARD FORCEINLINE constexpr I Begin() const requires ( CCopyable<I>) { return          First;  }

	NODISCARD FORCEINLINE constexpr S End() const { return Last; }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedSentinelFor<S, I>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

	FORCEINLINE constexpr void AdvanceTo(I Iter) { First = MoveTemp(Iter); }

	template <typename F> requires (((sizeof...(Ts) >= 1) && ... && CInvocable<F&&, Ts&>) && CCommonReference<TInvokeResult<F&&, const Ts&>...>)
	FORCEINLINE constexpr decltype(auto) Visit(F&& Func, size_t Index) const { return ArgsTuple.Visit(Forward<F>(Func), Index); }

	template <typename Ret, typename F> requires ((sizeof...(Ts) >= 1) && ... && CInvocableResult<Ret, F&&, const Ts&>)
	FORCEINLINE constexpr Ret Visit(F&& Func, size_t Index) const { return ArgsTuple.template Visit<Ret>(Forward<F>(Func), Index); }

private:

	NO_UNIQUE_ADDRESS I First;
	NO_UNIQUE_ADDRESS S Last;

	TTuple<Ts&...> ArgsTuple;
};

static_assert(CFormatObjectContext<TFormatObjectContext<IOutputIterator<char&>, ISentinelFor<IOutputIterator<char&>>, int>>);

NAMESPACE_PRIVATE_END

/** A concept specifies a type is formattable by the 'Algorithms::Format()'. */
template <typename T, typename CharType = char>
concept CFormattable = CCharType<CharType> && CSemiregular<TFormatter<TRemoveCVRef<T>, CharType>>
	&& requires(TFormatter<TRemoveCVRef<T>, CharType>& Formatter, T& Object,
		NAMESPACE_PRIVATE::TFormatStringContext< IInputIterator<CharType >, ISentinelFor< IInputIterator<CharType >>, T> FormatStringContext,
		NAMESPACE_PRIVATE::TFormatObjectContext<IOutputIterator<CharType&>, ISentinelFor<IOutputIterator<CharType&>>, T> FormatObjectContext)
	{
		{ Formatter.Parse (        FormatStringContext) } -> CSameAs< IInputIterator<CharType >>;
		{ Formatter.Format(Object, FormatObjectContext) } -> CSameAs<IOutputIterator<CharType&>>;
	};

NAMESPACE_BEGIN(Algorithms)

/**
 * Formats the objects and writes the result to the output range.
 * Assert that the format description string is valid.
 * If the output range is insufficient, return directly without asserting.
 *
 * @param Output - The output range to write the result.
 * @param Fmt    - The format description string.
 * @param Args   - The objects to format.
 *
 * @return The iterator that points to the next position of the output.
 */
template <CInputRange R1, COutputRange<TRangeElement<R1>> R2, CFormattable<TRangeElement<R1>>... Ts> requires (CBorrowedRange<R2>)
FORCEINLINE constexpr TRangeIterator<R2> Format(R2&& Output, R1&& Fmt, Ts&&... Args)
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Fmt) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Fmt)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Output) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Output)."));
	}

	using FCharType   = TRangeElement<R1>;
	using FCharTraits = TChar<FCharType>;

	using FFormatStringContext = NAMESPACE_PRIVATE::TFormatStringContext<TRangeIterator<R1>, TRangeSentinel<R1>, Ts...>;
	using FFormatObjectContext = NAMESPACE_PRIVATE::TFormatObjectContext<TRangeIterator<R2>, TRangeSentinel<R2>, Ts...>;

	FFormatStringContext FormatStringContext(Ranges::Begin(Fmt   ), Ranges::End(Fmt   ));
	FFormatObjectContext FormatObjectContext(Ranges::Begin(Output), Ranges::End(Output), Args...);

	auto FmtIter = Ranges::Begin(FormatStringContext);
	auto FmtSent = Ranges::End  (FormatStringContext);

	auto OutIter = Ranges::Begin(FormatObjectContext);
	auto OutSent = Ranges::End  (FormatObjectContext);

	// If the output range is insufficient.
	if (OutIter == OutSent) UNLIKELY return OutIter;

	TTuple<TFormatter<TRemoveCVRef<Ts>, FCharType>...> Formatters;

	// For each character in the format string.
	for (FCharType Char; FmtIter != FmtSent; ++FmtIter)
	{
		Char = *FmtIter;

		// If the character may be a replacement field.
		if (Char == LITERAL(FCharType, '{'))
		{
			if (++FmtIter == FmtSent) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. Unmatched '{' in format string."));

				break;
			}

			Char = *FmtIter;

			// If the character just an escaped '{'.
			if (Char == LITERAL(FCharType, '{'))
			{
				if (OutIter == OutSent) UNLIKELY return OutIter;

				*OutIter++ = LITERAL(FCharType, '{');

				continue;
			}

			// If available replacement fields.
			if constexpr (sizeof...(Ts) >= 1)
			{
				size_t Index;

				// If the replacement field has a manual index.
				if (Char != LITERAL(FCharType, ':') && Char != LITERAL(FCharType, '}'))
				{
					Index = 0;

					bool bIsValid = true;

					do
					{
						const uint Digit = FCharTraits::ToDigit(Char);

						if (Digit >= 10) bIsValid = false;

						Index = Index * 10 + Digit;

						if (++FmtIter == FmtSent) UNLIKELY break;

						Char = *FmtIter;
					}
					while (Char != LITERAL(FCharType, ':') && Char != LITERAL(FCharType, '}'));

					// If the index string contains illegal characters or the index is out of range.
					if (!bIsValid || !FormatStringContext.CheckIndex(Index)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the replacement field."));

						break;
					}
				}

				// If the replacement field need automatic indexing.
				else
				{
					Index = FormatStringContext.GetNextIndex();

					if (Index == INDEX_NONE)
					{
						checkf(false, TEXT("Illegal index. Please check the replacement field."));

						break;
					}
				}

				// Jump over the ':' character.
				if (Char == LITERAL(FCharType, ':')) ++FmtIter;

				FormatStringContext.AdvanceTo(MoveTemp(FmtIter));

				// Parse the format description string.
				FmtIter = Formatters.Visit([&FormatStringContext](auto& Formatter) -> decltype(FmtIter) { return Formatter.Parse(FormatStringContext); }, Index);

				if (FmtIter == FmtSent || *FmtIter != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					break;
				}

				FormatObjectContext.AdvanceTo(MoveTemp(OutIter));

				auto FormatHandler = [&]<size_t... Indices>(TIndexSequence<Indices...>)
				{
					TTuple<TConstant<size_t, Indices>...> Visitor;

					return Visitor.Visit([&]<size_t ConstantIndex>(TConstant<size_t, ConstantIndex>)
					{
						check(ConstantIndex == Index);

						return Formatters.template GetValue<ConstantIndex>().Format(ForwardAsTuple(Forward<Ts>(Args)...).template GetValue<ConstantIndex>(), FormatObjectContext);
					}
					, Index);
				};

				// Format the object and write the result to the context.
				OutIter = FormatHandler(TIndexSequenceFor<Ts...>());
			}

			else
			{
				checkf(false, TEXT("Illegal index. Please check the replacement field."));

				break;
			}
		}

		// If the character just an escaped '}'.
		else if (Char == LITERAL(FCharType, '}'))
		{
			// Confirm the character is an escaped '}'.
			if (++FmtIter != FmtSent && *FmtIter == LITERAL(FCharType, '}'))
			{
				if (OutIter == OutSent) UNLIKELY return OutIter;

				*OutIter++ = LITERAL(FCharType, '}');

				continue;
			}

			checkf(false, TEXT("Illegal format string. Missing '{' in format string."));

			break;
		}

		// If the output range is insufficient.
		else if (OutIter == OutSent) UNLIKELY return OutIter;

		// If the character is not a replacement field.
		else *OutIter++ = Char;
	}

	return OutIter;
}

/**
 * Formats the objects and writes the result to the output range.
 * Assert that the format description string is valid.
 * If the output range is insufficient, return directly without asserting.
 *
 * @param OutputFirst - The iterator of the output range to write the result.
 * @param OutputLast  - The sentinel of the output range to write the result.
 * @param FmtFirst    - The iterator of the format description string.
 * @param FmtLast     - The sentinel of the format description string.
 * @param Args        - The objects to format.
 *
 * @return The iterator that points to the next position of the output.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, COutputIterator<TIteratorElement<I1>> I2, CSentinelFor<I2> S2, CFormattable<TIteratorElement<I1>>... Ts>
FORCEINLINE constexpr I2 Format(I2 OutputFirst, S2 OutputLast, I1 FmtFirst, S1 FmtLast, Ts&&... Args)
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(FmtFirst - FmtLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(OutputFirst - OutputLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::Format(Ranges::View(MoveTemp(OutputFirst), OutputLast), Ranges::View(MoveTemp(FmtFirst), FmtLast), Forward<Ts>(Args)...);
}

NAMESPACE_END(Algorithms)

/**
 * A formatter for null-terminated string.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Width] [Precision] [Type] [!] [?]
 *
 * 1. The fill and align part:
 *
 *	[Fill Character] <Align Option>
 *
 *	i.   Fill Character: The character is used to fill width of the object. It is optional and cannot be '{' or '}'.
 *	                     It should be representable as a single unicode otherwise it is undefined behavior.
 *
 *  ii.  Align Option: The character is used to indicate the direction of alignment.
 *
 *		- '<': Align the formatted argument to the left of the available space
 *		       by inserting n fill characters after the formatted argument.
 *		       This is default option.
 *		- '^': Align the formatted argument to the center of the available space
 *		       by inserting n fill characters around the formatted argument.
 *		       If cannot absolute centering, offset to the left.
 *		- '>': Align the formatted argument ro the right of the available space
 *		       by inserting n fill characters before the formatted argument.
 *
 * 2. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integral argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 3. The precision part:
 *
 *	- '.N':   The number is used to specify the maximum field width of the object.
 *	          N should be an unsigned non-zero decimal number.
 *	- '.{N}': Dynamically determine the maximum field width of the object.
 *	          N should be a valid index of the format integral argument.
 *	          N is optional, and the default value is automatic indexing.
 *
 * 4. The type indicator part:
 *
 *	- none: Indicates the as-is formatting.
 *	- 'S':  Indicates the as-is formatting.
 *	- 's':  Indicates lowercase formatting.
 *
 * 5. The case indicators part:
 *
 *	- '!': Indicates capitalize the entire string.
 *
 * 6. The escape indicators part:
 *
 *	- '?': Indicates the escape formatting.
 *
 */
template <CCharType T>
class TFormatter<T*, T>
{
private:

	using FCharType      = T;
	using FCharTraits    = TChar<FCharType>;
	using FFillCharacter = TStaticArray<FCharType, FCharTraits::MaxCodeUnitLength>;

public:

	template <CFormatStringContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Parse(CTX& Context)
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		// Set the default values.
		{
			FillUnitLength   = 1;
			FillCharacter[0] = LITERAL(FCharType, ' ');
			AlignOption      = LITERAL(FCharType, '<');

			MinFieldWidth =  0;
			MaxFieldWidth = -1;

			bDynamicMin = false;
			bDynamicMax = false;

			bLowercase = false;
			bUppercase = false;
			bEscape    = false;
		}

		// If the format description string is empty.
		if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

		FCharType Char = *Iter; ++Iter;

		// Try to parse the fill and align part.
		// This code assumes that the format string does not contain multi-unit characters, except for fill character.

		// If the fill character is multi-unit.
		if (!FCharTraits::IsValid(Char))
		{
			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			while (true)
			{
				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;

				// If the fill character ends.
				if (FillUnitLength == FCharTraits::MaxCodeUnitLength || FCharTraits::IsValid(Char)) break;

				FillCharacter[FillUnitLength++] = Char;
			}

			if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. The fill character is not representable as a single unicode."));

				return Iter;
			}

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// If the fill character is single-unit.
		else do
		{
			if (Iter == Sent) break;

			// If the fill character is specified.
			if (*Iter == LITERAL(FCharType, '<') || *Iter == LITERAL(FCharType, '^') || *Iter == LITERAL(FCharType, '>'))
			{
				FillUnitLength   = 1;
				FillCharacter[0] = Char;

				Char = *Iter; ++Iter;
			}

			// If the fill character is not specified and the align option is not specified.
			else if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) break;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}
		while (false);

		// Try to parse the width part.
		{
			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicMin   = true;
				MinFieldWidth = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicMin || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				MinFieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicMin, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicMin && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					MinFieldWidth = MinFieldWidth * 10 + Digit;
				}
			}

			if (bDynamicMin)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (MinFieldWidth == INDEX_NONE)
					{
						MinFieldWidth = Context.GetNextIndex();

						if (MinFieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the field width."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(MinFieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the field width."));
					}

					else break;

					bDynamicMin   = false;
					MinFieldWidth = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the precision part.
		if (Char == LITERAL(FCharType, '.'))
		{
			if (Iter == Sent) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			Char = *Iter; ++Iter;

			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicMax   = true;
				MaxFieldWidth = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicMax || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				MaxFieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicMax, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicMax && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					MaxFieldWidth = MaxFieldWidth * 10 + Digit;
				}
			}

			else if (!bDynamicMax)
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			if (bDynamicMax)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (MaxFieldWidth == INDEX_NONE)
					{
						MaxFieldWidth = Context.GetNextIndex();

						if (MaxFieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the precision."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(MaxFieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the precision."));
					}

					else break;

					bDynamicMax   = false;
					MaxFieldWidth = -1;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the type indicators part.

		switch (Char)
		{
		case LITERAL(FCharType, 's'): bLowercase = true; break;
		default: { }
		}

		switch (Char)
		{
		case LITERAL(FCharType, 'S'):
		case LITERAL(FCharType, 's'): if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; break;
		default: { }
		}

		// Try to parse the case indicators part.
		if (Char == LITERAL(FCharType, '!'))
		{
			bUppercase = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// Try to parse the escape indicators part.
		if (Char == LITERAL(FCharType, '?'))
		{
			bEscape = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

		return Iter;
	}

	template <CFormatObjectContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Format(const FCharType* Object, CTX& Context) const
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		size_t MinDynamicField = MinFieldWidth;
		size_t MaxDynamicField = MaxFieldWidth;

		// Visit the dynamic width argument.
		if (bDynamicMin)
		{
			MinDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic width argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic width argument must be an integral."));

					return 0;
				}
			}
			, MinFieldWidth);
		}

		// Visit the dynamic precision argument.
		if (bDynamicMax)
		{
			MaxDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic precision argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic precision argument must be an integral."));

					return 0;
				}
			}
			, MaxFieldWidth);
		}

		size_t LeftPadding  = 0;
		size_t RightPadding = 0;

		// Estimate the field width.
		if (MinDynamicField != 0)
		{
			// If escape formatting is enabled, add quotes characters.
			size_t FieldWidth = bEscape ? 2 : 0;

			for (const FCharType* Ptr = Object; *Ptr != LITERAL(FCharType, '\0'); ++Ptr)
			{
				if (bEscape)
				{
					switch (const FCharType Char = *Ptr)
					{
					case LITERAL(FCharType, '\"'):
					case LITERAL(FCharType, '\\'):
					case LITERAL(FCharType, '\a'):
					case LITERAL(FCharType, '\b'):
					case LITERAL(FCharType, '\f'):
					case LITERAL(FCharType, '\n'):
					case LITERAL(FCharType, '\r'):
					case LITERAL(FCharType, '\t'):
					case LITERAL(FCharType, '\v'): FieldWidth += 2; break;
					default:
						{
							// Use '\x00' format for other non-printable characters.
							if (!FCharTraits::IsASCII(Char) || !FCharTraits::IsPrint(Char))
							{
								FieldWidth += 2 + sizeof(FCharType) * 2;
							}

							else ++FieldWidth;
						}
					}
				}

				else ++FieldWidth;
			}

			const size_t PaddingWidth = MinDynamicField - Math::Min(FieldWidth, MinDynamicField, MaxDynamicField);

			switch (AlignOption)
			{
			default:
			case LITERAL(FCharType, '<'): RightPadding = PaddingWidth; break;
			case LITERAL(FCharType, '>'): LeftPadding  = PaddingWidth; break;
			case LITERAL(FCharType, '^'):
				LeftPadding  = Math::DivAndFloor(PaddingWidth, 2);
				RightPadding = PaddingWidth - LeftPadding;
			}
		}

		// Write the left padding.
		for (size_t Index = 0; Index != LeftPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		// Write the left quote.
		if (bEscape)
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = LITERAL(FCharType, '\"');
		}

		const FCharType* Ptr = Object - 1;

		// Write the object, include escaped quotes in the counter.
		for (size_t Index = bEscape ? 1 : 0; Index != MaxDynamicField; ++Index)
		{
			FCharType Char = *++Ptr;

			if (Char == LITERAL(FCharType, '\0')) break;

			if (Iter == Sent) UNLIKELY return Iter;

			// Convert the character case.
			if (bLowercase) Char = FCharTraits::ToLower(Char);
			if (bUppercase) Char = FCharTraits::ToUpper(Char);

			if (bEscape)
			{
				switch (Char)
				{
				case LITERAL(FCharType, '\"'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, '\"'); break;
				case LITERAL(FCharType, '\\'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, '\\'); break;
				case LITERAL(FCharType, '\a'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'a');  break;
				case LITERAL(FCharType, '\b'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'b');  break;
				case LITERAL(FCharType, '\f'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'f');  break;
				case LITERAL(FCharType, '\n'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'n');  break;
				case LITERAL(FCharType, '\r'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'r');  break;
				case LITERAL(FCharType, '\t'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 't');  break;
				case LITERAL(FCharType, '\v'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'v');  break;
				default:
					{
						// Use '\x00' format for other non-printable characters.
						if (!FCharTraits::IsASCII(Char) || !FCharTraits::IsPrint(Char))
						{
							*Iter++ = LITERAL(FCharType, '\\');
							*Iter++ = LITERAL(FCharType, 'x' );

							using FUnsignedT = TMakeUnsigned<FCharType>;

							constexpr size_t DigitNum = sizeof(FCharType) * 2;

							FUnsignedT IntValue = static_cast<FUnsignedT>(Char);

							TStaticArray<FCharType, DigitNum> Buffer;

							for (size_t Jndex = 0; Jndex != DigitNum; ++Jndex)
							{
								Buffer[DigitNum - Jndex - 1] = FCharTraits::FromDigit(IntValue & 0xF);

								IntValue >>= 4;
							}

							check(IntValue == 0);

							for (size_t Jndex = 0; Jndex != DigitNum; ++Jndex)
							{
								if (Iter == Sent) UNLIKELY return Iter;

								*Iter++ = Buffer[Jndex];
							}
						}

						else *Iter++ = Char;
					}
				}
			}

			else *Iter++ = Char;
		}

		// Write the right quote, if the field width is enough.
		if (bEscape && *Ptr == LITERAL(FCharType, '\0'))
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = LITERAL(FCharType, '\"');
		}

		// Write the right padding.
		for (size_t Index = 0; Index != RightPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		return Iter;
	}

private:

	size_t         FillUnitLength = 1;
	FFillCharacter FillCharacter  = { LITERAL(FCharType, ' ') };
	FCharType      AlignOption    =   LITERAL(FCharType, '<');

	size_t MinFieldWidth =  0;
	size_t MaxFieldWidth = -1;

	bool bDynamicMin = false;
	bool bDynamicMax = false;

	bool bLowercase = false;
	bool bUppercase = false;
	bool bEscape    = false;

};

template <CCharType T>           class TFormatter<const T*  , T> : public TFormatter<T*, T> { };
template <CCharType T, size_t N> class TFormatter<      T[N], T> : public TFormatter<T*, T> { };
template <CCharType T, size_t N> class TFormatter<const T[N], T> : public TFormatter<T*, T> { };

static_assert(CFormattable<      char*>);
static_assert(CFormattable<const char*>);

static_assert(CFormattable<      char[256]>);
static_assert(CFormattable<const char[256]>);

/**
 * A formatter for integral like types.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Sign] [#] [0] [Width] [Base] [Type] [!] [?]
 *
 * 1. The fill and align part:
 *
 *	[Fill Character] <Align Option>
 *
 *	i.   Fill Character: The character is used to fill width of the object. It is optional and cannot be '{' or '}'.
 *	                     It should be representable as a single unicode otherwise it is undefined behavior.
 *
 *  ii.  Align Option: The character is used to indicate the direction of alignment.
 *
 *		- '<': Align the formatted argument to the left of the available space
 *		       by inserting n fill characters after the formatted argument.
 *		- '^': Align the formatted argument to the center of the available space
 *		       by inserting n fill characters around the formatted argument.
 *		       If cannot absolute centering, offset to the left.
 *		- '>': Align the formatted argument ro the right of the available space
 *		       by inserting n fill characters before the formatted argument.
 *		       This is default option.
 *
 * 2. The sign part:
 *
 *	This part is allowed only if the type indicator part is not 'C', 'c', 'S' or 's'.
 *
 *	- '+': Always include a sign character before the number. Use '+' for positive.
 *	- '-': Only include a sign character before the number if the number is negative. This is default option.
 *	- ' ': Always include a sign character before the number. Use ' ' for positive.
 *
 * 3. The alternate form indicator part:
 *
 *	This part is allowed only if the type indicator part is not 'C', 'c', 'S' or 's'.
 *
 *	- '#': Insert the prefix '0x' for hexadecimal numbers, '0' for octal numbers, '0b' for binary numbers.
 *
 * 4. The zero padding part:
 *
 *	This part is allowed only if the type indicator part is not 'C', 'c', 'S' or 's'.
 *
 *	- '0': By adding the prefix '0' to satisfy the minimum field width of the object.
 *	       if the object is normal number.
 *
 * 5. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integral argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 5. The base part:
 *
 *	This part is allowed only if the type indicator part is 'I' or 'i'.
 *
 *	- '_N':   The number is override the base of the number.
 *	          N should be an unsigned non-zero decimal number.
 *	- '_{N}': Dynamically override the base of the number.
 *	          N should be a valid index of the format integral argument.
 *	          N is optional, and the default value is automatic indexing.
 *
 * 7. The type indicator part:
 *
 *	- none: Same as 'D' if the object is integral type.
 *	        Same as 'C' if the object is target character type.
 *	        Same as 'S' if the object is boolean type.
 *	        Others are asserted failure.
 *
 *	- 'I': Indicates the uppercase integer formatting.
 *	- 'i': Indicates the lowercase integer formatting.
 *
 *	- 'B': Indicates the binary formatting.                Same as '_2I'.
 *	- 'b': Indicates the binary formatting.                Same as '_2I'.
 *	- 'O': Indicates the octal formatting.                 Same as '_8I'.
 *	- 'o': Indicates the octal formatting.                 Same as '_8I'.
 *	- 'D': Indicates the decimal formatting.               Same as '_10I'.
 *	- 'd': Indicates the decimal formatting.               Same as '_10I'.
 *	- 'X': Indicates the uppercase hexadecimal formatting. Same as '_16I'.
 *	- 'x': Indicates the lowercase hexadecimal formatting. Same as '_16I'.
 *
 *	If the object is not boolean type and is a valid character value.
 *
 *	- 'C': Indicates the the as-is character formatting.
 *	- 'c': Indicates the lowercase character formatting.
 *
 *	If the object is boolean type.
 *
 *	- 'C': Indicates the uppercase character formatting, as 'T' or 'F'.
 *	- 'c': Indicates the lowercase character formatting, as 't' or 'f'.
 *
 *	- 'S': Indicates the normal string formatting,    as 'True' or 'False'.
 *	- 's': Indicates the lowercase string formatting, as 'true' or "false'.
 *
 * 8. The case indicators part:
 *
 *	- '!': Indicates capitalize the entire string.
 *
 * 9. The escape indicators part:
 *
 *	This part is allowed only if the type indicator part is 'C', 'c', 'S' or 's'.
 *
 *	- '?': Indicates the escape formatting.
 *
 */
template <CIntegral T, CCharType CharType> requires (CSameAs<TRemoveCVRef<T>, T>)
class TFormatter<T, CharType>
{
private:

	using FCharType      = CharType;
	using FCharTraits    = TChar<FCharType>;
	using FFillCharacter = TStaticArray<FCharType, FCharTraits::MaxCodeUnitLength>;

public:

	template <CFormatStringContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Parse(CTX& Context)
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		// Set the default values.
		{
			FillUnitLength   = 1;
			FillCharacter[0] = LITERAL(FCharType, ' ');
			AlignOption      = CSameAs<T, FCharType> || CSameAs<T, bool> ? LITERAL(FCharType, '<') : LITERAL(FCharType, '>');

			SignOption = LITERAL(FCharType, '-');

			bAlternateForm = false;
			bZeroPadding   = false;

			FieldWidth   = 0;
			IntegralBase = 10;

			bDynamicWidth = false;
			bDynamicBase  = false;

			bCharacter = CSameAs<T, FCharType>;
			bString    = CSameAs<T, bool>;

			bLowercase = false;
			bUppercase = false;
			bEscape    = false;
		}

		// If the format description string is empty.
		if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

		FCharType Char = *Iter; ++Iter;

		// Flag indicates that the part is specified.
		bool bHasFillAndAlign = false;
		bool bHasSignOption   = false;
		bool bHasIntegralBase = false;

		// Try to parse the fill and align part.
		// This code assumes that the format string does not contain multi-unit characters, except for fill character.

		// If the fill character is multi-unit.
		if (!FCharTraits::IsValid(Char))
		{
			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			while (true)
			{
				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;

				// If the fill character ends.
				if (FillUnitLength == FCharTraits::MaxCodeUnitLength || FCharTraits::IsValid(Char)) break;

				FillCharacter[FillUnitLength++] = Char;
			}

			if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. The fill character is not representable as a single unicode."));

				return Iter;
			}

			bHasFillAndAlign = true;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// If the fill character is single-unit.
		else do
		{
			if (Iter == Sent) break;

			// If the fill character is specified.
			if (*Iter == LITERAL(FCharType, '<') || *Iter == LITERAL(FCharType, '^') || *Iter == LITERAL(FCharType, '>'))
			{
				FillUnitLength   = 1;
				FillCharacter[0] = Char;

				Char = *Iter; ++Iter;
			}

			// If the fill character is not specified and the align option is not specified.
			else if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) break;

			bHasFillAndAlign = true;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}
		while (false);

		// The flag indicates that the type indicator part defaults to 'D'.
		// Use to check the sign part, alternate form indicator part or zero padding part is allowed
		// when there is no type indicator part.
		constexpr bool bIntegral = !CSameAs<T, FCharType> && !CSameAs<T, bool>;

		// Try to parse the sign part.
		switch (Char)
		{
		case LITERAL(FCharType, '+'):
		case LITERAL(FCharType, '-'):
		case LITERAL(FCharType, ' '):

			bHasSignOption = true;

			SignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}'))
			{
				checkf(bIntegral, TEXT("Illegal format string. The sign option is not allowed for 'C', 'c', 'S' or 's' type."));

				return Iter;
			}

			Char = *Iter; ++Iter;

		default: { }
		}

		// Try to parse the alternate form indicator part.
		if (Char == LITERAL(FCharType, '#'))
		{
			bAlternateForm = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}'))
			{
				checkf(bIntegral, TEXT("Illegal format string. The alternate form is not allowed for 'C', 'c', 'S' or 's' type."));

				return Iter;
			}

			Char = *Iter; ++Iter;
		}

		// Try to parse the zero padding part.
		if (Char == LITERAL(FCharType, '0'))
		{
			bZeroPadding = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}'))
			{
				checkf(bIntegral, TEXT("Illegal format string. The zero padding is not allowed for 'C', 'c', 'S' or 's' type."));

				return Iter;
			}

			Char = *Iter; ++Iter;
		}

		// Try to parse the width part.
		{
			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicWidth = true;
				FieldWidth    = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicWidth || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				FieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicWidth, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicWidth && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					FieldWidth = FieldWidth * 10 + Digit;
				}
			}

			if (bDynamicWidth)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (FieldWidth == INDEX_NONE)
					{
						FieldWidth = Context.GetNextIndex();

						if (FieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the field width."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(FieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the field width."));
					}

					else break;

					bDynamicWidth = false;
					FieldWidth    = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the base part.
		if (Char == LITERAL(FCharType, '_'))
		{
			bHasIntegralBase = true;

			if (Iter == Sent) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. Missing base in format string."));

				return Iter;
			}

			Char = *Iter; ++Iter;

			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicBase = true;
				IntegralBase = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicBase || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				IntegralBase = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicBase, TEXT("Illegal format string. Missing '}' in format string."));

						checkf(false, TEXT("Illegal format string. Missing 'I' or 'i' in format string."));

						return Iter;
					}

					if (!bDynamicBase && *Iter == LITERAL(FCharType, '}'))
					{
						checkf(false, TEXT("Illegal format string. Missing 'I' or 'i' in format string."));

						return Iter;
					}

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					IntegralBase = IntegralBase * 10 + Digit;
				}
			}

			else if (!bDynamicBase)
			{
				checkf(false, TEXT("Illegal format string. Missing base in format string."));

				return Iter;
			}

			if (bDynamicBase)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (IntegralBase == INDEX_NONE)
					{
						IntegralBase = Context.GetNextIndex();

						if (IntegralBase == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the base."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(IntegralBase)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the base."));
					}

					else break;

					bDynamicBase = false;
					IntegralBase = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}'))
				{
					checkf(false, TEXT("Illegal format string. Missing 'I' or 'i' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the type indicators part.

		const bool bHasAlternateForm = bAlternateForm == true;
		const bool bHasZeroPadding   = bZeroPadding   == true;

		// If indicates this is lowercase.
		switch (Char)
		{
		case LITERAL(FCharType, 'i'):
		case LITERAL(FCharType, 'b'):
		case LITERAL(FCharType, 'o'):
		case LITERAL(FCharType, 'd'):
		case LITERAL(FCharType, 'x'):
		case LITERAL(FCharType, 'c'):
		case LITERAL(FCharType, 's'): bLowercase = true; break;
		default: { }
		}

		// If indicates this is variable base integer.
		switch (Char)
		{
		case LITERAL(FCharType, 'I'): case LITERAL(FCharType, 'i'):

			checkf( bHasIntegralBase, TEXT("Illegal format string. The base is required for 'I' or 'i' type."));

			break;

		default:
			checkf(!bHasIntegralBase, TEXT("Illegal format string. The base is only allowed for 'I' or 'i' type."));
		}

		// If indicates this is integral.
		switch (Char)
		{
		case LITERAL(FCharType, 'I'): case LITERAL(FCharType, 'i'):                                          break;
		case LITERAL(FCharType, 'B'): case LITERAL(FCharType, 'b'): IntegralBase = 2;  bDynamicBase = false; break;
		case LITERAL(FCharType, 'O'): case LITERAL(FCharType, 'o'): IntegralBase = 8;  bDynamicBase = false; break;
		case LITERAL(FCharType, 'D'): case LITERAL(FCharType, 'd'): IntegralBase = 10; bDynamicBase = false; break;
		case LITERAL(FCharType, 'X'): case LITERAL(FCharType, 'x'): IntegralBase = 16; bDynamicBase = false; break;
		default: if constexpr (bIntegral) break;
		case LITERAL(FCharType, 'C'): case LITERAL(FCharType, 'c'):
		case LITERAL(FCharType, 'S'): case LITERAL(FCharType, 's'):
			checkf(!bHasSignOption,    TEXT("Illegal format string. The sign option is not allowed for 'C', 'c', 'S' or 's' type."));
			checkf(!bHasAlternateForm, TEXT("Illegal format string. The alternate form is not allowed for 'C', 'c', 'S' or 's' type."));
			checkf(!bHasZeroPadding,   TEXT("Illegal format string. The zero padding is not allowed for 'C', 'c', 'S' or 's' type."));
		}

		// If indicates this is character or string.
		switch (Char)
		{
		case LITERAL(FCharType, 'I'): case LITERAL(FCharType, 'i'):
		case LITERAL(FCharType, 'B'): case LITERAL(FCharType, 'b'):
		case LITERAL(FCharType, 'O'): case LITERAL(FCharType, 'o'):
		case LITERAL(FCharType, 'D'): case LITERAL(FCharType, 'd'):
		case LITERAL(FCharType, 'X'): case LITERAL(FCharType, 'x'): bCharacter = false; bString = false; break;
		case LITERAL(FCharType, 'C'): case LITERAL(FCharType, 'c'): bCharacter = true;  bString = false; break;
		case LITERAL(FCharType, 'S'): case LITERAL(FCharType, 's'): bCharacter = false; bString = true;  break;
		default: { }
		}

		if (!bHasFillAndAlign) AlignOption = bCharacter || bString ? LITERAL(FCharType, '<') : LITERAL(FCharType, '>');

		checkf((!bString || CSameAs<T, bool>), TEXT("Illegal format string. The 'S' or 's' type is only allowed for boolean type."));

		// If exists the type indicators part.
		switch (Char)
		{
		case LITERAL(FCharType, 'I'): case LITERAL(FCharType, 'i'):
		case LITERAL(FCharType, 'B'): case LITERAL(FCharType, 'b'):
		case LITERAL(FCharType, 'O'): case LITERAL(FCharType, 'o'):
		case LITERAL(FCharType, 'D'): case LITERAL(FCharType, 'd'):
		case LITERAL(FCharType, 'X'): case LITERAL(FCharType, 'x'):
		case LITERAL(FCharType, 'C'): case LITERAL(FCharType, 'c'):
		case LITERAL(FCharType, 'S'): case LITERAL(FCharType, 's'): if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; break;
		default: { }
		}

		// Try to parse the case indicators part.
		if (Char == LITERAL(FCharType, '!'))
		{
			bUppercase = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// Try to parse the escape indicators part.
		if (Char == LITERAL(FCharType, '?') && (bCharacter || bString))
		{
			bEscape = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

		return Iter;
	}

	template <CFormatObjectContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Format(T Object, CTX& Context) const
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		size_t TargetField = FieldWidth;
		size_t TargetBase  = IntegralBase;

		// Visit the dynamic width argument.
		if (bDynamicWidth)
		{
			TargetField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic width argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic width argument must be an integral."));

					return 0;
				}
			}
			, FieldWidth);
		}

		// Visit the dynamic base argument.
		if (bDynamicBase)
		{
			TargetBase = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Math::IsWithinInclusive(Value, 2, 36), TEXT("Illegal format argument. The dynamic base argument must be in the range [2, 36]."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic base argument must be an integral."));

					return 0;
				}
			}
			, IntegralBase);
		}

		bool bNegative = false;

		bool bNormal = false;

		size_t TargetWidth;

		const FCharType* Target = nullptr;

		constexpr size_t BufferSize = sizeof(T) * 8;

		TStaticArray<FCharType, BufferSize> Buffer;

		do
		{
			// Handle the literal boolean type.
			if constexpr (CSameAs<T, bool>) if (bCharacter || bString)
			{
				TargetWidth = bCharacter ? 1 : Object ? 4 : 5;

				Target = Object ? LITERAL(FCharType, "True") : LITERAL(FCharType, "False");

				// Convert the character case.
				if (bLowercase) Target = Object ? LITERAL(FCharType, "true") : LITERAL(FCharType, "false");
				if (bUppercase) Target = Object ? LITERAL(FCharType, "TRUE") : LITERAL(FCharType, "FALSE");

				break;
			}

			// Handle the literal character type.
			if constexpr (!CSameAs<T, bool>) if (bCharacter)
			{
				TargetWidth = 1;

				FCharType Char = static_cast<FCharType>(Object);

				checkf(Char == Object, TEXT("Illegal format argument. The integral value is not a valid character."));

				// Convert the character case.
				if (bLowercase) Char = FCharTraits::ToLower(Char);
				if (bUppercase) Char = FCharTraits::ToUpper(Char);

				Buffer[0] = Char;

				Target = Buffer.GetData();

				break;
			}

			bNormal = true;

			// Handle the illegal base.
			if (!Math::IsWithinInclusive(TargetBase, 2, 36))
			{
				checkf(false, TEXT("Illegal format argument. The base must be in the range [2, 36]."));

				TargetBase = 10;
			}

			// Handle the integral boolean type.
			if constexpr (CSameAs<T, bool>)
			{
				TargetWidth = 1;

				Buffer[0] = Object ? LITERAL(FCharType, '1') : LITERAL(FCharType, '0');

				Target = Buffer.GetData();

				break;
			}

			// Handle the integral type.
			else
			{
				using FUnsignedT = TMakeUnsigned<T>;

				FUnsignedT Unsigned = static_cast<FUnsignedT>(Object);

				if constexpr (CSigned<T>)
				{
					if (Object < 0)
					{
						bNegative = true;

						Unsigned = static_cast<FUnsignedT>(-Object);
					}
				}

				FCharType* DigitIter = Buffer.GetData() + BufferSize;
				FCharType* DigitSent = Buffer.GetData() + BufferSize;

				switch (TargetBase)
				{
				case 0x02: do { *--DigitIter = static_cast<FCharType>('0' + (Unsigned & 0b00001));                            Unsigned >>= 1; } while (Unsigned != 0); break;
				case 0x04: do { *--DigitIter = static_cast<FCharType>('0' + (Unsigned & 0b00011));                            Unsigned >>= 2; } while (Unsigned != 0); break;
				case 0x08: do { *--DigitIter = static_cast<FCharType>('0' + (Unsigned & 0b00111));                            Unsigned >>= 3; } while (Unsigned != 0); break;
				case 0x10: do { *--DigitIter =        FCharTraits::FromDigit(Unsigned & 0b01111, bLowercase && !bUppercase);  Unsigned >>= 4; } while (Unsigned != 0); break;
				case 0X20: do { *--DigitIter =        FCharTraits::FromDigit(Unsigned & 0b11111, bLowercase && !bUppercase);  Unsigned >>= 5; } while (Unsigned != 0); break;

				case 3:
				case 5:
				case 6:
				case 7:
				case 9:
				case 10: do { *--DigitIter = static_cast<FCharType>('0' + Unsigned % TargetBase);                            Unsigned = static_cast<FUnsignedT>(Unsigned / TargetBase); } while (Unsigned != 0); break;
				default: do { *--DigitIter =       FCharTraits::FromDigit(Unsigned % TargetBase, bLowercase && !bUppercase); Unsigned = static_cast<FUnsignedT>(Unsigned / TargetBase); } while (Unsigned != 0); break;
				}

				TargetWidth = DigitSent - DigitIter;

				Target = DigitIter;

				break;
			}
		}
		while (false);

		size_t ZeroPadding  = 0;
		size_t LeftPadding  = 0;
		size_t RightPadding = 0;

		// Estimate the field width.
		if (TargetField != 0)
		{
			size_t LiteralWidth = TargetWidth;

			// Handle the escape option.
			if (bEscape) LiteralWidth += 2;

			// Handle the sign option.
			switch (SignOption)
			{
			case LITERAL(FCharType, '+'):
			case LITERAL(FCharType, ' '): LiteralWidth += 1; break;
			default: if (bNegative)       LiteralWidth += 1;
			}

			// Handle the alternate form.
			if (bAlternateForm) switch (TargetBase)
			{
			case 0x02: LiteralWidth += 2; break;
			case 0x08: LiteralWidth += 1; break;
			case 0x10: LiteralWidth += 2; break;
			default: { }
			}

			const size_t PaddingWidth = TargetField - Math::Min(LiteralWidth, TargetField);

			if (!bZeroPadding || !bNormal)
			{
				switch (AlignOption)
				{
				case LITERAL(FCharType, '<'): RightPadding = PaddingWidth; break;
				case LITERAL(FCharType, '>'): LeftPadding  = PaddingWidth; break;
				case LITERAL(FCharType, '^'):
					LeftPadding  = Math::DivAndFloor(PaddingWidth, 2);
					RightPadding = PaddingWidth - LeftPadding;
					break;
				default: check_no_entry();
				}
			}
			else ZeroPadding = PaddingWidth;
		}

		// Write the left padding.
		for (size_t Index = 0; Index != LeftPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		// Write the left quote.
		if (bEscape)
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = bCharacter ? LITERAL(FCharType, '\'') : LITERAL(FCharType, '\"');
		}

		// Write the object.
		{
			if (Iter == Sent) UNLIKELY return Iter;

			// Handle the sign option.
			switch (SignOption)
			{
			case LITERAL(FCharType, '+'): *Iter++ = bNegative ? LITERAL(FCharType, '-') : LITERAL(FCharType, '+'); break;
			case LITERAL(FCharType, ' '): *Iter++ = bNegative ? LITERAL(FCharType, '-') : LITERAL(FCharType, ' '); break;
			default: if (bNegative)       *Iter++ =             LITERAL(FCharType, '-');
			}

			// Handle the alternate form.
			if (bAlternateForm)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				switch (TargetBase)
				{
				case 0x02:
				case 0x08:
				case 0x10: *Iter++ = LITERAL(FCharType, '0'); break;
				default: { }
				}

				if (Iter == Sent) UNLIKELY return Iter;

				switch (TargetBase)
				{
				case 0x02: *Iter++ = bUppercase ? LITERAL(FCharType, 'B') : LITERAL(FCharType, 'b'); break;
				case 0x10: *Iter++ = bUppercase ? LITERAL(FCharType, 'X') : LITERAL(FCharType, 'x'); break;
				default: { }
				}
			}

			// Handle the zero padding.
			for (size_t Index = 0; Index != ZeroPadding; ++Index)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = LITERAL(FCharType, '0');
			}

			// Write the target object.
			for (size_t Index = 0; Index != TargetWidth; ++Index)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = Target[Index];
			}
		}

		// Write the right quote, if the field width is enough.
		if (bEscape)
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = bCharacter ? LITERAL(FCharType, '\'') : LITERAL(FCharType, '\"');
		}

		// Write the right padding.
		for (size_t Index = 0; Index != RightPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		return Iter;
	}

private:

	size_t         FillUnitLength = 1;
	FFillCharacter FillCharacter  = { LITERAL(FCharType, ' ') };
	FCharType      AlignOption    = CSameAs<T, FCharType> || CSameAs<T, bool> ? LITERAL(FCharType, '<') : LITERAL(FCharType, '>');

	FCharType SignOption = LITERAL(FCharType, '-');

	bool bAlternateForm = false;
	bool bZeroPadding   = false;

	size_t FieldWidth   = 0;
	size_t IntegralBase = 10;

	bool bDynamicWidth = false;
	bool bDynamicBase  = false;

	bool bCharacter = CSameAs<T, FCharType>;
	bool bString    = CSameAs<T, bool>;

	bool bLowercase = false;
	bool bUppercase = false;
	bool bEscape    = false;

};

static_assert(CFormattable<int >);
static_assert(CFormattable<char>);
static_assert(CFormattable<bool>);

/**
 * A formatter for the floating-point types.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Sign] [#] [0] [Width] [Precision] [Type] [!]
 *
 * 1. The fill and align part:
 *
 *	[Fill Character] <Align Option>
 *
 *	i.   Fill Character: The character is used to fill width of the object. It is optional and cannot be '{' or '}'.
 *	                     It should be representable as a single unicode otherwise it is undefined behavior.
 *
 *  ii.  Align Option: The character is used to indicate the direction of alignment.
 *
 *		- '<': Align the formatted argument to the left of the available space
 *		       by inserting n fill characters after the formatted argument.
 *		- '^': Align the formatted argument to the center of the available space
 *		       by inserting n fill characters around the formatted argument.
 *		       If cannot absolute centering, offset to the left.
 *		- '>': Align the formatted argument ro the right of the available space
 *		       by inserting n fill characters before the formatted argument.
 *		       This is default option.
 *
 * 2. The sign part:
 *
 *	- '+': Always include a sign character before the number. Use '+' for positive.
 *	- '-': Only include a sign character before the number if the number is negative. This is default option.
 *	- ' ': Always include a sign character before the number. Use ' ' for positive.
 *
 * 3. The alternate form indicator part:
 *
 *	- '#': Insert the decimal point character unconditionally,
 *	       and do not remove trailing zeros if the type indicator part is 'G' or 'g'.
 *
 * 4. The zero padding part:
 *
 *	- '0': By adding the prefix '0' to satisfy the minimum field width of the object.
 *	       if the object is normal number.
 *
 * 5. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integral argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 6. The precision part:
 *
 *	- '.N':   The number is used to specify the precision of the floating-point number.
 *	          N should be an unsigned non-zero decimal number.
 *	- '.{N}': Dynamically determine the precision of the floating-point number.
 *	          N should be a valid index of the format integral argument.
 *	          N is optional, and the default value is automatic indexing.
 *
 * 7. The type indicator part:
 *
 *	- none: Indicates the normal formatting.
 *	- 'G':  Indicates the general formatting.
 *	- 'g':  Indicates the general formatting.
 *	- 'F':  Indicates the fixed-point formatting.
 *	- 'f':  Indicates the fixed-point formatting.
 *	- 'E':  Indicates the scientific formatting.
 *	- 'e':  Indicates the scientific formatting.
 *	- 'A':  Indicates the uppercase hexadecimal formatting.
 *	- 'a':  Indicates the lowercase hexadecimal formatting.
 *
 * 8. The case indicators part:
 *
 *	- '!': Indicates capitalize the entire string.
 *
 */
template <CFloatingPoint T, CCharType CharType> requires (CSameAs<TRemoveCVRef<T>, T>)
class TFormatter<T, CharType>
{
private:

	using FCharType      = CharType;
	using FCharTraits    = TChar<FCharType>;
	using FFillCharacter = TStaticArray<FCharType, FCharTraits::MaxCodeUnitLength>;

public:

	template <CFormatStringContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Parse(CTX& Context)
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		// Set the default values.
		{
			FillUnitLength = 1;
			FillCharacter  = { LITERAL(FCharType, ' ') };
			AlignOption    =   LITERAL(FCharType, '>');

			SignOption = LITERAL(FCharType, '-');

			bAlternateForm = false;
			bZeroPadding   = false;

			bHasPrecision = false;

			FieldWidth = 0;
			Precision  = 0;

			bDynamicWidth     = false;
			bDynamicPrecision = false;

			bGeneral     = false;
			bFixedPoint  = false;
			bScientific  = false;
			bHexadecimal = false;

			bLowercase = false;
			bUppercase = false;
		}

		// If the format description string is empty.
		if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

		FCharType Char = *Iter; ++Iter;

		// Flag indicates that the part is specified.
		bool bHasFillAndAlign = false;

		// Try to parse the fill and align part.
		// This code assumes that the format string does not contain multi-unit characters, except for fill character.

		// If the fill character is multi-unit.
		if (!FCharTraits::IsValid(Char))
		{
			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			while (true)
			{
				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;

				// If the fill character ends.
				if (FillUnitLength == FCharTraits::MaxCodeUnitLength || FCharTraits::IsValid(Char)) break;

				FillCharacter[FillUnitLength++] = Char;
			}

			if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. The fill character is not representable as a single unicode."));

				return Iter;
			}

			bHasFillAndAlign = true;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// If the fill character is single-unit.
		else do
		{
			if (Iter == Sent) break;

			// If the fill character is specified.
			if (*Iter == LITERAL(FCharType, '<') || *Iter == LITERAL(FCharType, '^') || *Iter == LITERAL(FCharType, '>'))
			{
				FillUnitLength   = 1;
				FillCharacter[0] = Char;

				Char = *Iter; ++Iter;
			}

			// If the fill character is not specified and the align option is not specified.
			else if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) break;

			bHasFillAndAlign = true;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}
		while (false);

		// Try to parse the sign part.
		switch (Char)
		{
		case LITERAL(FCharType, '+'):
		case LITERAL(FCharType, '-'):
		case LITERAL(FCharType, ' '):

			SignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;

		default: { }
		}

		// Try to parse the alternate form indicator part.
		if (Char == LITERAL(FCharType, '#'))
		{
			bAlternateForm = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// Try to parse the zero padding part.
		if (Char == LITERAL(FCharType, '0'))
		{
			bZeroPadding = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// Try to parse the width part.
		{
			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicWidth = true;
				FieldWidth    = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicWidth || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				FieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicWidth, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicWidth && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					FieldWidth = FieldWidth * 10 + Digit;
				}
			}

			if (bDynamicWidth)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (FieldWidth == INDEX_NONE)
					{
						FieldWidth = Context.GetNextIndex();

						if (FieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the field width."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(FieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the field width."));
					}

					else break;

					bDynamicWidth = false;
					FieldWidth    = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the precision part.
		if (Char == LITERAL(FCharType, '.'))
		{
			bHasPrecision = true;

			if (Iter == Sent) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			Char = *Iter; ++Iter;

			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicPrecision = true;
				Precision         = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicPrecision || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				Precision = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicPrecision, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicPrecision && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					Precision = Precision * 10 + Digit;
				}
			}

			else if (!bDynamicPrecision)
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			if (bDynamicPrecision)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (Precision == INDEX_NONE)
					{
						Precision = Context.GetNextIndex();

						if (Precision == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the precision."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(Precision)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the precision."));
					}

					else break;

					bDynamicPrecision = false;
					Precision         = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the type indicators part.

		// If indicates this is lowercase.
		switch (Char)
		{
		case LITERAL(FCharType, 'g'):
		case LITERAL(FCharType, 'f'):
		case LITERAL(FCharType, 'e'):
		case LITERAL(FCharType, 'a'): bLowercase = true; break;
		default: { }
		}

		// If indicates this is not normal formatting.
		switch (Char)
		{
		case LITERAL(FCharType, 'G'): case LITERAL(FCharType, 'g'): bGeneral     = true; break;
		case LITERAL(FCharType, 'F'): case LITERAL(FCharType, 'f'): bFixedPoint  = true; break;
		case LITERAL(FCharType, 'E'): case LITERAL(FCharType, 'e'): bScientific  = true; break;
		case LITERAL(FCharType, 'A'): case LITERAL(FCharType, 'a'): bHexadecimal = true; break;
		default: { }
		}

		// If exists the type indicators part.
		switch (Char)
		{
		case LITERAL(FCharType, 'G'): case LITERAL(FCharType, 'g'):
		case LITERAL(FCharType, 'F'): case LITERAL(FCharType, 'f'):
		case LITERAL(FCharType, 'E'): case LITERAL(FCharType, 'e'):
		case LITERAL(FCharType, 'A'): case LITERAL(FCharType, 'a'): if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; break;
		default: { }
		}

		// Try to parse the case indicators part.
		if (Char == LITERAL(FCharType, '!'))
		{
			bUppercase = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

		return Iter;
	}

	template <CFormatObjectContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Format(T Object, CTX& Context) const
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		size_t TargetField     = FieldWidth;
		size_t TargetPrecision = Precision;

		// Visit the dynamic width argument.
		if (bDynamicWidth)
		{
			TargetField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic width argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic width argument must be an integral."));

					return 0;
				}
			}
			, FieldWidth);
		}

		// Visit the dynamic precision argument.
		if (bDynamicPrecision)
		{
			TargetPrecision = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value >= 0, TEXT("Illegal format argument. The dynamic precision argument must be a unsigned number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic precision argument must be an integral."));

					return 0;
				}
			}
			, Precision);
		}

		const bool bNegative = Math::IsNegative(Object);

		bool bNormal = false;

		size_t TargetWidth;

		const char* Target = nullptr;

		constexpr size_t StartingBufferSize = 64;

		TArray<char, TInlineAllocator<StartingBufferSize>> Buffer(StartingBufferSize);

		// Handle the infinite value.
		if (Math::IsInfinity(Object))
		{
			TargetWidth = 8;

			Target = TEXT("Infinity");

			// Convert the character case.
			if (bLowercase) Target = TEXT("infinity");
			if (bUppercase) Target = TEXT("INFINITY");
		}

		// Handle the NaN value.
		else if (Math::IsNaN(Object))
		{
			TargetWidth = 3;

			Target = TEXT("NaN");

			// Convert the character case.
			if (bLowercase) Target = TEXT("nan");
			if (bUppercase) Target = TEXT("NAN");
		}

		// Handle the normal value.
		else
		{
			bNormal = true;

			NAMESPACE_STD::to_chars_result ConvertResult;

			while (true)
			{
				if (bHasPrecision)
				{
					check(static_cast<int>(TargetPrecision) >= 0);

					if      (bGeneral    ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::general   , static_cast<int>(TargetPrecision));
					else if (bFixedPoint ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::fixed     , static_cast<int>(TargetPrecision));
					else if (bScientific ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::scientific, static_cast<int>(TargetPrecision));
					else if (bHexadecimal) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::hex       , static_cast<int>(TargetPrecision));
					else                   ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::general   , static_cast<int>(TargetPrecision));
				}

				else
				{
					if      (bGeneral    ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::general   );
					else if (bFixedPoint ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::fixed     );
					else if (bScientific ) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::scientific);
					else if (bHexadecimal) ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object, NAMESPACE_STD::chars_format::hex       );
					else                   ConvertResult = NAMESPACE_STD::to_chars(Buffer.GetData(), Buffer.GetData() + Buffer.Num(), Object);
				}

				if (ConvertResult.ec != NAMESPACE_STD::errc::value_too_large) break;

				Buffer.SetNum(Buffer.Num() * 2);
			}

			Buffer.SetNum(ConvertResult.ptr - Buffer.GetData());

			// Remove the negative sign.
			if (Buffer.Front() == TEXT('-')) Buffer.StableErase(Buffer.Begin());

			// Handle the alternate form.
			if (bAlternateForm)
			{
				const char ExponentChar = bHexadecimal ? TEXT('p') : TEXT('e');

				auto BufferIter = Buffer.Begin();

				// Insert the decimal point character.
				while (true)
				{
					if (BufferIter == Buffer.End())
					{
						Buffer.PushBack(TEXT('.'));

						BufferIter = Algorithms::Prev(Buffer.End());

						break;
					}

					if (*BufferIter == ExponentChar)
					{
						BufferIter = Buffer.Insert(BufferIter, TEXT('.'));

						break;
					}

					if (*BufferIter == TEXT('.')) break;

					++BufferIter;
				}

				// Restore trailing zeros.
				if (bGeneral)
				{
					if (!bHasPrecision) TargetPrecision = 6;

					size_t DigitNum = BufferIter - Buffer.Begin();

					++BufferIter;

					while (true)
					{
						if (DigitNum >= TargetPrecision) break;

						if (BufferIter == Buffer.End())
						{
							Buffer.SetNum(Buffer.Num() + TargetPrecision - DigitNum, TEXT('0'));

							break;
						}

						if (*BufferIter == ExponentChar)
						{
							Buffer.Insert(BufferIter, TargetPrecision - DigitNum, TEXT('0'));

							break;
						}

						++BufferIter;
						++DigitNum;
					}
				}
			}

			// Convert the character case.
			if (!bLowercase || bUppercase) for (char& Char : Buffer)
			{
				// Convert the exponent character.
				if      ( bHexadecimal && Char == TEXT('p')) Char = bUppercase ? TEXT('P') : TEXT('p');
				else if (!bHexadecimal && Char == TEXT('e')) Char = bUppercase ? TEXT('E') : TEXT('e');

				// Convert the digit character.
				else if (!bLowercase) Char = FChar::ToUpper(Char);
			}

			TargetWidth = Buffer.Num();

			Target = Buffer.GetData();
		}

		size_t ZeroPadding  = 0;
		size_t LeftPadding  = 0;
		size_t RightPadding = 0;

		// Estimate the field width.
		if (TargetField != 0)
		{
			size_t LiteralWidth = TargetWidth;

			// Handle the sign option.
			switch (SignOption)
			{
			case LITERAL(FCharType, '+'):
			case LITERAL(FCharType, ' '): LiteralWidth += 1; break;
			default: if (bNegative)       LiteralWidth += 1;
			}

			const size_t PaddingWidth = TargetField - Math::Min(LiteralWidth, TargetField);

			if (!bZeroPadding || !bNormal)
			{
				switch (AlignOption)
				{
				case LITERAL(FCharType, '<'): RightPadding = PaddingWidth; break;
				case LITERAL(FCharType, '>'): LeftPadding  = PaddingWidth; break;
				case LITERAL(FCharType, '^'):
					LeftPadding  = Math::DivAndFloor(PaddingWidth, 2);
					RightPadding = PaddingWidth - LeftPadding;
					break;
				default: check_no_entry();
				}
			}
			else ZeroPadding = PaddingWidth;
		}

		// Write the left padding.
		for (size_t Index = 0; Index != LeftPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		// Write the object.
		{
			static_assert(FChar::IsASCII() && FCharTraits::IsASCII());

			if (Iter == Sent) UNLIKELY return Iter;

			// Handle the sign option.
			switch (SignOption)
			{
			case LITERAL(FCharType, '+'): *Iter++ = bNegative ? LITERAL(FCharType, '-') : LITERAL(FCharType, '+'); break;
			case LITERAL(FCharType, ' '): *Iter++ = bNegative ? LITERAL(FCharType, '-') : LITERAL(FCharType, ' '); break;
			default: if (bNegative)       *Iter++ =             LITERAL(FCharType, '-');
			}

			// Handle the zero padding.
			for (size_t Index = 0; Index != ZeroPadding; ++Index)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = LITERAL(FCharType, '0');
			}

			// Write the target object.
			for (size_t Index = 0; Index != TargetWidth; ++Index)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = static_cast<FCharType>(Target[Index]);
			}
		}

		// Write the right padding.
		for (size_t Index = 0; Index != RightPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		return Iter;
	}

private:

	size_t         FillUnitLength = 1;
	FFillCharacter FillCharacter  = { LITERAL(FCharType, ' ') };
	FCharType      AlignOption    =   LITERAL(FCharType, '>');

	FCharType SignOption = LITERAL(FCharType, '-');

	bool bAlternateForm = false;
	bool bZeroPadding   = false;

	bool bHasPrecision = false;

	size_t FieldWidth = 0;
	size_t Precision  = 0;

	bool bDynamicWidth     = false;
	bool bDynamicPrecision = false;

	bool bGeneral     = false;
	bool bFixedPoint  = false;
	bool bScientific  = false;
	bool bHexadecimal = false;

	bool bLowercase = false;
	bool bUppercase = false;

};

static_assert(CFormattable<float>);

/**
 * A formatter for pointer or member pointer types.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Width] [Type] [!]
 *
 * 1. The fill and align part:
 *
 *	[Fill Character] <Align Option>
 *
 *	i.   Fill Character: The character is used to fill width of the object. It is optional and cannot be '{' or '}'.
 *	                     It should be representable as a single unicode otherwise it is undefined behavior.
 *
 *  ii.  Align Option: The character is used to indicate the direction of alignment.
 *
 *		- '<': Align the formatted argument to the left of the available space
 *		       by inserting n fill characters after the formatted argument.
 *		       This is default option.
 *		- '^': Align the formatted argument to the center of the available space
 *		       by inserting n fill characters around the formatted argument.
 *		       If cannot absolute centering, offset to the left.
 *		- '>': Align the formatted argument ro the right of the available space
 *		       by inserting n fill characters before the formatted argument.
 *
 * 2. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integral argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 3. The type indicator part:
 *
 *	- none: Indicates the normal formatting.
 *	- 'P':  Indicates the normal formatting.
 *	- 'p':  Indicates lowercase formatting.
 *
 * 4. The case indicators part:
 *
 *	- '!': Indicates capitalize the entire string.
 *
 */
template <typename T, CCharType CharType> requires ((CNullPointer<T> || CPointer<T> || CMemberPointer<T>) && CSameAs<TRemoveCVRef<T>, T>)
class TFormatter<T, CharType>
{
private:

	using FCharType      = CharType;
	using FCharTraits    = TChar<FCharType>;
	using FFillCharacter = TStaticArray<FCharType, FCharTraits::MaxCodeUnitLength>;

public:

	template <CFormatStringContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Parse(CTX& Context)
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		// Set the default values.
		{
			FillUnitLength   = 1;
			FillCharacter[0] = LITERAL(FCharType, ' ');
			AlignOption      = LITERAL(FCharType, '>');

			FieldWidth = 0;

			bDynamicWidth = false;

			bLowercase = false;
			bUppercase = false;
		}

		// If the format description string is empty.
		if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

		FCharType Char = *Iter; ++Iter;

		// Try to parse the fill and align part.
		// This code assumes that the format string does not contain multi-unit characters, except for fill character.

		// If the fill character is multi-unit.
		if (!FCharTraits::IsValid(Char))
		{
			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			while (true)
			{
				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;

				// If the fill character ends.
				if (FillUnitLength == FCharTraits::MaxCodeUnitLength || FCharTraits::IsValid(Char)) break;

				FillCharacter[FillUnitLength++] = Char;
			}

			if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. The fill character is not representable as a single unicode."));

				return Iter;
			}

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// If the fill character is single-unit.
		else do
		{
			if (Iter == Sent) break;

			// If the fill character is specified.
			if (*Iter == LITERAL(FCharType, '<') || *Iter == LITERAL(FCharType, '^') || *Iter == LITERAL(FCharType, '>'))
			{
				FillUnitLength   = 1;
				FillCharacter[0] = Char;

				Char = *Iter; ++Iter;
			}

			// If the fill character is not specified and the align option is not specified.
			else if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) break;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}
		while (false);

		// Try to parse the width part.
		{
			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicWidth = true;
				FieldWidth    = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicWidth || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				FieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicWidth, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicWidth && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					FieldWidth = FieldWidth * 10 + Digit;
				}
			}

			if (bDynamicWidth)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (FieldWidth == INDEX_NONE)
					{
						FieldWidth = Context.GetNextIndex();

						if (FieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the field width."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(FieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the field width."));
					}

					else break;

					bDynamicWidth = false;
					FieldWidth    = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the type indicators part.

		switch (Char)
		{
		case LITERAL(FCharType, 'p'): bLowercase = true; break;
		default: { }
		}

		switch (Char)
		{
		case LITERAL(FCharType, 'P'):
		case LITERAL(FCharType, 'p'): if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; break;
		default: { }
		}

		// Try to parse the case indicators part.
		if (Char == LITERAL(FCharType, '!'))
		{
			bUppercase = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

		return Iter;
	}

	template <CFormatObjectContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Format(T Object, CTX& Context) const
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		size_t TargetField = FieldWidth;

		// Visit the dynamic width argument.
		if (bDynamicWidth)
		{
			TargetField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic width argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic width argument must be an integral."));

					return 0;
				}
			}
			, FieldWidth);
		}

		size_t LeftPadding  = 0;
		size_t RightPadding = 0;

		// Estimate the field width.
		if (TargetField != 0)
		{
			const size_t LiteralWidth = 2 * sizeof(T) + 2;

			const size_t PaddingWidth = TargetField - Math::Min(LiteralWidth, TargetField);

			switch (AlignOption)
			{
			case LITERAL(FCharType, '<'): RightPadding = PaddingWidth; break;
			case LITERAL(FCharType, '>'): LeftPadding  = PaddingWidth; break;
			case LITERAL(FCharType, '^'):
				LeftPadding  = Math::DivAndFloor(PaddingWidth, 2);
				RightPadding = PaddingWidth - LeftPadding;
				break;
			default: check_no_entry();
			}
		}

		// Write the left padding.
		for (size_t Index = 0; Index != LeftPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		// Write the object, include escaped quotes in the counter.
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = LITERAL(FCharType, '0');

			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = bUppercase ? LITERAL(FCharType, 'X') : LITERAL(FCharType, 'x');

			const uint8* Ptr = reinterpret_cast<const uint8*>(&Object);

			if constexpr (Math::EEndian::Native != Math::EEndian::Little)
			{
				for (size_t Index = 0; Index != sizeof(T); ++Index)
				{
					if (Iter == Sent) UNLIKELY return Iter;

					*Iter++ = FCharTraits::FromDigit(Ptr[Index] >> 4, bLowercase);

					if (Iter == Sent) UNLIKELY return Iter;

					*Iter++ = FCharTraits::FromDigit(Ptr[Index] & 0x0F, bLowercase);
				}
			}

			else
			{
				for (size_t Index = 0; Index != sizeof(T); ++Index)
				{
					if (Iter == Sent) UNLIKELY return Iter;

					*Iter++ = FCharTraits::FromDigit(Ptr[sizeof(T) - Index - 1] >> 4, bLowercase);

					if (Iter == Sent) UNLIKELY return Iter;

					*Iter++ = FCharTraits::FromDigit(Ptr[sizeof(T) - Index - 1] & 0x0F, bLowercase);
				}
			}
		}

		// Write the right padding.
		for (size_t Index = 0; Index != RightPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		return Iter;
	}

private:

	size_t         FillUnitLength = 1;
	FFillCharacter FillCharacter  = { LITERAL(FCharType, ' ') };
	FCharType      AlignOption    =   LITERAL(FCharType, '>');

	size_t FieldWidth = 0;

	bool bDynamicWidth = false;

	bool bLowercase = false;
	bool bUppercase = false;

};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
