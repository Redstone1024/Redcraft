#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Memory/Allocator.h"
#include "Containers/Array.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Containers/StaticArray.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/Iterator.h"
#include "Miscellaneous/Container.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

inline constexpr size_t DynamicExtent = INDEX_NONE;

/**
 * The class template TArrayView describes an object that can refer to a contiguous sequence of objects with the first element of
 * the sequence at position zero. A TArrayView can either have a static extent, in which case the number of elements in the sequence
 * is known at compile-time and encoded in the type, or a dynamic extent.
 */
template <CObject T, size_t InExtent = DynamicExtent>
class TArrayView
{
public:

	using FElementType = T;

	using FReference = T&;

	class FIterator;

	using FReverseIterator = TReverseIterator<FIterator>;

	static_assert(CContiguousIterator<FIterator>);

	static constexpr size_t Extent = InExtent;

	/** Constructs an empty array view. */
	FORCEINLINE constexpr TArrayView() requires (Extent == 0 || Extent == DynamicExtent)
	{
		Impl.Pointer = nullptr;

		if constexpr (Extent == DynamicExtent)
		{
			Impl.ArrayNum = 0;
		}
	}

	/** Constructs an array view that is a view over the range ['InFirst', 'InFirst' + 'Count'). */
	template <CContiguousIterator I> requires (CConvertibleTo<TIteratorElementType<I>(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent) TArrayView(I InFirst, size_t InCount)
	{
		checkf(Extent == DynamicExtent || Extent == InCount, TEXT("Illegal range count. Please check InCount."));

		Impl.Pointer = ToAddress(InFirst);

		if constexpr (Extent == DynamicExtent)
		{
			Impl.ArrayNum = InCount;
		}
	}

	/** Constructs an array view that is a view over the range ['InFirst', 'InLast'). */
	template <CContiguousIterator I, CSizedSentinelFor<I> S> requires (CConvertibleTo<TIteratorElementType<I>(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent) TArrayView(I InFirst, S InLast)
	{
		checkf(Extent == DynamicExtent || Extent == InLast - InFirst, TEXT("Illegal range iterator. Please check InLast - InFirst."));

		Impl.Pointer = ToAddress(InFirst);

		if constexpr (Extent == DynamicExtent)
		{
			Impl.ArrayNum = InLast - InFirst;
		}
	}

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <size_t N> requires (Extent == DynamicExtent || N == Extent)
	FORCEINLINE constexpr TArrayView(FElementType(&InArray)[N])
	{
		Impl.Pointer = InArray;

		if constexpr (Extent == DynamicExtent)
		{
			Impl.ArrayNum = N;
		}
	}

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, size_t N> requires (CConvertibleTo<U(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(TStaticArray<U, N>& InArray) : TArrayView(InArray.GetData(), InArray.Num()) { }

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, size_t N> requires (CConvertibleTo<const U(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(const TStaticArray<U, N>& InArray) : TArrayView(InArray.GetData(), InArray.Num()) { }

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, typename Allocator> requires (CConvertibleTo<U(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(TArray<U, Allocator>& InArray) : TArrayView(InArray.GetData(), InArray.Num()) { }

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, typename Allocator> requires (CConvertibleTo<const U(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(const TArray<U, Allocator>& InArray) : TArrayView(InArray.GetData(), InArray.Num()) { }

	/** Converting constructor from another array view 'InValue'. */
	template <typename U, size_t N> requires ((Extent == DynamicExtent || N == DynamicExtent || N == Extent) && CConvertibleTo<U(*)[], FElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent && N == DynamicExtent) TArrayView(TArrayView<U, N> InValue)
	{
		checkf(Extent == DynamicExtent || Extent == InValue.Num(), TEXT("Illegal view extent. Please check InValue.Num()."));

		Impl.Pointer = InValue.GetData();

		if constexpr (Extent == DynamicExtent)
		{
			Impl.ArrayNum = InValue.Num();
		}
	}

	/** Defaulted copy constructor copies the size and data pointer. */
	FORCEINLINE constexpr TArrayView(const TArrayView&) = default;

	/** Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and the size. */
	FORCEINLINE constexpr TArrayView& operator=(const TArrayView&) noexcept = default;

	/** Compares the contents of two array views. */
	NODISCARD friend constexpr bool operator==(TArrayView LHS, TArrayView RHS) requires (CWeaklyEqualityComparable<FElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		for (size_t Index = 0; Index < LHS.Num(); ++Index)
		{
			if (LHS[Index] != RHS[Index]) return false;
		}

		return true;
	}

	/** Compares the contents of two array views. */
	NODISCARD friend constexpr auto operator<=>(TArrayView LHS, TArrayView RHS) requires (CSynthThreeWayComparable<FElementType>)
	{
		const size_t NumToCompare = LHS.Num() < RHS.Num() ? LHS.Num() : RHS.Num();

		for (size_t Index = 0; Index < NumToCompare; ++Index)
		{
			if (const auto Result = SynthThreeWayCompare(LHS[Index], RHS[Index]); Result != 0) return Result;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** Obtains an array view that is a view over the first 'Count' elements of this array view. */
	template <size_t Count> requires (Extent == DynamicExtent || Extent >= Count)
	NODISCARD FORCEINLINE constexpr TArrayView<FElementType, Count> First() const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<FElementType, Count>(Begin(), Count);
	}

	/** Obtains an array view that is a view over the first 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TArrayView<FElementType, DynamicExtent> First(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<FElementType, DynamicExtent>(Begin(), Count);
	}

	/** Obtains an array view that is a view over the last 'Count' elements of this array view. */
	template <size_t Count> requires (Extent == DynamicExtent || Extent >= Count)
	NODISCARD FORCEINLINE constexpr TArrayView<FElementType, Count> Last() const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<FElementType, Count>(End() - Count, Count);
	}

	/** Obtains an array view that is a view over the last 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TArrayView<FElementType, DynamicExtent> Last(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<FElementType, DynamicExtent>(End() - Count, Count);
	}

	/** Obtains an array view that is a view over the 'Count' elements of this array view starting at 'Offset'.  */
	template <size_t Offset, size_t Count = DynamicExtent> requires (Extent == DynamicExtent || Count == DynamicExtent || Extent >= Offset + Count)
	NODISCARD FORCEINLINE constexpr auto Subview() const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		constexpr size_t SubviewExtent = Count != DynamicExtent ? Count : (Extent != DynamicExtent ? Extent - Offset : DynamicExtent);

		if constexpr (Count != DynamicExtent)
		{
			return TArrayView<FElementType, SubviewExtent>(Begin() + Offset, Count);
		}
		else
		{
			return TArrayView<FElementType, SubviewExtent>(Begin() + Offset, Num() - Offset);
		}
	}

	/** Obtains an array view that is a view over the 'Count' elements of this array view starting at 'Offset'.  */
	NODISCARD FORCEINLINE constexpr auto Subview(size_t Offset, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		if (Count != DynamicExtent)
		{
			return TArrayView<FElementType, DynamicExtent>(Begin() + Offset, Count);
		}
		else
		{
			return TArrayView<FElementType, DynamicExtent>(Begin() + Offset, Num() - Offset);
		}
	}

	/** Obtains an array view to the object representation of the elements of the array view. */
	NODISCARD FORCEINLINE constexpr auto AsBytes()
	{
		constexpr size_t BytesExtent = Extent != DynamicExtent ? sizeof(FElementType) * Extent : DynamicExtent;

		if constexpr (!CConst<FElementType>)
		{
			return TArrayView<uint8, BytesExtent>(reinterpret_cast<uint8*>(GetData()), NumBytes());
		}
		else
		{
			return TArrayView<const uint8, BytesExtent>(reinterpret_cast<const uint8*>(GetData()), NumBytes());
		}
	}

	/** Obtains an array view to the object representation of the elements of the array view. */
	NODISCARD FORCEINLINE constexpr auto AsBytes() const
	{
		constexpr size_t BytesExtent = Extent != DynamicExtent ? sizeof(FElementType) * Extent : DynamicExtent;

		return TArrayView<const uint8, BytesExtent>(reinterpret_cast<const uint8*>(GetData()), NumBytes());
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr FElementType* GetData() const { return Impl.Pointer; }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr FIterator Begin() const { return FIterator(this, Impl.Pointer);         }
	NODISCARD FORCEINLINE constexpr FIterator End()   const { return FIterator(this, Impl.Pointer + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr FReverseIterator RBegin() const { return FReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr FReverseIterator REnd()   const { return FReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { if constexpr (Extent == DynamicExtent) { return Impl.ArrayNum; } return Extent; }

	/** @return The number of bytes in the container. */
	NODISCARD FORCEINLINE constexpr size_t NumBytes() const { return Num() * sizeof(FElementType); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(FIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr FReference operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Impl.Pointer[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr FReference Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr FReference Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TArrayView. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TArrayView A) requires (CHashable<FElementType>)
	{
		size_t Result = 0;

		for (FIterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	struct FImplWithoutNum { FElementType* Pointer; };

	struct FImplWithNum : FImplWithoutNum { size_t ArrayNum; };

	TConditional<InExtent == DynamicExtent, FImplWithNum, FImplWithoutNum> Impl;

public:

	class FIterator final
	{
	public:

		using FElementType = TRemoveCV<T>;

		FORCEINLINE constexpr FIterator()                            = default;
		FORCEINLINE constexpr FIterator(const FIterator&)            = default;
		FORCEINLINE constexpr FIterator(FIterator&&)                 = default;
		FORCEINLINE constexpr FIterator& operator=(const FIterator&) = default;
		FORCEINLINE constexpr FIterator& operator=(FIterator&&)      = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const FIterator& LHS, const FIterator& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const FIterator& LHS, const FIterator& RHS) { return LHS.Pointer <=> RHS.Pointer; }

		NODISCARD FORCEINLINE constexpr T& operator*()  const { CheckThis(true ); return *Pointer; }
		NODISCARD FORCEINLINE constexpr T* operator->() const { CheckThis(false); return  Pointer; }

		NODISCARD FORCEINLINE constexpr T& operator[](ptrdiff Index) const { FIterator Temp = *this + Index; return *Temp; }

		FORCEINLINE constexpr FIterator& operator++() { ++Pointer; CheckThis(); return *this; }
		FORCEINLINE constexpr FIterator& operator--() { --Pointer; CheckThis(); return *this; }

		FORCEINLINE constexpr FIterator operator++(int) { FIterator Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr FIterator operator--(int) { FIterator Temp = *this; --*this; return Temp; }

		FORCEINLINE constexpr FIterator& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
		FORCEINLINE constexpr FIterator& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE constexpr FIterator operator+(FIterator Iter, ptrdiff Offset) { FIterator Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr FIterator operator+(ptrdiff Offset, FIterator Iter) { FIterator Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr FIterator operator-(ptrdiff Offset) const { FIterator Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const FIterator& LHS, const FIterator& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	private:

#		if DO_CHECK
		const TArrayView* Owner = nullptr;
#		endif

		T* Pointer = nullptr;

#		if DO_CHECK
		FORCEINLINE constexpr FIterator(const TArrayView* InContainer, T* InPointer)
			: Owner(InContainer), Pointer(InPointer)
		{ }
#		else
		FORCEINLINE constexpr FIterator(const TArrayView* InContainer, T* InPointer)
			: Pointer(InPointer)
		{ }
#		endif

		FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		friend TArrayView;

	};

};

template <typename I, typename S>
TArrayView(I, S) -> TArrayView<TRemoveReference<TIteratorReferenceType<I>>>;

template <typename T, size_t N>
TArrayView(T(&)[N]) -> TArrayView<T, N>;

template <typename T, size_t N>
TArrayView(TStaticArray<T, N>&) -> TArrayView<T, N>;

template <typename T, size_t N>
TArrayView(const TStaticArray<T, N>&) -> TArrayView<const T, N>;

template <typename T>
TArrayView(TArray<T>&) -> TArrayView<T, DynamicExtent>;

template <typename T>
TArrayView(const TArray<T>&) -> TArrayView<const T, DynamicExtent>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
