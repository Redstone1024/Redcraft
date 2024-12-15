#pragma once

#include "CoreTypes.h"
#include "Range/Range.h"
#include "Memory/Address.h"
#include "Numeric/Numeric.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Iterator.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename R>
using TRangeIteratorType = TRangeIterator<R>;

template <typename R>
using TRangeSentinelType = TRangeSentinel<R>;

template <typename R>
using TRangeElementType = TRangeElement<R>;

template <typename R>
using TRangeReferenceType = TRangeReference<R>;

template <typename R>
using TRangeRValueReferenceType = TRangeRValueReference<R>;

NAMESPACE_BEGIN(Range)

/** A view adapter that references the elements of some other range. */
template <CRange R> requires (CObject<R>)
class TRefView : public IBasicViewInterface<TRefView<R>>
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
class TOwningView : public IBasicViewInterface<TOwningView<R>>
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
class TFilterView : public IBasicViewInterface<TFilterView<V, Pred>>
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
class TTransformView : public IBasicViewInterface<TTransformView<V, F>>
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
