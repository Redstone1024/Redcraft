#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T> using WithReference = T&;

template <typename I>
struct TIteratorElementType
{
	using Type = typename I::ElementType;
};

template <typename T>
struct TIteratorElementType<T*>
{
	using Type = T;
};

NAMESPACE_PRIVATE_END

template <typename T>
concept CReferenceable = requires { typename NAMESPACE_PRIVATE::WithReference<T>; };

template <typename T>
concept CDereferenceable = requires(T& A) { { *A } -> CReferenceable; };

template <typename I>
using TIteratorElementType = typename NAMESPACE_PRIVATE::TIteratorElementType<I>::Type;

template <CReferenceable I>
using TIteratorReferenceType = decltype(*DeclVal<I&>());

template <CReferenceable T> requires (requires(T& Iter) { { MoveTemp(*Iter) } -> CReferenceable; })
using TIteratorRValueReferenceType = decltype(MoveTemp(*DeclVal<T&>()));

template <typename I>
concept CIndirectlyReadable =
	requires(const I Iter)
	{
		typename TIteratorElementType<I>;
		typename TIteratorReferenceType<I>;
		typename TIteratorRValueReferenceType<I>;
		{          *Iter  } -> CSameAs<TIteratorReferenceType<I>>;
		{ MoveTemp(*Iter) } -> CSameAs<TIteratorRValueReferenceType<I>>;
	}
	&& CCommonReference<TIteratorReferenceType<I>&&, TIteratorElementType<I>&>
	&& CCommonReference<TIteratorReferenceType<I>&&, TIteratorRValueReferenceType<I>&&>
	&& CCommonReference<TIteratorRValueReferenceType<I>&&, const TIteratorElementType<I>&>;

template <typename I, typename T>
concept CIndirectlyWritable =
	requires(I && Iter, T && A)
	{
		*Iter             = Forward<T>(A);
		*Forward<I>(Iter) = Forward<T>(A);
		const_cast<const TIteratorElementType<I>&&>(*Iter)             = Forward<T>(A);
		const_cast<const TIteratorElementType<I>&&>(*Forward<I>(Iter)) = Forward<T>(A);
	};

template <typename I>
concept CWeaklyIncrementable = CDefaultConstructible<I> && CMovable<I>
	&& requires(I Iter) { { ++Iter } -> CSameAs<I&>; Iter++; };

template <typename I>
concept CIncrementable = CRegular<I> && CWeaklyIncrementable<I>
	&& requires(I Iter) { { Iter++ } -> CSameAs<I>; };

template <typename I>
concept CInputOrOutputIterator = CWeaklyIncrementable<I>
	&& requires(I Iter) { { *Iter } -> CReferenceable; };

template <typename S, typename I>
concept CSentinelFor = CSemiregular<S> && CInputOrOutputIterator<I> && CWeaklyEqualityComparable<S, I>;

template <typename S, typename I>
inline constexpr bool bDisableSizedSentinelFor = false;

template <typename S, typename I>
concept CSizedSentinelFor = CSentinelFor<S, I> && !bDisableSizedSentinelFor<TRemoveCV<S>, TRemoveCV<I>>
	&& requires(const I& Iter, const S& Sentinel) { Sentinel - Iter; Iter - Sentinel; };

template <typename I>
concept CInputIterator = CInputOrOutputIterator<I> && CIndirectlyReadable<I>;

template <typename I, typename T>
concept COutputIterator = CInputOrOutputIterator<I> && CIndirectlyWritable<I, T>
	&& requires(I Iter, T&& A) { *Iter++ = Forward<T>(A); };

template <typename I>
concept CForwardIterator = CInputIterator<I> && CIncrementable<I> && CSentinelFor<I, I>;

template <typename I>
concept CBidirectionalIterator = CForwardIterator<I>
	&& requires(I Iter) {
		{ --Iter } -> CSameAs<I&>;
		{ Iter-- } -> CSameAs<I >;
	};

template <typename I>
concept CRandomAccessIterator = CBidirectionalIterator<I> && CTotallyOrdered<I> && CSizedSentinelFor<I, I>
	&& requires(I Iter, const I Jter, const ptrdiff N) {
		{ Iter   += N } -> CSameAs<I&>;
		{ Jter   +  N } -> CSameAs<I >;
		{ N +  Jter   } -> CSameAs<I >;
		{ Iter   -= N } -> CSameAs<I&>;
		{ Jter   -  N } -> CSameAs<I >;
		{   Jter[N]   } -> CSameAs<TIteratorReferenceType<I>>;
	};

template <typename I>
concept CContiguousIterator = CRandomAccessIterator<I> && CLValueReference<TIteratorReferenceType<I>>
	&& CSameAs<TIteratorElementType<I>, TRemoveReference<TIteratorReferenceType<I>>>
	&& requires(I& Iter)
	{
		static_cast<TAddPointer<TIteratorReferenceType<I>>>(Iter);
		{ AddressOf(*Iter) } -> CSameAs<TAddPointer<TIteratorReferenceType<I>>>;
	};

static_assert(CContiguousIterator<int32*>);

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
	if constexpr (CSizedSentinelFor<I, S>)
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
	Advance(Iter, N);
	return Iter;
}

/** @return The 'N'-th predecessor of iterator 'Iter'. */
template <CBidirectionalIterator I>
FORCEINLINE constexpr I Prev(I Iter, TMakeUnsigned<ptrdiff> N = 1)
{
	Advance(Iter, -N);
	return Iter;
}

/** @return The iterator to the beginning of a container. */
template <typename T> requires (requires(T&& Container) { { Container.Begin() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) Begin(T&& Container)
{
	return Container.Begin();
}

/** Overloads the Begin algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr       T* Begin(      T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr       T* Begin(      T(&& Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* Begin(const T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* Begin(const T(&& Container)[N]) { return Container; }

/** Overloads the Begin algorithm for T::begin(). */
template <typename T> requires (requires(T&& Container) { { Container.begin() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) Begin(T&& Container)
{
	return Container.begin();
}

/** @return The iterator to the end of a container. */
template <typename T> requires (requires(T&& Container) { { Container.End() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) End(T&& Container)
{
	return Container.End();
}

/** Overloads the End algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr       T* End(      T(&  Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr       T* End(      T(&& Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr const T* End(const T(&  Container)[N]) { return Container + N; }
template <typename T, size_t N> FORCEINLINE constexpr const T* End(const T(&& Container)[N]) { return Container + N; }

/** Overloads the End algorithm for T::end(). */
template <typename T> requires (requires(T&& Container) { { Container.end() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) End(T&& Container)
{
	return Container.end();
}

NAMESPACE_END(Iteration)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
