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
 * An iterator adaptor for reverse-order traversal.
 * When based on at least a bidirectional iterator, the reverse iterator satisfies at least a bidirectional iterator
 * up to a random access iterator. When based on an output iterator, the reverse iterator satisfies an output iterator.
 */
template <CBidirectionalIterator I>
class TReverseIterator final
{
public:

	using FIteratorType = I;

	using FElementType = TIteratorElement<I>;

	FORCEINLINE constexpr TReverseIterator()                                   = default;
	FORCEINLINE constexpr TReverseIterator(const TReverseIterator&)            = default;
	FORCEINLINE constexpr TReverseIterator(TReverseIterator&&)                 = default;
	FORCEINLINE constexpr TReverseIterator& operator=(const TReverseIterator&) = default;
	FORCEINLINE constexpr TReverseIterator& operator=(TReverseIterator&&)      = default;
	FORCEINLINE constexpr ~TReverseIterator()                                  = default;

	FORCEINLINE constexpr explicit TReverseIterator(FIteratorType InValue) : Current(InValue) { }

	template <CBidirectionalIterator J> requires (!CSameAs<I, J> && CConstructibleFrom<I, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, I>) TReverseIterator(const TReverseIterator<J>& InValue) : Current(InValue.GetBase()) { }

	template <CBidirectionalIterator J> requires (!CSameAs<I, J> && CConvertibleTo<const J&, I> && CAssignableFrom<I&, const J&>)
	FORCEINLINE constexpr TReverseIterator& operator=(const TReverseIterator<J>& InValue) { Current = InValue.GetBase(); return *this; }

	template <CBidirectionalIterator J> requires (CEqualityComparable<I, J>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return LHS.GetBase() == RHS.GetBase(); }

	template <CBidirectionalIterator J> requires (CThreeWayComparable<I, J>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<I, J> operator<=>(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return RHS.GetBase() <=> LHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr TIteratorReference<I> operator*() const { FIteratorType Temp = GetBase(); return *--Temp; }

	NODISCARD FORCEINLINE constexpr auto operator->() const requires (requires(const I Iter) { { ToAddress(Iter) } -> CSameAs<TIteratorPointer<I>>; }) { FIteratorType Temp = GetBase(); return ToAddress(--Temp); }

	NODISCARD FORCEINLINE constexpr TIteratorReference<I> operator[](ptrdiff Index) const requires (CRandomAccessIterator<I>) { return GetBase()[-Index - 1]; }

	FORCEINLINE constexpr TReverseIterator& operator++() { --Current; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator--() { ++Current; return *this; }

	FORCEINLINE constexpr TReverseIterator operator++(int) { TReverseIterator Temp = *this; ++*this; return Temp; }
	FORCEINLINE constexpr TReverseIterator operator--(int) { TReverseIterator Temp = *this; --*this; return Temp; }

	FORCEINLINE constexpr TReverseIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current -= Offset; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current += Offset; return *this; }

	NODISCARD FORCEINLINE constexpr TReverseIterator operator+(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TReverseIterator Temp = *this; Temp -= Offset; return Temp; }
	NODISCARD FORCEINLINE constexpr TReverseIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TReverseIterator Temp = *this; Temp += Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr TReverseIterator operator+(ptrdiff Offset, const TReverseIterator& Iter) requires (CRandomAccessIterator<I>) { return Iter + Offset; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TReverseIterator& LHS, const TReverseIterator& RHS) requires (CSizedSentinelFor<I, I>) { return RHS.GetBase() - LHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr const FIteratorType& GetBase() const& { return          Current;  }
	NODISCARD FORCEINLINE constexpr       FIteratorType  GetBase() &&     { return MoveTemp(Current); }

private:

	FIteratorType Current;

};

template <typename I, typename J> requires (!CSizedSentinelFor<I, J>)
inline constexpr bool bDisableSizedSentinelFor<TReverseIterator<I>, TReverseIterator<J>> = true;

static_assert(CBidirectionalIterator<TReverseIterator<IBidirectionalIterator<int&>>>);
static_assert( CRandomAccessIterator<TReverseIterator< IRandomAccessIterator<int&>>>);
static_assert( CRandomAccessIterator<TReverseIterator<   IContiguousIterator<int&>>>);

static_assert(COutputIterator<TReverseIterator<IBidirectionalIterator<int&>>, int>);

/** Creates a TReverseIterator of type inferred from the argument. */
template <typename I> requires (CBidirectionalIterator<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeReverseIterator(I&& Iter)
{
	return TReverseIterator<TDecay<I>>(Forward<I>(Iter));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
