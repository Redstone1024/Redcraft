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
concept CSizedSentinelFor = CSentinelFor<S, I> && CPartiallyOrdered<S, I> && !bDisableSizedSentinelFor<TRemoveCV<S>, TRemoveCV<I>>
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

	FORCEINLINE constexpr explicit TReverseIterator(IteratorType InValue) : Current(InValue) { }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType>)
	FORCEINLINE constexpr TReverseIterator(const TReverseIterator<J>& InValue) : Current(InValue.GetBase()) { }

	template <CBidirectionalIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TReverseIterator& operator=(const TReverseIterator<J>& InValue) { Current = InValue.GetBase(); return *this; }

	template <CBidirectionalIterator J> requires (CSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return LHS.GetBase() == RHS.GetBase(); }

	template <CBidirectionalIterator J> requires (CSizedSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<J, IteratorType> operator<=>(const TReverseIterator& LHS, const TReverseIterator<J>& RHS) { return RHS.GetBase() <=> LHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr ElementType& operator*()  const { IteratorType Temp = GetBase(); return *--Temp; }
	NODISCARD FORCEINLINE constexpr ElementType* operator->() const { return AddressOf(operator*());                 }

	NODISCARD FORCEINLINE constexpr ElementType& operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { return GetBase()[-Index - 1]; }

	FORCEINLINE constexpr TReverseIterator& operator++() { --Current; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator--() { ++Current; return *this; }

	FORCEINLINE constexpr TReverseIterator operator++(int) { TReverseIterator Temp = *this; --Current; return Temp; }
	FORCEINLINE constexpr TReverseIterator operator--(int) { TReverseIterator Temp = *this; ++Current; return Temp; }

	FORCEINLINE constexpr TReverseIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current -= Offset; return *this; }
	FORCEINLINE constexpr TReverseIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current += Offset; return *this; }

	NODISCARD friend FORCEINLINE constexpr TReverseIterator operator+(TReverseIterator Iter, ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = Iter; Temp -= Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TReverseIterator operator+(ptrdiff Offset, TReverseIterator Iter) requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = Iter; Temp -= Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TReverseIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<IteratorType>) { TReverseIterator Temp = *this; Temp += Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TReverseIterator& LHS, const TReverseIterator& RHS) { return RHS.GetBase() - LHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr IteratorType GetBase() const { return Current; }

private:

	IteratorType Current;

};

static_assert(CRandomAccessIterator<TReverseIterator<int32*>>);

template <typename I>
TReverseIterator(I) -> TReverseIterator<I>;

template <typename I, typename J> requires (!CSizedSentinelFor<I, J>)
inline constexpr bool bDisableSizedSentinelFor<TReverseIterator<I>, TReverseIterator<J>> = true;

/** An iterator adaptor which dereferences to an rvalue reference. */
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

	FORCEINLINE constexpr explicit TMoveIterator(IteratorType InValue) : Current(InValue) { }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType>)
	FORCEINLINE constexpr TMoveIterator(const TMoveIterator<J>& InValue) : Current(InValue.GetBase()) { }

	template <CInputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TMoveIterator& operator=(const TMoveIterator<J>& InValue) { Current = InValue.GetBase(); return *this; }

	template <CInputIterator J> requires (CSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.GetBase() == RHS.GetBase(); }

	template <CInputIterator J> requires (CSizedSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<J, IteratorType> operator<=>(const TMoveIterator& LHS, const TMoveIterator<J>& RHS) { return LHS.GetBase() <=> RHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr ElementType&& operator*()  const { return MoveTemp(*GetBase()); }
	NODISCARD FORCEINLINE constexpr ElementType*  operator->() const = delete;

	NODISCARD FORCEINLINE constexpr ElementType&& operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { return MoveTemp(GetBase()[Index]); }

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

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator& LHS, const TMoveIterator& RHS) { return LHS.GetBase() - RHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr IteratorType GetBase() const { return Current; }

private:

	IteratorType Current;

};

static_assert(CRandomAccessIterator<TMoveIterator<int32*>>);

template <typename I>
TMoveIterator(I) -> TMoveIterator<I>;

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

	FORCEINLINE constexpr explicit TMoveSentinel(SentinelType InValue) : Last(InValue) { }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConvertibleTo<const T&, SentinelType>)
	FORCEINLINE constexpr TMoveSentinel(const TMoveSentinel<T>& InValue) : Last(InValue.GetBase()) { }

	template <CSemiregular T> requires (!CSameAs<SentinelType, T> && CConvertibleTo<const T&, SentinelType> && CAssignableFrom<SentinelType&, const T&>)
	FORCEINLINE constexpr TMoveSentinel& operator=(const TMoveSentinel<T>& InValue) { Last = InValue.GetBase(); return *this; }

	template <CInputIterator I> requires (CSentinelFor<SentinelType, I>)
	NODISCARD FORCEINLINE constexpr bool operator==(const TMoveIterator<I>& InValue) const& { return GetBase() == InValue.GetBase(); }
	
	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD FORCEINLINE constexpr TCompareThreeWayResult<SentinelType, I> operator<=>(const TMoveIterator<I>& InValue) const& { return GetBase() <=> InValue.GetBase(); }

	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveSentinel& Sentinel, const TMoveIterator<I>& Iter) { return Sentinel.GetBase() - Iter.GetBase(); }

	template <CInputIterator I> requires (CSizedSentinelFor<SentinelType, I>)
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TMoveIterator<I>& Iter, const TMoveSentinel& Sentinel) { return Iter.GetBase() - Sentinel.GetBase(); }

	NODISCARD FORCEINLINE constexpr SentinelType GetBase() const { return Last; }

private:

	SentinelType Last;

};

static_assert(CSizedSentinelFor<TMoveSentinel<int32*>, TMoveIterator<int32*>>);

template <typename I>
TMoveSentinel(I) -> TMoveSentinel<I>;

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
	FORCEINLINE constexpr TCountedIterator() requires CDefaultConstructible<IteratorType> : Length(1), MaxLength(0) { };
#	else
	FORCEINLINE constexpr TCountedIterator() requires CDefaultConstructible<IteratorType> = default;
#	endif

	FORCEINLINE constexpr TCountedIterator(const TCountedIterator&)            = default;
	FORCEINLINE constexpr TCountedIterator(TCountedIterator&&)                 = default;
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator&) = default;
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator&&)      = default;

	FORCEINLINE constexpr explicit TCountedIterator(IteratorType InValue, ptrdiff N) : Current(InValue), Length(N) { check_code({ MaxLength = N; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType>)
	FORCEINLINE constexpr TCountedIterator(const TCountedIterator<J>& InValue) : Current(InValue.GetBase()), Length(InValue.Num()) { check_code({ MaxLength = InValue.Max(); }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator<J>& InValue) { Current = InValue.GetBase(); Length = InValue.Num(); check_code({ MaxLength = InValue.Max(); }); return *this; }

	template <CInputOrOutputIterator J> requires (CSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.GetBase() == RHS.GetBase(); }

	template <CInputOrOutputIterator J> requires (CSizedSentinelFor<J, IteratorType>)
	NODISCARD friend FORCEINLINE constexpr TCompareThreeWayResult<IteratorType, J> operator<=>(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.GetBase() <=> RHS.GetBase(); }

	NODISCARD FORCEINLINE constexpr bool operator==(FDefaultSentinel) const& { return Length == static_cast<ptrdiff>(0); }

	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(FDefaultSentinel) const& { return static_cast<ptrdiff>(0) <=> Length; }

	NODISCARD FORCEINLINE constexpr decltype(auto) operator*()  const { CheckThis(true); return *Current;               }
	NODISCARD FORCEINLINE constexpr decltype(auto) operator->() const { CheckThis(true); return AddressOf(operator*()); }

	NODISCARD FORCEINLINE constexpr decltype(auto) operator[](ptrdiff Index) const requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE constexpr TCountedIterator& operator++()                                                 { ++Current; --Length; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator--() requires (CBidirectionalIterator<IteratorType>) { --Current; ++Length; CheckThis(); return *this; }

	FORCEINLINE constexpr decltype(auto)   operator++(int)                                                 {                                           --Length; CheckThis(); return Current++; }
	FORCEINLINE constexpr TCountedIterator operator++(int) requires       (CForwardIterator<IteratorType>) { TCountedIterator Temp = *this; ++Current; --Length; CheckThis(); return Temp;      }
	FORCEINLINE constexpr TCountedIterator operator--(int) requires (CBidirectionalIterator<IteratorType>) { TCountedIterator Temp = *this; --Current; ++Length; CheckThis(); return Temp;      }

	FORCEINLINE constexpr TCountedIterator& operator+=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current += Offset; Length -= Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator-=(ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { Current -= Offset; Length += Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(TCountedIterator Iter, ptrdiff Offset) requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(ptrdiff Offset, TCountedIterator Iter) requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TCountedIterator operator-(ptrdiff Offset) const requires (CRandomAccessIterator<IteratorType>) { TCountedIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, const TCountedIterator& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.GetBase() - RHS.GetBase(); }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, FDefaultSentinel) { CheckThis(); return -LHS.Num(); }
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(FDefaultSentinel, const TCountedIterator& RHS) { CheckThis(); return  RHS.Num(); }

	NODISCARD FORCEINLINE constexpr explicit operator       ElementType*()       requires (CContiguousIterator<IteratorType> && !CConst<ElementType>) { CheckThis(); return Current; }
	NODISCARD FORCEINLINE constexpr explicit operator const ElementType*() const requires (CContiguousIterator<IteratorType>)                         { CheckThis(); return Current; }

	NODISCARD FORCEINLINE constexpr IteratorType GetBase() const { CheckThis(); return Current; }
	NODISCARD FORCEINLINE constexpr ptrdiff          Num() const { CheckThis(); return Length;  }

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

/** Overloads the Begin algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr decltype(auto) Begin(initializer_list<T> Container)
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

/** Overloads the End algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr decltype(auto) End(initializer_list<T> Container)
{
	return Container.end();
}

/** @return The reverse iterator to the beginning of a container. */
template <typename T> requires (requires(T&& Container) { { Container.RBegin() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) RBegin(T&& Container)
{
	return Container.RBegin();
}

/** Overloads the RBegin algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) RBegin(      T(&  Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) RBegin(      T(&& Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) RBegin(const T(&  Container)[N]) { return TReverseIterator(End(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) RBegin(const T(&& Container)[N]) { return TReverseIterator(End(Container)); }

/** Overloads the RBegin algorithm for T::rbegin(). */
template <typename T>
FORCEINLINE constexpr decltype(auto) RBegin(initializer_list<T> Container)
{
	return TReverseIterator(Container.end());
}

/** @return The reverse iterator to the end of a container. */
template <typename T> requires (requires(T&& Container) { { Container.REnd() } -> CForwardIterator; })
FORCEINLINE constexpr decltype(auto) REnd(T&& Container)
{
	return Container.REnd();
}

/** Overloads the REnd algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) REnd(      T(&  Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) REnd(      T(&& Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) REnd(const T(&  Container)[N]) { return TReverseIterator(Begin(Container)); }
template <typename T, size_t N> FORCEINLINE constexpr decltype(auto) REnd(const T(&& Container)[N]) { return TReverseIterator(Begin(Container)); }

/** Overloads the REnd algorithm for T::end(). */
template <typename T>
FORCEINLINE constexpr decltype(auto) REnd(initializer_list<T> Container)
{
	return TReverseIterator(Container.begin());
}

NAMESPACE_END(Iteration)

#define ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT public:                                  \
	NODISCARD FORCEINLINE constexpr decltype(auto) begin()       { return Begin(); } \
	NODISCARD FORCEINLINE constexpr decltype(auto) begin() const { return Begin(); } \
	NODISCARD FORCEINLINE constexpr decltype(auto) end()         { return End();   } \
	NODISCARD FORCEINLINE constexpr decltype(auto) end()   const { return End();   }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
