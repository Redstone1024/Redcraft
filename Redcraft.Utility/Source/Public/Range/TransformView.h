#pragma once

#include "CoreTypes.h"
#include "Range/View.h"
#include "Range/Pipe.h"
#include "Range/Utility.h"
#include "Range/AllView.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * A view adapter of a sequence that applies a transformation function to each element.
 * When based on an input view, the transform view satisfies at least an input view up to a random access view.
 * When based on a common view, the transform view satisfies a common view. When based on a sized view,
 * the transform view satisfies a sized view. When based on a forward view and the function return
 * an assignable value, the transform view satisfies an output view.
 */
template <CInputRange V, CMoveConstructible F> requires (CView<V> && CObject<F>
	&& CRegularInvocable<F&, TRangeReference<V>> && CReferenceable<TInvokeResult<F&, TRangeReference<V>>>)
class TTransformView : public IBasicViewInterface<TTransformView<V, F>>
{
private:

	template <bool bConst> class FIteratorImpl;
	template <bool bConst> class FSentinelImpl;

public:

	FORCEINLINE constexpr TTransformView() requires (CDefaultConstructible<V>&& CDefaultConstructible<F>) = default;

	FORCEINLINE constexpr explicit TTransformView(V InBase, F InFunc) : Base(MoveTemp(InBase)), Func(MoveTemp(InFunc)) { }

	NODISCARD FORCEINLINE constexpr auto Begin()
	{
		return FIteratorImpl<false>(*this, Range::Begin(Base));
	}

	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const V> && CRegularInvocable<const F&, TRangeReference<const V>>)
	{
		return FIteratorImpl<true>(*this, Range::Begin(Base));
	}

	NODISCARD FORCEINLINE constexpr auto End()
	{
		if constexpr (CCommonRange<V>)
		{
			return FIteratorImpl<false>(*this, Range::End(Base));
		}

		else return FSentinelImpl<false>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr auto End() const requires (CRange<const V> && CRegularInvocable<const F&, TRangeReference<const V>>)
	{
		if constexpr (CCommonRange<const V>)
		{
			return FIteratorImpl<true>(*this, Range::End(Base));
		}

		else return FSentinelImpl<true>(*this, Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CSizedRange<      V>) { return Range::Num(Base); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<const V>) { return Range::Num(Base); }

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

		using FElementType = TRemoveCVRef<TInvokeResult<FFunc&, TRangeReference<FBase>>>;

		FORCEINLINE constexpr FIteratorImpl() requires (CDefaultConstructible<TRangeIterator<FBase>>) = default;

		FORCEINLINE constexpr FIteratorImpl(FIteratorImpl<!bConst> Iter) requires (bConst && CConvertibleTo<TRangeIterator<V>, TRangeIterator<FBase>>)
			: Owner(Iter.Owner), Current(MoveTemp(Iter).GetBase())
		{ }

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIteratorImpl& LHS, const FIteratorImpl& RHS) requires (CEqualityComparable<TRangeIterator<FBase>>)
		{
			return LHS.GetBase() == RHS.GetBase();
		}

		NODISCARD friend FORCEINLINE constexpr auto operator<=>(const FIteratorImpl& LHS, const FIteratorImpl& RHS) requires (CThreeWayComparable<TRangeIterator<FBase>>)
		{
			return LHS.GetBase() <=> RHS.GetBase();
		}

		NODISCARD FORCEINLINE constexpr decltype(auto) operator*() const { return Invoke(Owner->Func, *GetBase()); }

		NODISCARD FORCEINLINE constexpr decltype(auto) operator[](ptrdiff Index) const requires (CRandomAccessRange<FBase>) { return Invoke(Owner->Func, GetBase()[Index]); }

		FORCEINLINE constexpr FIteratorImpl& operator++()                                       { ++Current; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator--() requires (CBidirectionalRange<FBase>) { --Current; return *this; }

		FORCEINLINE constexpr void          operator++(int)                                       {                             ++*this;              }
		FORCEINLINE constexpr FIteratorImpl operator++(int) requires       (CForwardRange<FBase>) { FIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr FIteratorImpl operator--(int) requires (CBidirectionalRange<FBase>) { FIteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE constexpr FIteratorImpl& operator+=(ptrdiff Offset) requires (CRandomAccessRange<FBase>) { Current += Offset; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator-=(ptrdiff Offset) requires (CRandomAccessRange<FBase>) { Current -= Offset; return *this; }

		NODISCARD FORCEINLINE constexpr FIteratorImpl operator+(ptrdiff Offset) const requires (CRandomAccessRange<FBase>) { FIteratorImpl Temp = *this; Temp += Offset; return Temp; }
		NODISCARD FORCEINLINE constexpr FIteratorImpl operator-(ptrdiff Offset) const requires (CRandomAccessRange<FBase>) { FIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr FIteratorImpl operator+(ptrdiff Offset, const FIteratorImpl& Iter) requires (CRandomAccessRange<FBase>) { return Iter + Offset; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIteratorImpl& LHS, const FIteratorImpl& RHS) requires (CSizedSentinelFor<TRangeIterator<FBase>, TRangeIterator<FBase>>)
		{
			return LHS.GetBase() - RHS.GetBase();
		}

		NODISCARD FORCEINLINE constexpr const TRangeIterator<FBase>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIterator<FBase>  GetBase() &&     { return MoveTemp(Current); }

	private:

		NO_UNIQUE_ADDRESS FOwner* Owner;

		NO_UNIQUE_ADDRESS TRangeIterator<FBase> Current;

		FORCEINLINE constexpr FIteratorImpl(FOwner& InOwner, TRangeIterator<FBase> InCurrent) : Owner(&InOwner), Current(MoveTemp(InCurrent)) { }

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

		FORCEINLINE constexpr FSentinelImpl() requires (CDefaultConstructible<TRangeSentinel<FBase>>) = default;

		FORCEINLINE constexpr FSentinelImpl(FSentinelImpl<!bConst> Sentinel) requires (bConst && CConvertibleTo<TRangeSentinel<V>, TRangeSentinel<FBase>>)
			: Current(Sentinel.GetBase())
		{ }

		NODISCARD FORCEINLINE constexpr bool operator==(const FIteratorImpl<bConst>& InValue) const& { return Current == InValue.GetBase(); }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIteratorImpl<bConst>& LHS, const FSentinelImpl& RHS)
			requires CSizedSentinelFor<TRangeSentinel<FBase>, TRangeIterator<FBase>>
		{
			return LHS.GetBase() - RHS.GetBase();
		}

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FSentinelImpl& LHS, const FIteratorImpl<bConst>& RHS)
			requires CSizedSentinelFor<TRangeSentinel<FBase>, TRangeIterator<FBase>>
		{
			return RHS.GetBase() - LHS.GetBase();
		}

		NODISCARD FORCEINLINE constexpr const TRangeIterator<FBase>& GetBase() const& { return          Current;  }
		NODISCARD FORCEINLINE constexpr       TRangeIterator<FBase>  GetBase() &&     { return MoveTemp(Current); }

	private:

		NO_UNIQUE_ADDRESS TRangeSentinel<FBase> Current;

		FORCEINLINE constexpr FSentinelImpl(FOwner& InOwner, TRangeSentinel<FBase> InCurrent) : Current(InCurrent) { }

		friend TTransformView;
	};

};

template <typename R, typename F>
TTransformView(R&&, F) -> TTransformView<TAllView<R>, F>;

static_assert(        CInputRange<TTransformView<TAllView<IRange<        IInputIterator<int&>>>, int(*)(int)>>);
static_assert(      CForwardRange<TTransformView<TAllView<IRange<      IForwardIterator<int&>>>, int(*)(int)>>);
static_assert(CBidirectionalRange<TTransformView<TAllView<IRange<IBidirectionalIterator<int&>>>, int(*)(int)>>);
static_assert( CRandomAccessRange<TTransformView<TAllView<IRange< IRandomAccessIterator<int&>>>, int(*)(int)>>);
static_assert( CRandomAccessRange<TTransformView<TAllView<IRange<   IContiguousIterator<int&>>>, int(*)(int)>>);

static_assert(CCommonRange<TTransformView<TAllView<ICommonRange<IForwardIterator<int>>>, int(*)(int)>>);
static_assert( CSizedRange<TTransformView<TAllView< ISizedRange<  IInputIterator<int>>>, int(*)(int)>>);
static_assert(       CView<TTransformView<TAllView<      IRange<  IInputIterator<int>>>, int(*)(int)>>);

static_assert(COutputRange<TTransformView<TAllView<IRange<IForwardIterator<int>>>, int&(*)(int)>, int>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

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
	using FClosure = decltype([]<CViewableRange R, typename T> requires (requires { Range::Transform(DeclVal<R>(), DeclVal<T>()); }) (R&& Base, T&& Func)
	{
		return Range::Transform(Forward<R>(Base), Forward<T>(Func));
	});

	return TAdaptorClosure<FClosure, TDecay<F>>(Forward<F>(Func));
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
