#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/Invoke.h"
#include "Iterators/Utility.h"
#include "Iterators/BasicIterator.h"
#include "Memory/Address.h"
#include "Ranges/Utility.h"
#include "Ranges/Pipe.h"
#include "Ranges/View.h"
#include "Ranges/AllView.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * A view adapter that consists of the elements of a range that satisfies a predicate.
 * When based on an input view, the filter view satisfies at least an input view up to a bidirectional view.
 * When based on a common view, the filter view satisfies a common view.
 */
template <CInputRange V, CPredicate<TRangeReference<V>> Pred> requires (CView<V> && CObject<Pred> && CMoveConstructible<Pred>)
class TFilterView : public IBasicViewInterface<TFilterView<V, Pred>>
{
private:

	class FIteratorImpl;
	class FSentinelImpl;

public:

	using FElementType = TRangeElement<V>;
	using FReference = TRangeReference<V>;

	using FIterator = FIteratorImpl;

	using FSentinel = TConditional<CCommonRange<V>, FIteratorImpl, FSentinelImpl>;

	FORCEINLINE constexpr TFilterView() requires (CDefaultConstructible<V> && CDefaultConstructible<Pred>) = default;

	FORCEINLINE constexpr explicit TFilterView(V InBase, Pred InPredicate) : Base(MoveTemp(InBase)), Predicate(MoveTemp(InPredicate)) { }

	NODISCARD FORCEINLINE constexpr FIterator Begin()
	{
		FIterator Iter(*this, Range::Begin(Base));

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

	NODISCARD FORCEINLINE constexpr FSentinel End() { return FSentinel(*this, Range::End(Base)); }

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

	NODISCARD FORCEINLINE constexpr const Pred& GetPredicate() const { return Predicate; }

private:

	NO_UNIQUE_ADDRESS V Base;

	NO_UNIQUE_ADDRESS Pred Predicate;

	class FIteratorImpl final
	{

	public:

		using FElementType = TIteratorElement<TRangeIterator<V>>;

		FORCEINLINE constexpr FIteratorImpl() requires (CDefaultConstructible<TRangeIterator<V>>) { } // Use '{ }' instead of '= default;' to avoid MSVC bug.

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIteratorImpl& LHS, const FIteratorImpl& RHS)
		{
			return LHS.GetBase() == RHS.GetBase();
		}

		NODISCARD FORCEINLINE constexpr TRangeReference<V> operator*() const { return *GetBase(); }

		NODISCARD FORCEINLINE constexpr auto operator->() const requires (requires(const TRangeIterator<V> Iter) { { ToAddress(Iter) } -> CSameAs<TIteratorPointer<TRangeIterator<V>>>; }) { return ToAddress(GetBase()); }

		FORCEINLINE constexpr FIteratorImpl& operator++()
		{
			do ++Current; while (*this != Owner->End() && !InvokeResult<bool>(Owner->GetPredicate(), *Current));

			return *this;
		}

		FORCEINLINE constexpr FIteratorImpl& operator--() requires (CBidirectionalIterator<TRangeIterator<V>>)
		{
			do --Current; while (!InvokeResult<bool>(Owner->GetPredicate(), *Current));

			return *this;
		}

		FORCEINLINE constexpr void          operator++(int)                                                      {                             ++*this;              }
		FORCEINLINE constexpr FIteratorImpl operator++(int) requires       (CForwardIterator<TRangeIterator<V>>) { FIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr FIteratorImpl operator--(int) requires (CBidirectionalIterator<TRangeIterator<V>>) { FIteratorImpl Temp = *this; --*this; return Temp; }

		NODISCARD FORCEINLINE constexpr const TRangeIterator<V>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIterator<V>  GetBase() &&     { return MoveTemp(Current); }

	private:

		TFilterView* Owner;

		NO_UNIQUE_ADDRESS TRangeIterator<V> Current;

		FORCEINLINE constexpr FIteratorImpl(TFilterView& InOwner, TRangeIterator<V> InCurrent) : Owner(&InOwner), Current(MoveTemp(InCurrent)) { }

		friend FSentinelImpl;

		friend TFilterView;
	};

	class FSentinelImpl final
	{
	public:

		FORCEINLINE constexpr FSentinelImpl() = default;

		NODISCARD FORCEINLINE constexpr bool operator==(const FIteratorImpl& InValue) const& { return Current == InValue.Current; }

		NODISCARD FORCEINLINE constexpr TRangeSentinel<V> GetBase() const { return Current; }

	private:

		TRangeSentinel<V> Current;

		FORCEINLINE constexpr FSentinelImpl(TFilterView& InOwner, TRangeSentinel<V> InCurrent) : Current(InCurrent) { }

		friend TFilterView;
	};

};

template <typename R, typename Pred>
TFilterView(R&&, Pred) -> TFilterView<TAllView<R>, Pred>;

static_assert(        CInputRange<TFilterView<TAllView<IRange<        IInputIterator<int&>>>, bool(*)(int)>>);
static_assert(      CForwardRange<TFilterView<TAllView<IRange<      IForwardIterator<int&>>>, bool(*)(int)>>);
static_assert(CBidirectionalRange<TFilterView<TAllView<IRange<IBidirectionalIterator<int&>>>, bool(*)(int)>>);
static_assert(CBidirectionalRange<TFilterView<TAllView<IRange< IRandomAccessIterator<int&>>>, bool(*)(int)>>);
static_assert(CBidirectionalRange<TFilterView<TAllView<IRange<   IContiguousIterator<int&>>>, bool(*)(int)>>);

static_assert(CCommonRange<TFilterView<TAllView<ICommonRange<IForwardIterator<int>>>, bool(*)(int)>>);
static_assert(       CView<TFilterView<TAllView<      IRange<  IInputIterator<int>>>, bool(*)(int)>>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

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
	using FClosure = decltype([]<CViewableRange R, typename T> requires (requires { Range::Filter(DeclVal<R>(), DeclVal<T>()); }) (R&& Base, T&& Predicate)
	{
		return Range::Filter(Forward<R>(Base), Forward<T>(Predicate));
	});

	return TAdaptorClosure<FClosure, TDecay<Pred>>(Forward<Pred>(Predicate));
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
