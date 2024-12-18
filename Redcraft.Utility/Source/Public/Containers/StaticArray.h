#pragma once

#include "CoreTypes.h"
#include "Range/Range.h"
#include "Templates/Meta.h"
#include "Iterator/Iterator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** TStaticArray is a container that encapsulates fixed size arrays. */
template <CObject T, size_t N> requires (!CConst<T> && !CVolatile<T>)
struct TStaticArray
{
private:

	template <bool bConst, typename = TConditional<bConst, const T, T>>
	class TIteratorImpl;

public:

	using FElementType = T;

	using      FReference =       T&;
	using FConstReference = const T&;

	using      FIterator = TIteratorImpl<false>;
	using FConstIterator = TIteratorImpl<true >;

	using      FReverseIterator = TReverseIterator<     FIterator>;
	using FConstReverseIterator = TReverseIterator<FConstIterator>;

	static_assert(CContiguousIterator<     FIterator>);
	static_assert(CContiguousIterator<FConstIterator>);

	/** Compares the contents of two arrays. */
	NODISCARD friend constexpr bool operator==(const TStaticArray& LHS, const TStaticArray& RHS) requires (CWeaklyEqualityComparable<FElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		for (size_t Index = 0; Index < LHS.Num(); ++Index)
		{
			if (LHS[Index] != RHS[Index]) return false;
		}

		return true;
	}

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend constexpr auto operator<=>(const TStaticArray& LHS, const TStaticArray& RHS) requires (CSynthThreeWayComparable<FElementType>)
	{
		const size_t NumToCompare = LHS.Num() < RHS.Num() ? LHS.Num() : RHS.Num();

		for (size_t Index = 0; Index < NumToCompare; ++Index)
		{
			if (const auto Result = SynthThreeWayCompare(LHS[Index], RHS[Index]); Result != 0) return Result;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr       FElementType* GetData()       { return _; }
	NODISCARD FORCEINLINE constexpr const FElementType* GetData() const { return _; }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr      FIterator Begin()       { return      FIterator(this, _);         }
	NODISCARD FORCEINLINE constexpr FConstIterator Begin() const { return FConstIterator(this, _);         }
	NODISCARD FORCEINLINE constexpr      FIterator End()         { return      FIterator(this, _ + Num()); }
	NODISCARD FORCEINLINE constexpr FConstIterator End()   const { return FConstIterator(this, _ + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr      FReverseIterator RBegin()       { return      FReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr FConstReverseIterator RBegin() const { return FConstReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr      FReverseIterator REnd()         { return      FReverseIterator(Begin()); }
	NODISCARD FORCEINLINE constexpr FConstReverseIterator REnd()   const { return FConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { return N; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(FConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr       FElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return _[Index]; }
	NODISCARD FORCEINLINE constexpr const FElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return _[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr       FElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE constexpr const FElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr       FElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE constexpr const FElementType& Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TStaticArray. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TStaticArray& A) requires (CHashable<FElementType>)
	{
		size_t Result = 0;

		for (FConstIterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	/** Overloads the Swap algorithm for TStaticArray. */
	friend FORCEINLINE constexpr void Swap(TStaticArray& A, TStaticArray& B) requires (CSwappable<FElementType>) { Swap(A._, B._); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

	T _[N];

private:

	template <bool bConst, typename U>
	class TIteratorImpl final
	{
	public:

		using FElementType = TRemoveCV<T>;

		FORCEINLINE constexpr TIteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer)
		{ }
#		else
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer)
		{ }
#		endif

		FORCEINLINE constexpr TIteratorImpl(const TIteratorImpl&)            = default;
		FORCEINLINE constexpr TIteratorImpl(TIteratorImpl&&)                 = default;
		FORCEINLINE constexpr TIteratorImpl& operator=(const TIteratorImpl&) = default;
		FORCEINLINE constexpr TIteratorImpl& operator=(TIteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { return LHS.Pointer <=> RHS.Pointer; }

		NODISCARD FORCEINLINE constexpr U& operator*()  const { CheckThis(true ); return *Pointer; }
		NODISCARD FORCEINLINE constexpr U* operator->() const { CheckThis(false); return  Pointer; }

		NODISCARD FORCEINLINE constexpr U& operator[](ptrdiff Index) const { TIteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE constexpr TIteratorImpl& operator++() { ++Pointer; CheckThis(); return *this; }
		FORCEINLINE constexpr TIteratorImpl& operator--() { --Pointer; CheckThis(); return *this; }

		FORCEINLINE constexpr TIteratorImpl operator++(int) { TIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr TIteratorImpl operator--(int) { TIteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE constexpr TIteratorImpl& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
		FORCEINLINE constexpr TIteratorImpl& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE constexpr TIteratorImpl operator+(TIteratorImpl Iter, ptrdiff Offset) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr TIteratorImpl operator+(ptrdiff Offset, TIteratorImpl Iter) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr TIteratorImpl operator-(ptrdiff Offset) const { TIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	private:

#		if DO_CHECK
		const TStaticArray* Owner = nullptr;
#		endif

		U* Pointer = nullptr;

#		if DO_CHECK
		FORCEINLINE constexpr TIteratorImpl(const TStaticArray* InContainer, U* InPointer)
			: Owner(InContainer), Pointer(InPointer)
		{ }
#		else
		FORCEINLINE constexpr TIteratorImpl(const TStaticArray* InContainer, U* InPointer)
			: Pointer(InPointer)
		{ }
#		endif

		FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool, typename> friend class TIteratorImpl;

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

// ReSharper disable CppInconsistentNaming

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

// ReSharper restore CppInconsistentNaming
