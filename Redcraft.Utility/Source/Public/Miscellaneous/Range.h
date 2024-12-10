#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Iterator.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// NOTE: The range that holds the object is called a container, and the range that only references the object is called a view.

template <typename R>
inline constexpr bool bEnableBorrowedRange = false;

NAMESPACE_BEGIN(Range)

/** @return The iterator to the beginning of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.Begin() } -> CInputOrOutputIterator; })
NODISCARD FORCEINLINE constexpr auto Begin(T&& Container)
{
	return Container.Begin();
}

/** Overloads the Begin algorithm for arrays. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& CArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr auto Begin(T&& Container)
{
	return Container + 0;
}

/** Overloads the Begin algorithm for initializer_list. */
template <typename T>
NODISCARD FORCEINLINE constexpr auto Begin(initializer_list<T>& Container)
{
	return Container.begin();
}

NAMESPACE_END(Range)

template <typename R>
using TRangeIteratorType = decltype(Range::Begin(DeclVal<R&>()));

NAMESPACE_BEGIN(Range)

/** @return The iterator to the end of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.End() } -> CSentinelFor<TRangeIteratorType<T>>; })
NODISCARD FORCEINLINE constexpr auto End(T&& Container)
{
	return Container.End();
}

/** Overloads the End algorithm for arrays. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& CBoundedArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr auto End(T&& Container)
{
	return Container + TExtent<TRemoveReference<T>>;
}

/** Overloads the End algorithm for initializer_list. */
template <typename T>
NODISCARD FORCEINLINE constexpr auto End(initializer_list<T>& Container)
{
	return Container.end();
}

NAMESPACE_END(Range)

template <typename R>
using TRangeSentinelType = decltype(Range::End(DeclVal<R&>()));

NAMESPACE_BEGIN(Range)

/** @return The reverse iterator to the beginning of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.RBegin() } -> CInputOrOutputIterator; })
NODISCARD FORCEINLINE constexpr auto RBegin(T&& Container)
{
	return Container.RBegin();
}

/** Overloads the RBegin algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.RBegin() } -> CInputOrOutputIterator; }
	&& (CSameAs<TRangeIteratorType<T>, TRangeSentinelType<T>> && CBidirectionalIterator<TRangeIteratorType<T>>))
NODISCARD FORCEINLINE constexpr auto RBegin(T&& Container)
{
	return MakeReverseIterator(Range::End(Forward<T>(Container)));
}

/** @return The reverse iterator to the end of a container. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.REnd() } -> CSentinelFor<decltype(Range::RBegin(DeclVal<T&>()))>; })
NODISCARD FORCEINLINE constexpr auto REnd(T&& Container)
{
	return Container.REnd();
}

/** Overloads the REnd algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.REnd() } -> CSentinelFor<decltype(Range::RBegin(DeclVal<T&>()))>; }
	&& (CSameAs<TRangeIteratorType<T>, TRangeSentinelType<T>> && CBidirectionalIterator<TRangeIteratorType<T>>))
NODISCARD FORCEINLINE constexpr auto REnd(T&& Container)
{
	return MakeReverseIterator(Range::Begin(Forward<T>(Container)));
}

NAMESPACE_END(Range)

NAMESPACE_PRIVATE_BEGIN

template <typename R>           struct TRangeElementType       { using Type = typename R::ElementType; };
template <typename T>           struct TRangeElementType<T[ ]> { using Type = T;                       };
template <typename T, size_t N> struct TRangeElementType<T[N]> { using Type = T;                       };

NAMESPACE_PRIVATE_END

template <typename R>
using TRangeElementType = typename NAMESPACE_PRIVATE::TRangeElementType<TRemoveCVRef<R>>::Type;

template <typename R>
using TRangeReferenceType = TIteratorReferenceType<TRangeIteratorType<R>>;

template <typename R>
using TRangeRValueReferenceType = TIteratorRValueReferenceType<TRangeIteratorType<R>>;

NAMESPACE_BEGIN(Range)

/** @return The pointer to the container element storage. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& requires(T&& Container) { { Container.GetData() } -> CSameAs<TAddPointer<TRangeReferenceType<T>>>; })
NODISCARD FORCEINLINE constexpr auto GetData(T&& Container)
{
	return Container.GetData();
}

/** Overloads the GetData algorithm for synthesized. */
template <typename T> requires ((CLValueReference<T> || bEnableBorrowedRange<TRemoveCVRef<T>>)
	&& !requires(T&& Container) { { Container.GetData() } -> CSameAs<TAddPointer<TRangeReferenceType<T>>>; }
	&&  requires(T&& Container) { { Range::Begin(Forward<T>(Container)) } -> CContiguousIterator; })
NODISCARD FORCEINLINE constexpr auto GetData(T&& Container)
{
	return ToAddress(Range::Begin(Forward<T>(Container)));
}

NAMESPACE_END(Range)

template <typename R>
inline constexpr bool bDisableSizedRange = false;

NAMESPACE_BEGIN(Range)

/** @return The number of elements in the container. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; })
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return Container.Num();
}

/** Overloads the Num algorithm for arrays. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& CBoundedArray<TRemoveReference<T>>)
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return TExtent<TRemoveReference<T>>;
}

/** Overloads the Num algorithm for synthesized. */
template <typename T> requires (!bDisableSizedRange<TRemoveCVRef<T>>
	&& !requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; } && !CBoundedArray<TRemoveReference<T>>
	&& CSizedSentinelFor<TRangeIteratorType<T>, TRangeSentinelType<T>> && CForwardIterator<TRangeIteratorType<T>>)
NODISCARD FORCEINLINE constexpr size_t Num(T&& Container)
{
	return Range::End(Forward<T>(Container)) - Range::Begin(Forward<T>(Container));
}

/** @return true if the container is empty, false otherwise. */
template <typename T> requires (requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; })
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Container.IsEmpty();
}

/** Overloads the IsEmpty algorithm for synthesized. */
template <typename T> requires ((CBoundedArray<TRemoveReference<T>>
	||  requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; })
	&& !requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; })
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Range::Num(Forward<T>(Container)) == 0;
}

/** Overloads the IsEmpty algorithm for synthesized. */
template <typename T> requires (!CBoundedArray<TRemoveReference<T>>
	&& !requires(T&& Container) { { Container.Num() } -> CSameAs<size_t>; }
	&& !requires(T&& Container) { { Container.IsEmpty() } -> CBooleanTestable; }
	&& CForwardIterator<TRangeIteratorType<T>>)
NODISCARD FORCEINLINE constexpr bool IsEmpty(T&& Container)
{
	return Range::End(Forward<T>(Container)) == Range::Begin(Forward<T>(Container));
}

NAMESPACE_END(Range)

template <typename R>
concept CRange =
	requires(R Range)
	{
		typename TRangeIteratorType<R>;
		typename TRangeSentinelType<R>;
	}
	&& CInputOrOutputIterator<TRangeIteratorType<R>>
	&& CSentinelFor<TRangeSentinelType<R>, TRangeIteratorType<R>>;

template <typename R>
concept CBorrowedRange = CRange<R> && (CLValueReference<R> || bEnableBorrowedRange<TRemoveCVRef<R>>);

template <typename R>
concept CSizedRange = CRange<R>
	&& requires(R Range)
	{
		{ Range::Num(Range) } -> CConvertibleTo<size_t>;
	};

template <typename R>
concept CInputRange = CRange<R> && CInputIterator<TRangeIteratorType<R>>;

template <typename R, typename T>
concept COutputRange = CRange<R> && COutputIterator<TRangeIteratorType<R>, T>;

template <typename R>
concept CForwardRange = CInputRange<R> && CForwardIterator<TRangeIteratorType<R>>;

template <typename R>
concept CBidirectionalRange = CForwardRange<R> && CBidirectionalIterator<TRangeIteratorType<R>>;

template <typename R>
concept CRandomAccessRange = CBidirectionalRange<R> && CRandomAccessIterator<TRangeIteratorType<R>>;

template <typename R>
concept CContiguousRange = CRandomAccessRange<R> && CContiguousIterator<TRangeIteratorType<R>>
	&& requires(R& Range)
	{
		{ Range::GetData(Range) } -> CSameAs<TAddPointer<TRangeReferenceType<R>>>;
	};

template <typename R>
concept CCommonRange = CRange<R> && CSameAs<TRangeIteratorType<R>, TRangeSentinelType<R>>;

static_assert(CContiguousRange<int[8]>);
static_assert(    CCommonRange<int[8]>);

NAMESPACE_BEGIN(Range)

template <typename T> requires (CClass<T> && CSameAs<T, TRemoveCV<T>>)
class TViewInterface
{
public:

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr auto Data()       requires (CContiguousRange<      T>) { return Range::GetData(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto Data() const requires (CContiguousRange<const T>) { return Range::GetData(static_cast<const T&>(*this)); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr auto Begin()       requires (CRange<      T>) { return Range::Begin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto End()         requires (CRange<      T>) { return Range::End  (static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const T>) { return Range::Begin(static_cast<const T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto End()   const requires (CRange<const T>) { return Range::End  (static_cast<const T&>(*this)); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr auto RBegin()       requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return Range::RBegin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto REnd()         requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return Range::REnd  (static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto RBegin() const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return Range::RBegin(static_cast<const T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto REnd()   const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return Range::REnd  (static_cast<const T&>(*this)); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CSizedRange<      T>) { return Range::Num(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<const T>) { return Range::Num(static_cast<const T&>(*this)); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty()       requires (CSizedRange<      T> || CForwardRange<      T>) { return Range::IsEmpty(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const requires (CSizedRange<const T> || CForwardRange<const T>) { return Range::IsEmpty(static_cast<const T&>(*this)); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr explicit operator bool()       requires (CSizedRange<      T> || CForwardRange<      T>) { return !Range::IsEmpty(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const requires (CSizedRange<const T> || CForwardRange<const T>) { return !Range::IsEmpty(static_cast<const T&>(*this)); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr decltype(auto) operator[](size_t Index)       requires (CRandomAccessRange<      T>) { return Range::Begin(static_cast<      T&>(*this))[Index]; }
	NODISCARD FORCEINLINE constexpr decltype(auto) operator[](size_t Index) const requires (CRandomAccessRange<const T>) { return Range::Begin(static_cast<const T&>(*this))[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr decltype(auto) Front()       requires (CForwardRange<      T>)                                { return  *Range::Begin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Front() const requires (CForwardRange<const T>)                                { return  *Range::Begin(static_cast<const T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Back()        requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return *Range::RBegin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Back()  const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return *Range::RBegin(static_cast<const T&>(*this)); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	FORCEINLINE constexpr TViewInterface() = default;

	FORCEINLINE constexpr TViewInterface(const TViewInterface&)            = default;
	FORCEINLINE constexpr TViewInterface(TViewInterface&&)                 = default;
	FORCEINLINE constexpr TViewInterface& operator=(const TViewInterface&) = default;
	FORCEINLINE constexpr TViewInterface& operator=(TViewInterface&&)      = default;

	FORCEINLINE constexpr ~TViewInterface() = default;

	friend T;
};

NAMESPACE_END(Range)

template <typename R> requires (bEnableBorrowedRange<R>)
inline constexpr bool bEnableBorrowedRange<Range::TViewInterface<R>> = true;

template <typename V>
concept CView = CRange<V> && CMovable<V> && CDerivedFrom<V, Range::TViewInterface<TRemoveCVRef<V>>>;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsInitializerList                      : FFalse { };
template <typename T> struct TIsInitializerList<initializer_list<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename R>
concept CViewableRange = CRange<R>
	&& ((CView<TRemoveCVRef<R>> && CConstructibleFrom<TRemoveCVRef<R>, R>)
	|| (!CView<TRemoveCVRef<R>> && (CLValueReference<R> || (CMovable<TRemoveReference<R>>
	&& !NAMESPACE_PRIVATE::TIsInitializerList<TRemoveCVRef<R>>::Value))));

NAMESPACE_BEGIN(Range)

/** A view type that produces a view of no elements of a particular type. */
template <CObject T>
class TEmptyView : public TViewInterface<TEmptyView<T>>
{
public:

	using ElementType = T;
	using Reference   = T&;
	using Iterator    = T*;
	using Sentinel    = T*;

	FORCEINLINE constexpr TEmptyView() = default;

	NODISCARD static FORCEINLINE constexpr Iterator Begin()   { return nullptr; }
	NODISCARD static FORCEINLINE constexpr Sentinel End()     { return nullptr; }
	NODISCARD static FORCEINLINE constexpr T*       GetData() { return nullptr; }
	NODISCARD static FORCEINLINE constexpr size_t   Num()     { return 0;       }
	NODISCARD static FORCEINLINE constexpr bool     IsEmpty() { return true;    }

};

static_assert(CContiguousRange<TEmptyView<int>>);
static_assert(    CCommonRange<TEmptyView<int>>);
static_assert(     CSizedRange<TEmptyView<int>>);
static_assert(           CView<TEmptyView<int>>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TEmptyView<T>> = true;

NAMESPACE_BEGIN(Range)

/** A view type that contains exactly one element of a specified value. */
template <CObject T> requires (CMoveConstructible<T>)
class TSingleView : public TViewInterface<TSingleView<T>>
{
public:

	using ElementType = T;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator =       T*;
	using ConstIterator = const T*;

	using      Sentinel =       T*;
	using ConstSentinel = const T*;

	FORCEINLINE constexpr TSingleView() requires (CDefaultConstructible<T>) = default;

	FORCEINLINE constexpr explicit TSingleView(const T& InValue) requires (CCopyConstructible<T>) : Value(InValue) { }

	FORCEINLINE constexpr explicit TSingleView(T&& InValue) : Value(MoveTemp(InValue)) { }

	template <typename... Ts> requires (CConstructibleFrom<T, Ts...>)
	FORCEINLINE explicit TSingleView(FInPlace, Ts&&... Args) : Value(Forward<Ts>(Args)...) { }

	FORCEINLINE constexpr      Iterator Begin()       { return GetData();     }
	FORCEINLINE constexpr ConstIterator Begin() const { return GetData();     }
	FORCEINLINE constexpr      Sentinel End()         { return GetData() + 1; }
	FORCEINLINE constexpr ConstSentinel End()   const { return GetData() + 1; }

	NODISCARD FORCEINLINE constexpr       T* GetData()       { return AddressOf(Value); }
	NODISCARD FORCEINLINE constexpr const T* GetData() const { return AddressOf(Value); }

	NODISCARD static FORCEINLINE constexpr size_t Num()     { return 1;     }
	NODISCARD static FORCEINLINE constexpr bool   IsEmpty() { return false; }

private:

	NO_UNIQUE_ADDRESS T Value;

};

template <typename T>
TSingleView(T) -> TSingleView<T>;

static_assert(CContiguousRange<TSingleView<int>>);
static_assert(    CCommonRange<TSingleView<int>>);
static_assert(     CSizedRange<TSingleView<int>>);
static_assert(           CView<TSingleView<int>>);

/** A view type that generates a sequence of elements by repeatedly incrementing an initial value. Can be either bounded or unbounded. */
template <CWeaklyIncrementable W, CWeaklyEqualityComparable<W> S = FUnreachableSentinel> requires (CSemiregular<S> && CCopyable<W>)
class TIotaView : public TViewInterface<TIotaView<W, S>>
{
private:

	class FSentinelImpl;

public:

	using ElementType = W;

	using Reference = const W&;

	class Iterator;

	using Sentinel = TConditional<CSameAs<W, S>, Iterator, FSentinelImpl>;

	FORCEINLINE constexpr TIotaView() requires (CDefaultConstructible<W>) = default;

	FORCEINLINE constexpr explicit TIotaView(W InValue) requires (CDefaultConstructible<S>) : First(InValue), Last() { }

	FORCEINLINE constexpr explicit TIotaView(TIdentity<W> InValue, TIdentity<S> InLast) : First(InValue), Last(InLast) { }

	FORCEINLINE constexpr explicit TIotaView(Iterator InFirst, Sentinel InLast) : First(InFirst.Value), Last(InLast.Value) { }

	FORCEINLINE constexpr explicit TIotaView(Iterator InFirst, FUnreachableSentinel) requires (CSameAs<S, FUnreachableSentinel>) : First(InFirst.Value) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Iterator(First); }

	NODISCARD FORCEINLINE constexpr Sentinel End() const { return Sentinel(Last); }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires ((CIntegral<W> && CIntegral<S>) || CSizedSentinelFor<S, W>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

private:

	NO_UNIQUE_ADDRESS W First;
	NO_UNIQUE_ADDRESS S Last;

public:

	class Iterator final
	{
	public:

		using ElementType = TRemoveCV<W>;

		FORCEINLINE constexpr Iterator() requires (CDefaultConstructible<W>) = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const Iterator& LHS, const Iterator& RHS) requires (CEqualityComparable<W>) { return LHS.Value == RHS.Value; }

		NODISCARD FORCEINLINE constexpr       W  operator*()  const { return           Value;  }
		NODISCARD FORCEINLINE constexpr const W* operator->() const { return AddressOf(Value); }

		FORCEINLINE constexpr Iterator& operator++() { ++Value; return *this; }

		FORCEINLINE constexpr Iterator operator++(int) { Iterator Temp = *this; ++Value; return Temp; }

	private:

		W Value;

		constexpr explicit Iterator(W InValue) : Value(InValue) { }

		friend FSentinelImpl;

		friend TIotaView;
	};

private:

	class FSentinelImpl final
	{
	public:

		FORCEINLINE constexpr FSentinelImpl() = default;

		NODISCARD FORCEINLINE constexpr bool operator==(const Iterator& InValue) const& { return Value == InValue.Value; }

	private:

		S Value;

		FORCEINLINE constexpr FSentinelImpl(S InValue) : Value(InValue) { }

		friend TIotaView;
	};

};

template <typename T, typename U>
TIotaView(T, U) -> TIotaView<T, U>;

static_assert(CForwardRange<TIotaView<int>>);
static_assert(        CView<TIotaView<int>>);

NAMESPACE_END(Range)

template <typename T, typename U>
constexpr bool bEnableBorrowedRange<Range::TIotaView<T, U>> = true;

NAMESPACE_BEGIN(Range)

/** A view type that generates a sequence of elements by repeatedly producing the same value. Can be either bounded or unbounded. */
template <CObject W, bool bIsUnreachable = true> requires (CMoveConstructible<W> && CSameAs<W, TRemoveCV<W>>)
class TRepeatView : public TViewInterface<TRepeatView<W, bIsUnreachable>>
{
public:

	using ElementType = W;

	using Reference = const W&;

	class Iterator;

	using Sentinel = TConditional<bIsUnreachable, FUnreachableSentinel, Iterator>;

	FORCEINLINE constexpr TRepeatView() requires CDefaultConstructible<W> = default;

	FORCEINLINE constexpr explicit TRepeatView(W InValue) requires (bIsUnreachable) : Value(MoveTemp(InValue)) { }

	FORCEINLINE constexpr explicit TRepeatView(W InValue, size_t InCount) requires (!bIsUnreachable) : Value(MoveTemp(InValue)), Count(InCount) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Iterator(*this, 0); }

	NODISCARD FORCEINLINE constexpr Sentinel End() const
	{
		if constexpr (bIsUnreachable)
		{
			return UnreachableSentinel;
		}

		else return Sentinel(*this, Count);
	}

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (!bIsUnreachable) { return Count; }

private:

	using FSizeType = TConditional<bIsUnreachable, FUnreachableSentinel, size_t>;

	NO_UNIQUE_ADDRESS W Value;

	NO_UNIQUE_ADDRESS FSizeType Count;

public:

	class Iterator final
	{
	public:

		using ElementType = W;

		FORCEINLINE constexpr Iterator() requires (CDefaultConstructible<W>) = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const Iterator& LHS, const Iterator& RHS) { return LHS.Current == RHS.Current; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const Iterator& LHS, const Iterator& RHS) { return LHS.Current <=> RHS.Current; }

		NODISCARD FORCEINLINE constexpr const W& operator*()  const { return           Owner->Value;  }
		NODISCARD FORCEINLINE constexpr const W* operator->() const { return AddressOf(Owner->Value); }

		NODISCARD FORCEINLINE constexpr const W& operator[](ptrdiff Index) const { return *(*this + Index); }

		FORCEINLINE constexpr Iterator& operator++() { ++Current; return *this; }
		FORCEINLINE constexpr Iterator& operator--() { --Current; return *this; }

		FORCEINLINE constexpr Iterator operator++(int) { Iterator Temp = *this; --Current; return Temp; }
		FORCEINLINE constexpr Iterator operator--(int) { Iterator Temp = *this; ++Current; return Temp; }

		FORCEINLINE constexpr Iterator& operator+=(ptrdiff Offset) { Current -= Offset; return *this; }
		FORCEINLINE constexpr Iterator& operator-=(ptrdiff Offset) { Current += Offset; return *this; }

		NODISCARD friend FORCEINLINE constexpr Iterator operator+(Iterator Iter, ptrdiff Offset) { Iterator Temp = Iter; Temp -= Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr Iterator operator+(ptrdiff Offset, Iterator Iter) { Iterator Temp = Iter; Temp -= Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr Iterator operator-(ptrdiff Offset) const { Iterator Temp = *this; Temp += Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const Iterator& LHS, const Iterator& RHS) { return RHS.Current - LHS.Current; }

	private:

		const TRepeatView* Owner;

		NO_UNIQUE_ADDRESS size_t Current;

		FORCEINLINE constexpr Iterator(const TRepeatView& InOwner, size_t InCurrent) : Owner(&InOwner), Current(InCurrent) { }

		friend TRepeatView;
	};

};

template <typename W>
TRepeatView(W) -> TRepeatView<W>;

template <typename W>
TRepeatView(W, size_t) -> TRepeatView<W, false>;

static_assert(CRandomAccessRange<TRepeatView<int, false>>);
static_assert(      CCommonRange<TRepeatView<int, false>>);
static_assert(       CSizedRange<TRepeatView<int, false>>);
static_assert(             CView<TRepeatView<int, false>>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

/** A view of no elements of a particular type. */
template <CObject T>
inline constexpr TEmptyView<T> Empty;

/** Creates a view that contains exactly one element of a specified value. */
template <typename T> requires (CObject<TDecay<T>> && CMoveConstructible<TDecay<T>>)
NODISCARD FORCEINLINE constexpr TSingleView<TDecay<T>> Single(T&& Value)
{
	return TSingleView<TDecay<T>>(Forward<T>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly incrementing an initial value. */
template <typename W> requires (CWeaklyIncrementable<TDecay<W>> && CCopyable<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TIotaView<TDecay<W>> Iota(W&& Value)
{
	return TIotaView<TDecay<W>>(Forward<W>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly incrementing an initial value. */
template <typename W, typename S> requires (CWeaklyIncrementable<TDecay<W>> && CWeaklyEqualityComparable<W, S> && CCopyable<TDecay<W>> && CSemiregular<TDecay<S>>)
NODISCARD FORCEINLINE constexpr TIotaView<TDecay<W>, TDecay<S>> Iota(W&& Value, S&& Last)
{
	return TIotaView<TDecay<W>, TDecay<S>>(Forward<W>(Value), Forward<S>(Last));
}

/** Creates a view that generates a sequence of elements by repeatedly producing the same value. */
template <typename W> requires (CObject<TDecay<W>> && CMoveConstructible<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TRepeatView<TDecay<W>> Repeat(W&& Value)
{
	return TRepeatView<TDecay<W>>(Forward<W>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly producing the same value. */
template <typename W> requires (CObject<TDecay<W>> && CMoveConstructible<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TRepeatView<TDecay<W>, false> Repeat(W&& Value, size_t Count)
{
	return TRepeatView<TDecay<W>, false>(Forward<W>(Value), Count);
}

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

/** A view adapter that combines an iterator-sentinel pair. */
template <CInputOrOutputIterator I, CSentinelFor<I> S = I>
class TRangeView : public TViewInterface<TRangeView<I, S>>
{
public:

	using ElementType = TIteratorElementType<I>;
	using Reference   = TIteratorReferenceType<I>;
	using Iterator    = I;
	using Sentinel    = S;

	FORCEINLINE constexpr TRangeView() requires (CDefaultConstructible<I>) = default;

	FORCEINLINE constexpr TRangeView(I InFirst, S InLast) : First(MoveTemp(InFirst)), Last(InLast) { }

	template <CBorrowedRange R> requires (!CSameAs<TRemoveCVRef<R>, TRangeView>
		&& CConvertibleTo<TRangeIteratorType<R>, I> && CConvertibleTo<TRangeSentinelType<R>, S>)
	FORCEINLINE constexpr TRangeView(R&& InRange) : First(Range::Begin(Forward<R>(InRange))), Last(Range::End(Forward<R>(InRange))) { }

	NODISCARD FORCEINLINE constexpr I Begin()       requires (!CCopyable<I>) { return MoveTemp(First); }
	NODISCARD FORCEINLINE constexpr S End()         requires (!CCopyable<S>) { return MoveTemp(Last);  }
	NODISCARD FORCEINLINE constexpr I Begin() const requires ( CCopyable<I>) { return          First;  }
	NODISCARD FORCEINLINE constexpr S End()   const requires ( CCopyable<S>) { return          Last;   }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedSentinelFor<S, I>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

private:

	NO_UNIQUE_ADDRESS I First;
	NO_UNIQUE_ADDRESS S Last;

};

template <CInputOrOutputIterator I, CSentinelFor<I> S>
TRangeView(I, S) -> TRangeView<I, S>;

template <CBorrowedRange R>
TRangeView(R&&) -> TRangeView<TRangeIteratorType<R>, TRangeSentinelType<R>>;

NAMESPACE_END(Range)

template <typename I, typename S>
constexpr bool bEnableBorrowedRange<Range::TRangeView<I, S>> = true;

NAMESPACE_BEGIN(Range)

/** A view adapter that references the elements of some other range. */
template <CRange R> requires (CObject<R>)
class TRefView : public TViewInterface<TRefView<R>>
{
public:

	using ElementType = TRangeElementType<R>;
	using Reference   = TRangeReferenceType<R>;
	using Iterator    = TRangeIteratorType<R>;
	using Sentinel    = TRangeSentinelType<R>;

	template <typename T> requires (!CSameAs<TRemoveCVRef<T>, TRefView> && CConvertibleTo<T, R&> && CLValueReference<T>)
	FORCEINLINE constexpr TRefView(T&& InRange) : Ptr(AddressOf(static_cast<R&>(Forward<T>(InRange)))) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Range::Begin(*Ptr); }
	NODISCARD FORCEINLINE constexpr Sentinel End()   const { return Range::End  (*Ptr); }

	NODISCARD FORCEINLINE constexpr auto GetData() const requires (CContiguousRange<R>) { return Range::GetData(*Ptr); }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<R>) { return Range::Num(*Ptr); }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const requires (requires(R* Ptr) { Range::IsEmpty(*Ptr); }) { return Range::IsEmpty(*Ptr); }

	NODISCARD FORCEINLINE constexpr R& GetBase() const { return *Ptr; }

private:

	R* Ptr;

};

template <typename R>
TRefView(R&) -> TRefView<R>;

static_assert(CContiguousRange<TRefView<TSingleView<int>>>);
static_assert(    CCommonRange<TRefView<TSingleView<int>>>);
static_assert(     CSizedRange<TRefView<TSingleView<int>>>);
static_assert(           CView<TRefView<TSingleView<int>>>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TRefView<T>> = true;

NAMESPACE_BEGIN(Range)

/** A view adapter that has unique ownership of a range. */
template <CRange R> requires (CMovable<R> && !NAMESPACE_PRIVATE::TIsInitializerList<R>::Value)
class TOwningView : public TViewInterface<TOwningView<R>>
{
public:

	using ElementType = TRangeElementType<R>;
	using Reference   = TRangeReferenceType<R>;
	using Iterator    = TRangeIteratorType<R>;
	using Sentinel    = TRangeSentinelType<R>;

	FORCEINLINE constexpr TOwningView() requires (CDefaultConstructible<R>) = default;

	FORCEINLINE constexpr TOwningView(const TOwningView&) = delete;
	FORCEINLINE constexpr TOwningView(TOwningView&&)      = default;

	FORCEINLINE constexpr TOwningView(R&& InRange) : Base(MoveTemp(InRange)) { }

	FORCEINLINE constexpr TOwningView& operator=(const TOwningView&) = delete;
	FORCEINLINE constexpr TOwningView& operator=(TOwningView&&)      = default;

	NODISCARD FORCEINLINE constexpr Iterator Begin()                                  { return Range::Begin(Base); }
	NODISCARD FORCEINLINE constexpr Sentinel End()                                    { return Range::End  (Base); }
	NODISCARD FORCEINLINE constexpr Iterator Begin() const requires (CRange<const R>) { return Range::Begin(Base); }
	NODISCARD FORCEINLINE constexpr Sentinel End()   const requires (CRange<const R>) { return Range::End  (Base); }

	NODISCARD FORCEINLINE constexpr auto GetData()       requires (CContiguousRange<      R>) { return Range::GetData(Base); }
	NODISCARD FORCEINLINE constexpr auto GetData() const requires (CContiguousRange<const R>) { return Range::GetData(Base); }

	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CSizedRange<      R>) { return Range::Num(Base); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<const R>) { return Range::Num(Base); }

	NODISCARD FORCEINLINE constexpr bool IsEmpty()       requires (requires(      R Base) { Range::IsEmpty(Base); }) { return Range::IsEmpty(Base); }
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const requires (requires(const R Base) { Range::IsEmpty(Base); }) { return Range::IsEmpty(Base); }

	NODISCARD FORCEINLINE constexpr       R&  GetBase() &       { return                  Base;   }
	NODISCARD FORCEINLINE constexpr       R&& GetBase() &&      { return         MoveTemp(Base);  }
	NODISCARD FORCEINLINE constexpr const R&  GetBase() const&  { return          AsConst(Base);  }
	NODISCARD FORCEINLINE constexpr const R&& GetBase() const&& { return MoveTemp(AsConst(Base)); }

private:

	NO_UNIQUE_ADDRESS R Base;

};

static_assert(CContiguousRange<TOwningView<TSingleView<int>>>);
static_assert(    CCommonRange<TOwningView<TSingleView<int>>>);
static_assert(     CSizedRange<TOwningView<TSingleView<int>>>);
static_assert(           CView<TOwningView<TSingleView<int>>>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TOwningView<T>> = bEnableBorrowedRange<T>;

NAMESPACE_BEGIN(Range)

/** A view adapter that includes all elements of a range. */
template <CViewableRange R>
using TAllView =
	TConditional<CView<TDecay<R>>, TDecay<R>,
	TConditional<CLValueReference<R>, TRefView<TRemoveReference<R>>, TOwningView<TRemoveReference<R>>>>;

/** A view adapter that consists of the elements of a range that satisfies a predicate. */
template <CInputRange V, CPredicate<TRangeReferenceType<V>> Pred> requires (CView<V> && CObject<Pred> && CMoveConstructible<Pred>)
class TFilterView : public TViewInterface<TFilterView<V, Pred>>
{
private:

	class FSentinelImpl;

public:

	using ElementType = TRangeElementType<V>;

	using Reference = TRangeReferenceType<V>;

	class Iterator;

	using Sentinel = TConditional<CCommonRange<V>, Iterator, FSentinelImpl>;

	FORCEINLINE constexpr TFilterView() requires (CDefaultConstructible<V> && CDefaultConstructible<Pred>) = default;

	FORCEINLINE constexpr explicit TFilterView(V InBase, Pred InPredicate) : Base(MoveTemp(InBase)), Predicate(MoveTemp(InPredicate)) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin()
	{
		Iterator Iter(*this, Range::Begin(Base));

		do
		{
			if (Iter == End()) break;

			if (InvokeResult<bool>(GetPredicate(), *Iter)) break;

			++Iter;
		}
		while (false);

		if constexpr (!CForwardRange<V>) return MoveTemp(Iter);

		return Iter;
	}

	NODISCARD FORCEINLINE constexpr Sentinel End() { return Sentinel(*this, Range::End(Base)); }

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

	NODISCARD FORCEINLINE constexpr const Pred& GetPredicate() const { return Predicate; }

private:

	NO_UNIQUE_ADDRESS V Base;

	NO_UNIQUE_ADDRESS Pred Predicate;

public:

	class Iterator final
	{

	public:

		using ElementType = TIteratorElementType<TRangeIteratorType<V>>;

		FORCEINLINE constexpr Iterator() requires (CDefaultConstructible<TRangeIteratorType<V>>) = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const Iterator& LHS, const Iterator& RHS)
		{
			return LHS.Current == RHS.Current;
		}

		NODISCARD FORCEINLINE constexpr TRangeReferenceType<V> operator*()  const { return *Current; }
		NODISCARD FORCEINLINE constexpr TRangeIteratorType<V>  operator->() const { return  Current; }

		FORCEINLINE constexpr Iterator& operator++()
		{
			do ++Current; while (*this != Owner->End() && !InvokeResult<bool>(Owner->GetPredicate(), *Current));

			return *this;
		}

		FORCEINLINE constexpr Iterator& operator--() requires (CBidirectionalIterator<TRangeIteratorType<V>>)
		{
			do --Current; while (!InvokeResult<bool>(Owner->GetPredicate(), *Current));

			return *this;
		}

		FORCEINLINE constexpr void     operator++(int)                                                          {                        Current++;  }
		FORCEINLINE constexpr Iterator operator++(int) requires       (CForwardIterator<TRangeIteratorType<V>>) { return Iterator(Owner, Current++); }
		FORCEINLINE constexpr Iterator operator--(int) requires (CBidirectionalIterator<TRangeIteratorType<V>>) { return Iterator(Owner, Current--); }

		friend FORCEINLINE void IndirectlyCopy(const Iterator& Iter, const Iterator& Jter) requires (CIndirectlyCopyable <TRangeIteratorType<V>, TRangeIteratorType<V>>) { IndirectlyCopy(Iter.Current, Jter.Current); }
		friend FORCEINLINE void IndirectlyMove(const Iterator& Iter, const Iterator& Jter) requires (CIndirectlyMovable  <TRangeIteratorType<V>, TRangeIteratorType<V>>) { IndirectlyMove(Iter.Current, Jter.Current); }
		friend FORCEINLINE void IndirectlySwap(const Iterator& Iter, const Iterator& Jter) requires (CIndirectlySwappable<TRangeIteratorType<V>, TRangeIteratorType<V>>) { IndirectlySwap(Iter.Current, Jter.Current); }

		NODISCARD FORCEINLINE constexpr const TRangeIteratorType<V>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIteratorType<V>  GetBase() &&     { return MoveTemp(Current); }

	private:

		TFilterView* Owner;

		NO_UNIQUE_ADDRESS TRangeIteratorType<V> Current;

		FORCEINLINE constexpr Iterator(TFilterView& InOwner, TRangeIteratorType<V> InCurrent) : Owner(&InOwner), Current(MoveTemp(InCurrent)) { }

		friend FSentinelImpl;

		friend TFilterView;
	};

private:

	class FSentinelImpl final
	{
	public:

		FORCEINLINE constexpr FSentinelImpl() requires (CDefaultConstructible<TRangeSentinelType<V>>) = default;

		NODISCARD FORCEINLINE constexpr bool operator==(const Iterator& InValue) const& { return Current == InValue.Current; }

		NODISCARD FORCEINLINE constexpr const TRangeIteratorType<V>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIteratorType<V>  GetBase() &&     { return MoveTemp(Current); }

	private:

		TRangeSentinelType<V> Current;

		FORCEINLINE constexpr FSentinelImpl(TFilterView& InOwner, TRangeSentinelType<V> InCurrent) : Current(InCurrent) { }

		friend TFilterView;
	};

};

template <typename R, typename Pred>
TFilterView(R&&, Pred) -> TFilterView<TAllView<R>, Pred>;

static_assert(CBidirectionalRange<TFilterView<TSingleView<int>, decltype([](auto) { return true; })>>);
static_assert(       CCommonRange<TFilterView<TSingleView<int>, decltype([](auto) { return true; })>>);
static_assert(              CView<TFilterView<TSingleView<int>, decltype([](auto) { return true; })>>);

/** A view adapter of a sequence that applies a transformation function to each element. */
template <CInputRange V, CMoveConstructible F> requires (CView<V> && CObject<F>
	&& CRegularInvocable<F&, TRangeReferenceType<V>> && CReferenceable<TInvokeResult<F&, TRangeReferenceType<V>>>)
class TTransformView : public TViewInterface<TTransformView<V, F>>
{
private:

	template <bool bConst> class FIteratorImpl;
	template <bool bConst> class FSentinelImpl;

public:

	using ElementType = TRemoveReference<TInvokeResult<F&, TRangeReferenceType<V>>>;

	FORCEINLINE constexpr TTransformView() requires (CDefaultConstructible<V>&& CDefaultConstructible<F>) = default;

	FORCEINLINE constexpr explicit TTransformView(V InBase, F InFunc) : Base(MoveTemp(InBase)), Func(MoveTemp(InFunc)) { }

	NODISCARD FORCEINLINE constexpr FIteratorImpl<false> Begin()
	{
		return FIteratorImpl<false>(*this, Range::Begin(Base));
	}

	NODISCARD FORCEINLINE constexpr FIteratorImpl<true> Begin() const
		requires (CRange<const V> && CRegularInvocable<const F&, TRangeReferenceType<const V>>)
	{
		return FIteratorImpl<true>(*this, Range::Begin(Base));
	}

	NODISCARD FORCEINLINE constexpr FSentinelImpl<false> End()
	{
		return FSentinelImpl<false>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr FIteratorImpl<false> End() requires (CCommonRange<const V>)
	{
		return FIteratorImpl<false>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr FSentinelImpl<true> End() const
		requires (CRange<const V> && CRegularInvocable<const F&, TRangeReferenceType<const V>>)
	{
		return FSentinelImpl<true>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr FIteratorImpl<true> End() const
		requires (CCommonRange<const V> && CRegularInvocable<const F&, TRangeReferenceType<const V>>)
	{
		return FIteratorImpl<true>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr size_t Num()       requires CSizedRange<      V> { return Range::Num(Base); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires CSizedRange<const V> { return Range::Num(Base); }

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

private:

	NO_UNIQUE_ADDRESS V Base;
	NO_UNIQUE_ADDRESS F Func;

	template <bool bConst>
	class FIteratorImpl
	{
	private:

		using FOwner = TConditional<bConst, const TTransformView, TTransformView>;
		using FBase  = TConditional<bConst, const V, V>;
		using FFunc  = TConditional<bConst, const F, F>;

	public:

		using ElementType = TRemoveCVRef<TInvokeResult<FFunc&, TRangeReferenceType<FBase>>>;

		FORCEINLINE constexpr FIteratorImpl() requires (CDefaultConstructible<TRangeIteratorType<FBase>>) = default;

		FORCEINLINE constexpr FIteratorImpl(FIteratorImpl<!bConst> Iter) requires (bConst && CConvertibleTo<TRangeIteratorType<V>, TRangeIteratorType<FBase>>)
			: Owner(Iter.Owner), Current(MoveTemp(Iter.Current))
		{ }

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIteratorImpl& LHS, const FIteratorImpl& RHS)
			requires (CSentinelFor<TRangeIteratorType<FBase>, TRangeIteratorType<FBase>>)
		{
			return LHS.Current == RHS.Current;
		}

		NODISCARD friend FORCEINLINE constexpr auto operator<=>(const FIteratorImpl& LHS, const FIteratorImpl& RHS)
			requires (CSizedSentinelFor<TRangeIteratorType<FBase>, TRangeIteratorType<FBase>>)
		{
			return LHS.Current <=> RHS.Current;
		}

		NODISCARD FORCEINLINE constexpr decltype(auto) operator*() const { return Invoke(Owner->Func, *Current); }

		NODISCARD FORCEINLINE constexpr decltype(auto) operator[](ptrdiff Index) const requires (CRandomAccessRange<FBase>) { return Invoke(Owner->Func, Current[Index]); }

		FORCEINLINE constexpr FIteratorImpl& operator++()                                       { ++Current; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator--() requires (CBidirectionalRange<FBase>) { --Current; return *this; }

		FORCEINLINE constexpr void          operator++(int)                                       {                      Current++;  }
		FORCEINLINE constexpr FIteratorImpl operator++(int) requires       (CForwardRange<FBase>) { return FIteratorImpl(Current++); }
		FORCEINLINE constexpr FIteratorImpl operator--(int) requires (CBidirectionalRange<FBase>) { return FIteratorImpl(Current--); }

		FORCEINLINE constexpr FIteratorImpl& operator+=(ptrdiff Offset) requires (CRandomAccessRange<FBase>) { Current += Offset; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator-=(ptrdiff Offset) requires (CRandomAccessRange<FBase>) { Current -= Offset; return *this; }

		NODISCARD friend FORCEINLINE constexpr FIteratorImpl operator+(FIteratorImpl Iter, ptrdiff Offset) requires (CRandomAccessRange<FBase>) { FIteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr FIteratorImpl operator+(ptrdiff Offset, FIteratorImpl Iter) requires (CRandomAccessRange<FBase>) { FIteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr FIteratorImpl operator-(ptrdiff Offset) const requires (CRandomAccessRange<FBase>) { FIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIteratorImpl& LHS, const FIteratorImpl& RHS)
			requires (CSizedSentinelFor<TRangeIteratorType<FBase>, TRangeIteratorType<FBase>>)
		{
			return LHS.Current - RHS.Current;
		}

		NODISCARD FORCEINLINE constexpr const TRangeIteratorType<FBase>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIteratorType<FBase>  GetBase() &&     { return MoveTemp(Current); }

	private:

		NO_UNIQUE_ADDRESS FOwner* Owner;

		NO_UNIQUE_ADDRESS TRangeIteratorType<FBase> Current;

		FORCEINLINE constexpr FIteratorImpl(FOwner& InOwner, TRangeIteratorType<FBase> InCurrent) : Owner(&InOwner), Current(MoveTemp(InCurrent)) { }

		template <bool> friend class FIteratorImpl;
		template <bool> friend class FSentinelImpl;

		friend TTransformView;
	};

	template <bool bConst>
	class FSentinelImpl
	{
	private:

		using FOwner = TConditional<bConst, const TTransformView, TTransformView>;
		using FBase  = TConditional<bConst, const V, V>;

	public:

		FORCEINLINE constexpr FSentinelImpl() requires (CDefaultConstructible<TRangeSentinelType<FBase>>) = default;

		FORCEINLINE constexpr FSentinelImpl(FSentinelImpl<!bConst> Sentinel) requires (bConst && CConvertibleTo<TRangeSentinelType<V>, TRangeSentinelType<FBase>>)
			: Current(Sentinel.Current)
		{ }

		NODISCARD FORCEINLINE constexpr bool operator==(const FIteratorImpl<bConst>& InValue) const& { return Current == InValue.Current; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIteratorImpl<bConst>& LHS, const FSentinelImpl& RHS)
			requires CSizedSentinelFor<TRangeSentinelType<FBase>, TRangeIteratorType<FBase>>
		{
			return LHS.Current - RHS.Current;
		}

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FSentinelImpl& LHS, const FIteratorImpl<bConst>& RHS)
			requires CSizedSentinelFor<TRangeSentinelType<FBase>, TRangeIteratorType<FBase>>
		{
			return RHS.Current - LHS.Current;
		}

		NODISCARD FORCEINLINE constexpr const TRangeIteratorType<FBase>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIteratorType<FBase>  GetBase() &&     { return MoveTemp(Current); }

	private:

		NO_UNIQUE_ADDRESS TRangeSentinelType<FBase> Current;

		FORCEINLINE constexpr FSentinelImpl(FOwner& InOwner, TRangeSentinelType<FBase> InCurrent) : Current(InCurrent) { }

		friend TTransformView;
	};

};

template <typename R, typename F>
TTransformView(R&&, F) -> TTransformView<TAllView<R>, F>;

static_assert(CRandomAccessRange<TTransformView<TSingleView<int>, decltype([](auto) { return 0; })>>);
static_assert(      CCommonRange<TTransformView<TSingleView<int>, decltype([](auto) { return 0; })>>);
static_assert(       CSizedRange<TTransformView<TSingleView<int>, decltype([](auto) { return 0; })>>);
static_assert(             CView<TTransformView<TSingleView<int>, decltype([](auto) { return 0; })>>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

/** Creates A view adapter that combines an iterator-sentinel pair. */
template <CInputOrOutputIterator I, CSentinelFor<I> S = I>
NODISCARD FORCEINLINE constexpr TRangeView<I, S> View(I First, S Last)
{
	return TRangeView<I, S>(MoveTemp(First), MoveTemp(Last));
}

/** Creates A view adapter that includes all elements of a range. */
template <CViewableRange R>
NODISCARD FORCEINLINE constexpr TAllView<R> All(R&& InRange)
{
	return TAllView<R>(Forward<R>(InRange));
}

/** Creates A view adapter that consists of the elements of a range that satisfies a predicate. */
template <CViewableRange R, typename Pred> requires (requires { TFilterView(DeclVal<R>(), DeclVal<Pred>()); })
NODISCARD FORCEINLINE constexpr auto Filter(R&& Base, Pred&& Predicate)
{
	return TFilterView(Forward<R>(Base), Forward<Pred>(Predicate));
}

/** Creates A view adapter that consists of the elements of a range that satisfies a predicate. */
template <typename Pred>
NODISCARD FORCEINLINE constexpr auto Filter(Pred&& Predicate)
{
	return [&Predicate]<CViewableRange R>(R&& Base) requires (requires { TFilterView(DeclVal<R>(), DeclVal<Pred>()); })
	{
		return TFilterView(Forward<R>(Base), Forward<Pred>(Predicate));
	};
}

/** Creates A view adapter of a sequence that applies a transformation function to each element. */
template <CViewableRange R, typename F> requires (requires { TTransformView(DeclVal<R>(), DeclVal<F>()); })
NODISCARD FORCEINLINE constexpr auto Transform(R&& Base, F&& Func)
{
	return TTransformView(Forward<R>(Base), Forward<F>(Func));
}

/** Creates A view adapter of a sequence that applies a transformation function to each element. */
template <typename F>
NODISCARD FORCEINLINE constexpr auto Transform(F&& Func)
{
	return [&Func]<CViewableRange R>(R&& Base) requires (requires { TTransformView(DeclVal<R>(), DeclVal<F>()); })
	{
		return TTransformView(Forward<R>(Base), Forward<F>(Func));
	};
}

NAMESPACE_END(Range)

template <CRange R, typename T> requires (CRange<TInvokeResult<T, R>>)
NODISCARD FORCEINLINE constexpr auto operator|(R&& Range, T&& View)
{
	return Invoke(Forward<T>(View), Forward<R>(Range));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
