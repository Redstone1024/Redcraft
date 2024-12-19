#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
#include "Iterator/BasicIterator.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Memory/Address.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * An iterator adaptor which dereferences to a rvalue reference.
 * When based on at least an input iterator, the move iterator satisfies at least an input iterator
 * up to a random access iterator.
 */
template <CInputIterator I>
class TMoveIterator final
{
public:

	using FIteratorType = I;

	using FElementType = TIteratorElement<I>;

	FORCEINLINE constexpr TMoveIterator() requires (CDefaultConstructible<I>) = default;

	FORCEINLINE constexpr TMoveIterator(const TMoveIterator&)            = default;
	FORCEINLINE constexpr TMoveIterator(TMoveIterator&&)                 = default;
	FORCEINLINE constexpr TMoveIterator& operator=(const TMoveIterator&) = default;
	FORCEINLINE constexpr TMoveIterator& operator=(TMoveIterator&&)      = default;
	FORCEINLINE constexpr ~TMoveIterator()                               = default;

	FORCEINLINE constexpr explicit TMoveIterator(FIteratorType InValue) : Current(MoveTemp(InValue)) { }

	template <CInputIterator J> requires (!CSameAs<I, J> && CConstructibleFrom<I, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, I>) TMoveIterator(const TMoveIterator<J>& InValue) : Current(InValue.GetBase()) { }

	template <CInputIterator J> requires (!CSameAs<I, J> && CConvertibleTo<const J&, I> && CAssignableFrom<I&, const J&>)
	FORCEINLINE constexpr TMoveIterator& operator=(const TMoveIterator<J>& InValue) { Current = InValue.GetBase(); return *this; }

	template <CInputIterator J> requires (CEqualityComparable<I, J>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.GetBase() == RHS.GetBase(); }

	template <CInputIterator J> requires (CThreeWayComparable<I, J>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<I, J> operator<=>(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.GetBase() <=> RHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr TIteratorRValueReference<I> operator*() const { return MoveTemp(*GetBase()); }

	NODISCARD FORCEINLINE constexpr TIteratorRValueReference<I> operator[](ptrdiff Index) const requires (CRandomAccessIterator<I>) { return MoveTemp(GetBase()[Index]); }

	FORCEINLINE constexpr TMoveIterator& operator++()                                      { ++Current; return *this; }
	FORCEINLINE constexpr TMoveIterator& operator--() requires (CBidirectionalIterator<I>) { --Current; return *this; }

	FORCEINLINE constexpr void          operator++(int)                                      {                      Current++;  }
	FORCEINLINE constexpr TMoveIterator operator++(int) requires       (CForwardIterator<I>) { return TMoveIterator(Current++); }
	FORCEINLINE constexpr TMoveIterator operator--(int) requires (CBidirectionalIterator<I>) { return TMoveIterator(Current--); }

	FORCEINLINE constexpr TMoveIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current += Offset; return *this; }
	FORCEINLINE constexpr TMoveIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current -= Offset; return *this; }

	NODISCARD FORCEINLINE constexpr TMoveIterator operator+(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TMoveIterator Temp = *this; Temp += Offset; return Temp; }
	NODISCARD FORCEINLINE constexpr TMoveIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TMoveIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr TMoveIterator operator+(ptrdiff Offset, const TMoveIterator& Iter) requires (CRandomAccessIterator<I>) { return Iter + Offset; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator& LHS, const TMoveIterator& RHS) requires (CSizedSentinelFor<I, I>) { return LHS.GetBase() - RHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr const FIteratorType& GetBase() const& { return          Current;  }
	NODISCARD FORCEINLINE constexpr       FIteratorType  GetBase() &&     { return MoveTemp(Current); }

private:

	FIteratorType Current;

};

template <typename I, typename J> requires (!CSizedSentinelFor<I, J>)
inline constexpr bool bDisableSizedSentinelFor<TMoveIterator<I>, TMoveIterator<J>> = true;

static_assert(        CInputIterator<TMoveIterator<        IInputIterator<int&>>>);
static_assert(      CForwardIterator<TMoveIterator<      IForwardIterator<int&>>>);
static_assert(CBidirectionalIterator<TMoveIterator<IBidirectionalIterator<int&>>>);
static_assert( CRandomAccessIterator<TMoveIterator< IRandomAccessIterator<int&>>>);
static_assert( CRandomAccessIterator<TMoveIterator<   IContiguousIterator<int&>>>);

/**
 * A sentinel adaptor for use with TMoveIterator.
 * Whether based on un-sized or sized sentinel, the move sentinel satisfies the corresponding concept.
 */
template <CSemiregular S>
class TMoveSentinel
{
public:

	using FSentinelType = S;

	FORCEINLINE constexpr TMoveSentinel()                                = default;
	FORCEINLINE constexpr TMoveSentinel(const TMoveSentinel&)            = default;
	FORCEINLINE constexpr TMoveSentinel(TMoveSentinel&&)                 = default;
	FORCEINLINE constexpr TMoveSentinel& operator=(const TMoveSentinel&) = default;
	FORCEINLINE constexpr TMoveSentinel& operator=(TMoveSentinel&&)      = default;
	FORCEINLINE constexpr ~TMoveSentinel()                               = default;

	FORCEINLINE constexpr explicit TMoveSentinel(FSentinelType InValue) : Last(InValue) { }

	template <CSemiregular T> requires (!CSameAs<S, T> && CConstructibleFrom<S, const T&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const T&, S>) TMoveSentinel(const TMoveSentinel<T>& InValue) : Last(InValue.Last) { }

	template <CSemiregular T> requires (!CSameAs<S, T> && CConvertibleTo<const T&, S> && CAssignableFrom<S&, const T&>)
	FORCEINLINE constexpr TMoveSentinel& operator=(const TMoveSentinel<T>& InValue) { Last = InValue.GetBase(); return *this; }

	template <CInputIterator I> requires (CSentinelFor<S, I>)
	NODISCARD FORCEINLINE constexpr bool operator==(const TMoveIterator<I>& InValue) const& { return GetBase() == InValue.GetBase(); }

	template <CInputIterator I> requires (CSizedSentinelFor<S, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveSentinel& Sentinel, const TMoveIterator<I>& Iter) { return Sentinel.GetBase() - Iter.GetBase(); }

	template <CInputIterator I> requires (CSizedSentinelFor<S, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator<I>& Iter, const TMoveSentinel& Sentinel) { return Iter.GetBase() - Sentinel.GetBase(); }

	NODISCARD FORCEINLINE constexpr const FSentinelType& GetBase() const& { return          Last;  }
	NODISCARD FORCEINLINE constexpr       FSentinelType  GetBase() &&     { return MoveTemp(Last); }

private:

	FSentinelType Last;

};

static_assert(     CSentinelFor<TMoveSentinel<     ISentinelFor<IInputIterator<int>>>, TMoveIterator<IInputIterator<int>>>);
static_assert(CSizedSentinelFor<TMoveSentinel<ISizedSentinelFor<IInputIterator<int>>>, TMoveIterator<IInputIterator<int>>>);

/** Creates a TMoveIterator of type inferred from the argument. */
template <typename I> requires (CInputIterator<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeMoveIterator(I&& Iter)
{
	return TMoveIterator<TDecay<I>>(Forward<I>(Iter));
}

/** Creates a TMoveSentinel of type inferred from the argument. */
template <typename I> requires (CSemiregular<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeMoveSentinel(I&& Iter)
{
	return TMoveSentinel<TDecay<I>>(Forward<I>(Iter));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
