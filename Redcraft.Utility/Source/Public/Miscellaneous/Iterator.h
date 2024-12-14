#pragma once

#include "CoreTypes.h"
#include "Iterator/Iterator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename I>
using TIteratorElementType = TIteratorElement<I>;

template <typename I>
using TIteratorPointerType = TIteratorPointer<I>;

template <CReferenceable I>
using TIteratorReferenceType = TIteratorReference<I>;

template <CReferenceable I> requires (requires(I& Iter) { { MoveTemp(*Iter) } -> CReferenceable; })
using TIteratorRValueReferenceType = TIteratorRValueReference<I>;

NAMESPACE_BEGIN(Iteration)

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
FORCEINLINE constexpr ptrdiff Distance(I First, S Last)
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

/** @return The 'N'-th successor of iterator 'Iter'. */
template <CInputIterator I>
FORCEINLINE constexpr I Next(I Iter, TMakeUnsigned<ptrdiff> N = 1)
{
	Iteration::Advance(Iter, N);
	return Iter;
}

/** @return The 'N'-th predecessor of iterator 'Iter'. */
template <CBidirectionalIterator I>
FORCEINLINE constexpr I Prev(I Iter, TMakeUnsigned<ptrdiff> N = 1)
{
	Iteration::Advance(Iter, -N);
	return Iter;
}

NAMESPACE_END(Iteration)

template <CIndirectlyReadable J, CIndirectlyWritable<TIteratorReferenceType<J>> I>
FORCEINLINE void IndirectlyCopy(I&& Iter, J&& Jter)
{
	*Iter = *Jter;
}

template <CIndirectlyReadable J, CIndirectlyWritable<TIteratorRValueReferenceType<J>> I>
FORCEINLINE void IndirectlyMove(I&& Iter, J&& Jter)
{
	*Iter = MoveTemp(*Jter);
}

template <CIndirectlyReadable I, CIndirectlyReadable J> requires (CSwappable<TIteratorReferenceType<I>, TIteratorReferenceType<J>>)
FORCEINLINE void IndirectlySwap(I&& Iter, J&& Jter)
{
	Swap(*Iter, *Jter);
}

template <typename I, typename J = I>
concept CIndirectlyCopyable = requires(const I Iter, const J Jter) { IndirectlyCopy(Iter, Jter); };

template <typename I, typename J = I>
concept CIndirectlyMovable = requires(const I Iter, const J Jter) { IndirectlyMove(Iter, Jter); };

template <typename I, typename J = I>
concept CIndirectlySwappable = CIndirectlyReadable<I> && CIndirectlyReadable<J>
	&& requires(const I Iter, const J Jter)
	{
		IndirectlySwap(Iter, Iter);
		IndirectlySwap(Jter, Jter);
		IndirectlySwap(Iter, Jter);
		IndirectlySwap(Jter, Iter);
	};

NAMESPACE_BEGIN(Iteration)

/** @return The iterator to the beginning of a container. */
template <typename T> requires (requires(T&& Container) { { Container.Begin() } -> CForwardIterator; })
FORCEINLINE constexpr auto Begin(T&& Container)
{
	return Container.Begin();
}

/** Overloads the Begin algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr       T* Begin(      T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr       T* Begin(      T(&& Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* Begin(const T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* Begin(const T(&& Container)[N]) { return Container; }

/** Overloads the Begin algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr auto Begin(initializer_list<T> Container)
{
	return Container.begin();
}

/** @return The iterator to the end of a container. */
template <typename T> requires (requires(T&& Container) { { Container.End() } -> CForwardIterator; })
FORCEINLINE constexpr auto End(T&& Container)
{
	return Container.End();
}

/** Overloads the End algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr       T* End(      T(&  Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr       T* End(      T(&& Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr const T* End(const T(&  Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr const T* End(const T(&& Container)[N]) { return Container + N; }

/** Overloads the End algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr auto End(initializer_list<T> Container)
{
	return Container.end();
}

/** @return The reverse iterator to the beginning of a container. */
template <typename T> requires (requires(T&& Container) { { Container.RBegin() } -> CForwardIterator; })
FORCEINLINE constexpr auto RBegin(T&& Container)
{
	return Container.RBegin();
}

/** Overloads the RBegin algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr auto RBegin(      T(&  Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto RBegin(      T(&& Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto RBegin(const T(&  Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto RBegin(const T(&& Container)[N]) { return TReverseIterator(End(Container)); }

/** Overloads the RBegin algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr auto RBegin(initializer_list<T> Container)
{
	return TReverseIterator(Container.end());
}

/** @return The reverse iterator to the end of a container. */
template <typename T> requires (requires(T&& Container) { { Container.REnd() } -> CForwardIterator; })
FORCEINLINE constexpr auto REnd(T&& Container)
{
	return Container.REnd();
}

/** Overloads the REnd algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr auto REnd(      T(&  Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto REnd(      T(&& Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto REnd(const T(&  Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr auto REnd(const T(&& Container)[N]) { return TReverseIterator(Begin(Container)); }

/** Overloads the REnd algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr auto REnd(initializer_list<T> Container)
{
	return TReverseIterator(Container.begin());
}

NAMESPACE_END(Iteration)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
