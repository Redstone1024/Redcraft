#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Iterators/BasicIterator.h"
#include "Ranges/Utility.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Algorithms)

/** Increments given iterator 'Iter' by 'N' elements. */
template <CInputIterator I>
FORCEINLINE constexpr void Advance(I& Iter, ptrdiff N)
{
	if constexpr (CRandomAccessIterator<I>)
	{
		Iter += N;
	}

	else if constexpr (CBidirectionalIterator<I>)
	{
		for (; N > 0; --N) ++Iter;
		for (; N < 0; ++N) --Iter;
	}

	else
	{
		checkf(N >= 0, TEXT("The iterator must satisfy the CBidirectionalIterator in order to be decremented."));
		for (; N > 0; --N) ++Iter;
	}
}

/** @return The number of hops from 'First' to 'Last'. */
template <CInputIterator I, CSentinelFor<I> S>
NODISCARD FORCEINLINE constexpr ptrdiff Distance(I First, S Last)
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		return Last - First;
	}
	else
	{
		ptrdiff Result = 0;
		for (; First != Last; ++First) ++Result;
		return Result;
	}
}

/** @return The size of the range. */
template <CRange R>
NODISCARD FORCEINLINE constexpr ptrdiff Distance(R&& Range)
{
	if constexpr (CSizedRange<R>)
	{
		return static_cast<ptrdiff>(Ranges::Num(Range));
	}
	else return Algorithms::Distance(Ranges::Begin(Range), Ranges::End(Range));
}

/** @return The 'N'-th successor of iterator 'Iter'. */
template <CInputIterator I>
NODISCARD FORCEINLINE constexpr I Next(I Iter, ptrdiff N = 1)
{
	Algorithms::Advance(Iter, N);
	return Iter;
}

/** @return The 'N'-th predecessor of iterator 'Iter'. */
template <CBidirectionalIterator I>
NODISCARD FORCEINLINE constexpr I Prev(I Iter, ptrdiff N = 1)
{
	Algorithms::Advance(Iter, -N);
	return Iter;
}

NAMESPACE_END(Algorithms)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
