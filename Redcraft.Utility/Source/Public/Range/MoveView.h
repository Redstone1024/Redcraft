#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Iterator/Utility.h"
#include "Iterator/BasicIterator.h"
#include "Iterator/MoveIterator.h"
#include "Range/Utility.h"
#include "Range/Pipe.h"
#include "Range/View.h"
#include "Range/AllView.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * A view adapter which dereferences to a rvalue reference.
 * When based on an input view, the move view satisfies at least an input view up to a random access view.
 * When based on a common view, the move view satisfies a common view.
 */
template <CInputRange V> requires (CView<V>)
class TMoveView : public IBasicViewInterface<TMoveView<V>>
{
public:

	using FElementType = TRangeElement<V>;
	using FReference = TRangeRValueReference<V>;

	FORCEINLINE constexpr TMoveView() requires (CDefaultConstructible<V>) = default;

	FORCEINLINE constexpr explicit TMoveView(V InBase) : Base(MoveTemp(InBase)) { }

	NODISCARD FORCEINLINE constexpr auto Begin() { return MakeMoveIterator(Range::Begin(Base)); }

	NODISCARD FORCEINLINE constexpr auto End()
	{
		if constexpr (CCommonRange<V>)
		{
			return MakeMoveIterator(Range::End(Base));
		}
		else return MakeMoveSentinel(Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const V>) { return MakeMoveIterator(Range::Begin(Base)); }

	NODISCARD FORCEINLINE constexpr auto End()   const requires (CRange<const V>)
	{
		if constexpr (CCommonRange<V>)
		{
			return MakeMoveIterator(Range::End(Base));
		}
		else return MakeMoveSentinel(Range::End(Base));
	}

	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CSizedRange<      V>) { return Range::Num(Base); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<const V>) { return Range::Num(Base); }

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

private:

	NO_UNIQUE_ADDRESS V Base;

};

template <typename R>
TMoveView(R&&) -> TMoveView<TAllView<R>>;

static_assert(        CInputRange<TMoveView<TAllView<IRange<        IInputIterator<int&>>>>>);
static_assert(      CForwardRange<TMoveView<TAllView<IRange<      IForwardIterator<int&>>>>>);
static_assert(CBidirectionalRange<TMoveView<TAllView<IRange<IBidirectionalIterator<int&>>>>>);
static_assert( CRandomAccessRange<TMoveView<TAllView<IRange< IRandomAccessIterator<int&>>>>>);
static_assert( CRandomAccessRange<TMoveView<TAllView<IRange<   IContiguousIterator<int&>>>>>);

static_assert(CCommonRange<TMoveView<TAllView<ICommonRange<IForwardIterator<int>>>>>);
static_assert(       CView<TMoveView<TAllView<      IRange<  IInputIterator<int>>>>>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TMoveView<T>> = bEnableBorrowedRange<T>;

NAMESPACE_BEGIN(Range)

/** Creates A view adapter that dereferences to a rvalue reference. */
template <CViewableRange R> requires (requires { TMoveView(DeclVal<R>()); })
NODISCARD FORCEINLINE constexpr auto Move(R&& Base)
{
	return TMoveView(Forward<R>(Base));
}

/** Creates A view adapter that dereferences to a rvalue reference. */
NODISCARD FORCEINLINE constexpr auto Move()
{
	using FClosure = decltype([]<CViewableRange R> requires (requires { Range::Move(DeclVal<R>()); }) (R&& Base)
	{
		return Range::Move(Forward<R>(Base));
	});

	return TAdaptorClosure<FClosure>();
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
