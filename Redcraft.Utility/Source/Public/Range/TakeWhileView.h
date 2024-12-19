#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Iterator/Utility.h"
#include "Iterator/BasicIterator.h"
#include "Iterator/CountedIterator.h"
#include "Numeric/Math.h"
#include "Range/Utility.h"
#include "Range/Pipe.h"
#include "Range/View.h"
#include "Range/AllView.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * A view adapter that includes elements that satisfy the predicate from the beginning of the range.
 * When based on an input view, the take while view satisfies at least an input view up to a contiguous view.
 * When based on a forward and output view, the take while view satisfies an output view.
 */
template <CInputRange V, CPredicate<TRangeReference<V>> Pred> requires (CView<V> && CObject<Pred> && CMoveConstructible<Pred>)
class TTakeWhileView : public IBasicViewInterface<TTakeWhileView<V, Pred>>
{
private:

	template <bool bConst> class FSentinelImpl;

public:

	using FElementType = TRangeElement<V>;
	using FReference = TRangeReference<V>;

	FORCEINLINE constexpr TTakeWhileView() requires (CDefaultConstructible<V>&& CDefaultConstructible<Pred>) = default;

	FORCEINLINE constexpr explicit TTakeWhileView(V InBase, Pred InPredicate) : Base(MoveTemp(InBase)), Predicate(MoveTemp(InPredicate)) { }

	NODISCARD FORCEINLINE constexpr auto Begin() requires (!CSimpleView<V>)
	{
		return Range::Begin(Base);
	}

	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const V> && CPredicate<const Pred&, TRangeReference<const V>>)
	{
		return Range::Begin(Base);
	}

	NODISCARD FORCEINLINE constexpr auto End() requires (!CSimpleView<V>)
	{
		return FSentinelImpl<false>(Range::End(Base), AddressOf(Predicate));
	}

	NODISCARD FORCEINLINE constexpr auto End() const requires (CRange<const V> && CPredicate<const Pred&, TRangeReference<const V>>)
	{
		return FSentinelImpl<true>(Range::End(Base), AddressOf(Predicate));
	}

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

	NODISCARD FORCEINLINE constexpr const Pred& GetPredicate() const { return Predicate; }

private:

	NO_UNIQUE_ADDRESS V Base;

	NO_UNIQUE_ADDRESS Pred Predicate;

	template <bool bConst>
	class FSentinelImpl final
	{
	private:

		using FBase = TConditional<bConst, const V, V>;
		using FPred = TConditional<bConst, const Pred, Pred>;

	public:

		FORCEINLINE constexpr FSentinelImpl() = default;

		FORCEINLINE constexpr FSentinelImpl(FSentinelImpl<!bConst> Sentinel) requires (bConst && CConvertibleTo<TRangeSentinel<V>, TRangeSentinel<FBase>>)
			: Current(Sentinel.Current), Predicate(Sentinel.Predicate)
		{ }

		NODISCARD FORCEINLINE constexpr bool operator==(const TRangeIterator<FBase>& InValue) const&
		{
			return InValue == Current || !InvokeResult<bool>(*Predicate, *InValue);
		}

		NODISCARD FORCEINLINE constexpr TRangeSentinel<FBase> GetBase() const { return Current; }

	private:

		NO_UNIQUE_ADDRESS TRangeSentinel<FBase> Current;

		FPred* Predicate;

		FORCEINLINE constexpr FSentinelImpl(TRangeSentinel<FBase> InCurrent, FPred* InPredicate) : Current(InCurrent), Predicate(InPredicate) { }

		friend TTakeWhileView;
	};

};

template <typename R, typename Pred>
TTakeWhileView(R&&, Pred) -> TTakeWhileView<TAllView<R>, Pred>;

static_assert(        CInputRange<TTakeWhileView<TAllView<IRange<        IInputIterator<int&>>>, bool(*)(int)>>);
static_assert(      CForwardRange<TTakeWhileView<TAllView<IRange<      IForwardIterator<int&>>>, bool(*)(int)>>);
static_assert(CBidirectionalRange<TTakeWhileView<TAllView<IRange<IBidirectionalIterator<int&>>>, bool(*)(int)>>);
static_assert( CRandomAccessRange<TTakeWhileView<TAllView<IRange< IRandomAccessIterator<int&>>>, bool(*)(int)>>);
static_assert(   CContiguousRange<TTakeWhileView<TAllView<IRange<   IContiguousIterator<int&>>>, bool(*)(int)>>);

static_assert(CView<TTakeWhileView<TAllView<IRange<IInputIterator<int>>>, bool(*)(int)>>);

static_assert(COutputRange<TTakeWhileView<TAllView<IRange<IForwardIterator<int&>>>, bool(*)(int)>, int>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

/** Creates A view adapter that includes elements that satisfy the predicate from the beginning of the range. */
template <CViewableRange R, typename Pred> requires (requires { TTakeWhileView(DeclVal<R>(), DeclVal<Pred>()); })
NODISCARD FORCEINLINE constexpr auto TakeWhile(R&& Base, Pred&& Predicate)
{
	return TTakeWhileView(Forward<R>(Base), Forward<Pred>(Predicate));
}

/** Creates A view adapter that includes elements that satisfy the predicate from the beginning of the range. */
template <typename Pred>
NODISCARD FORCEINLINE constexpr auto TakeWhile(Pred&& Predicate)
{
	using FClosure = decltype([]<CViewableRange R, typename T> requires (requires { Range::TakeWhile(DeclVal<R>(), DeclVal<T>()); }) (R&& Base, T&& Predicate)
	{
		return Range::TakeWhile(Forward<R>(Base), Forward<T>(Predicate));
	});

	return TAdaptorClosure<FClosure, TDecay<Pred>>(Forward<Pred>(Predicate));
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
