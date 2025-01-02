#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Ranges/Utility.h"
#include "Ranges/View.h"
#include "Algorithms/Basic.h"
#include "Containers/StaticArray.h"
#include "Numerics/Math.h"
#include "Strings/Char.h"
#include "Miscellaneous/AssertionMacros.h"

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

						if (Digit > 10) bIsValid = false;

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
 * A formatter the null-terminated string.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Width] [Precision] [Type]
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
 *
 * 2. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integer argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 3. The precision part:
 *
 *	- '.N':   The number is used to specify the maximum field width of the object.
 *	          N should be an unsigned non-zero decimal number.
 *	- '.{N}': Dynamically determine the maximum field width of the object.
 *	          N should be a valid index of the format integer argument.
 *	          N is optional, and the default value is automatic indexing.
 *
 * 4. The type indicators part:
 *
 *	- none: Indicates the as-is formatting.
 *	- 'S':  Indicates the as-is formatting.
 *	- '!':  Indicates uppercase formatting.
 *	- 's':  Indicates lowercase formatting.
 *	- '?':  Indicates escape formatting.
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

			if (*Iter != LITERAL(FCharType, '<') && *Iter != LITERAL(FCharType, '^') && *Iter != LITERAL(FCharType, '>')) break;

			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			Char = *Iter; ++Iter;

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

					if (Digit > 10) break;

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

					if (Digit > 10) break;

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
		if      (Char == LITERAL(FCharType, 'S')) {                    if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; }
		else if (Char == LITERAL(FCharType, '!')) { bUppercase = true; if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; }
		else if (Char == LITERAL(FCharType, 's')) { bLowercase = true; if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; }
		else if (Char == LITERAL(FCharType, '?')) { bEscape    = true; if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; }

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

		if (bDynamicMin) MinDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
		{

			if constexpr (CIntegral<U>)
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

		if (bDynamicMax) MaxDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
		{
			if constexpr (CIntegral<U>)
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
					case LITERAL(FCharType, '\"'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\\'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\a'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\b'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\f'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\n'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\r'): FieldWidth += 2; break;
					case LITERAL(FCharType, '\t'): FieldWidth += 2; break;
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

			if      (AlignOption == LITERAL(FCharType, '<')) RightPadding = PaddingWidth;
			else if (AlignOption == LITERAL(FCharType, '>')) LeftPadding  = PaddingWidth;
			else if (AlignOption == LITERAL(FCharType, '^'))
			{
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
			if      (bLowercase) Char = FCharTraits::ToLower(Char);
			else if (bUppercase) Char = FCharTraits::ToUpper(Char);

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

							FCharType Buffer[DigitNum];

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

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
