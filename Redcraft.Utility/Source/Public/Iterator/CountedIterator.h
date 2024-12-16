#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
#include "Iterator/BidirectionalIterator.h"
#include "Iterator/RandomAccessIterator.h"
#include "Iterator/ContiguousIterator.h"
#include "Miscellaneous/AssertionMacros.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Memory/Address.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename            T> class TCountedIteratorImpl    {                                                   };
template <CIndirectlyReadable T> class TCountedIteratorImpl<T> { public: using FElementType = TIteratorElement<T>; };

NAMESPACE_PRIVATE_END

/**
 * An iterator adaptor that tracks the distance to the end of the range.
 * When based on an input or output iterator, the counted iterator satisfies at least an input or output iterator
 * up to a contiguous iterator. When based on an output iterator, the counted iterator satisfies an output iterator.
 * When based on iterator satisfies sentinel for itself, the counted iterator satisfies sized sentinel for itself.
 */
template <CInputOrOutputIterator I>
class TCountedIterator final : public NAMESPACE_PRIVATE::TCountedIteratorImpl<I>
{
public:

	using FIteratorType = I;

#	if DO_CHECK
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<I>) : Length(1), MaxLength(0) { }
#	else
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<I>) = default;
#	endif

	FORCEINLINE constexpr TCountedIterator(const TCountedIterator&)            = default;
	FORCEINLINE constexpr TCountedIterator(TCountedIterator&&)                 = default;
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator&) = default;
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator&&)      = default;
	FORCEINLINE constexpr ~TCountedIterator()                                  = default;

	FORCEINLINE constexpr explicit TCountedIterator(FIteratorType InValue, ptrdiff N) : Current(MoveTemp(InValue)) { check_code({ MaxLength = N; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<I, J> && CConstructibleFrom<I, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, I>) TCountedIterator(const TCountedIterator<J>& InValue)
		: Current(InValue.GetBase()), Length(InValue.Num())
	{
		check_code({ MaxLength = InValue.MaxLength; });
	}

	template <CInputOrOutputIterator J> requires (!CSameAs<I, J> && CConvertibleTo<const J&, I> && CAssignableFrom<I&, const J&>)
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator<J>& InValue)
	{
		Current = InValue.GetBase();
		Length  = InValue.Num();

		check_code({ MaxLength = InValue.MaxLength; });

		return *this;
	}

	template <CInputOrOutputIterator J> requires (CCommonType<I, J>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length == RHS.Length; }

	template <CInputOrOutputIterator J> requires (CCommonType<I, J>)
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length <=> RHS.Length; }

	NODISCARD FORCEINLINE constexpr bool operator==(FDefaultSentinel) const& { return Length == static_cast<ptrdiff>(0); }

	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(FDefaultSentinel) const& { return static_cast<ptrdiff>(0) <=> Length; }

	NODISCARD FORCEINLINE constexpr TIteratorReference<I> operator*()                                             { CheckThis(true); return *GetBase();  }
	NODISCARD FORCEINLINE constexpr TIteratorReference<I> operator*()  const requires (CDereferenceable<const I>) { CheckThis(true); return *GetBase();  }

	NODISCARD FORCEINLINE constexpr auto operator->() const requires (requires(const I Iter) { { ToAddress(Iter) } -> CSameAs<TIteratorPointer<I>>; }) { return ToAddress(GetBase()); }

	NODISCARD FORCEINLINE constexpr TIteratorReference<I> operator[](ptrdiff Index) const requires (CRandomAccessIterator<I>) { TCountedIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE constexpr TCountedIterator& operator++()                                      { ++Current; --Length; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator--() requires (CBidirectionalIterator<I>) { --Current; ++Length; CheckThis(); return *this; }

	FORCEINLINE constexpr auto             operator++(int)                                      {                                           --Length; CheckThis(); return Current++; }
	FORCEINLINE constexpr TCountedIterator operator++(int) requires       (CForwardIterator<I>) { TCountedIterator Temp = *this; ++Current; --Length; CheckThis(); return Temp;      }
	FORCEINLINE constexpr TCountedIterator operator--(int) requires (CBidirectionalIterator<I>) { TCountedIterator Temp = *this; --Current; ++Length; CheckThis(); return Temp;      }

	FORCEINLINE constexpr TCountedIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current += Offset; Length -= Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<I>) { Current -= Offset; Length += Offset; CheckThis(); return *this; }

	NODISCARD FORCEINLINE constexpr TCountedIterator operator+(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TCountedIterator Temp = *this; Temp += Offset; return Temp; }
	NODISCARD FORCEINLINE constexpr TCountedIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<I>) { TCountedIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(ptrdiff Offset, TCountedIterator Iter) requires (CRandomAccessIterator<I>) { return Iter + Offset; }

	template <CInputOrOutputIterator J> requires (CCommonType<I, J>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Length - RHS.Length; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, FDefaultSentinel) { LHS.CheckThis(); return -LHS.Num(); }
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(FDefaultSentinel, const TCountedIterator& RHS) { RHS.CheckThis(); return  RHS.Num(); }

	NODISCARD FORCEINLINE constexpr const FIteratorType& GetBase() const& { CheckThis(); return          Current;  }
	NODISCARD FORCEINLINE constexpr       FIteratorType  GetBase() &&     { CheckThis(); return MoveTemp(Current); }
	NODISCARD FORCEINLINE constexpr       ptrdiff            Num() const  { CheckThis(); return          Length;   }

private:

	FIteratorType Current;
	ptrdiff       Length;

#	if DO_CHECK
	ptrdiff MaxLength;
#	endif

	FORCEINLINE void CheckThis(bool bExceptEnd = false) const
	{
		checkf(static_cast<ptrdiff>(0) <= Length && Length <= MaxLength, TEXT("Read access violation. Please check Num()."));
		checkf(!(bExceptEnd && Length == static_cast<ptrdiff>(0)),       TEXT("Read access violation. Please check Num()."));
	}

	template <CInputOrOutputIterator J>
	friend class TCountedIterator;

};

static_assert(        CInputIterator<TCountedIterator<        IInputIterator<int&>>>);
static_assert(      CForwardIterator<TCountedIterator<      IForwardIterator<int&>>>);
static_assert(CBidirectionalIterator<TCountedIterator<IBidirectionalIterator<int&>>>);
static_assert( CRandomAccessIterator<TCountedIterator< IRandomAccessIterator<int&>>>);
static_assert(   CContiguousIterator<TCountedIterator<   IContiguousIterator<int&>>>);

//static_assert(COutputIterator<TCountedIterator<IOutputIterator<int&>>, int>);

static_assert(CSizedSentinelFor<TCountedIterator<IForwardIterator<int>>, TCountedIterator<IForwardIterator<int>>>);

/** Creates a TCountedIterator of type inferred from the argument. */
template <typename I> requires (CInputOrOutputIterator<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeCountedIterator(I&& Iter, ptrdiff N)
{
	return TCountedIterator<TDecay<I>>(Forward<I>(Iter), N);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
