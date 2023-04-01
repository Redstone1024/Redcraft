#pragma once

#include "CoreTypes.h"
#include "Templates/Meta.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "Containers/ArrayView.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/ObserverPointer.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** TStaticArray is a container that encapsulates fixed size arrays. */
template <CObject T, size_t N>
struct TStaticArray final
{
private:

	template <bool bConst>
	class IteratorImpl;

public:

	using ElementType = T;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator = IteratorImpl<false>;
	using ConstIterator = IteratorImpl<true >;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Compares the contents of two arrays. */
	NODISCARD friend constexpr bool operator==(const TStaticArray& LHS, const TStaticArray& RHS) requires (CWeaklyEqualityComparable<ElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		for (size_t Index = 0; Index < LHS.Num(); ++Index)
		{
			if (LHS[Index] != RHS[Index]) return false;
		}

		return true;
	}

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend constexpr auto operator<=>(const TStaticArray& LHS, const TStaticArray& RHS) requires (CSynthThreeWayComparable<ElementType>)
	{
		const size_t NumToCompare = LHS.Num() < RHS.Num() ? LHS.Num() : RHS.Num();

		for (size_t Index = 0; Index < NumToCompare; ++Index)
		{
			if (const auto Result = SynthThreeWayCompare(LHS[Index], RHS[Index]); Result != 0) return Result;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr TObserverPtr<      ElementType[]> GetData()       { return TObserverPtr<      ElementType[]>(_); }
	NODISCARD FORCEINLINE constexpr TObserverPtr<const ElementType[]> GetData() const { return TObserverPtr<const ElementType[]>(_); }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr      Iterator Begin()       { return      Iterator(this, _);         }
	NODISCARD FORCEINLINE constexpr ConstIterator Begin() const { return ConstIterator(this, _);         }
	NODISCARD FORCEINLINE constexpr      Iterator End()         { return      Iterator(this, _ + Num()); }
	NODISCARD FORCEINLINE constexpr ConstIterator End()   const { return ConstIterator(this, _ + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE constexpr ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { return N; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr       ElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return _[Index]; }
	NODISCARD FORCEINLINE constexpr const ElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return _[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr       ElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE constexpr const ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr       ElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE constexpr const ElementType& Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TStaticArray. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TStaticArray& A) requires (CHashable<ElementType>)
	{
		size_t Result = 0;

		for (ConstIterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	/** Overloads the Swap algorithm for TStaticArray. */
	friend FORCEINLINE constexpr void Swap(TStaticArray& A, TStaticArray& B) requires (CSwappable<ElementType>) { Swap(A._, B._); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

	T _[N];

private:

	template <bool bConst>
	class IteratorImpl
	{
	public:

		using ElementType = TConditional<bConst, const T, T>;

		FORCEINLINE constexpr IteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE IteratorImpl(const IteratorImpl<false>& InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer)
		{ }
#		else
		FORCEINLINE IteratorImpl(const IteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer)
		{ }
#		endif

		FORCEINLINE constexpr IteratorImpl(const IteratorImpl&)            = default;
		FORCEINLINE constexpr IteratorImpl(IteratorImpl&&)                 = default;
		FORCEINLINE constexpr IteratorImpl& operator=(const IteratorImpl&) = default;
		FORCEINLINE constexpr IteratorImpl& operator=(IteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const IteratorImpl& LHS, const IteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const IteratorImpl& LHS, const IteratorImpl& RHS) { return LHS.Pointer <=> RHS.Pointer; }

		NODISCARD FORCEINLINE constexpr ElementType& operator*()  const { CheckThis(true); return *Pointer; }
		NODISCARD FORCEINLINE constexpr ElementType* operator->() const { CheckThis(true); return  Pointer; }

		NODISCARD FORCEINLINE constexpr ElementType& operator[](ptrdiff Index) const { IteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE constexpr IteratorImpl& operator++() { ++Pointer; CheckThis(); return *this; }
		FORCEINLINE constexpr IteratorImpl& operator--() { --Pointer; CheckThis(); return *this; }

		FORCEINLINE constexpr IteratorImpl operator++(int) { IteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr IteratorImpl operator--(int) { IteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE constexpr IteratorImpl& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
		FORCEINLINE constexpr IteratorImpl& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE constexpr IteratorImpl operator+(IteratorImpl Iter, ptrdiff Offset) { IteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr IteratorImpl operator+(ptrdiff Offset, IteratorImpl Iter) { IteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr IteratorImpl operator-(ptrdiff Offset) const { IteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const IteratorImpl& LHS, const IteratorImpl& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

		NODISCARD FORCEINLINE constexpr explicit operator TObserverPtr<ElementType[]>() const { CheckThis(); return TObserverPtr<ElementType[]>(Pointer); }

	private:

#		if DO_CHECK
		const TStaticArray* Owner = nullptr;
#		endif

		ElementType* Pointer = nullptr;

#		if DO_CHECK
		FORCEINLINE constexpr IteratorImpl(const TStaticArray* InContainer, ElementType* InPointer)
			: Owner(InContainer), Pointer(InPointer)
		{ }
#		else
		FORCEINLINE constexpr IteratorImpl(const TStaticArray* InContainer, ElementType* InPointer)
			: Pointer(InPointer)
		{ }
#		endif

		FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool> friend class IteratorImpl;

		friend TStaticArray;

	};

};

template <typename T, typename... U> requires (true && ... && CSameAs<T, U>)
TStaticArray(T, U...) -> TStaticArray<T, 1 + sizeof...(U)>;

/** Creates a TStaticArray object from a built-in array. */
template <typename T, size_t N>
constexpr TStaticArray<TRemoveCV<T>, N> ToArray(T(&  Array)[N])
{
	return [&Array]<size_t... Indices>(TIndexSequence<Indices...>) -> TStaticArray<TRemoveCV<T>, N> { return { Array[Indices]... }; } (TMakeIndexSequence<N>());
}

/** Creates a TStaticArray object from a built-in array. */
template <typename T, size_t N>
constexpr TStaticArray<TRemoveCV<T>, N> ToArray(T(&& Array)[N])
{
	return [&Array]<size_t... Indices>(TIndexSequence<Indices...>) -> TStaticArray<TRemoveCV<T>, N> { return { MoveTemp(Array[Indices])... }; } (TMakeIndexSequence<N>());
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

NAMESPACE_STD_BEGIN

// Support structure binding, should not be directly used.
template <typename T, size_t N> struct tuple_size<NAMESPACE_REDCRAFT::TStaticArray<T, N>> : integral_constant<size_t, N> { };
template <size_t I, typename T, size_t N> struct tuple_element<I, NAMESPACE_REDCRAFT::TStaticArray<T, N>> { using type = T; };

NAMESPACE_STD_END

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Support structure binding, should not be directly used.
template <size_t Index, typename T, size_t N> FORCEINLINE constexpr decltype(auto) get(      TStaticArray<T, N>&  InValue) { return static_cast<      TStaticArray<T, N>& >(InValue)[Index]; }
template <size_t Index, typename T, size_t N> FORCEINLINE constexpr decltype(auto) get(const TStaticArray<T, N>&  InValue) { return static_cast<const TStaticArray<T, N>& >(InValue)[Index]; }
template <size_t Index, typename T, size_t N> FORCEINLINE constexpr decltype(auto) get(      TStaticArray<T, N>&& InValue) { return static_cast<      TStaticArray<T, N>&&>(InValue)[Index]; }
template <size_t Index, typename T, size_t N> FORCEINLINE constexpr decltype(auto) get(const TStaticArray<T, N>&& InValue) { return static_cast<const TStaticArray<T, N>&&>(InValue)[Index]; }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
