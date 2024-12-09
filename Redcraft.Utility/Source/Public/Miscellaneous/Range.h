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
	|| (!CView<TRemoveCVRef<R>> && (CLValueReference<R> || CMovable<TRemoveReference<R>>
	&& !NAMESPACE_PRIVATE::TIsInitializerList<TRemoveCVRef<R>>::Value)));

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

	NODISCARD FORCEINLINE constexpr auto Num() const requires ((CIntegral<W> && CIntegral<S>) || CSizedSentinelFor<S, W>) { return Last - First; }

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

template <CRange R, typename T> requires (CRange<TInvokeResult<T, R>>)
NODISCARD FORCEINLINE constexpr auto operator|(R&& Range, T&& View)
{
	return Invoke(Forward<T>(View), Forward<R>(Range));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
