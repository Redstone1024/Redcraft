#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Iterators/BasicIterator.h"
#include "Ranges/Utility.h"
#include "Numerics/Math.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Algorithms)

/** Increments given iterator 'Iter' by 'N' elements. */
template <CInputOrOutputIterator I>
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

/** Increments given iterator 'Iter' to the 'Sent' position. */
template <CInputOrOutputIterator I, CSentinelFor<I> S>
FORCEINLINE constexpr void Advance(I& Iter, S Sent)
{
	if constexpr (CAssignableFrom<I&, S>)
	{
		Iter = Sent;
	}

	else if constexpr (CSizedSentinelFor<S, I>)
	{
		Algorithms::Advance(Iter, Sent - Iter);
	}

	else
	{
		for (; Iter != Sent; ++Iter);
	}
}

/** Increments given iterator 'Iter' by 'N' elements, up to the 'Sent' position. */
template <CInputOrOutputIterator I, CSentinelFor<I> S>
FORCEINLINE constexpr ptrdiff Advance(I& Iter, ptrdiff N, S Sent)
{
	if constexpr (CSizedSentinelFor<S, I>)
	{
		const ptrdiff Distance = Sent - Iter;

		if (Math::Abs(N) > Math::Abs(Distance))
		{
			Algorithms::Advance(Iter, Sent);

			return N - Distance;
		}

		Algorithms::Advance(Iter, N);

		return 0;
	}

	else if constexpr (CBidirectionalIterator<I>)
	{
		for (; N > 0 && Iter != Sent; --N) ++Iter;
		for (; N < 0 && Iter != Sent; ++N) --Iter;

		return N;
	}

	else
	{
		checkf(N >= 0, TEXT("The iterator must satisfy the CBidirectionalIterator in order to be decremented."));

		for (; N > 0 && Iter != Sent; --N) ++Iter;

		return N;
	}
}

/** @return The number of hops from 'First' to 'Last'. */
template <CInputOrOutputIterator I, CSentinelFor<I> S>
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

/** @return The 1-th successor of iterator 'Iter'. */
template <CInputOrOutputIterator I>
NODISCARD FORCEINLINE constexpr I Next(I Iter)
{
	return ++Iter;
}

/** @return The 'N'-th successor of iterator 'Iter'. */
template <CInputOrOutputIterator I>
NODISCARD FORCEINLINE constexpr I Next(I Iter, ptrdiff N)
{
	Algorithms::Advance(Iter, N);
	return Iter;
}

/** @return The successor of iterator 'Iter' to the 'Sent' position. */
template <CInputOrOutputIterator I, CSentinelFor<I> S>
NODISCARD FORCEINLINE constexpr I Next(I Iter, S Sent)
{
	Algorithms::Advance(Iter, Sent);
	return Iter;
}

/** @return The 'N'-th successor of iterator 'Iter', up to the 'Sent' position. */
template <CInputOrOutputIterator I, CSentinelFor<I> S>
NODISCARD FORCEINLINE constexpr I Next(I Iter, ptrdiff N, S Sent)
{
	Algorithms::Advance(Iter, N, Sent);
	return Iter;
}

/** @return The 1-th predecessor of iterator 'Iter'. */
template <CBidirectionalIterator I>
NODISCARD FORCEINLINE constexpr I Prev(I Iter)
{
	return --Iter;
}

/** @return The 'N'-th predecessor of iterator 'Iter'. */
template <CBidirectionalIterator I>
NODISCARD FORCEINLINE constexpr I Prev(I Iter, ptrdiff N)
{
	Algorithms::Advance(Iter, -N);
	return Iter;
}

/** @return The predecessor of iterator 'Iter', up to the 'First' position. */
template <CBidirectionalIterator I>
NODISCARD FORCEINLINE constexpr I Prev(I Iter, ptrdiff N, I First)
{
	Algorithms::Advance(Iter, -N, First);
	return Iter;
}

NAMESPACE_END(Algorithms)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
