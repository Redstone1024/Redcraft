#pragma once

#include "CoreTypes.h"
#include "Range/View.h"
#include "Range/Pipe.h"
#include "Range/Utility.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * A view adapter that references other range.
 * No matter which it is base range, the reference view always satisfies the same range concept.
 */
template <CRange R> requires (CObject<R>)
class TRefView : public IBasicViewInterface<TRefView<R>>
{
private:

	// Use the function to check constructability.
	static void Func(R&);
	static void Func(R&&) = delete;

public:

	template <typename T> requires (!CSameAs<TRemoveCVRef<T>, TRefView> && CConvertibleTo<T, R&> && requires { Func(DeclVal<T>()); })
	FORCEINLINE constexpr TRefView(T&& InRange) : Ptr(AddressOf(static_cast<R&>(Forward<T>(InRange)))) { }

	NODISCARD FORCEINLINE constexpr TRangeIterator<R> Begin() const { return Range::Begin(*Ptr); }
	NODISCARD FORCEINLINE constexpr TRangeSentinel<R> End()   const { return Range::End  (*Ptr); }

	NODISCARD FORCEINLINE constexpr auto   GetData() const requires (CContiguousRange<R>)                          { return Range::GetData(*Ptr); }
	NODISCARD FORCEINLINE constexpr size_t Num()     const requires (CSizedRange<R>)                               { return Range::Num    (*Ptr); }
	NODISCARD FORCEINLINE constexpr bool   IsEmpty() const requires (requires(R Range) { Range::IsEmpty(Range); }) { return Range::IsEmpty(*Ptr); }

	NODISCARD FORCEINLINE constexpr R& GetBase() const { return *Ptr; }

private:

	R* Ptr;

};

template <typename R>
TRefView(R&) -> TRefView<R>;

static_assert(        CInputRange<TRefView<IRange<        IInputIterator<int&>>>>);
static_assert(      CForwardRange<TRefView<IRange<      IForwardIterator<int&>>>>);
static_assert(CBidirectionalRange<TRefView<IRange<IBidirectionalIterator<int&>>>>);
static_assert( CRandomAccessRange<TRefView<IRange< IRandomAccessIterator<int&>>>>);
static_assert(   CContiguousRange<TRefView<IRange<   IContiguousIterator<int&>>>>);

static_assert(CCommonRange<TRefView<ICommonRange<      IForwardIterator<int>>>>);
static_assert( CSizedRange<TRefView< ISizedRange<IInputOrOutputIterator<int>>>>);
static_assert(       CView<TRefView<      IRange<IInputOrOutputIterator<int>>>>);

static_assert(COutputRange<TRefView<IRange<IOutputIterator<int&>>>, int>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TRefView<T>> = true;

NAMESPACE_BEGIN(Range)

/**
 * A view adapter that has unique ownership of a range.
 * No matter which it is base range, the reference view always satisfies the same range concept.
 * Specify, the base range type must be movable, and the owning view always is movable but not copyable.
 */
template <CRange R> requires (CMovable<R> && !NAMESPACE_PRIVATE::TIsInitializerList<R>::Value)
class TOwningView : public IBasicViewInterface<TOwningView<R>>
{
public:

	FORCEINLINE constexpr TOwningView() requires (CDefaultConstructible<R>) = default;

	FORCEINLINE constexpr TOwningView(const TOwningView&) = delete;
	FORCEINLINE constexpr TOwningView(TOwningView&&)      = default;

	FORCEINLINE constexpr TOwningView(R&& InRange) : Base(MoveTemp(InRange)) { }

	FORCEINLINE constexpr TOwningView& operator=(const TOwningView&) = delete;
	FORCEINLINE constexpr TOwningView& operator=(TOwningView&&)      = default;

	NODISCARD FORCEINLINE constexpr auto Begin()                                  { return Range::Begin(Base); }
	NODISCARD FORCEINLINE constexpr auto End()                                    { return Range::End  (Base); }
	NODISCARD FORCEINLINE constexpr auto Begin() const requires (CRange<const R>) { return Range::Begin(Base); }
	NODISCARD FORCEINLINE constexpr auto End()   const requires (CRange<const R>) { return Range::End  (Base); }

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

static_assert(        CInputRange<TOwningView<IRange<        IInputIterator<int&>>>>);
static_assert(      CForwardRange<TOwningView<IRange<      IForwardIterator<int&>>>>);
static_assert(CBidirectionalRange<TOwningView<IRange<IBidirectionalIterator<int&>>>>);
static_assert( CRandomAccessRange<TOwningView<IRange< IRandomAccessIterator<int&>>>>);
static_assert(   CContiguousRange<TOwningView<IRange<   IContiguousIterator<int&>>>>);

static_assert(CCommonRange<TOwningView<ICommonRange<      IForwardIterator<int>>>>);
static_assert( CSizedRange<TOwningView< ISizedRange<IInputOrOutputIterator<int>>>>);
static_assert(       CView<TOwningView<      IRange<IInputOrOutputIterator<int>>>>);

static_assert(COutputRange<TOwningView<IRange<IOutputIterator<int&>>>, int>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TOwningView<T>> = bEnableBorrowedRange<T>;

NAMESPACE_BEGIN(Range)

/** Creates A view adapter that includes all elements of a range. */
template <CViewableRange R>
NODISCARD FORCEINLINE constexpr auto All(R&& InRange)
{
	if constexpr (CView<TDecay<R>>)
	{
		return TDecay<R>(Forward<R>(InRange));
	}

	else if constexpr (requires { TRefView(Forward<R>(InRange)); })
	{
		return TRefView(Forward<R>(InRange));
	}

	else return TOwningView(Forward<R>(InRange));
}

/** Creates A view adapter that includes all elements of a range. */
NODISCARD FORCEINLINE constexpr auto All()
{
	return TAdaptorClosure([]<CViewableRange R> requires (requires { All(DeclVal<R&&>()); }) (R && Base)
	{
		return All(Forward<R>(Base));
	});
}

/** A view adapter that includes all elements of a range. */
template <CViewableRange R>
using TAllView = decltype(Range::All(DeclVal<R>()));

static_assert(        CInputRange<TAllView<IRange<        IInputIterator<int&>>>>);
static_assert(      CForwardRange<TAllView<IRange<      IForwardIterator<int&>>>>);
static_assert(CBidirectionalRange<TAllView<IRange<IBidirectionalIterator<int&>>>>);
static_assert( CRandomAccessRange<TAllView<IRange< IRandomAccessIterator<int&>>>>);
static_assert(   CContiguousRange<TAllView<IRange<   IContiguousIterator<int&>>>>);

static_assert(CCommonRange<TAllView<ICommonRange<      IForwardIterator<int>>>>);
static_assert( CSizedRange<TAllView< ISizedRange<IInputOrOutputIterator<int>>>>);
static_assert(       CView<TAllView<      IRange<IInputOrOutputIterator<int>>>>);

static_assert(COutputRange<TAllView<IRange<IOutputIterator<int&>>>, int>);

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
