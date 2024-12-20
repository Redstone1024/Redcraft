#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Iterators/BasicIterator.h"
#include "Iterators/ReverseIterator.h"
#include "Memory/Address.h"
#include "Ranges/Utility.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/** An interface class template for defining a view. Not directly instantiable. */
template <CClass T> requires (CSameAs<T, TRemoveCV<T>>)
class IBasicViewInterface
{
public:

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr auto GetData()       requires (CContiguousIterator<TRangeIterator<      T>>) { return ToAddress(Range::Begin(static_cast<      T&>(*this))); }
	NODISCARD FORCEINLINE constexpr auto GetData() const requires (CContiguousIterator<TRangeIterator<const T>>) { return ToAddress(Range::Begin(static_cast<const T&>(*this))); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr auto RBegin()       requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return MakeReverseIterator(Range::End  (static_cast<      T&>(*this))); }
	NODISCARD FORCEINLINE constexpr auto REnd()         requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return MakeReverseIterator(Range::Begin(static_cast<      T&>(*this))); }
	NODISCARD FORCEINLINE constexpr auto RBegin() const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return MakeReverseIterator(Range::End  (static_cast<const T&>(*this))); }
	NODISCARD FORCEINLINE constexpr auto REnd()   const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return MakeReverseIterator(Range::Begin(static_cast<const T&>(*this))); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CForwardRange<      T> && CSizedSentinelFor<TRangeSentinel<      T>, TRangeIterator<      T>>) {       T& Derived = static_cast<      T&>(*this); return Range::End(Derived) - Range::Begin(Derived); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CForwardRange<const T> && CSizedSentinelFor<TRangeSentinel<const T>, TRangeIterator<const T>>) { const T& Derived = static_cast<const T&>(*this); return Range::End(Derived) - Range::Begin(Derived); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty()       requires (CSizedRange<      T> || CForwardRange<      T>) {       T& Derived = static_cast<      T&>(*this); if constexpr (CSizedRange<      T>) return Range::Num(Derived) == 0; else return Range::Begin(Derived) == Range::End(Derived); }
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const requires (CSizedRange<const T> || CForwardRange<const T>) { const T& Derived = static_cast<const T&>(*this); if constexpr (CSizedRange<const T>) return Range::Num(Derived) == 0; else return Range::Begin(Derived) == Range::End(Derived); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr explicit operator bool()       requires (requires { Range::IsEmpty(DeclVal<      T&>()); }) { return !Range::IsEmpty(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const requires (requires { Range::IsEmpty(DeclVal<const T&>()); }) { return !Range::IsEmpty(static_cast<const T&>(*this)); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr decltype(auto) operator[](size_t Index)       requires (CRandomAccessRange<      T>) { return Range::Begin(static_cast<      T&>(*this))[Index]; }
	NODISCARD FORCEINLINE constexpr decltype(auto) operator[](size_t Index) const requires (CRandomAccessRange<const T>) { return Range::Begin(static_cast<const T&>(*this))[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr decltype(auto) Front()       requires (CForwardRange<      T>)                                { return  *Range::Begin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Front() const requires (CForwardRange<const T>)                                { return  *Range::Begin(static_cast<const T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Back()        requires (CBidirectionalRange<      T> && CCommonRange<      T>) { return *Range::RBegin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr decltype(auto) Back()  const requires (CBidirectionalRange<const T> && CCommonRange<const T>) { return *Range::RBegin(static_cast<const T&>(*this)); }

	// ~Begin ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT.

	NODISCARD FORCEINLINE constexpr auto begin()       requires (CRange<      T>) { return Range::Begin(static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto end()         requires (CRange<      T>) { return Range::End  (static_cast<      T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto begin() const requires (CRange<const T>) { return Range::Begin(static_cast<const T&>(*this)); }
	NODISCARD FORCEINLINE constexpr auto end()   const requires (CRange<const T>) { return Range::End  (static_cast<const T&>(*this)); }

	// ~End ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT.

private:

	FORCEINLINE constexpr IBasicViewInterface()                                      = default;
	FORCEINLINE constexpr IBasicViewInterface(const IBasicViewInterface&)            = default;
	FORCEINLINE constexpr IBasicViewInterface(IBasicViewInterface&&)                 = default;
	FORCEINLINE constexpr IBasicViewInterface& operator=(const IBasicViewInterface&) = default;
	FORCEINLINE constexpr IBasicViewInterface& operator=(IBasicViewInterface&&)      = default;
	FORCEINLINE constexpr ~IBasicViewInterface()                                     = default;

	friend T;
};

NAMESPACE_END(Range)

/**
 * A concept specifies that a range is a view, that is, it has constant time copy, move and assignment.
 * Specify, a view can be movable only but not copyable, or it can be both movable and copyable.
 */
template <typename V>
concept CView = CRange<V> && CMovable<V> && CDerivedFrom<V, Range::IBasicViewInterface<TRemoveCVRef<V>>>;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsInitializerList                      : FFalse { };
template <typename T> struct TIsInitializerList<initializer_list<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

/** A concept specifies that a viewable range that can be converted into a view through Range::All. */
template <typename R>
concept CViewableRange = CRange<R>
	&& ((CView<TRemoveCVRef<R>> && CConstructibleFrom<TRemoveCVRef<R>, R>)
	|| (!CView<TRemoveCVRef<R>> && (CLValueReference<R> || (CMovable<TRemoveReference<R>>
	&& !NAMESPACE_PRIVATE::TIsInitializerList<TRemoveCVRef<R>>::Value))));

/** A concept specifies that a view uses the same iterator and sentinel type for both const and non-const views. */
template <typename V>
concept CSimpleView = CView<V> && CRange<const V>
	&& CSameAs<TRangeIterator<V>, TRangeIterator<const V>>
	&& CSameAs<TRangeSentinel<V>, TRangeSentinel<const V>>;

NAMESPACE_BEGIN(Range)

/** A simple view that combines an iterator-sentinel pair into a view. */
template <CInputOrOutputIterator I, CSentinelFor<I> S = I>
class TRangeView : public IBasicViewInterface<TRangeView<I, S>>
{
public:

	using FElementType = TIteratorElement<I>;

	FORCEINLINE constexpr TRangeView() requires (CDefaultConstructible<I>) = default;

	FORCEINLINE constexpr TRangeView(I InFirst, S InLast) : First(MoveTemp(InFirst)), Last(InLast) { }

	NODISCARD FORCEINLINE constexpr I Begin()       requires (!CCopyable<I>) { return MoveTemp(First); }
	NODISCARD FORCEINLINE constexpr I Begin() const requires ( CCopyable<I>) { return          First;  }

	NODISCARD FORCEINLINE constexpr S End() const { return Last; }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedSentinelFor<S, I>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

private:

	NO_UNIQUE_ADDRESS I First;
	NO_UNIQUE_ADDRESS S Last;

};

template <CInputOrOutputIterator I, CSentinelFor<I> S>
TRangeView(I, S) -> TRangeView<I, S>;

NAMESPACE_END(Range)

template <typename I, typename S>
constexpr bool bEnableBorrowedRange<Range::TRangeView<I, S>> = true;

NAMESPACE_BEGIN(Range)

/** Creates A simple view that combines an iterator-sentinel pair. */
template <CInputOrOutputIterator I, CSentinelFor<I> S = I>
NODISCARD FORCEINLINE constexpr TRangeView<I, S> View(I First, S Last)
{
	return TRangeView<I, S>(MoveTemp(First), Last);
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
