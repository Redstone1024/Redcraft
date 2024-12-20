#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Iterators/Utility.h"
#include "Iterators/BasicIterator.h"
#include "Iterators/CountedIterator.h"
#include "Numerics/Math.h"
#include "Ranges/Utility.h"
#include "Ranges/Pipe.h"
#include "Ranges/View.h"
#include "Ranges/AllView.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Ranges)

/**
 * A view adapter that includes a specified number of elements from the beginning of a range.
 * When based on any view, the take view satisfies the corresponding any view.
 * When based on a random access and sized view, the take view satisfies a common view.
 */
template <CView V>
class TTakeView : public IBasicViewInterface<TTakeView<V>>
{
private:

	template <bool bConst> class FSentinelImpl;

public:

	FORCEINLINE constexpr TTakeView() requires (CDefaultConstructible<V>) = default;

	FORCEINLINE constexpr TTakeView(V InBase, size_t InCount) : Base(MoveTemp(InBase)), Count(InCount) { }

	NODISCARD FORCEINLINE constexpr auto Begin() requires (!CSimpleView<V>)
	{
		if constexpr (CSizedRange<V>)
		{
			if constexpr (CRandomAccessRange<V>)
			{
				return Ranges::Begin(Base);
			}
			else return MakeCountedIterator(Ranges::Begin(Base), Num());
		}
		else return MakeCountedIterator(Ranges::Begin(Base), Count);
	}

	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const V>)
	{
		if constexpr (CSizedRange<const V>)
		{
			if constexpr (CRandomAccessRange<const V>)
			{
				return Ranges::Begin(Base);
			}
			else return MakeCountedIterator(Ranges::Begin(Base), Num());
		}
		else return MakeCountedIterator(Ranges::Begin(Base), Count);
	}

	NODISCARD FORCEINLINE constexpr auto End() requires (!CSimpleView<V>)
	{
		if constexpr (CSizedRange<V>)
		{
			if constexpr (CRandomAccessRange<V>)
			{
				return Ranges::Begin(Base) + Num();
			}
			else return DefaultSentinel;
		}
		else return FSentinelImpl<false>(Ranges::End(Base));
	}

	NODISCARD FORCEINLINE constexpr auto End() const requires (CRange<const V>)
	{
		if constexpr (CSizedRange<const V>)
		{
			if constexpr (CRandomAccessRange<const V>)
			{
				return Ranges::Begin(Base) + Num();
			}
			else return DefaultSentinel;
		}
		else return FSentinelImpl<true>(Ranges::End(Base));
	}

	NODISCARD FORCEINLINE constexpr size_t Num()       requires (CSizedRange<      V>) { return Math::Min(Ranges::Num(Base), Count); }
	NODISCARD FORCEINLINE constexpr size_t Num() const requires (CSizedRange<const V>) { return Math::Min(Ranges::Num(Base), Count); }

	NODISCARD FORCEINLINE constexpr V GetBase() const& requires (CCopyConstructible<V>) { return          Base;  }
	NODISCARD FORCEINLINE constexpr V GetBase() &&                                      { return MoveTemp(Base); }

private:

	NO_UNIQUE_ADDRESS V Base;

	size_t Count;

	template <bool bConst>
	class FSentinelImpl final
	{
	private:

		using FBase = TConditional<bConst, const V, V>;

	public:

		FORCEINLINE constexpr FSentinelImpl() = default;

		FORCEINLINE constexpr FSentinelImpl(FSentinelImpl<!bConst> Sentinel) requires (bConst && CConvertibleTo<TRangeSentinel<V>, TRangeSentinel<FBase>>)
			: Current(Sentinel.Current)
		{ }

		NODISCARD FORCEINLINE constexpr bool operator==(const TCountedIterator<TRangeIterator<FBase>>& InValue) const&
		{
			return InValue.Num() == 0 || InValue.GetBase() == Current;
		}

		template <bool bOther = !bConst> requires (CSentinelFor<TRangeSentinel<FBase>, TRangeIterator<TConditional<bOther, const V, V>>>)
		NODISCARD FORCEINLINE constexpr bool operator==(const TCountedIterator<TRangeIterator<TConditional<bOther, const V, V>>>& InValue)
		{
			return InValue.Num() == 0 || InValue.GetBase() == Current;
		}

		NODISCARD FORCEINLINE constexpr TRangeSentinel<FBase> GetBase() const { return Current; }

	private:

		NO_UNIQUE_ADDRESS TRangeSentinel<FBase> Current;

		FORCEINLINE constexpr FSentinelImpl(TRangeSentinel<FBase> InCurrent) : Current(InCurrent) { }

		friend TTakeView;
	};

};

template <typename R>
TTakeView(R&&, size_t) -> TTakeView<TAllView<R>>;

static_assert(        CInputRange<TTakeView<TAllView<IRange<        IInputIterator<int&>>>>>);
static_assert(      CForwardRange<TTakeView<TAllView<IRange<      IForwardIterator<int&>>>>>);
static_assert(CBidirectionalRange<TTakeView<TAllView<IRange<IBidirectionalIterator<int&>>>>>);
static_assert( CRandomAccessRange<TTakeView<TAllView<IRange< IRandomAccessIterator<int&>>>>>);
static_assert(   CContiguousRange<TTakeView<TAllView<IRange<   IContiguousIterator<int&>>>>>);

static_assert(CCommonRange<TTakeView<TAllView<ISizedRange< IRandomAccessIterator<int>>>>>);
static_assert( CSizedRange<TTakeView<TAllView<ISizedRange<IInputOrOutputIterator<int>>>>>);
static_assert(       CView<TTakeView<TAllView<     IRange<IInputOrOutputIterator<int>>>>>);

static_assert(COutputRange<TTakeView<TAllView<IRange<IOutputIterator<int&>>>>, int>);

NAMESPACE_END(Ranges)

template <typename T>
constexpr bool bEnableBorrowedRange<Ranges::TTakeView<T>> = bEnableBorrowedRange<T>;

NAMESPACE_BEGIN(Ranges)

/** Creates A view adapter that includes a specified number of elements from the beginning of a range. */
template <CViewableRange R> requires (requires { TTakeView(DeclVal<R>(), DeclVal<size_t>()); })
NODISCARD FORCEINLINE constexpr auto Take(R&& Base, size_t Count)
{
	return TTakeView(Forward<R>(Base), Count);
}

/** Creates A view adapter that includes a specified number of elements from the beginning of a range. */
NODISCARD FORCEINLINE constexpr auto Take(size_t Count)
{
	using FClosure = decltype([]<CViewableRange R> requires (requires { Ranges::Take(DeclVal<R>(), DeclVal<size_t>()); }) (R&& Base, size_t Count)
	{
		return Ranges::Take(Forward<R>(Base), Count);
	});

	return TAdaptorClosure<FClosure, size_t>(Count);
}

NAMESPACE_END(Ranges)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
