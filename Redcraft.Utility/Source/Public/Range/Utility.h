#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Iterator/Iterator.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * The bool value that indicates whether the range always is borrowed range.
 * When the range always is borrowed range, it means that the iterators and sentinels
 * of the range remain valid even if the range object is destructed.
 */
template <typename R>
inline constexpr bool bEnableBorrowedRange = false;

NAMESPACE_BEGIN(Range)

/** @return The iterator to the beginning of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.Begin() } -> CInputOrOutputIterator; })
NODISCARD FORCEINLINE constexpr auto Begin(T&& Container)
{
	return Container.Begin();
}

/** Overloads the Begin algorithm for arrays. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& CArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr auto Begin(T&& Container)
{
	return Container + 0;
}

/** Overloads the Begin algorithm for initializer_list. */
template <typename T>
NODISCARD FORCEINLINE constexpr auto Begin(initializer_list<T>& Container)
{
	return Container.begin();
}

NAMESPACE_END(Range)

template <typename R>
using TRangeIterator = decltype(Range::Begin(DeclVal<R&>()));


NAMESPACE_BEGIN(Range)

/** @return The iterator to the end of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.End() } -> CSentinelFor<TRangeIterator<T>>; })
NODISCARD FORCEINLINE constexpr auto End(T&& Container)
{
	return Container.End();
}

/** Overloads the End algorithm for arrays. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& CBoundedArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr auto End(T&& Container)
{
	return Container + TExtent<TRemoveReference<T>>;
}

/** Overloads the End algorithm for initializer_list. */
template <typename T>
NODISCARD FORCEINLINE constexpr auto End(initializer_list<T>& Container)
{
	return Container.end();
}

NAMESPACE_END(Range)

template <typename R>
using TRangeSentinel = decltype(Range::End(DeclVal<R&>()));


NAMESPACE_BEGIN(Range)

/** @return The reverse iterator to the beginning of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.RBegin() } -> CInputOrOutputIterator; })
NODISCARD FORCEINLINE constexpr auto RBegin(T&& Container)
{
	return Container.RBegin();
}

/** Overloads the RBegin algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.RBegin() } -> CInputOrOutputIterator; }
	&& (CSameAs<TRangeIterator<T>, TRangeSentinel<T>> && CBidirectionalIterator<TRangeIterator<T>>))
NODISCARD FORCEINLINE constexpr auto RBegin(T&& Container)
{
	return MakeReverseIterator(Range::End(Forward<T>(Container)));
}

/** @return The reverse iterator to the end of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.REnd() } -> CSentinelFor<decltype(Range::RBegin(DeclVal<T&>()))>; })
NODISCARD FORCEINLINE constexpr auto REnd(T&& Container)
{
	return Container.REnd();
}

/** Overloads the REnd algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.REnd() } -> CSentinelFor<decltype(Range::RBegin(DeclVal<T&>()))>; }
	&& (CSameAs<TRangeIterator<T>, TRangeSentinel<T>> && CBidirectionalIterator<TRangeIterator<T>>))
NODISCARD FORCEINLINE constexpr auto REnd(T&& Container)
{
	return MakeReverseIterator(Range::Begin(Forward<T>(Container)));
}

NAMESPACE_END(Range)

template <typename R>
using TRangeElement = TIteratorElement<TRangeIterator<R>>;

template <typename R>
using TRangePointer = TIteratorPointer<TRangeIterator<R>>;

template <typename R>
using TRangeReference = TIteratorReference<TRangeIterator<R>>;

template <typename R>
using TRangeRValueReference = TIteratorRValueReference<TRangeIterator<R>>;

NAMESPACE_BEGIN(Range)

/** @return The pointer to the container element storage. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.GetData() } -> CSameAs<TAddPointer<TRangeReference<T>>>; })
NODISCARD FORCEINLINE constexpr auto GetData(T&& Container)
{
	return Container.GetData();
}

/** Overloads the GetData algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.GetData() } -> CSameAs<TAddPointer<TRangeReference<T>>>; }
	&&  requires(T&& Container) { { Range::Begin(Forward<T>(Container)) } -> CContiguousIterator; })
NODISCARD FORCEINLINE constexpr auto GetData(T&& Container)
{
	return ToAddress(Range::Begin(Forward<T>(Container)));
}

NAMESPACE_END(Range)

/** Disable the CSizedRange concept for specific types. */
template <typename R>
inline constexpr bool bDisableSizedRange = false;

NAMESPACE_BEGIN(Range)

/** @return The number of elements in the container. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; })
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return Container.Num();
}

/** Overloads the Num algorithm for arrays. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& CBoundedArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return TExtent<TRemoveReference<T>>;
}

/** Overloads the Num algorithm for synthesized. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& !requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; } && !CBoundedArray<TRemoveReference<T>>
	&& CSizedSentinelFor<TRangeSentinel<T>, TRangeIterator<T>> && CForwardIterator<TRangeIterator<T>>)
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return Range::End(Forward<T>(Container)) - Range::Begin(Forward<T>(Container));
}

/** @return true if the container is empty, false otherwise. */
template <typename T> requires (requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; })
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Container.IsEmpty();
}

/** Overloads the IsEmpty algorithm for synthesized. */
template <typename T> requires ((CBoundedArray<TRemoveReference<T>>
	||  requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; })
	&& !requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; })
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Range::Num(Forward<T>(Container)) == 0;
}

/** Overloads the IsEmpty algorithm for synthesized. */
template <typename T> requires (!CBoundedArray<TRemoveReference<T>>
	&& !requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; }
	&& !requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; }
	&& CForwardIterator<TRangeIterator<T>>)
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Range::End(Forward<T>(Container)) == Range::Begin(Forward<T>(Container));
}

NAMESPACE_END(Range)

/**
 * A concept specifies a type is a range.
 * A range is an iterator-sentinel pair that represents a sequence of elements.
 * This concept does not require that iterator-sentinel pair can be fetched multiple times
 * from the range object. again this means that const R may not be a range if R is a range,
 * e.g. fetching the iterator-sentinel pair from the input range may require moving the iterator
 * directly from the range object and thus the range object may be modified.
 */
template <typename R>
concept CRange =
	requires(R Range)
	{
		typename TRangeIterator<R>;
		typename TRangeSentinel<R>;
	}
	&& CInputOrOutputIterator<TRangeIterator<R>>
	&& CSentinelFor<TRangeSentinel<R>, TRangeIterator<R>>;

/** This is an example of a range type, indicate the traits that define a range type. */
template <CInputOrOutputIterator I, CSentinelFor<I> S = ISentinelFor<I>>
struct IRange
{
	/**
	 * Get the iterator-sentinel pair.
	 * If the function is const, it means that the const IRange satisfies CRange.
	 */
	I Begin() /* const */;
	S End()   /* const */;
};

// Use IRange<...> represents an range type.
static_assert(CRange<IRange<IInputOrOutputIterator<int&>>>);

/**
 * A concept specifies a type is a borrowed range.
 * When the range is borrowed range, it means that the iterators and sentinels
 * of the range remain valid even if the range value (note not object) is destructed.
 */
template <typename R>
concept CBorrowedRange = CRange<R> && (CLValueReference<R> || bEnableBorrowedRange<TRemoveCVRef<R>>);

/**
 * A concept specifies a type is a sized range.
 * Indicates the expression 'Range::Num(Range)' can get the size of the range at constant time
 * without modifying the range object. Modifying the range usually occurs when the iterator of
 * the range is an input iterator. Indirect calculation of the range by obtaining the iterator
 * may cause the range to become invalid, that is, the iterator cannot be obtained again.
 */
template <typename R>
concept CSizedRange = CRange<R>
	&& requires(R Range)
	{
		{ Range::Num(Range) } -> CConvertibleTo<size_t>;
	};

/** This is an example of a sized range type, indicate the traits that define a sized range type. */
template <CInputOrOutputIterator I, CSizedSentinelFor<I> S = ISizedSentinelFor<I>>
struct ISizedRange /* : IRange<I, S> */
{
	// ~Begin CRange

	I Begin() /* const */;
	S End()   /* const */;

	// ~End CRange

	/**
	 * Get the number of elements in the range.
	 * The function is optional if the range size can be computed indirectly from the iterator-sentinel pair.
	 * If this function is provided so that types that satisfy CSizedRange but do not satisfy the comments
	 * requirements of CSizedRange are undefined behavior, this should be resolved by specializing bDisableSizedRange.
	 * If the function is const, it means that the const ISizedRange satisfies CSizedRange.
	 */
	size_t Num() /* const */;
};

// Use ISizedRange<...> represents a sized range type.
static_assert(CSizedRange<ISizedRange<IInputOrOutputIterator<int&>>>);

/** A concept specifies a type is a range with an input iterator. */
template <typename R>
concept CInputRange = CRange<R> && CInputIterator<TRangeIterator<R>>;

// Use IRange<IInputIterator<...>> represents an input range type.
static_assert(CInputRange<IRange<IInputIterator<int&>>>);

/** A concept specifies a type is a range with an output iterator. */
template <typename R, typename T>
concept COutputRange = CRange<R> && COutputIterator<TRangeIterator<R>, T>;

// Use IRange<IOutputIterator<...>, int> represents an output range type.
static_assert(COutputRange<IRange<IOutputIterator<int&>>, int>);

/** A concept specifies a type is a range with a forward iterator. */
template <typename R>
concept CForwardRange = CInputRange<R> && CForwardIterator<TRangeIterator<R>>;

// Use IRange<IForwardIterator<...>> represents a forward range type.
static_assert(CForwardRange<IRange<IForwardIterator<int&>>>);

/** A concept specifies a type is a range with a bidirectional iterator. */
template <typename R>
concept CBidirectionalRange = CForwardRange<R> && CBidirectionalIterator<TRangeIterator<R>>;

// Use IRange<IBidirectionalIterator<...>> represents a bidirectional range type.
static_assert(CBidirectionalRange<IRange<IBidirectionalIterator<int&>>>);

/** A concept specifies a type is a range with a random access iterator. */
template <typename R>
concept CRandomAccessRange = CBidirectionalRange<R> && CRandomAccessIterator<TRangeIterator<R>>;

// Use IRange<IRandomAccessIterator<...>> represents a random access range type.
static_assert(CRandomAccessRange<IRange<IRandomAccessIterator<int&>>>);

/** A concept specifies a type is a range with a contiguous iterator. */
template <typename R>
concept CContiguousRange = CRandomAccessRange<R> && CContiguousIterator<TRangeIterator<R>>
	&& requires(R& Range)
	{
		{ Range::GetData(Range) } -> CSameAs<TAddPointer<TRangeReference<R>>>;
	};

/** This is an example of a contiguous range type, indicate the traits that define a contiguous range type. */
template <CContiguousIterator I, CSentinelFor<I> S = ISentinelFor<I>>
struct IContiguousRange /* : IRange<I, S> */
{
	// ~Begin CRange

	I Begin() /* const */;
	S End()   /* const */;

	// ~End CRange

	/**
	 * Get the pointer to the container element storage.
	 * The function is optional if the range size can be computed indirectly from the iterator.
	 * If the function is provided, then the expression 'ToAddress(Range::Begin(Range)) == Range::GetData(Range)'
	 * must be satisfied to always be true.
	 * If the function is const, it means that the const IContiguousRange satisfies CContiguousRange.
	 */
	TIteratorPointer<I> GetData() /* const */;
};

// Use IContiguousRange<...> represents a contiguous range type.
static_assert(CContiguousRange<IContiguousRange<IContiguousIterator<int&>>>);

/** A concept specifies a type is a range and its iterators and sentinel types are the same. */
template <typename R>
concept CCommonRange = CRange<R> && CSameAs<TRangeIterator<R>, TRangeSentinel<R>>;

/** This is an example of a common range type, indicate the traits that define a common range type. */
template <CForwardIterator I>
using TCommonRange = IRange<I, I>;

// Use TCommonRange<...> represents a common range type.
static_assert(CCommonRange<TCommonRange<IForwardIterator<int&>>>);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
