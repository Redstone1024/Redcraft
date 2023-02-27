#pragma once

#include "CoreTypes.h"
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

template <CElementalObject T, size_t InExtent>
class TArrayView;

NAMESPACE_PRIVATE_BEGIN

template <typename T>
class TArrayViewIterator
{
public:

	using ElementType = T;

	FORCEINLINE constexpr TArrayViewIterator() = default;

#	if DO_CHECK
	FORCEINLINE constexpr TArrayViewIterator(const TArrayViewIterator<TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Pointer(InValue.Pointer), FirstSentinel(InValue.FirstSentinel), EndSentinel(InValue.EndSentinel)
	{ }
#	else
	FORCEINLINE constexpr TArrayViewIterator(const TArrayViewIterator<TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Pointer(InValue.Pointer)
	{ }
#	endif

	FORCEINLINE constexpr TArrayViewIterator(const TArrayViewIterator&)            = default;
	FORCEINLINE constexpr TArrayViewIterator(TArrayViewIterator&&)                 = default;
	FORCEINLINE constexpr TArrayViewIterator& operator=(const TArrayViewIterator&) = default;
	FORCEINLINE constexpr TArrayViewIterator& operator=(TArrayViewIterator&&)      = default;

	NODISCARD friend FORCEINLINE constexpr bool operator==(const TArrayViewIterator& LHS, const TArrayViewIterator& RHS) { return LHS.Pointer == RHS.Pointer; }

	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TArrayViewIterator & LHS, const TArrayViewIterator & RHS) { return LHS.Pointer <=> RHS.Pointer; }

	NODISCARD FORCEINLINE constexpr ElementType& operator*()  const { CheckThis(true); return *Pointer; }
	NODISCARD FORCEINLINE constexpr ElementType* operator->() const { CheckThis(true); return  Pointer; }

	NODISCARD FORCEINLINE constexpr ElementType& operator[](ptrdiff Index) const { TArrayViewIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE constexpr TArrayViewIterator& operator++() { ++Pointer; CheckThis(); return *this; }
	FORCEINLINE constexpr TArrayViewIterator& operator--() { --Pointer; CheckThis(); return *this; }

	FORCEINLINE constexpr TArrayViewIterator operator++(int) { TArrayViewIterator Temp = *this; ++*this; return Temp; }
	FORCEINLINE constexpr TArrayViewIterator operator--(int) { TArrayViewIterator Temp = *this; --*this; return Temp; }

	FORCEINLINE constexpr TArrayViewIterator& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
	FORCEINLINE constexpr TArrayViewIterator& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE constexpr TArrayViewIterator operator+(TArrayViewIterator Iter, ptrdiff Offset) { TArrayViewIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE constexpr TArrayViewIterator operator+(ptrdiff Offset, TArrayViewIterator Iter) { TArrayViewIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE constexpr TArrayViewIterator operator-(ptrdiff Offset) const { TArrayViewIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TArrayViewIterator& LHS, const TArrayViewIterator& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	NODISCARD FORCEINLINE constexpr explicit operator TObserverPtr<ElementType[]>() const { CheckThis(); return TObserverPtr<ElementType[]>(Pointer); }

private:

	ElementType* Pointer = nullptr;

#	if DO_CHECK
	ElementType* FirstSentinel = nullptr;
	ElementType*   EndSentinel = nullptr;
#	endif

#	if DO_CHECK
	FORCEINLINE constexpr TArrayViewIterator(ElementType* InPointer, ElementType* InFirstSentinel, ElementType* InEndSentinel)
		: Pointer(InPointer), FirstSentinel(InFirstSentinel), EndSentinel(InEndSentinel)
	{ }
#	else
	FORCEINLINE constexpr TArrayViewIterator(ElementType* InPointer, ElementType* InFirstSentinel, ElementType* InEndSentinel)
		: Pointer(InPointer)
	{ }
#	endif

	FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
	{
		check_code
		({
			const bool bInLegalRange = FirstSentinel && EndSentinel && FirstSentinel <= Pointer && Pointer <= EndSentinel;
			const bool bIsDereferenceable = Pointer != EndSentinel;

			checkf(bInLegalRange && (!bExceptEnd || bIsDereferenceable), TEXT("Read access violation. Please check IsValidIterator()."));
		});
	}

	template <typename U>
	friend class TArrayViewIterator;

	template <CElementalObject U, size_t InExtent>
	friend class NAMESPACE_REDCRAFT::TArrayView;

};

template <bool bEnable>
struct TEnableArrayNum { size_t ArrayNum; };

template <>
struct TEnableArrayNum<false> { size_t ArrayNum; };

NAMESPACE_PRIVATE_END

template <CElementalObject T, size_t N>
struct TStaticArray;

template <CElementalObject T, CInstantiableAllocator A> requires (!CConst<T>)
class TArray;

inline constexpr size_t DynamicExtent = INDEX_NONE;

/**
 * The class template TArrayView describes an object that can refer to a contiguous sequence of objects with the first element of
 * the sequence at position zero. A TArrayView can either have a static extent, in which case the number of elements in the sequence
 * is known at compile-time and encoded in the type, or a dynamic extent.
 */
template <CElementalObject T, size_t InExtent = DynamicExtent>
class TArrayView final : private NAMESPACE_PRIVATE::TEnableArrayNum<InExtent == DynamicExtent>
{
private:

	using Impl = NAMESPACE_PRIVATE::TEnableArrayNum<InExtent == DynamicExtent>;

public:

	using ElementType = T;

	using Iterator = NAMESPACE_PRIVATE::TArrayViewIterator<ElementType>;

	using ReverseIterator = TReverseIterator<Iterator>;

	static_assert(CContiguousIterator<Iterator>);

	static constexpr size_t Extent = InExtent;

	/** Constructs an empty array view. */
	FORCEINLINE constexpr TArrayView() requires (Extent == 0 || Extent == DynamicExtent) = default;

	/** Constructs an array view that is a view over the range ['InFirst', 'InFirst' + 'Count'). */
	template <CContiguousIterator I> requires (CConvertibleTo<TIteratorElementType<I>(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent) TArrayView(I InFirst, size_t InCount) : Pointer(static_cast<TObserverPtr<TIteratorElementType<I>[]>>(InFirst))
	{
		checkf(Extent == DynamicExtent || Extent == InCount, TEXT("Illegal range count. Please check InCount."));

		if constexpr (Extent == DynamicExtent)
		{
			Impl::ArrayNum = InCount;
		}
	}

	/** Constructs an array view that is a view over the range ['InFirst', 'InLast'). */
	template <CContiguousIterator I, CSizedSentinelFor<I> S> requires (CConvertibleTo<TIteratorElementType<I>(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent) TArrayView(I InFirst, S InLast) : Pointer(static_cast<TObserverPtr<TIteratorElementType<I>[]>>(InFirst))
	{
		checkf(Extent == DynamicExtent || Extent == InLast - InFirst, TEXT("Illegal range iterator. Please check InLast - InFirst."));

		if constexpr (Extent == DynamicExtent)
		{
			Impl::ArrayNum = InLast - InFirst;
		}
	}

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <size_t N> requires (Extent == DynamicExtent || N == Extent)
	FORCEINLINE constexpr TArrayView(ElementType(&InArray)[N]) : Pointer(InArray)
	{
		if constexpr (Extent == DynamicExtent)
		{
			Impl::ArrayNum = N;
		}
	}

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, size_t N> requires (CConvertibleTo<U(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(TStaticArray<U, N>& InArray) : TArrayView(InArray.GetData().Get(), InArray.Num()) { }

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U, size_t N> requires (CConvertibleTo<U(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(const TStaticArray<U, N>& InArray) : TArrayView(InArray.GetData().Get(), InArray.Num()) { }

	template <typename U, size_t N>
	FORCEINLINE constexpr TArrayView(const TStaticArray<U, N>&&) = delete;

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U> requires (CConvertibleTo<U(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(TArray<U>& InArray) : TArrayView(InArray.GetData().Get(), InArray.Num()) { }

	/** Constructs an array view that is a view over the array 'InArray'. */
	template <typename U> requires (CConvertibleTo<U(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr TArrayView(const TArray<U>& InArray) : TArrayView(InArray.GetData().Get(), InArray.Num()) { }

	template <typename U>
	FORCEINLINE constexpr TArrayView(const TArray<U>&&) = delete;

	/** Converting constructor from another array view 'InValue'. */
	template <typename U, size_t N> requires ((Extent == DynamicExtent || N == DynamicExtent || N == Extent) && CConvertibleTo<U(*)[], ElementType(*)[]>)
	FORCEINLINE constexpr explicit (Extent != DynamicExtent && N == DynamicExtent) TArrayView(TArrayView<U, N> InValue) : Pointer(InValue.GetData())
	{
		checkf(Extent == DynamicExtent || Extent == InValue.Num(), TEXT("Illegal view extent. Please check InValue.Num()."));

		if constexpr (Extent == DynamicExtent)
		{
			Impl::ArrayNum = InValue.Num();
		}
	}

	/** Defaulted copy constructor copies the size and data pointer. */
	FORCEINLINE constexpr TArrayView(const TArrayView&) = default;

	/** Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and the size. */
	FORCEINLINE constexpr TArrayView& operator=(const TArrayView&) noexcept = default;

	/** Compares the contents of two array views. */
	NODISCARD friend constexpr bool operator==(TArrayView LHS, TArrayView RHS) requires (CWeaklyEqualityComparable<ElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		Iterator LHSIter = LHS.Begin();
		Iterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End())
		{
			if (*LHSIter != *RHSIter) return false;

			++LHSIter;
			++RHSIter;
		}

		check(RHSIter == RHS.End());

		return true;
	}

	/** Compares the contents of two array views. */
	NODISCARD friend constexpr auto operator<=>(TArrayView LHS, TArrayView RHS) requires (CSynthThreeWayComparable<ElementType>)
	{
		using OrderingType = TSynthThreeWayResult<ElementType>;

		if (LHS.Num() < RHS.Num()) return OrderingType::less;
		if (LHS.Num() > RHS.Num()) return OrderingType::greater;

		Iterator LHSIter = LHS.Begin();
		Iterator RHSIter = RHS.Begin();

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

	/** Obtains an array view that is a view over the first 'Count' elements of this array view. */
	template <size_t Count> requires (Extent == DynamicExtent || Extent >= Count)
	NODISCARD FORCEINLINE constexpr TArrayView<ElementType, Count> First() const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<ElementType, Count>(Begin(), Count);
	}

	/** Obtains an array view that is a view over the first 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TArrayView<ElementType, DynamicExtent> First(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<ElementType, DynamicExtent>(Begin(), Count);
	}

	/** Obtains an array view that is a view over the last 'Count' elements of this array view. */
	template <size_t Count> requires (Extent == DynamicExtent || Extent >= Count)
	NODISCARD FORCEINLINE constexpr TArrayView<ElementType, Count> Last() const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<ElementType, Count>(End() - Count, Count);
	}

	/** Obtains an array view that is a view over the last 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TArrayView<ElementType, DynamicExtent> Last(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TArrayView<ElementType, DynamicExtent>(End() - Count, Count);
	}

	/** Obtains an array view that is a view over the 'Count' elements of this array view starting at 'Offset'.  */
	template <size_t Offset, size_t Count = DynamicExtent> requires (Extent == DynamicExtent || Count == DynamicExtent || Extent >= Offset + Count)
	NODISCARD FORCEINLINE constexpr auto Subview() const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		constexpr size_t SubviewExtent = Count != DynamicExtent ? Count : (Extent != DynamicExtent ? Extent - Offset : DynamicExtent);

		if constexpr (Count != DynamicExtent)
		{
			return TArrayView<ElementType, SubviewExtent>(Begin() + Offset, Count);
		}
		else
		{
			return TArrayView<ElementType, SubviewExtent>(Begin() + Offset, Num() - Offset);
		}
	}

	/** Obtains an array view that is a view over the 'Count' elements of this array view starting at 'Offset'.  */
	NODISCARD FORCEINLINE constexpr auto Subview(size_t Offset, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		if (Count != DynamicExtent)
		{
			return TArrayView<ElementType, DynamicExtent>(Begin() + Offset, Count);
		}
		else
		{
			return TArrayView<ElementType, DynamicExtent>(Begin() + Offset, Num() - Offset);
		}
	}

	/** Obtains an array view to the object representation of the elements of the array view. */
	NODISCARD FORCEINLINE constexpr auto AsBytes()
	{
		constexpr size_t BytesExtent = Extent != DynamicExtent ? sizeof(ElementType) * Extent : DynamicExtent;

		if constexpr (!CConst<ElementType>)
		{
			return TArrayView<uint8, BytesExtent>(reinterpret_cast<uint8*>(GetData().Get()), NumBytes());
		}
		else
		{
			return TArrayView<const uint8, BytesExtent>(reinterpret_cast<const uint8*>(GetData().Get()), NumBytes());
		}
	}

	/** Obtains an array view to the object representation of the elements of the array view. */
	NODISCARD FORCEINLINE constexpr auto AsBytes() const
	{
		constexpr size_t BytesExtent = Extent != DynamicExtent ? sizeof(ElementType) * Extent : DynamicExtent;

		return TArrayView<const uint8, BytesExtent>(reinterpret_cast<const uint8*>(GetData().Get()), NumBytes());
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr TObserverPtr<ElementType[]> GetData() const { return Pointer; }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return Iterator(Pointer.Get()        , Pointer.Get(), Pointer.Get() + Num()); }
	NODISCARD FORCEINLINE constexpr Iterator End()   const { return Iterator(Pointer.Get() + Num(), Pointer.Get(), Pointer.Get() + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr ReverseIterator RBegin() const { return ReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr ReverseIterator REnd()   const { return ReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { if constexpr (Extent == DynamicExtent) return Impl::ArrayNum; return Extent; }

	/** @return The number of bytes in the container. */
	NODISCARD FORCEINLINE constexpr size_t NumBytes() const { return Num() * sizeof(ElementType); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(Iterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr ElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Pointer[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr ElementType& Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TArrayView. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TArrayView A) requires (CHashable<ElementType>)
	{
		size_t Result = 0;

		for (Iterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	TObserverPtr<ElementType[]> Pointer;

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
