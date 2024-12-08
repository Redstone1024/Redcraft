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

template <CObject T>
class TEmptyView : public TViewInterface<TEmptyView<T>>
{
public:

	using ElementType = T;
	using Reference   = T&;
	using Iterator    = T*;
	using Sentinel    = T*;

	using ReverseIterator = TReverseIterator<Iterator>;

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

template <typename  T>
constexpr bool bEnableBorrowedRange<Range::TEmptyView<T>> = true;

NAMESPACE_BEGIN(Range)

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

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

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

NAMESPACE_END(Range)

template <CRange R, typename T> requires (CRange<TInvokeResult<T, R>>)
NODISCARD FORCEINLINE constexpr auto operator|(R&& Range, T&& View)
{
	return Invoke(Forward<T>(View), Forward<R>(Range));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
