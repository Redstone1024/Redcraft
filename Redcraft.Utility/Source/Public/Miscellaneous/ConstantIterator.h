#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Templates/Utility.h"
#include "Miscellaneous/Iterator.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** An iterator that always points to the same value. */
template <typename T> requires (CDestructible<T> || CLValueReference<T>)
class TConstantIterator final
{
public:

	using ElementType = TRemoveCV<T>;

	FORCEINLINE constexpr TConstantIterator() = default;

	FORCEINLINE constexpr ~TConstantIterator() = default;

	template <typename U = T> requires (!CSameAs<TConstantIterator, TRemoveCVRef<U>> && CConstructibleFrom<T, U&&>)
	FORCEINLINE constexpr explicit TConstantIterator(U&& InValue) : Value(Forward<U>(InValue)) { }

	template <typename U> requires (CConstructibleFrom<T, const U&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const U&, T>) TConstantIterator(const TConstantIterator<U>& InValue) : Value(InValue.Value) { }

	template <typename U> requires (CConstructibleFrom<T, U&&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&&, T>) TConstantIterator(TConstantIterator<U>&& InValue) : Value(MoveTemp(InValue.Value)) { }

	FORCEINLINE constexpr TConstantIterator(const TConstantIterator&) requires (CCopyConstructible<T>) = default;

	FORCEINLINE constexpr TConstantIterator(TConstantIterator&&) requires (CMoveConstructible<T>) = default;

	template <typename U> requires (CConvertibleTo<const U&, T> && CAssignableFrom<T&, const U&>)
	FORCEINLINE constexpr TConstantIterator& operator=(const TConstantIterator<U>& InValue) { Value = InValue.Value; return *this; }

	template <typename U> requires (CConvertibleTo<U&&, T> && CAssignableFrom<T&, U&&>)
	FORCEINLINE constexpr TConstantIterator& operator=(TConstantIterator<U>&& InValue) { Value = MoveTemp(InValue.Value); return *this; }

	FORCEINLINE constexpr TConstantIterator& operator=(const TConstantIterator&) requires (CCopyAssignable<T>) = default;

	FORCEINLINE constexpr TConstantIterator& operator=(TConstantIterator&&) requires (CMoveAssignable<T>) = default;

	NODISCARD FORCEINLINE constexpr const T& operator*()  const { return           Value;  }
	NODISCARD FORCEINLINE constexpr const T* operator->() const { return AddressOf(Value); }

	FORCEINLINE constexpr TConstantIterator& operator++() { return *this; }

	FORCEINLINE constexpr void operator++(int) { }

private:

	T Value;

	template <typename U> requires (CDestructible<U> || CLValueReference<U>)
	friend class TConstantIterator;

};

static_assert(CInputIterator<TConstantIterator<int>>);

/** An iterator that always points to the same value. */
template <typename T>
class TConstantIterator<T&> final
{
public:

	using ElementType = TRemoveCV<T>;

	FORCEINLINE constexpr TConstantIterator() = default;

	FORCEINLINE constexpr TConstantIterator(const TConstantIterator&)            = default;
	FORCEINLINE constexpr TConstantIterator(TConstantIterator&&)                 = default;
	FORCEINLINE constexpr TConstantIterator& operator=(const TConstantIterator&) = default;
	FORCEINLINE constexpr TConstantIterator& operator=(TConstantIterator&&)      = default;

	FORCEINLINE constexpr ~TConstantIterator() = default;

	FORCEINLINE constexpr explicit TConstantIterator(const T& InValue) : Ptr(AddressOf(InValue)) { }

	FORCEINLINE constexpr explicit TConstantIterator(const T&& InValue) = delete;

	template <typename U> requires (CConvertibleTo<U*, T*>)
	FORCEINLINE constexpr TConstantIterator(const TConstantIterator<U>& InValue) : Ptr(InValue.Ptr) { }

	template <typename U> requires (CConvertibleTo<U*, T*>)
	FORCEINLINE constexpr TConstantIterator& operator=(const TConstantIterator<U>& InValue) { Ptr = InValue.Ptr; return *this; }

	NODISCARD FORCEINLINE constexpr const T& operator*()  const { return *Ptr; }
	NODISCARD FORCEINLINE constexpr const T* operator->() const { return  Ptr; }

	FORCEINLINE constexpr TConstantIterator& operator++() { return *this; }

	FORCEINLINE constexpr void operator++(int) { }

private:

	const T* Ptr;

	template <typename U> requires (CDestructible<U> || CLValueReference<U>)
	friend class TConstantIterator;

};

static_assert(CInputIterator<TConstantIterator<int&>>);

/** An iterator adapter specialization that tracks the distance of a constant iterator to the end of the range. */
template <typename T> requires (CDestructible<T> || CLValueReference<T>)
class TCountedIterator<TConstantIterator<T>> final
{
public:

	using IteratorType = TConstantIterator<T>;

	using ElementType = typename TConstantIterator<T>::ElementType;

#	if DO_CHECK
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<IteratorType>) : Length(1), MaxLength(0) { }
#	else
	FORCEINLINE constexpr TCountedIterator() requires (CDefaultConstructible<IteratorType>) = default;
#	endif

	FORCEINLINE constexpr TCountedIterator(const TCountedIterator&)            = default;
	FORCEINLINE constexpr TCountedIterator(TCountedIterator&&)                 = default;
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator&) = default;
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator&&)      = default;

	FORCEINLINE constexpr ~TCountedIterator() = default;

	template <typename U = IteratorType> requires (!CSameAs<TCountedIterator, TRemoveCVRef<U>> && CConstructibleFrom<IteratorType, U>)
	FORCEINLINE constexpr explicit TCountedIterator(U&& InValue, ptrdiff N) : Current(Forward<U>(InValue)), Length(N) { check_code({ MaxLength = N; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, const J&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const J&, IteratorType>) TCountedIterator(const TCountedIterator<J>& InValue) : Current(InValue.Current), Length(InValue.Num()) { check_code({ MaxLength = InValue.MaxLength; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConstructibleFrom<IteratorType, J>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<J&&, IteratorType>) TCountedIterator(TCountedIterator<J>&& InValue) : Current(MoveTemp(InValue).Current), Length(InValue.Num()) { check_code({ MaxLength = InValue.MaxLength; }); }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<const J&, IteratorType> && CAssignableFrom<IteratorType&, const J&>)
	FORCEINLINE constexpr TCountedIterator& operator=(const TCountedIterator<J>& InValue) { Current = InValue.Current; Length = InValue.Num(); check_code({ MaxLength = InValue.MaxLength; }); return *this; }

	template <CInputOrOutputIterator J> requires (!CSameAs<IteratorType, J> && CConvertibleTo<J&&, IteratorType> && CAssignableFrom<IteratorType&, J&&>)
	FORCEINLINE constexpr TCountedIterator& operator=(TCountedIterator<J>&& InValue) { Current = MoveTemp(InValue).Current; Length = InValue.Num(); check_code({ MaxLength = InValue.MaxLength; }); return *this; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length == RHS.Length; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { return LHS.Length <=> RHS.Length; }

	NODISCARD FORCEINLINE constexpr bool operator==(FDefaultSentinel) const& { return Length == static_cast<ptrdiff>(0); }

	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(FDefaultSentinel) const& { return static_cast<ptrdiff>(0) <=> Length; }

	NODISCARD FORCEINLINE constexpr const TRemoveReference<T>& operator*()  const { CheckThis(true ); return          *Current;  }
	NODISCARD FORCEINLINE constexpr const TRemoveReference<T>* operator->() const { CheckThis(false); return ToAddress(Current); }

	NODISCARD FORCEINLINE constexpr const TRemoveReference<T>& operator[](ptrdiff) const { return *this; }

	FORCEINLINE constexpr TCountedIterator& operator++() { --Length; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator--() { ++Length; CheckThis(); return *this; }

	FORCEINLINE constexpr TCountedIterator operator++(int) { TCountedIterator Temp = *this; --Length; CheckThis(); return Temp; }
	FORCEINLINE constexpr TCountedIterator operator--(int) { TCountedIterator Temp = *this; ++Length; CheckThis(); return Temp; }

	FORCEINLINE constexpr TCountedIterator& operator+=(ptrdiff Offset) { Length -= Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TCountedIterator& operator-=(ptrdiff Offset) { Length += Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(TCountedIterator Iter, ptrdiff Offset) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TCountedIterator operator+(ptrdiff Offset, TCountedIterator Iter) { TCountedIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TCountedIterator operator-(ptrdiff Offset) const { TCountedIterator Temp = *this; Temp -= Offset; return Temp; }

	template <CCommonType<IteratorType> J>
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, const TCountedIterator<J>& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Length - RHS.Length; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TCountedIterator& LHS, FDefaultSentinel) { LHS.CheckThis(); return -LHS.Num(); }
	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(FDefaultSentinel, const TCountedIterator& RHS) { RHS.CheckThis(); return  RHS.Num(); }

	NODISCARD FORCEINLINE constexpr const IteratorType& GetBase() const& { CheckThis(); return          Current;  }
	NODISCARD FORCEINLINE constexpr       IteratorType  GetBase() &&     { CheckThis(); return MoveTemp(Current); }
	NODISCARD FORCEINLINE constexpr       ptrdiff           Num() const  { CheckThis(); return          Length;   }

private:

	IteratorType Current;
	ptrdiff      Length;

#	if DO_CHECK
	ptrdiff MaxLength;
#	endif

	FORCEINLINE void CheckThis(bool bExceptEnd = false) const
	{
		checkf(static_cast<ptrdiff>(0) <= Length && Length <= MaxLength, TEXT("Read access violation. Please check Num()."));
		checkf(!(bExceptEnd && Length == static_cast<ptrdiff>(0)),       TEXT("Read access violation. Please check Num()."));
	}

	template <CInputOrOutputIterator J>
	friend class TCountedIterator;

};

static_assert(CRandomAccessIterator<TCountedIterator<TConstantIterator<int>>>);
static_assert(CSizedSentinelFor<FDefaultSentinel, TCountedIterator<TConstantIterator<int>>>);

template <typename T> requires (CDestructible<T> || CLValueReference<T>)
NODISCARD FORCEINLINE constexpr auto MakeConstantIterator(T&& Value)
{
	return TConstantIterator<T>(Forward<T>(Value));
}

template <typename T> requires (CDestructible<T> || CLValueReference<T>)
NODISCARD FORCEINLINE constexpr auto MakeCountedConstantIterator(T&& Value, ptrdiff N)
{
	return TCountedIterator<TConstantIterator<T>>(MakeConstantIterator(Forward<T>(Value)), N);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
