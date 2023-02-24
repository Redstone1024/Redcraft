#pragma once

#include "CoreTypes.h"
#include "Templates/Meta.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/ObserverPointer.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename ArrayType, typename T>
class TStaticArrayIterator
{
public:

	using ElementType = T;

	FORCEINLINE constexpr TStaticArrayIterator() = default;

#	if DO_CHECK
	FORCEINLINE constexpr TStaticArrayIterator(const TStaticArrayIterator<ArrayType, TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Owner(InValue.Owner), Pointer(InValue.Pointer)
	{ }
#	else
	FORCEINLINE constexpr TStaticArrayIterator(const TStaticArrayIterator<ArrayType, TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Pointer(InValue.Pointer)
	{ }
#	endif

	FORCEINLINE constexpr TStaticArrayIterator(const TStaticArrayIterator&)            = default;
	FORCEINLINE constexpr TStaticArrayIterator(TStaticArrayIterator&&)                 = default;
	FORCEINLINE constexpr TStaticArrayIterator& operator=(const TStaticArrayIterator&) = default;
	FORCEINLINE constexpr TStaticArrayIterator& operator=(TStaticArrayIterator&&)      = default;

	NODISCARD friend FORCEINLINE constexpr bool operator==(const TStaticArrayIterator& LHS, const TStaticArrayIterator& RHS) { return LHS.Pointer == RHS.Pointer; }

	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TStaticArrayIterator & LHS, const TStaticArrayIterator & RHS) { return LHS.Pointer <=> RHS.Pointer; }

	NODISCARD FORCEINLINE constexpr ElementType& operator*()  const { CheckThis(true); return *Pointer; }
	NODISCARD FORCEINLINE constexpr ElementType* operator->() const { CheckThis(true); return  Pointer; }

	NODISCARD FORCEINLINE constexpr ElementType& operator[](ptrdiff Index) const { TStaticArrayIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE constexpr TStaticArrayIterator& operator++() { ++Pointer; CheckThis(); return *this; }
	FORCEINLINE constexpr TStaticArrayIterator& operator--() { --Pointer; CheckThis(); return *this; }

	FORCEINLINE constexpr TStaticArrayIterator operator++(int) { TStaticArrayIterator Temp = *this; ++Pointer; CheckThis(); return Temp; }
	FORCEINLINE constexpr TStaticArrayIterator operator--(int) { TStaticArrayIterator Temp = *this; --Pointer; CheckThis(); return Temp; }

	FORCEINLINE constexpr TStaticArrayIterator& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TStaticArrayIterator& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE constexpr TStaticArrayIterator operator+(TStaticArrayIterator Iter, ptrdiff Offset) { TStaticArrayIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TStaticArrayIterator operator+(ptrdiff Offset, TStaticArrayIterator Iter) { TStaticArrayIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TStaticArrayIterator operator-(ptrdiff Offset) const { TStaticArrayIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TStaticArrayIterator& LHS, const TStaticArrayIterator& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	NODISCARD FORCEINLINE constexpr explicit operator       ElementType*()       requires (!CConst<ElementType>) { CheckThis(); return Pointer; }
	NODISCARD FORCEINLINE constexpr explicit operator const ElementType*() const                                 { CheckThis(); return Pointer; }

private:

#	if DO_CHECK
	const ArrayType* Owner = nullptr;
#	endif

	ElementType* Pointer = nullptr;

#	if DO_CHECK
	FORCEINLINE constexpr TStaticArrayIterator(const ArrayType* InContainer, ElementType* InPointer)
		: Owner(InContainer), Pointer(InPointer)
	{ }
#	else
	FORCEINLINE constexpr TStaticArrayIterator(const ArrayType* InContainer, ElementType* InPointer)
		: Pointer(InPointer)
	{ }
#	endif

	FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
	{
		checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
		checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
	}

	friend ArrayType;

	template <typename InArrayType, typename InElementType>
	friend class TStaticArrayIterator;

};

NAMESPACE_PRIVATE_END

/** TStaticArray is a container that encapsulates fixed size arrays. */
template <CObject T, size_t N>
struct TStaticArray final
{

	using ElementType = T;
	
	using      Iterator = NAMESPACE_PRIVATE::TStaticArrayIterator<TStaticArray,       ElementType>;
	using ConstIterator = NAMESPACE_PRIVATE::TStaticArrayIterator<TStaticArray, const ElementType>;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Compares the contents of two arrays. */
	NODISCARD friend constexpr bool operator==(const TStaticArray& LHS, const TStaticArray& RHS) requires (CWeaklyEqualityComparable<ElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		ConstIterator LHSIter = LHS.Begin();
		ConstIterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End())
		{
			if (*LHSIter != *RHSIter) return false;

			++LHSIter;
			++RHSIter;
		}

		check(RHSIter == RHS.End());

		return true;
	}

	/** Compares the contents of two arrays. */
	NODISCARD friend constexpr auto operator<=>(const TStaticArray& LHS, const TStaticArray& RHS) requires (CSynthThreeWayComparable<ElementType>)
	{
		using OrderingType = TSynthThreeWayResult<ElementType>;

		if (LHS.Num() < RHS.Num()) return OrderingType::less;
		if (LHS.Num() > RHS.Num()) return OrderingType::greater;

		ConstIterator LHSIter = LHS.Begin();
		ConstIterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End())
		{
			TSynthThreeWayResult<ElementType> Ordering = SynthThreeWayCompare(*LHSIter, *RHSIter);

			if (Ordering != OrderingType::equivalent) return Ordering;

			++LHSIter;
			++RHSIter;
		}

		check(RHSIter == RHS.End());

		return OrderingType::equivalent;
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
	NODISCARD FORCEINLINE constexpr size_t Max() const { return N; }

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
