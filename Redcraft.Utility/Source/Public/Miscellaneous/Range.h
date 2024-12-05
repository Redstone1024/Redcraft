#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Iterator.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

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
NODISCARD FORCEINLINE constexpr auto Begin(initializer_list<T> Container)
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
NODISCARD FORCEINLINE constexpr auto End(initializer_list<T> Container)
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
	return MakeReverseIterator(Range::End(Container));
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
	return MakeReverseIterator(Range::Begin(Container));
}

NAMESPACE_END(Range)

template <typename R>
using TRangeElementType = TIteratorElementType<TRangeIteratorType<R>>;

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
	return Range::End(Container) - Range::Begin(Container);
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
inline constexpr bool bEnableView = false;

template <typename R>
concept CView = CRange<R> && CMovable<R> && bEnableView<R>;

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
		{ Range::GetData(Range) } -> CSameAs<TAddPointer<TRangeElementType<R>>>;
	};

template <typename R>
concept CCommonRange = CRange<R> && CSameAs<TRangeIteratorType<R>, TRangeSentinelType<R>>;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsInitializerList                      : FFalse { };
template <typename T> struct TIsInitializerList<initializer_list<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename R>
concept CViewableRange = CRange<R>
	&& ((CView<TRemoveCVRef<R>> && CConstructibleFrom<TRemoveCVRef<R>, R>)
	|| (!CView<TRemoveCVRef<R>> && (CLValueReference<R> || CMovable<TRemoveReference<R>>
	&& !NAMESPACE_PRIVATE::TIsInitializerList<TRemoveCVRef<R>>::Value)));

static_assert(CRange<int[8]> && CContiguousRange<int[8]> && CCommonRange<int[8]>);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
