#pragma once

#include "CoreTypes.h"
#include "Range/View.h"
#include "Range/Utility.h"
#include "Memory/Address.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/** A view type that produces a view of no elements of a particular type. */
template <CObject T>
class TEmptyView : public IBasicViewInterface<TEmptyView<T>>
{
public:

	using ElementType = T;
	using Reference   = T&;
	using Iterator    = T*;
	using Sentinel    = T*;

	FORCEINLINE constexpr TEmptyView() = default;

	NODISCARD static FORCEINLINE constexpr Iterator Begin()   { return nullptr; }
	NODISCARD static FORCEINLINE constexpr Sentinel End()     { return nullptr; }
	NODISCARD static FORCEINLINE constexpr T*       GetData() { return nullptr; }
	NODISCARD static FORCEINLINE constexpr size_t   Num()     { return 0;       }
	NODISCARD static FORCEINLINE constexpr bool     IsEmpty() { return true;    }

};

static_assert(CContiguousRange<TEmptyView<int>>);
static_assert(    CCommonRange<TEmptyView<int>>);
static_assert(     CSizedRange<TEmptyView<int>>);
static_assert(           CView<TEmptyView<int>>);

NAMESPACE_END(Range)

template <typename T>
constexpr bool bEnableBorrowedRange<Range::TEmptyView<T>> = true;

NAMESPACE_BEGIN(Range)

/** A view type that contains exactly one element of a specified value. */
template <CObject T> requires (CMoveConstructible<T>)
class TSingleView : public IBasicViewInterface<TSingleView<T>>
{
public:

	using ElementType = T;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator =       T*;
	using ConstIterator = const T*;

	using      Sentinel =       T*;
	using ConstSentinel = const T*;

	FORCEINLINE constexpr TSingleView() requires (CDefaultConstructible<T>) = default;

	FORCEINLINE constexpr explicit TSingleView(const T& InValue) requires (CCopyConstructible<T>) : Value(InValue) { }

	FORCEINLINE constexpr explicit TSingleView(T&& InValue) : Value(MoveTemp(InValue)) { }

	template <typename... Ts> requires (CConstructibleFrom<T, Ts...>)
	FORCEINLINE constexpr explicit TSingleView(FInPlace, Ts&&... Args) : Value(Forward<Ts>(Args)...) { }

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
static_assert(     CSizedRange<TSingleView<int>>);
static_assert(           CView<TSingleView<int>>);

/** A view type that generates a sequence of elements by repeatedly incrementing an initial value. Can be either bounded or unbounded. */
template <CWeaklyIncrementable W, CWeaklyEqualityComparable<W> S = FUnreachableSentinel> requires (CSemiregular<S> && CCopyable<W>)
class TIotaView : public IBasicViewInterface<TIotaView<W, S>>
{
private:

	class FIteratorImpl;
	class FSentinelImpl;

public:

	using ElementType = TRemoveCV<W>;

	using Reference = W;

	using Iterator = FIteratorImpl;

	using Sentinel = TConditional<CSameAs<W, S>, FIteratorImpl, FSentinelImpl>;

	FORCEINLINE constexpr TIotaView() requires (CDefaultConstructible<W>) = default;

	FORCEINLINE constexpr explicit TIotaView(W InValue) requires (CDefaultConstructible<S>) : First(InValue), Last() { }

	FORCEINLINE constexpr explicit TIotaView(TIdentity<W> InValue, TIdentity<S> InLast) : First(InValue), Last(InLast) { }

	FORCEINLINE constexpr explicit TIotaView(Iterator InFirst, Sentinel InLast) : First(InFirst.Value), Last(InLast.Value) { }

	FORCEINLINE constexpr explicit TIotaView(Iterator InFirst, FUnreachableSentinel) requires (CSameAs<S, FUnreachableSentinel>) : First(InFirst.Value) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Iterator(First); }

	NODISCARD FORCEINLINE constexpr Sentinel End() const { return Sentinel(Last); }

	NODISCARD FORCEINLINE constexpr size_t Num() const requires ((CIntegral<W> && CIntegral<S>) || CSizedSentinelFor<S, W>) { return Last - First; }

	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return First == Last; }

private:

	NO_UNIQUE_ADDRESS W First;
	NO_UNIQUE_ADDRESS S Last;

	class FIteratorImpl final
	{
	public:

		using ElementType = TRemoveCV<W>;

		FORCEINLINE constexpr FIteratorImpl() requires (CDefaultConstructible<W>) = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIteratorImpl& LHS, const FIteratorImpl& RHS) requires (CEqualityComparable<W>) { return LHS.Value == RHS.Value; }

		NODISCARD FORCEINLINE constexpr Reference operator*()  const { return           Value;  }
		NODISCARD FORCEINLINE constexpr const W*  operator->() const { return AddressOf(Value); }

		FORCEINLINE constexpr FIteratorImpl& operator++() { ++Value; return *this; }

		FORCEINLINE constexpr FIteratorImpl operator++(int) { FIteratorImpl Temp = *this; ++Value; return Temp; }

	private:

		NO_UNIQUE_ADDRESS W Value;

		constexpr explicit FIteratorImpl(W InValue) : Value(InValue) { }

		friend FSentinelImpl;
		friend TIotaView;
	};

	class FSentinelImpl final
	{
	public:

		FORCEINLINE constexpr FSentinelImpl() = default;

		NODISCARD FORCEINLINE constexpr bool operator==(const FIteratorImpl& InValue) const& { return Value == InValue.Value; }

	private:

		NO_UNIQUE_ADDRESS S Value;

		FORCEINLINE constexpr FSentinelImpl(S InValue) : Value(InValue) { }

		friend TIotaView;
	};
};

template <typename T, typename U>
TIotaView(T, U) -> TIotaView<T, U>;

static_assert(CForwardRange<TIotaView<int>>);
static_assert(        CView<TIotaView<int>>);

NAMESPACE_END(Range)

template <typename T, typename U>
constexpr bool bEnableBorrowedRange<Range::TIotaView<T, U>> = true;

NAMESPACE_BEGIN(Range)

/** A view type that generates a sequence of elements by repeatedly producing the same value. Can be either bounded or unbounded. */
template <CObject W, bool bIsUnreachable = true> requires (CMoveConstructible<W> && CSameAs<W, TRemoveCV<W>>)
class TRepeatView : public IBasicViewInterface<TRepeatView<W, bIsUnreachable>>
{
private:

	class FIteratorImpl;

public:

	using ElementType = W;

	using Reference = const W&;

	using Iterator = FIteratorImpl;

	using Sentinel = TConditional<bIsUnreachable, FUnreachableSentinel, Iterator>;

	FORCEINLINE constexpr TRepeatView() requires (CDefaultConstructible<W>) = default;

	FORCEINLINE constexpr explicit TRepeatView(const W& InValue) requires (bIsUnreachable && CCopyConstructible<W>) : Value(InValue) { }

	FORCEINLINE constexpr explicit TRepeatView(W&& InValue) requires (bIsUnreachable) : Value(MoveTemp(InValue)) { }

	FORCEINLINE constexpr explicit TRepeatView(const W& InValue, size_t InCount) requires (!bIsUnreachable && CCopyConstructible<W>) : Value(MoveTemp(InValue)), Count(InCount) { }

	FORCEINLINE constexpr explicit TRepeatView(W&& InValue, size_t InCount) requires (!bIsUnreachable) : Value(MoveTemp(InValue)), Count(InCount) { }

	template <typename... Ts> requires (CConstructibleFrom<W, Ts...>)
	FORCEINLINE constexpr explicit TRepeatView(FInPlace, Ts&&... Args, size_t InCount) : Value(Forward<Ts>(Args)...), Count(InCount) { }

	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Iterator(Value, 0); }

	NODISCARD FORCEINLINE constexpr Sentinel End() const
	{
		if constexpr (bIsUnreachable)
		{
			return UnreachableSentinel;
		}

		else return Sentinel(Value, Count);
	}

	NODISCARD FORCEINLINE constexpr size_t Num() const requires (!bIsUnreachable) { return Count; }

private:

	using FSizeType = TConditional<bIsUnreachable, FUnreachableSentinel, size_t>;

	NO_UNIQUE_ADDRESS W Value;

	NO_UNIQUE_ADDRESS FSizeType Count;

public:

	class FIteratorImpl final
	{
	public:

		using ElementType = W;

		FORCEINLINE constexpr FIteratorImpl() requires (CDefaultConstructible<W>) = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIteratorImpl& LHS, const FIteratorImpl& RHS) { return LHS.Current == RHS.Current; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const FIteratorImpl& LHS, const FIteratorImpl& RHS) { return LHS.Current <=> RHS.Current; }

		NODISCARD FORCEINLINE constexpr Reference operator*()  const { return *Ptr; }
		NODISCARD FORCEINLINE constexpr const W*  operator->() const { return  Ptr; }

		NODISCARD FORCEINLINE constexpr Reference operator[](ptrdiff) const { return *Ptr; }

		FORCEINLINE constexpr FIteratorImpl& operator++() { ++Current; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator--() { --Current; return *this; }

		FORCEINLINE constexpr FIteratorImpl operator++(int) { FIteratorImpl Temp = *this; --Current; return Temp; }
		FORCEINLINE constexpr FIteratorImpl operator--(int) { FIteratorImpl Temp = *this; ++Current; return Temp; }

		FORCEINLINE constexpr FIteratorImpl& operator+=(ptrdiff Offset) { Current -= Offset; return *this; }
		FORCEINLINE constexpr FIteratorImpl& operator-=(ptrdiff Offset) { Current += Offset; return *this; }

		NODISCARD friend FORCEINLINE constexpr FIteratorImpl operator+(FIteratorImpl Iter, ptrdiff Offset) { FIteratorImpl Temp = Iter; Temp -= Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr FIteratorImpl operator+(ptrdiff Offset, FIteratorImpl Iter) { FIteratorImpl Temp = Iter; Temp -= Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr FIteratorImpl operator-(ptrdiff Offset) const { FIteratorImpl Temp = *this; Temp += Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIteratorImpl& LHS, const FIteratorImpl& RHS) { return RHS.Current - LHS.Current; }

	private:

		const W* Ptr;

		NO_UNIQUE_ADDRESS size_t Current;

		FORCEINLINE constexpr FIteratorImpl(const W& InObject, size_t InCurrent) : Ptr(AddressOf(InObject)), Current(InCurrent) { }

		friend TRepeatView;
	};

};

template <typename W>
TRepeatView(W) -> TRepeatView<W>;

template <typename W>
TRepeatView(W, size_t) -> TRepeatView<W, false>;

static_assert(CRandomAccessRange<TRepeatView<int, false>>);
static_assert(      CCommonRange<TRepeatView<int, false>>);
static_assert(       CSizedRange<TRepeatView<int, false>>);
static_assert(             CView<TRepeatView<int, false>>);

NAMESPACE_END(Range)

NAMESPACE_BEGIN(Range)

/** A view of no elements of a particular type. */
template <CObject T>
inline constexpr TEmptyView<T> Empty;

/** Creates a view that contains exactly one element of a specified value. */
template <typename T> requires (CObject<TDecay<T>> && CMoveConstructible<TDecay<T>>)
NODISCARD FORCEINLINE constexpr TSingleView<TDecay<T>> Single(T&& Value)
{
	return TSingleView<TDecay<T>>(Forward<T>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly incrementing an initial value. */
template <typename W> requires (CWeaklyIncrementable<TDecay<W>> && CCopyable<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TIotaView<TDecay<W>> Iota(W&& Value)
{
	return TIotaView<TDecay<W>>(Forward<W>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly incrementing an initial value. */
template <typename W, typename S> requires (CWeaklyIncrementable<TDecay<W>> && CWeaklyEqualityComparable<W, S> && CCopyable<TDecay<W>> && CSemiregular<TDecay<S>>)
NODISCARD FORCEINLINE constexpr TIotaView<TDecay<W>, TDecay<S>> Iota(W&& Value, S&& Last)
{
	return TIotaView<TDecay<W>, TDecay<S>>(Forward<W>(Value), Forward<S>(Last));
}

/** Creates a view that generates a sequence of elements by repeatedly producing the same value. */
template <typename W> requires (CObject<TDecay<W>> && CMoveConstructible<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TRepeatView<TDecay<W>> Repeat(W&& Value)
{
	return TRepeatView<TDecay<W>>(Forward<W>(Value));
}

/** Creates a view that generates a sequence of elements by repeatedly producing the same value. */
template <typename W> requires (CObject<TDecay<W>> && CMoveConstructible<TDecay<W>>)
NODISCARD FORCEINLINE constexpr TRepeatView<TDecay<W>, false> Repeat(W&& Value, size_t Count)
{
	return TRepeatView<TDecay<W>, false>(Forward<W>(Value), Count);
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
