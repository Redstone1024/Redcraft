#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/Invoke.h"
#include "Templates/ReferenceWrapper.h"
#include "Templates/Optional.h"
#include "Templates/Tuple.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Iterators/BasicIterator.h"
#include "Iterators/ReverseIterator.h"
#include "Ranges/Utility.h"
#include "Ranges/View.h"
#include "Algorithms/Basic.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Algorithms)

/**
 * Checks if all elements in the range satisfy the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if all elements satisfy the predicate, false otherwise.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
NODISCARD constexpr bool AllOf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (!Invoke(Predicate, Invoke(Projection, *Iter)))
		{
			return false;
		}
	}

	return true;
}

/**
 * Checks if all elements in the range satisfy the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if all elements satisfy the predicate, false otherwise.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr bool AllOf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::AllOf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Checks if any elements in the range satisfy the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if any elements satisfy the predicate, false otherwise.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
NODISCARD constexpr bool AnyOf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter)))
		{
			return true;
		}
	}

	return false;
}

/**
 * Checks if any elements in the range satisfy the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if any elements satisfy the predicate, false otherwise.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr bool AnyOf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::AnyOf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Checks if no elements in the range satisfy the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if no elements satisfy the predicate, false otherwise.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
NODISCARD constexpr bool NoneOf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter)))
		{
			return false;
		}
	}

	return true;
}

/**
 * Checks if no elements in the range satisfy the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if no elements satisfy the predicate, false otherwise.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr bool NoneOf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::NoneOf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Checks if the range contains the given element.
 *
 * @param Range      - The range to check.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if the range contains the value, false otherwise.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TRangeReference<R>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TRangeReference<R>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TRangeReference<R>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool Contains(R&& Range, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter), Value))
		{
			return true;
		}
	}

	return false;
}

/**
 * Checks if the range contains the given element.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return true if the range contains the value, false otherwise.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TIteratorReference<I>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TIteratorReference<I>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TIteratorReference<I>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool Contains(I First, S Last, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Contains(Ranges::View(MoveTemp(First), Last), Value, Ref(Predicate), Ref(Projection));
}

/**
 * Checks if the range contains the given subrange.
 *
 * @param Haystack           - The range of elements to examine, aka the haystack.
 * @param Needle             - The range of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return true if the haystack contains the needle, false otherwise.
 */
template <CForwardRange R1, CForwardRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD constexpr bool Contains(R1&& Haystack, R2&& Needle, Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Haystack) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Haystack)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Needle) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Needle)."));
	}

	auto FirstA = Ranges::Begin(Haystack);
	auto SentA  = Ranges::End  (Haystack);

	auto FirstB = Ranges::Begin(Needle);
	auto SentB  = Ranges::End  (Needle);

	while (true)
	{
		auto IterA = FirstA;
		auto IterB = FirstB;

		while (true)
		{
			if (IterB == SentB) return true;
			if (IterA == SentA) return false;

			if (!Invoke(Predicate, Invoke(HaystackProjection, *IterA), Invoke(NeedleProjection, *IterB))) break;

			++IterA;
			++IterB;
		}

		++FirstA;
	}
}

/**
 * Checks if the range contains the given subrange.
 *
 * @param HaystackFirst      - The iterator of elements to examine, aka the haystack.
 * @param HaystackLast       - The sentinel of elements to examine, aka the haystack.
 * @param NeedleFirst        - The iterator of elements to search for, aka the needle.
 * @param NeedleLast         - The sentinel of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return true if the haystack contains the needle, false otherwise.
 */
template <CForwardIterator I1, CSentinelFor<I1> S1, CForwardIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool Contains(I1 HaystackFirst, S1 HaystackLast, I2 NeedleFirst, S2 NeedleLast,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(HaystackFirst - HaystackLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(NeedleFirst - NeedleLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::Contains(
		Ranges::View(MoveTemp(HaystackFirst), HaystackLast),
		Ranges::View(MoveTemp(NeedleFirst),   NeedleLast),
		Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));
}

/**
 * Finds the first element in the range that equals the given value.
 *
 * @param Range      - The range to check.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that equals the value, or the end iterator if not found.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TRangeReference<R>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TRangeReference<R>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TRangeReference<R>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> Find(R&& Range, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter), Value))
		{
			return Iter;
		}
	}

	return Iter;
}

/**
 * Finds the first element in the range that equals the given value.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that equals the value, or the end iterator if not found.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TIteratorReference<I>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TIteratorReference<I>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TIteratorReference<I>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr I Find(I First, S Last, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Find(Ranges::View(MoveTemp(First), Last), Value, Ref(Predicate), Ref(Projection));
}

/**
 * Finds the first subrange in the range that equals the given subrange.
 *
 * @param Haystack           - The range of elements to examine, aka the haystack.
 * @param Needle             - The range of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The subrange to the first subrange that equals the value, or the empty subrange if not found.
 */
template <CForwardRange R1, CForwardRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R1>)
NODISCARD constexpr Ranges::TRangeView<TRangeIterator<R1>> Find(R1&& Haystack, R2&& Needle,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Haystack) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Haystack)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Needle) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Needle)."));
	}

	auto FirstA = Ranges::Begin(Haystack);
	auto SentA  = Ranges::End  (Haystack);

	auto FirstB = Ranges::Begin(Needle);
	auto SentB  = Ranges::End  (Needle);

	while (true)
	{
		auto IterA = FirstA;
		auto IterB = FirstB;

		while (true)
		{
			if (IterB == SentB) return Ranges::View(FirstA, IterA);
			if (IterA == SentA) return Ranges::View(IterA,  IterA);

			if (!Invoke(Predicate, Invoke(HaystackProjection, *IterA), Invoke(NeedleProjection, *IterB))) break;

			++IterA;
			++IterB;
		}

		++FirstA;
	}
}

/**
 * Finds the first subrange in the range that equals the given subrange.
 *
 * @param HaystackFirst      - The iterator of elements to examine, aka the haystack.
 * @param HaystackLast       - The sentinel of elements to examine, aka the haystack.
 * @param NeedleFirst        - The iterator of elements to search for, aka the needle.
 * @param NeedleLast         - The sentinel of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The subrange to the first subrange that equals the value, or the empty subrange if not found.
 */
template <CForwardIterator I1, CSentinelFor<I1> S1, CForwardIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr Ranges::TRangeView<I1> Find(I1 HaystackFirst, S1 HaystackLast, I2 NeedleFirst, S2 NeedleLast,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(HaystackFirst - HaystackLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(NeedleFirst - NeedleLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::Find(
		Ranges::View(MoveTemp(HaystackFirst), HaystackLast),
		Ranges::View(MoveTemp(NeedleFirst),   NeedleLast),
		Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));
}

/**
 * Finds the first element in the range that satisfies the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that satisfies the predicate, or the end iterator if not found.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> FindIf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter)))
		{
			return Iter;
		}
	}

	return Iter;
}

/**
 * Finds the first element in the range that satisfies the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that satisfies the predicate, or the end iterator if not found.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr I FindIf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindIf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Finds the first element in the range that does not satisfy the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that does not satisfy the predicate, or the end iterator if not found.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
	requires (CBorrowedRange<R>)
NODISCARD FORCEINLINE constexpr TRangeIterator<R> FindIfNot(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto NotPredicate = [&Predicate]<typename T>(T&& A) { return !Invoke(Predicate, Forward<T>(A)); };

	return Algorithms::FindIf(Forward<R>(Range), NotPredicate, Ref(Projection));
}

/**
 * Finds the first element in the range that does not satisfy the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first element that does not satisfy the predicate, or the end iterator if not found.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr I FindIfNot(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindIfNot(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Finds the last element in the range that equals the given value.
 *
 * @param Range      - The range to check.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that equals the value, or the end iterator if not found.
 */
template <CForwardRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TRangeReference<R>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TRangeReference<R>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TRangeReference<R>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> FindLast(R&& Range, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	if constexpr (CBidirectionalRange<R&> && CCommonRange<R&>)
	{
		auto RIter = MakeReverseIterator(Sent);
		auto RSent = MakeReverseIterator(Iter);

		for (; RIter != RSent; ++RIter)
		{
			if (Invoke(Predicate, Invoke(Projection, *RIter), Value))
			{
				return Algorithms::Prev(RIter.GetBase());
			}
		}

		return Sent;
	}

	else if constexpr (CRandomAccessRange<R&> && CSizedRange<R&>)
	{
		const size_t Count = Ranges::Num(Range);

		auto RIter = MakeReverseIterator(Iter + Count);
		auto RSent = MakeReverseIterator(Iter);

		for (; RIter != RSent; ++RIter)
		{
			if (Invoke(Predicate, Invoke(Projection, *RIter), Value))
			{
				return Algorithms::Prev(RIter.GetBase());
			}
		}

		return Iter + Count;
	}

	else
	{
		TOptional<TRangeIterator<R>> Result;

		for (; Iter != Sent; ++Iter)
		{
			if (Invoke(Predicate, Invoke(Projection, *Iter), Value))
			{
				Result = Iter;
			}
		}

		if (!Result) return Iter;

		return *Result;
	}
}

/**
 * Finds the last element in the range that equals the given value.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Value      - The value to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that equals the value, or the end iterator if not found.
 */
template <CForwardIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TIteratorReference<I>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TIteratorReference<I>>, const T&> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TIteratorReference<I>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr I FindLast(I First, S Last, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindLast(Ranges::View(MoveTemp(First), Last), Value, Ref(Predicate), Ref(Projection));
}

/**
 * Finds the last subrange in the range that equals the given subrange.
 *
 * @param Haystack           - The range of elements to examine, aka the haystack.
 * @param Needle             - The range of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The subrange to the last subrange that equals the value, or the empty subrange if not found.
 */
template <CForwardRange R1, CForwardRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R1>)
NODISCARD constexpr Ranges::TRangeView<TRangeIterator<R1>> FindLast(R1&& Haystack, R2&& Needle,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Haystack) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Haystack)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Needle) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Needle)."));
	}

	auto FirstA = Ranges::Begin(Haystack);
	auto SentA  = Ranges::End  (Haystack);

	auto FirstB = Ranges::Begin(Needle);
	auto SentB  = Ranges::End  (Needle);

	if (FirstB == SentB)
	{
		auto LastA = Algorithms::Next(FirstA, SentA);

		return Ranges::View(LastA, LastA);
	}

	if constexpr (CBidirectionalRange<R1&> && CCommonRange<R1&>)
	{
		const ptrdiff NeedleCount = Algorithms::Distance(FirstB, SentB);

		auto Iter = SentA;

		if (Algorithms::Advance(Iter, -NeedleCount, FirstA) != 0) return Ranges::View(SentA, SentA);

		for (; Iter != FirstA; --Iter)
		{
			auto IterA = Algorithms::Prev(Iter);
			auto IterB = FirstB;

			while (true)
			{
				if (IterB == SentB) return Ranges::View(Algorithms::Prev(Iter), IterA);

				if (!Invoke(Predicate, Invoke(HaystackProjection, *IterA), Invoke(NeedleProjection, *IterB))) break;

				++IterA;
				++IterB;
			}
		}

		return Ranges::View(SentA, SentA);
	}

	else
	{
		auto Result = Algorithms::Find(FirstA, SentA, FirstB, SentB,
			Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));

		if (Result.IsEmpty()) return Result;

		while (true)
		{
			auto Next = Algorithms::Find(Algorithms::Next(Result.Begin()), SentA, FirstB, SentB,
				Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));

			if (Next.IsEmpty()) return Result;

			Result = Next;
		}
	}
}

/**
 * Finds the last subrange in the range that equals the given subrange.
 *
 * @param HaystackFirst      - The iterator of elements to examine, aka the haystack.
 * @param HaystackLast       - The sentinel of elements to examine, aka the haystack.
 * @param NeedleFirst        - The iterator of elements to search for, aka the needle.
 * @param NeedleLast         - The sentinel of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The subrange to the last subrange that equals the value, or the empty subrange if not found.
 */
template <CForwardIterator I1, CSentinelFor<I1> S1, CForwardIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr Ranges::TRangeView<I1> FindLast(I1 HaystackFirst, S1 HaystackLast, I2 NeedleFirst, S2 NeedleLast,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(HaystackFirst - HaystackLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(NeedleFirst - NeedleLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::FindLast(
		Ranges::View(MoveTemp(HaystackFirst), HaystackLast),
		Ranges::View(MoveTemp(NeedleFirst),   NeedleLast),
		Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));
}

/**
 * Finds the last element in the range that satisfies the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that satisfies the predicate, or the end iterator if not found.
 */
template <CForwardRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> FindLastIf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	if constexpr (CBidirectionalRange<R&> && CCommonRange<R&>)
	{
		auto RIter = MakeReverseIterator(Sent);
		auto RSent = MakeReverseIterator(Iter);

		for (; RIter != RSent; ++RIter)
		{
			if (Invoke(Predicate, Invoke(Projection, *RIter)))
			{
				return Algorithms::Prev(RIter.GetBase());
			}
		}

		return Sent;
	}

	else if constexpr (CRandomAccessRange<R&> && CSizedRange<R&>)
	{
		const size_t Count = Ranges::Num(Range);

		auto RIter = MakeReverseIterator(Iter + Count);
		auto RSent = MakeReverseIterator(Iter);

		for (; RIter != RSent; ++RIter)
		{
			if (Invoke(Predicate, Invoke(Projection, *RIter)))
			{
				return Algorithms::Prev(RIter.GetBase());
			}
		}

		return Iter + Count;
	}

	else
	{
		TOptional<TRangeIterator<R>> Result;

		for (; Iter != Sent; ++Iter)
		{
			if (Invoke(Predicate, Invoke(Projection, *Iter)))
			{
				Result = Iter;
			}
		}

		if (!Result) return Iter;

		return *Result;
	}
}

/**
 * Finds the last element in the range that satisfies the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that satisfies the predicate, or the end iterator if not found.
 */
template <CForwardIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr I FindLastIf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindLastIf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Finds the last element in the range that does not satisfy the predicate.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that does not satisfy the predicate, or the end iterator if not found.
 */
template <CForwardRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> FindLastIfNot(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto NotPredicate = [&Predicate]<typename T>(T&& A) { return !Invoke(Predicate, Forward<T>(A)); };

	return Algorithms::FindLastIf(Forward<R>(Range), NotPredicate, Ref(Projection));
}

/**
 * Finds the last element in the range that does not satisfy the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the last element that does not satisfy the predicate, or the end iterator if not found.
 */
template <CForwardIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr I FindLastIfNot(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindLastIfNot(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Finds the first element in the range that also contained in another range.
 *
 * @param Haystack           - The range of elements to examine, aka the haystack.
 * @param Needle             - The range of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The iterator to the first element that also contained in the needle, or the end iterator if not found.
 */
template <CInputRange R1, CForwardRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R1>)
NODISCARD FORCEINLINE constexpr TRangeIterator<R1> FindFirstOf(R1&& Haystack, R2&& Needle,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Haystack) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Haystack)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Needle) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Needle)."));
	}

	auto ContainsInNeedle = [&]<typename LHS>(LHS&& A)
	{
		auto ForwardPredicate = [&]<typename RHS>(RHS&& B)
		{
			return Invoke(Predicate, Forward<LHS>(A), Forward<RHS>(B));
		};

		return Algorithms::FindIf(Needle, ForwardPredicate, Ref(NeedleProjection)) != Ranges::End(Needle);
	};

	return Algorithms::FindIf(Forward<R1>(Haystack), ContainsInNeedle, Ref(HaystackProjection));
}

/**
 * Finds the first element in the range that also contained in another range.
 *
 * @param HaystackFirst      - The iterator of elements to examine, aka the haystack.
 * @param HaystackLast       - The sentinel of elements to examine, aka the haystack.
 * @param NeedleFirst        - The iterator of elements to search for, aka the needle.
 * @param NeedleLast         - The sentinel of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The iterator to the first element that also contained in the needle, or the end iterator if not found.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, CForwardIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr I1 FindFirstOf(I1 HaystackFirst, S1 HaystackLast, I2 NeedleFirst, S2 NeedleLast,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(HaystackFirst - HaystackLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(NeedleFirst - NeedleLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::FindFirstOf(
		Ranges::View(MoveTemp(HaystackFirst), HaystackLast),
		Ranges::View(MoveTemp(NeedleFirst),   NeedleLast),
		Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));
}

/**
 * Finds the last element in the range that also contained in another range.
 *
 * @param Haystack           - The range of elements to examine, aka the haystack.
 * @param Needle             - The range of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The iterator to the last element that also contained in the needle, or the end iterator if not found.
 */
template <CForwardRange R1, CForwardRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R1>)
NODISCARD FORCEINLINE constexpr TRangeIterator<R1> FindLastOf(R1&& Haystack, R2&& Needle,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Haystack) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Haystack)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Needle) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Needle)."));
	}

	auto ContainsInNeedle = [&]<typename LHS>(LHS&& A)
	{
		auto ForwardPredicate = [&]<typename RHS>(RHS&& B)
		{
			return Invoke(Predicate, Forward<LHS>(A), Forward<RHS>(B));
		};

		return Algorithms::FindIf(Needle, ForwardPredicate, Ref(NeedleProjection)) != Ranges::End(Needle);
	};

	return Algorithms::FindLastIf(Forward<R1>(Haystack), ContainsInNeedle, Ref(HaystackProjection));
}

/**
 * Finds the last element in the range that also contained in another range.
 *
 * @param HaystackFirst      - The iterator of elements to examine, aka the haystack.
 * @param HaystackLast       - The sentinel of elements to examine, aka the haystack.
 * @param NeedleFirst        - The iterator of elements to search for, aka the needle.
 * @param NeedleLast         - The sentinel of elements to search for, aka the needle.
 * @param Predicate          - The equivalence relation predicate between the projected elements.
 * @param HaystackProjection - The projection to apply to the haystack's elements before checking.
 * @param NeedleProjection   - The projection to apply to the needle's elements before checking.
 *
 * @return The iterator to the last element that also contained in the needle, or the end iterator if not found.
 */
template <CForwardIterator I1, CSentinelFor<I1> S1, CForwardIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr I1 FindLastOf(I1 HaystackFirst, S1 HaystackLast, I2 NeedleFirst, S2 NeedleLast,
	Pred Predicate = { }, Proj1 HaystackProjection = { }, Proj2 NeedleProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(HaystackFirst - HaystackLast <= 0, TEXT("Illegal range iterator. Please check HaystackFirst <= HaystackLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(NeedleFirst - NeedleLast <= 0, TEXT("Illegal range iterator. Please check NeedleFirst <= NeedleLast."));
	}

	return Algorithms::FindLastOf(
		Ranges::View(MoveTemp(HaystackFirst), HaystackLast),
		Ranges::View(MoveTemp(NeedleFirst),   NeedleLast),
		Ref(Predicate), Ref(HaystackProjection), Ref(NeedleProjection));
}

/**
 * Finds the first pair of equal adjacent elements in the range.
 *
 * @param Range      - The range to check.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first of the pair of equal adjacent elements, or the end iterator if not found.
 */
template <CForwardRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj, TRangeReference<R>>, TInvokeResult<Proj, TRangeReference<R>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TRangeReference<R>>, TInvokeResult<Proj, TRangeReference<R>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R>)
NODISCARD constexpr TRangeIterator<R> FindAdjacent(R&& Range, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	if (Iter == Sent) return Iter;

	auto Next = Algorithms::Next(Iter);

	for (; Next != Sent; ++Iter, ++Next)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter), Invoke(Projection, *Next)))
		{
			return Iter;
		}
	}

	return Next;
}

/**
 * Finds the first pair of equal adjacent elements in the range.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The iterator to the first of the pair of equal adjacent elements, or the end iterator if not found.
 */
template <CForwardIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj, TIteratorReference<I>>, TInvokeResult<Proj, TIteratorReference<I>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TIteratorReference<I>>, TInvokeResult<Proj, TIteratorReference<I>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr I FindAdjacent(I First, S Last, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::FindAdjacent(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Counts the number of elements in the range that equals the given value.
 *
 * @param Range      - The range of elements to examine.
 * @param Value      - The value to search for.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The number of elements that equals the value.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TRangeReference<R>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TRangeReference<R>>, T> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TRangeReference<R>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD constexpr size_t Count(R&& Range, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	size_t Result = 0;

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter), Value))
		{
			++Result;
		}
	}

	return Result;
}

/**
 * Counts the number of elements in the range that equals the given value.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Value      - The value to search for.
 * @param Predicate  - The equivalence relation predicate between the projected elements and the value.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The number of elements that equals the value.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CReferenceable T = TRemoveCVRef<TInvokeResult<Proj, TIteratorReference<I>>>,
	CEquivalenceRelation<TInvokeResult<Proj, TIteratorReference<I>>, T> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj, TIteratorReference<I>>, const T&>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr size_t Count(I First, S Last, const T& Value, Pred Predicate = { }, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::Count(Ranges::View(MoveTemp(First), Last), Value, Ref(Predicate), Ref(Projection));
}

/**
 * Counts the number of elements in the range that satisfies the predicate.
 *
 * @param Range      - The range of elements to examine.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The number of elements that satisfies the predicate.
 */
template <CInputRange R,
	CRegularInvocable<TRangeReference<R>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TRangeReference<R>>> Pred>
NODISCARD constexpr size_t CountIf(R&& Range, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedRange<R&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	auto Iter = Ranges::Begin(Range);
	auto Sent = Ranges::End  (Range);

	size_t Result = 0;

	for (; Iter != Sent; ++Iter)
	{
		if (Invoke(Predicate, Invoke(Projection, *Iter)))
		{
			++Result;
		}
	}

	return Result;
}

/**
 * Counts the number of elements in the range that satisfies the predicate.
 *
 * @param First      - The iterator of the range.
 * @param Last 	     - The sentinel of the range.
 * @param Predicate  - The unary predicate to satisfy.
 * @param Projection - The projection to apply to the elements before checking.
 *
 * @return The number of elements that satisfies the predicate.
 */
template <CInputIterator I, CSentinelFor<I> S,
	CRegularInvocable<TIteratorReference<I>> Proj =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CPredicate<TInvokeResult<Proj, TIteratorReference<I>>> Pred>
NODISCARD FORCEINLINE constexpr size_t CountIf(I First, S Last, Pred Predicate, Proj Projection = { })
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	return Algorithms::CountIf(Ranges::View(MoveTemp(First), Last), Ref(Predicate), Ref(Projection));
}

/**
 * Finds the first mismatch between two ranges.
 *
 * @param LHS           - The left hand side range of the elements to compare.
 * @param RHS           - The right hand side range of the elements to compare.
 * @param Predicate     - The equivalence relation predicate between the projected elements.
 * @param LHSProjection - The projection to apply to the left hand side elements before checking.
 * @param RHSProjection - The projection to apply to the right hand side elements before checking.
 *
 * @return The pair of iterators to the first mismatched elements, or the pair of end iterators if not found.
 */
template <CInputRange R1, CInputRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires (CBorrowedRange<R1> && CBorrowedRange<R2>)
NODISCARD constexpr TTuple<TRangeIterator<R1>, TRangeIterator<R2>> Mismatch(R1&& LHS, R2&& RHS,
	Pred Predicate = { }, Proj1 LHSProjection = { }, Proj2 RHSProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(LHS) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(LHS)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(RHS) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(RHS)."));
	}

	auto IterA = Ranges::Begin(LHS);
	auto SentA = Ranges::End  (LHS);

	auto IterB = Ranges::Begin(RHS);
	auto SentB = Ranges::End  (RHS);

	while (IterA != SentA && IterB != SentB)
	{
		if (!Invoke(Predicate, Invoke(LHSProjection, *IterA), Invoke(RHSProjection, *IterB)))
		{
			break;
		}

		++IterA;
		++IterB;
	}

	return { IterA, IterB };
}

/**
 * Finds the first mismatch between two ranges.
 *
 * @param LHSFirst      - The iterator of the left hand side range.
 * @param LHSLast       - The sentinel of the left hand side range.
 * @param RHSFirst      - The iterator of the right hand side range.
 * @param RHSLast       - The sentinel of the right hand side range.
 * @param Predicate     - The equivalence relation predicate between the projected elements.
 * @param LHSProjection - The projection to apply to the left hand side elements before checking.
 * @param RHSProjection - The projection to apply to the right hand side elements before checking.
 *
 * @return The pair of iterators to the first mismatched elements, or the pair of end iterators if not found.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, CInputIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr TTuple<I1, I2> Mismatch(I1 LHSFirst, S1 LHSLast, I2 RHSFirst, S2 RHSLast,
	Pred Predicate = { }, Proj1 LHSProjection = { }, Proj2 RHSProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(LHSFirst - LHSLast <= 0, TEXT("Illegal range iterator. Please check LHSFirst <= LHSLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(RHSFirst - RHSLast <= 0, TEXT("Illegal range iterator. Please check RHSFirst <= RHSLast."));
	}

	return Algorithms::Mismatch(
		Ranges::View(MoveTemp(LHSFirst), LHSLast),
		Ranges::View(MoveTemp(RHSFirst), RHSLast),
		Ref(Predicate), Ref(LHSProjection), Ref(RHSProjection));
}

/**
 * Checks if two ranges are equal.
 *
 * @param LHS           - The left hand side range of the elements to compare.
 * @param RHS           - The right hand side range of the elements to compare.
 * @param Predicate     - The equivalence relation predicate between the projected elements.
 * @param LHSProjection - The projection to apply to the left hand side elements before checking.
 * @param RHSProjection - The projection to apply to the right hand side elements before checking.
 *
 * @return true if the ranges are equal, false otherwise.
 */
template <CInputRange R1, CInputRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool Equal(R1&& LHS, R2&& RHS, Pred Predicate = { }, Proj1 LHSProjection = { }, Proj2 RHSProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(LHS) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(LHS)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(RHS) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(RHS)."));
	}

	if constexpr (CSizedRange<R1&> && CSizedRange<R2&>)
	{
		if (Ranges::Num(LHS) != Ranges::Num(RHS))
		{
			return false;
		}
	}

	auto FirstA = Ranges::Begin(LHS);
	auto SentA  = Ranges::End  (LHS);

	auto FirstB = Ranges::Begin(RHS);
	auto SentB  = Ranges::End  (RHS);

	auto [IterA, IterB] = Algorithms::Mismatch(
		MoveTemp(FirstA), SentA, MoveTemp(FirstB), SentB,
		Ref(Predicate), Ref(LHSProjection), Ref(RHSProjection));

	return IterA == SentA && IterB == SentB;
}

/**
 * Checks if two ranges are equal.
 *
 * @param LHSFirst      - The iterator of the left hand side range.
 * @param LHSLast       - The sentinel of the left hand side range.
 * @param RHSFirst      - The iterator of the right hand side range.
 * @param RHSLast       - The sentinel of the right hand side range.
 * @param Predicate     - The equivalence relation predicate between the projected elements.
 * @param LHSProjection - The projection to apply to the left hand side elements before checking.
 * @param RHSProjection - The projection to apply to the right hand side elements before checking.
 *
 * @return true if the ranges are equal, false otherwise.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, CInputIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool Equal(I1 LHSFirst, S1 LHSLast, I2 RHSFirst, S2 RHSLast,
	Pred Predicate = { }, Proj1 LHSProjection = { }, Proj2 RHSProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(LHSFirst - LHSLast <= 0, TEXT("Illegal range iterator. Please check LHSFirst <= LHSLast."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(RHSFirst - RHSLast <= 0, TEXT("Illegal range iterator. Please check RHSFirst <= RHSLast."));
	}

	return Algorithms::Equal(
		Ranges::View(MoveTemp(LHSFirst), LHSLast),
		Ranges::View(MoveTemp(RHSFirst), RHSLast),
		Ref(Predicate), Ref(LHSProjection), Ref(RHSProjection));
}

/**
 * Checks if the range starts with the given prefix.
 *
 * @param Range            - The range of elements to examine.
 * @param Prefix           - The range of elements to be used as the prefix.
 * @param Predicate        - The equivalence relation predicate between the projected elements.
 * @param Projection       - The projection to apply to the elements before checking.
 * @param PrefixProjection - The projection to apply to the prefix elements before checking.
 *
 * @return true if the range starts with the prefix, false otherwise.
 */
template <CInputRange R1, CInputRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool StartsWith(R1&& Range, R2&& Prefix, Pred Predicate = { }, Proj1 Projection = { }, Proj2 PrefixProjection = { })
{
	if constexpr (CSizedRange<R1&>)
	{
		checkf(Algorithms::Distance(Range) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	}

	if constexpr (CSizedRange<R2&>)
	{
		checkf(Algorithms::Distance(Prefix) >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Prefix)."));
	}

	if constexpr (CSizedRange<R1&> && CSizedRange<R2&>)
	{
		if (Ranges::Num(Range) < Ranges::Num(Prefix))
		{
			return false;
		}
	}

	auto FirstA = Ranges::Begin(Range);
	auto SentA  = Ranges::End  (Range);

	auto FirstB = Ranges::Begin(Prefix);
	auto SentB  = Ranges::End  (Prefix);

	auto [IterA, IterB] = Algorithms::Mismatch(
		MoveTemp(FirstA), SentA, MoveTemp(FirstB), SentB,
		Ref(Predicate), Ref(Projection), Ref(PrefixProjection));

	return IterB == SentB;
}

/**
 * Checks if the range starts with the given prefix.
 *
 * @param First            - The iterator of the range.
 * @param Last 	           - The sentinel of the range.
 * @param PrefixFirst      - The iterator of the prefix range.
 * @param PrefixLast       - The sentinel of the prefix range.
 * @param Predicate        - The equivalence relation predicate between the projected elements.
 * @param Projection       - The projection to apply to the elements before checking.
 * @param PrefixProjection - The projection to apply to the prefix elements before checking.
 *
 * @return true if the range starts with the prefix, false otherwise.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, CInputIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
NODISCARD FORCEINLINE constexpr bool StartsWith(I1 First, S1 Last, I2 PrefixFirst, S2 PrefixLast,
	Pred Predicate = { }, Proj1 Projection = { }, Proj2 PrefixProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(PrefixFirst - PrefixLast <= 0, TEXT("Illegal range iterator. Please check PrefixFirst <= PrefixLast."));
	}

	return Algorithms::StartsWith(
		Ranges::View(MoveTemp(      First),       Last),
		Ranges::View(MoveTemp(PrefixFirst), PrefixLast),
		Ref(Predicate), Ref(Projection), Ref(PrefixProjection));
}

/**
 * Checks if the range ends with the given suffix.
 *
 * @param Range            - The range of elements to examine.
 * @param Suffix           - The range of elements to be used as the suffix.
 * @param Predicate        - The equivalence relation predicate between the projected elements.
 * @param Projection       - The projection to apply to the elements before checking.
 * @param SuffixProjection - The projection to apply to the suffix elements before checking.
 *
 * @return true if the range ends with the suffix, false otherwise.
 */
template <CInputRange R1, CInputRange R2,
	CRegularInvocable<TRangeReference<R1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TRangeReference<R2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TRangeReference<R1>>, TInvokeResult<Proj2, TRangeReference<R2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires ((CForwardRange<R1> || CSizedRange<R1>) && (CForwardRange<R2> || CSizedRange<R2>))
NODISCARD FORCEINLINE constexpr bool EndsWith(R1&& Range, R2&& Suffix, Pred Predicate = { }, Proj1 Projection = { }, Proj2 SuffixProjection = { })
{
	const ptrdiff CountA = Algorithms::Distance(Range);
	const ptrdiff CountB = Algorithms::Distance(Suffix);

	checkf(CountA >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Range)."));
	checkf(CountB >= 0, TEXT("Illegal range. Please check Algorithms::Distance(Suffix)."));

	if (CountA < CountB) return false;

	auto Iter = Ranges::Begin(Range);

	Algorithms::Advance(Iter, CountA - CountB);

	return Algorithms::Equal(
		MoveTemp(Iter), Ranges::End(Range), Ranges::Begin(Suffix), Ranges::End(Suffix),
		Ref(Predicate), Ref(Projection), Ref(SuffixProjection));
}

/**
 * Checks if the range ends with the given suffix.
 *
 * @param First            - The iterator of the range.
 * @param Last 	           - The sentinel of the range.
 * @param SuffixFirst      - The iterator of the suffix range.
 * @param SuffixLast       - The sentinel of the suffix range.
 * @param Predicate        - The equivalence relation predicate between the projected elements.
 * @param Projection       - The projection to apply to the elements before checking.
 * @param SuffixProjection - The projection to apply to the suffix elements before checking.
 *
 * @return true if the range ends with the suffix, false otherwise.
 */
template <CInputIterator I1, CSentinelFor<I1> S1, CInputIterator I2, CSentinelFor<I2> S2,
	CRegularInvocable<TIteratorReference<I1>> Proj1 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CRegularInvocable<TIteratorReference<I2>> Proj2 =
		decltype([]<typename T>(T&& A) -> T&& { return Forward<T>(A); }),
	CEquivalenceRelation<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>> Pred =
		TConditional<CWeaklyEqualityComparable<TInvokeResult<Proj1, TIteratorReference<I1>>, TInvokeResult<Proj2, TIteratorReference<I2>>>,
			decltype([]<typename LHS, typename RHS>(const LHS& A, const RHS& B) { return A == B; }), void>>
	requires ((CForwardIterator<I1> || CSizedSentinelFor<S1, I1>) && (CForwardIterator<I2> || CSizedSentinelFor<S2, I2>))
NODISCARD FORCEINLINE constexpr bool EndsWith(I1 First, S1 Last, I2 SuffixFirst, S2 SuffixLast,
	Pred Predicate = { }, Proj1 Projection = { }, Proj2 SuffixProjection = { })
{
	if constexpr (CSizedSentinelFor<S1, I1>)
	{
		checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));
	}

	if constexpr (CSizedSentinelFor<S2, I2>)
	{
		checkf(SuffixFirst - SuffixLast <= 0, TEXT("Illegal range iterator. Please check SuffixFirst <= SuffixLast."));
	}

	return Algorithms::EndsWith(
		Ranges::View(MoveTemp(      First),       Last),
		Ranges::View(MoveTemp(SuffixFirst), SuffixLast),
		Ref(Predicate), Ref(Projection), Ref(SuffixProjection));
}

NAMESPACE_END(Algorithms)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
