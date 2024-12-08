#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T> using WithReference = T&;

template <typename I> struct TIteratorElementType     { using Type = typename I::ElementType; };
template <typename T> struct TIteratorElementType<T*> { using Type = TRemoveCV<T>;            };

template <typename I> struct TIteratorPointerType     { using Type = void; };
template <typename T> struct TIteratorPointerType<T*> { using Type = T*;   };

template <typename I> requires (requires(I& Iter) { { Iter.operator->() } -> CPointer; })
struct TIteratorPointerType<I> { using Type = decltype(DeclVal<I&>().operator->()); };

NAMESPACE_PRIVATE_END

template <typename T>
concept CReferenceable = requires { typename NAMESPACE_PRIVATE::WithReference<T>; };

template <typename T>
concept CDereferenceable = requires(T& A) { { *A } -> CReferenceable; };

template <typename I>
using TIteratorElementType = typename NAMESPACE_PRIVATE::TIteratorElementType<TRemoveCVRef<I>>::Type;

template <typename I>
using TIteratorPointerType = typename NAMESPACE_PRIVATE::TIteratorPointerType<TRemoveCVRef<I>>::Type;

template <CReferenceable I>
using TIteratorReferenceType = decltype(*DeclVal<I&>());

template <CReferenceable I> requires (requires(I& Iter) { { MoveTemp(*Iter) } -> CReferenceable; })
using TIteratorRValueReferenceType = decltype(MoveTemp(*DeclVal<I&>()));

template <typename I>
concept CIndirectlyReadable =
	requires(const TRemoveCVRef<I> Iter)
	{
		typename TIteratorElementType<I>;
		typename TIteratorReferenceType<I>;
		typename TIteratorRValueReferenceType<I>;
		{          *Iter  } -> CSameAs<TIteratorReferenceType<I>>;
		{ MoveTemp(*Iter) } -> CSameAs<TIteratorRValueReferenceType<I>>;
	}
	&& CSameAs<TIteratorElementType<I>, TRemoveCVRef<TIteratorElementType<I>>>
	&& CCommonReference<TIteratorReferenceType<I>&&, TIteratorElementType<I>&>
	&& CCommonReference<TIteratorReferenceType<I>&&, TIteratorRValueReferenceType<I>&&>
	&& CCommonReference<TIteratorRValueReferenceType<I>&&, const TIteratorElementType<I>&>;

template <typename I, typename T>
concept CIndirectlyWritable =
	requires(I&& Iter, T&& A)
	{
		*Iter             = Forward<T>(A);
		*Forward<I>(Iter) = Forward<T>(A);
		const_cast<const TIteratorReferenceType<I>&&>(*Iter)             = Forward<T>(A);
		const_cast<const TIteratorReferenceType<I>&&>(*Forward<I>(Iter)) = Forward<T>(A);
	};

template <typename I>
concept CWeaklyIncrementable = CMovable<I>
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
concept CSizedSentinelFor = CSentinelFor<S, I> && CPartiallyOrdered<S, I>
	&& !bDisableSizedSentinelFor<TRemoveCV<S>, TRemoveCV<I>>
	&& requires(const I& Iter, const S& Sentinel)
	{
		{ Sentinel - Iter } -> CSameAs<ptrdiff>;
		{ Iter - Sentinel } -> CSameAs<ptrdiff>;
	};

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
	&& CSameAs<TIteratorElementType<I>, TRemoveCVRef<TIteratorReferenceType<I>>>
	&& requires(I& Iter)
	{
		{ ToAddress(Iter) } -> CSameAs<TAddPointer<TIteratorReferenceType<I>>>;
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

template <typename I, typename J>
concept CIndirectlyCopyable = requires(const I Iter, const J Jter) { IndirectlyCopy(Iter, Jter); };

template <typename I, typename J>
concept CIndirectlyMovable = requires(const I Iter, const J Jter) { IndirectlyMove(Iter, Jter); };

template <typename I, typename J>
concept CIndirectlySwappable = CIndirectlyReadable<I> && CIndirectlyReadable<J>
	&& requires(const I Iter, const J Jter)
	{
		IndirectlySwap(Iter, Iter);
		IndirectlySwap(Jter, Jter);
		IndirectlySwap(Iter, Jter);
		IndirectlySwap(Jter, Iter);
	};

/** A iterator adaptor for reverse-order traversal. */
template <CBidirectionalIterator I>
class TReverseIterator final
{
public:

	using IteratorType = I;

	using ElementType = TIteratorElementType<I>;

	FORCEINLINE constexpr TReverseIterator() = default;

	FORCEINLINE constexpr TReverseIterator(const TReverseIterator&)            = default;
	FORCEINLINE constexpr TReverseIterator(TReverseIterator&&)                 = default;
	FORCEINLINE constexpr TReverseIterator& operator=(const TReverseIterator&) = default;
	FORCEINLINE constexpr TReverseIterator& operator=(TReverseIterator&&)      = default;

	FORCEINLINE constexpr ~TReverseIterator() = default;

	template <typename T = IteratorType> requires (!CSameAs<TReverseIterator, TRemoveCVRef<T>> && CConstructibleFrom<IteratorType, T>)
	FORCEINLINE constexpr explicit TReverseIterator(T&& InValue) : Current(Forward<T>(InValue)) { }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, IteratorType>) TReverseIterator(const TReverseIterator<J>& InValue) : Current(InValue.Current) { }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, J>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<J&&, IteratorType>) TReverseIterator(TReverseIterator<J>&& InValue) : Current(MoveTemp(InValue).Current) { }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TReverseIterator& operator=(const TReverseIterator<J>& InValue) { Current = InValue.Current; return *this; }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<J&&, IteratorType> && CAssignableFrom<IteratorType&, J&&>)
	FORCEINLINE constexpr TReverseIterator& operator=(TReverseIterator<J>&& InValue) { Current = MoveTemp(InValue).Current; return *this; }

	template <CBidirectionalIterator J> requires (CSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return LHS.Current == RHS.Current; }

	template <CBidirectionalIterator J> requires (CSizedSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<J, IteratorType> operator<=>(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return RHS.Current <=> LHS.Current; }

	NODISCARD FORCEINLINE constexpr TIteratorReferenceType<IteratorType> operator*()  const { IteratorType Temp = Current; return          *--Temp;  }
	NODISCARD FORCEINLINE constexpr TIteratorPointerType<IteratorType>   operator->() const { IteratorType Temp = Current; return ToAddress(--Temp); }

	NODISCARD FORCEINLINE constexpr TIteratorReferenceType<IteratorType> operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { return Current[-Index - 1]; }

	FORCEINLINE constexpr TReverseIterator& operator++() { --Current; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator--() { ++Current; return *this; }

	FORCEINLINE constexpr TReverseIterator operator++(int) { TReverseIterator Temp = *this; --Current; return Temp; }
	FORCEINLINE constexpr TReverseIterator operator--(int) { TReverseIterator Temp = *this; ++Current; return Temp; }

	FORCEINLINE constexpr TReverseIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current -= Offset; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current += Offset; return *this; }

	NODISCARD friend FORCEINLINE constexpr TReverseIterator operator+(TReverseIterator Iter, ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = Iter; Temp -= Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TReverseIterator operator+(ptrdiff Offset, TReverseIterator Iter) requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = Iter; Temp -= Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TReverseIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = *this; Temp += Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TReverseIterator& LHS, const TReverseIterator& RHS) { return RHS.Current - LHS.Current; }

	NODISCARD FORCEINLINE constexpr const IteratorType& GetBase() const& { return          Current;  }
	NODISCARD FORCEINLINE constexpr       IteratorType  GetBase() &&     { return MoveTemp(Current); }

private:

	IteratorType Current;

};

static_assert(CRandomAccessIterator<TReverseIterator<int32*>>);

template <typename I, typename J> requires (!CSizedSentinelFor<I, J>)
inline constexpr bool bDisableSizedSentinelFor<TReverseIterator<I>, TReverseIterator<J>> = true;

/** An iterator adaptor which dereferences to a rvalue reference. */
template <CInputIterator I>
class TMoveIterator final
{
public:

	using IteratorType = I;

	using ElementType = TIteratorElementType<I>;

	FORCEINLINE constexpr TMoveIterator() = default;

	FORCEINLINE constexpr TMoveIterator(const TMoveIterator&)            = default;
	FORCEINLINE constexpr TMoveIterator(TMoveIterator&&)                 = default;
	FORCEINLINE constexpr TMoveIterator& operator=(const TMoveIterator&) = default;
	FORCEINLINE constexpr TMoveIterator& operator=(TMoveIterator&&)      = default;

	FORCEINLINE constexpr ~TMoveIterator() = default;

	template <typename T = IteratorType> requires (!CSameAs<TMoveIterator, TRemoveCVRef<T>> && CConstructibleFrom<IteratorType, T>)
	FORCEINLINE constexpr explicit TMoveIterator(T&& InValue) : Current(Forward<T>(InValue)) { }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, IteratorType>) TMoveIterator(const TMoveIterator<J>& InValue) : Current(InValue.Current) { }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, J>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<J&&, IteratorType>) TMoveIterator(TMoveIterator<J>&& InValue) : Current(MoveTemp(InValue).Current) { }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TMoveIterator& operator=(const TMoveIterator<J>& InValue) { Current = InValue.Current; return *this; }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<J&&, IteratorType> && CAssignableFrom<IteratorType&, J&&>)
	FORCEINLINE constexpr TMoveIterator& operator=(TMoveIterator<J>&& InValue) { Current = MoveTemp(InValue).Current; return *this; }

	template <CInputIterator J> requires (CSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.Current == RHS.Current; }

	template <CInputIterator J> requires (CSizedSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<J, IteratorType> operator<=>(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.Current <=> RHS.Current; }

	NODISCARD FORCEINLINE constexpr TIteratorRValueReferenceType<IteratorType> operator*()  const { return MoveTemp(*Current); }
	NODISCARD FORCEINLINE constexpr TIteratorPointerType<IteratorType>         operator->() const = delete;

	NODISCARD FORCEINLINE constexpr TIteratorRValueReferenceType<IteratorType> operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { return MoveTemp(Current[Index]); }

	FORCEINLINE constexpr TMoveIterator& operator++()                                                 { ++Current; return *this; }
	FORCEINLINE constexpr TMoveIterator& operator--() requires (CBidirectionalIterator<IteratorType>) { --Current; return *this; }

	FORCEINLINE constexpr void          operator++(int)                                                 {                      Current++;  }
	FORCEINLINE constexpr TMoveIterator operator++(int) requires       (CForwardIterator<IteratorType>) { return TMoveIterator(Current++); }
	FORCEINLINE constexpr TMoveIterator operator--(int) requires (CBidirectionalIterator<IteratorType>) { return TMoveIterator(Current--); }

	FORCEINLINE constexpr TMoveIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current += Offset; return *this; }
	FORCEINLINE constexpr TMoveIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current -= Offset; return *this; }

	NODISCARD friend FORCEINLINE constexpr TMoveIterator operator+(TMoveIterator Iter, ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { TMoveIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TMoveIterator operator+(ptrdiff Offset, TMoveIterator Iter) requires (CRandomAccessIterator<IteratorType>) { TMoveIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TMoveIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<IteratorType>) { TMoveIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator& LHS, const TMoveIterator& RHS) { return LHS.Current - RHS.Current; }

	NODISCARD FORCEINLINE constexpr const IteratorType& GetBase() const& { return          Current;  }
	NODISCARD FORCEINLINE constexpr       IteratorType  GetBase() &&     { return MoveTemp(Current); }

private:

	IteratorType Current;

};

static_assert(CRandomAccessIterator<TMoveIterator<int32*>>);

/** A sentinel adaptor for use with TMoveIterator. */
template <CSemiregular S>
class TMoveSentinel
{
public:

	using SentinelType = S;

	FORCEINLINE constexpr TMoveSentinel() = default;

	FORCEINLINE constexpr TMoveSentinel(const TMoveSentinel&)            = default;
	FORCEINLINE constexpr TMoveSentinel(TMoveSentinel&&)                 = default;
	FORCEINLINE constexpr TMoveSentinel& operator=(const TMoveSentinel&) = default;
	FORCEINLINE constexpr TMoveSentinel& operator=(TMoveSentinel&&)      = default;

	FORCEINLINE constexpr ~TMoveSentinel() = default;

	template <typename T = SentinelType> requires (!CSameAs<TMoveSentinel, TRemoveCVRef<T>> && CConstructibleFrom<SentinelType, T>)
	FORCEINLINE constexpr explicit TMoveSentinel(T&& InValue) : Current(Forward<T>(InValue)) { }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConstructibleFrom<SentinelType, const T&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const T&, SentinelType>) TMoveSentinel(const TMoveSentinel<T>& InValue) : Current(InValue.Current) { }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConstructibleFrom<SentinelType, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<T&&, SentinelType>) TMoveSentinel(TMoveSentinel<T>&& InValue) : Current(MoveTemp(InValue).Current) { }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConvertibleTo<const T&, SentinelType> && CAssignableFrom<SentinelType&, const T&>)
	FORCEINLINE constexpr TMoveSentinel& operator=(const TMoveSentinel<T>& InValue) { Current = InValue.Current; return *this; }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConvertibleTo<T&&, SentinelType> && CAssignableFrom<SentinelType&, T&&>)
	FORCEINLINE constexpr TMoveSentinel& operator=(TMoveSentinel<T>&& InValue) { Current = MoveTemp(InValue).Current; return *this; }

	template <CInputIterator I> requires (CSentinelFor<SentinelType, I>)
	NODISCARD FORCEINLINE constexpr bool operator==(const TMoveIterator<I>& InValue) const& { return Current == InValue.Current; }

	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD FORCEINLINE constexpr TCompareThreeWayResult<SentinelType, I> operator<=>(const TMoveIterator<I>& InValue) const& { return Current <=> InValue.Current; }

	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveSentinel& Sentinel, const TMoveIterator<I>& Iter) { return Sentinel.Current - Iter.Current; }

	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator<I>& Iter, const TMoveSentinel& Sentinel) { return Iter.Current - Sentinel.Current; }

	NODISCARD FORCEINLINE constexpr const SentinelType& GetBase() const& { return          Current;  }
	NODISCARD FORCEINLINE constexpr       SentinelType  GetBase() &&     { return MoveTemp(Current); }

private:

	SentinelType Current;

};

static_assert(CSizedSentinelFor<TMoveSentinel<int32*>, TMoveIterator<int32*>>);

struct FDefaultSentinel { explicit FDefaultSentinel() = default; };

inline constexpr FDefaultSentinel DefaultSentinel{ };

struct FUnreachableSentinel
{
	explicit FUnreachableSentinel() = default;

	template<CInputOrOutputIterator I>
	NODISCARD FORCEINLINE constexpr bool operator==(const I&) const& { return false; }
};

inline constexpr FUnreachableSentinel UnreachableSentinel{ };

/** An iterator adaptor that tracks the distance to the end of the range. */
template <CInputOrOutputIterator I>
class TCountedIterator final
{
public:

	using IteratorType = I;

	using ElementType = TIteratorElementType<I>;

#	if DO_CHECK
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<IteratorType>) : Length(1), MaxLength(0) { }
#	else
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<IteratorType>) = default;
#	endif

	FORCEINLINE constexpr TCountedIterator(const TCountedIterator&)            = default;
	FORCEINLINE constexpr TCountedIterator(TCountedIterator&&)                 = default;
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator&) = default;
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator&&)      = default;

	FORCEINLINE constexpr ~TCountedIterator() = default;

	template <typename T = IteratorType> requires (!CSameAs<TCountedIterator, TRemoveCVRef<T>> && CConstructibleFrom<IteratorType, T>)
	FORCEINLINE constexpr explicit TCountedIterator(T&& InValue, ptrdiff N) : Current(Forward<T>(InValue)), Length(N) { check_code({ MaxLength = N; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, IteratorType>) TCountedIterator(const TCountedIterator<J>& InValue) : Current(InValue.Current), Length(InValue.Num()) { check_code({ MaxLength = InValue.MaxLength; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, J>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<J&&, IteratorType>) TCountedIterator(TCountedIterator<J>&& InValue) : Current(MoveTemp(InValue).Current), Length(InValue.Num()) { check_code({ MaxLength = InValue.MaxLength; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator<J>& InValue) { Current = InValue.Current; Length = InValue.Num(); check_code({ MaxLength = InValue.MaxLength; }); return *this; }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<J&&, IteratorType> && CAssignableFrom<IteratorType&, J&&>)
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator<J>&& InValue) { Current = MoveTemp(InValue).Current; Length = InValue.Num(); check_code({ MaxLength = InValue.MaxLength; }); return *this; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length == RHS.Length; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length <=> RHS.Length; }

	NODISCARD FORCEINLINE constexpr bool operator==(FDefaultSentinel) const& { return Length == static_cast<ptrdiff>(0); }

	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(FDefaultSentinel) const& { return static_cast<ptrdiff>(0) <=> Length; }

	NODISCARD FORCEINLINE constexpr TIteratorReferenceType<IteratorType> operator*() { CheckThis(true); return *Current; }

	NODISCARD FORCEINLINE constexpr TIteratorReferenceType<IteratorType> operator*() const requires (CDereferenceable<const IteratorType>) { CheckThis(true); return *Current; }

	NODISCARD FORCEINLINE constexpr TIteratorPointerType<IteratorType> operator->() const requires (CContiguousIterator<IteratorType>) { CheckThis(false); return ToAddress(Current); }

	NODISCARD FORCEINLINE constexpr TIteratorReferenceType<IteratorType> operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE constexpr TCountedIterator& operator++()                                                 { ++Current; --Length; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator--() requires (CBidirectionalIterator<IteratorType>) { --Current; ++Length; CheckThis(); return *this; }

	FORCEINLINE constexpr auto             operator++(int)                                                 {                                           --Length; CheckThis(); return Current++; }
	FORCEINLINE constexpr TCountedIterator operator++(int) requires       (CForwardIterator<IteratorType>) { TCountedIterator Temp = *this; ++Current; --Length; CheckThis(); return Temp;      }
	FORCEINLINE constexpr TCountedIterator operator--(int) requires (CBidirectionalIterator<IteratorType>) { TCountedIterator Temp = *this; --Current; ++Length; CheckThis(); return Temp;      }

	FORCEINLINE constexpr TCountedIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current += Offset; Length -= Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current -= Offset; Length += Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(TCountedIterator Iter, ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(ptrdiff Offset, TCountedIterator Iter) requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TCountedIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = *this; Temp -= Offset; return Temp; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Length - RHS.Length; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, FDefaultSentinel) { LHS.CheckThis(); return -LHS.Num(); }
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(FDefaultSentinel, const TCountedIterator& RHS) { RHS.CheckThis(); return  RHS.Num(); }

	NODISCARD FORCEINLINE constexpr const IteratorType& GetBase() const& { CheckThis(); return          Current;  }
	NODISCARD FORCEINLINE constexpr       IteratorType  GetBase() &&     { CheckThis(); return MoveTemp(Current); }
	NODISCARD FORCEINLINE constexpr       ptrdiff           Num() const  { CheckThis(); return          Length;   }

private:

	IteratorType Current;
	ptrdiff      Length;

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

static_assert(CContiguousIterator<TCountedIterator<int32*>>);
static_assert(CSizedSentinelFor<FDefaultSentinel, TCountedIterator<int32*>>);

NAMESPACE_PRIVATE_BEGIN

/** An output iterator adapter that wraps a callable object. */
template <CMoveConstructible F>
class TOutputIterator final : private FNoncopyable
{
public:

	using Outputer = F;

private:

	class FIndirectionProxy : private FSingleton
	{
	public:

		FORCEINLINE constexpr FIndirectionProxy(TOutputIterator& InIter) : Iter(InIter) { check_code({ bIsProduced = false; }); }

#		if	DO_CHECK
		FORCEINLINE ~FIndirectionProxy()
		{
			checkf(bIsProduced, TEXT("Exception output, Ensures that the value is assigned to the output iterator."));
		}
#		endif

		template <typename T> requires (CInvocable<Outputer, T>)
		FORCEINLINE constexpr void operator=(T&& InValue) const
		{
			checkf(!bIsProduced, TEXT("Exception output, Ensure that no multiple values are assigned to the output iterator."));
			Invoke(Iter.Storage, Forward<T>(InValue));
			check_code({ bIsProduced = true; });
		}

	private:

		TOutputIterator& Iter;

#		if	DO_CHECK
		mutable bool bIsProduced;
#		endif

	};

	class FPostIncrementProxy : private FSingleton
	{
	public:

		FORCEINLINE constexpr FPostIncrementProxy(TOutputIterator& InIter) : Iter(InIter) { check_code({ bIsProduced = false; }); }

#		if	DO_CHECK
		FORCEINLINE ~FPostIncrementProxy()
		{
			checkf(bIsProduced, TEXT("Exception output, Ensures that the value is assigned to the output iterator."));
		}
#		endif

		NODISCARD FORCEINLINE constexpr FIndirectionProxy operator*() const
		{
			checkf(!bIsProduced, TEXT("Exception output, Ensure that no multiple values are assigned to the output iterator."));
			check_code({ bIsProduced = true; });
			return FIndirectionProxy(Iter);
		}

	private:

		TOutputIterator& Iter;

#		if	DO_CHECK
		mutable bool bIsProduced;
#		endif

	};

public:

	FORCEINLINE constexpr TOutputIterator() requires (CDefaultConstructible<Outputer>) { check_code({ bIsProduced = false; }); }

	template <typename T> requires (!CSameAs<TOutputIterator, TRemoveCVRef<T>> && CConstructibleFrom<Outputer, T>)
	FORCEINLINE constexpr explicit TOutputIterator(T&& InOutputer) : Storage(Forward<T>(InOutputer)) { check_code({ bIsProduced = false; }); }

	NODISCARD FORCEINLINE constexpr FIndirectionProxy operator*()
	{
		checkf(!bIsProduced, TEXT("Exception output, Ensure that no multiple values are assigned to the output iterator."));
		check_code({ bIsProduced = true; });
		return FIndirectionProxy(*this);
	}

	FORCEINLINE constexpr TOutputIterator& operator++() { check_code({ bIsProduced = false; }); return *this; }

	FORCEINLINE constexpr FPostIncrementProxy operator++(int)
	{
		checkf(!bIsProduced, TEXT("Exception output, Ensure that no multiple values are assigned to the output iterator."));
		return FPostIncrementProxy(*this);
	}

	NODISCARD FORCEINLINE constexpr const Outputer& GetOutputer() const& { return Storage; }
	NODISCARD FORCEINLINE constexpr       Outputer  GetOutputer() &&     { return Storage; }

private:

	Outputer Storage;

#	if DO_CHECK
	bool bIsProduced;
#	endif

};

static_assert(COutputIterator<TOutputIterator<void(*)(int32)>, int32>);

template <typename F>
TOutputIterator(F) -> TOutputIterator<F>;

NAMESPACE_PRIVATE_END

/** Creates a TReverseIterator of type inferred from the argument. */
template <typename I> requires (CBidirectionalIterator<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeReverseIterator(I&& Iter)
{
	return TReverseIterator<TDecay<I>>(Forward<I>(Iter));
}

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

/** Creates a TCountedIterator of type inferred from the argument. */
template <typename I> requires (CInputOrOutputIterator<TDecay<I>> && CConstructibleFrom<TDecay<I>, I>)
NODISCARD FORCEINLINE constexpr auto MakeCountedIterator(I&& Iter, ptrdiff N)
{
	return TCountedIterator<TDecay<I>>(Forward<I>(Iter), N);
}

/** Creates an iterator adapter inserted in the front of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeFrontInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TOutputIterator([&Container]<typename T>(T&& A) { Container.PushFront(Forward<T>(A)); });
}

/** Creates an iterator adapter inserted in the back of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeBackInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TOutputIterator([&Container]<typename T>(T&& A) { Container.PushBack(Forward<T>(A)); });
}

/** Creates an iterator adapter inserted in the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeInserter(C& Container, const typename C::ConstIterator& InIter)
{
	return NAMESPACE_PRIVATE::TOutputIterator([&Container, Iter = InIter]<typename T>(T&& A) mutable { Iter = Container.Insert(Iter, Forward<T>(A)); });
}

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

#define ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT public:                        \
	NODISCARD FORCEINLINE constexpr auto begin()       { return Begin(); } \
	NODISCARD FORCEINLINE constexpr auto begin() const { return Begin(); } \
	NODISCARD FORCEINLINE constexpr auto end()         { return End();   } \
	NODISCARD FORCEINLINE constexpr auto end()   const { return End();   }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
