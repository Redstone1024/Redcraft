#pragma once

#include "CoreTypes.h"
#include "Range/Range.h"
#include "Memory/Allocator.h"
#include "Iterator/Iterator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** Dynamic array. The elements are stored contiguously, which means that elements can be accessed not only through iterators, but also using offsets to regular pointers to elements. */
template <CAllocatableObject T, CAllocator<T> Allocator = FHeapAllocator>
class TArray
{
private:

	template <bool bConst, typename = TConditional<bConst, const T, T>>
	class TIteratorImpl;

public:

	using FElementType   = T;
	using FAllocatorType = Allocator;

	using      FReference =       T&;
	using FConstReference = const T&;

	using      FIterator = TIteratorImpl<false>;
	using FConstIterator = TIteratorImpl<true >;

	using      FReverseIterator = TReverseIterator<     FIterator>;
	using FConstReverseIterator = TReverseIterator<FConstIterator>;

	static_assert(CContiguousIterator<     FIterator>);
	static_assert(CContiguousIterator<FConstIterator>);

	/** Default constructor. Constructs an empty container with a default-constructed allocator. */
	FORCEINLINE TArray() : TArray(0) { }

	/** Constructs the container with 'Count' default instances of T. */
	explicit TArray(size_t Count) requires (CDefaultConstructible<T>)
	{
		Impl.ArrayNum = Count;
		Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::DefaultConstruct<FElementType>(Impl.Pointer, Num());
	}

	/** Constructs the container with 'Count' copies of elements with 'InValue'. */
	FORCEINLINE explicit TArray(size_t Count, const FElementType& InValue) requires (CCopyConstructible<T>)
		: TArray(Range::Repeat(InValue, Count))
	{ }

	/** Constructs the container with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<T, TIteratorReference<I>> && CMovable<T>)
	explicit TArray(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			size_t Count = 0;

			if constexpr (CSizedSentinelFor<S, I>)
			{
				checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));

				Count = Last - First;
			}
			else for (I Iter = First; Iter != Last; ++Iter) ++Count;

			Impl.ArrayNum = Count;
			Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
			Impl.Pointer  = Impl->Allocate(Max());

			for (size_t Index = 0; Index != Count; ++Index)
			{
				new (Impl.Pointer + Index) FElementType(*First++);
			}
		}
		else
		{
			Impl.ArrayNum = 0;
			Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
			Impl.Pointer  = Impl->Allocate(Max());

			while (First != Last)
			{
				PushBack(*First);
				++First;
			}
		}
	}

	/** Constructs the container with the contents of the range. */
	template <CInputRange R> requires (!CSameAs<TRemoveCVRef<R>, TArray> && CConstructibleFrom<T, TRangeReference<R>> && CMovable<T>)
	FORCEINLINE explicit TArray(R&& Range) : TArray(Range::Begin(Range), Range::End(Range)) { }

	/** Copy constructor. Constructs the container with the copy of the contents of 'InValue'. */
	TArray(const TArray& InValue) requires (CCopyConstructible<T>)
	{
		Impl.ArrayNum = InValue.Num();
		Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::CopyConstruct<FElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());
	}

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	TArray(TArray&& InValue) requires (CMoveConstructible<T>)
	{
		Impl.ArrayNum = InValue.Num();

		if (InValue.Impl->IsTransferable(InValue.Impl.Pointer))
		{
			Impl.ArrayMax = InValue.Max();
			Impl.Pointer  = InValue.Impl.Pointer;

			InValue.Impl.ArrayNum = 0;
			InValue.Impl.ArrayMax = InValue.Impl->CalculateSlackReserve(InValue.Num());
			InValue.Impl.Pointer  = InValue.Impl->Allocate(InValue.Max());
		}
		else
		{
			Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());
		}

		InValue.Reset();
	}

	/** Constructs the container with the contents of the initializer list. */
	FORCEINLINE TArray(initializer_list<FElementType> IL) requires (CCopyConstructible<T>) : TArray(Range::Begin(IL), Range::End(IL)) { }

	/** Destructs the array. The destructors of the elements are called and the used storage is deallocated. */
	~TArray()
	{
		Memory::Destruct(Impl.Pointer,Num());
		Impl->Deallocate(Impl.Pointer);
	}

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	TArray& operator=(const TArray& InValue) requires (CCopyable<T>)
	{
		if (&InValue == this) UNLIKELY return *this;

		size_t NumToAllocate = InValue.Num();

		NumToAllocate = NumToAllocate > Max() ? Impl->CalculateSlackGrow(InValue.Num(), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Impl->CalculateSlackShrink(InValue.Num(), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = InValue.Num();
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::CopyConstruct<FElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());

			return *this;
		}

		if (InValue.Num() <= Num())
		{
			Memory::CopyAssign(Impl.Pointer, InValue.Impl.Pointer, InValue.Num());
			Memory::Destruct(Impl.Pointer + InValue.Num(), Num() - InValue.Num());
		}
		else if (InValue.Num() <= Max())
		{
			Memory::CopyAssign(Impl.Pointer, InValue.Impl.Pointer, Num());
			Memory::CopyConstruct<FElementType>(Impl.Pointer + Num(), InValue.Impl.Pointer + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = InValue.Num();

		return *this;
	}

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	TArray& operator=(TArray&& InValue) requires (CMovable<T>)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (InValue.Impl->IsTransferable(InValue.Impl.Pointer))
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = InValue.Num();
			Impl.ArrayMax = InValue.Max();
			Impl.Pointer  = InValue.Impl.Pointer;

			InValue.Impl.ArrayNum = 0;
			InValue.Impl.ArrayMax = InValue.Impl->CalculateSlackReserve(InValue.Num());
			InValue.Impl.Pointer  = InValue.Impl->Allocate(InValue.Max());

			return *this;
		}

		size_t NumToAllocate = InValue.Num();

		NumToAllocate = NumToAllocate > Max() ? Impl->CalculateSlackGrow(InValue.Num(), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Impl->CalculateSlackShrink(InValue.Num(), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = InValue.Num();
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());

			InValue.Reset();

			return *this;
		}

		if (InValue.Num() <= Num())
		{
			Memory::MoveAssign(Impl.Pointer, InValue.Impl.Pointer, InValue.Num());
			Memory::Destruct(Impl.Pointer + InValue.Num(), Num() - InValue.Num());
		}
		else if (InValue.Num() <= Max())
		{
			Memory::MoveAssign(Impl.Pointer, InValue.Impl.Pointer, Num());
			Memory::MoveConstruct<FElementType>(Impl.Pointer + Num(), InValue.Impl.Pointer + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = InValue.Num();

		InValue.Reset();

		return *this;
	}

	/** Replaces the contents with those identified by initializer list. */
	TArray& operator=(initializer_list<FElementType> IL) requires (CCopyable<T>)
	{
		size_t NumToAllocate = Range::Num(IL);

		NumToAllocate = NumToAllocate > Max() ? Impl->CalculateSlackGrow  (Range::Num(IL), Max()) : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Impl->CalculateSlackShrink(Range::Num(IL), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = Range::Num(IL);
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::CopyConstruct<FElementType>(Impl.Pointer, Range::GetData(IL), Num());

			return *this;
		}

		if (Range::Num(IL) <= Num())
		{
			Memory::CopyAssign(Impl.Pointer, Range::GetData(IL), Range::Num(IL));
			Memory::Destruct(Impl.Pointer + Range::Num(IL), Num() - Range::Num(IL));
		}
		else if (Range::Num(IL) <= Max())
		{
			Memory::CopyAssign(Impl.Pointer, Range::GetData(IL), Num());
			Memory::CopyConstruct<FElementType>(Impl.Pointer + Num(), Range::GetData(IL) + Num(), Range::Num(IL) - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = Range::Num(IL);

		return *this;
	}

	/** Compares the contents of two arrays. */
	NODISCARD friend bool operator==(const TArray& LHS, const TArray& RHS) requires (CWeaklyEqualityComparable<T>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		for (size_t Index = 0; Index < LHS.Num(); ++Index)
		{
			if (LHS[Index] != RHS[Index]) return false;
		}

		return true;
	}

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend auto operator<=>(const TArray& LHS, const TArray& RHS) requires (CSynthThreeWayComparable<T>)
	{
		const size_t NumToCompare = LHS.Num() < RHS.Num() ? LHS.Num() : RHS.Num();

		for (size_t Index = 0; Index < NumToCompare; ++Index)
		{
			if (const auto Result = SynthThreeWayCompare(LHS[Index], RHS[Index]); Result != 0) return Result;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	FIterator Insert(FConstIterator Iter, const FElementType& InValue) requires (CCopyable<T>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) FElementType(InValue);
			Memory::MoveConstruct<FElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return FIterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) FElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = InValue;
		}
		else new (Impl.Pointer + Num()) FElementType(InValue);

		Impl.ArrayNum = Num() + 1;

		return FIterator(this, Impl.Pointer + InsertIndex);
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	FIterator Insert(FConstIterator Iter, FElementType&& InValue) requires (CMovable<T>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) FElementType(MoveTemp(InValue));
			Memory::MoveConstruct<FElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return FIterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) FElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = MoveTemp(InValue);
		}
		else new (Impl.Pointer + Num()) FElementType(MoveTemp(InValue));

		Impl.ArrayNum = Num() + 1;

		return FIterator(this, Impl.Pointer + InsertIndex);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the container. */
	FIterator Insert(FConstIterator Iter, size_t Count, const FElementType& InValue) requires (CCopyable<T>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Insert(Iter, Range::Repeat(InValue, Count));
	}

	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<T, TIteratorReference<I>> && CAssignableFrom<T&, TIteratorReference<I>> && CMovable<T>)
	FIterator Insert(FConstIterator Iter, I First, S Last)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			const size_t InsertIndex = Iter - Begin();

			size_t Count = 0;

			if constexpr (CSizedSentinelFor<S, I>)
			{
				checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));

				Count = Last - First;
			}
			else for (I Jter = First; Jter != Last; ++Jter) ++Count;

			if (Count == 0) return FIterator(this, Impl.Pointer + InsertIndex);

			const size_t NumToAllocate = Num() + Count > Max() ? Impl->CalculateSlackGrow(Num() + Count, Max()) : Max();

			check(NumToAllocate >= Num() + Count);

			if (NumToAllocate != Max())
			{
				FElementType* OldAllocation = Impl.Pointer;
				const size_t  NumToDestruct = Num();

				Impl.ArrayNum = Num() + Count;
				Impl.ArrayMax = NumToAllocate;
				Impl.Pointer  = Impl->Allocate(Max());

				Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, InsertIndex);

				for (size_t Index = InsertIndex; Index != InsertIndex + Count; ++Index)
				{
					new (Impl.Pointer + Index) FElementType(*First++);
				}

				Memory::MoveConstruct<FElementType>(Impl.Pointer + InsertIndex + Count, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

				Memory::Destruct(OldAllocation, NumToDestruct);
				Impl->Deallocate(OldAllocation);

				return FIterator(this, Impl.Pointer + InsertIndex);
			}

			/*
			 * NO(XA) - No Operation
			 * IA(AB) - Insert Assignment
			 * IC(BC) - Insert Construction
			 * MA(CD) - Move Assignment
			 * MC(DO) - Move Construction
			 *
			 * IR(AC) - Insert Range
			 * UI(UO) - Uninitialized
			 *
			 * |X|-------------------| |-UI-|O|
			 * |X|----|A|-IR-| C|-----------|O|
			 * |X|-NO-|A|-IA-|BC|-MA-|D|-MC-|O|
			 *
			 * |X|-----------------|   |-UI-|O|
			 * |X|----------|A|-IR-| CD|----|O|
			 * |X|----NO----|A|-IA-|BCD|-MC-|O|
			 *
			 * |X|-----------| |-----UI-----|O|
			 * |X|----|A|----IR-----|C |----|O|
			 * |X|-NO-|A|-IA-|B|-IC-|CD|-MC-|O|
			 *
			 * |X|----------------|  |-UI-|  O|
			 * |X|----------------|A |-IR-|C O|
			 * |X|-------NO-------|AB|-IC-|CDO|
			 *
			 * |X|-----------| |----UI----|  O|
			 * |X|----------------|A |-IR-|C O|
			 * |X|-------NO-------|AB|-IC-|CDO|
			 */

			const size_t IndexA = InsertIndex;
			const size_t IndexC = InsertIndex + Count;
			const size_t IndexB = Num() > IndexA ? (Num() < IndexC ? Num() : IndexC) : IndexA;
			const size_t IndexD = Num() > IndexC ? Num() : IndexC;
			const size_t IndexO = Num() + Count;

			for (size_t TargetIndex = IndexO - 1; TargetIndex != IndexD - 1; --TargetIndex)
			{
				new (Impl.Pointer + TargetIndex) FElementType(MoveTemp(Impl.Pointer[TargetIndex - Count]));
			}

			for (size_t TargetIndex = IndexD - 1; TargetIndex != IndexC - 1; --TargetIndex)
			{
				Impl.Pointer[TargetIndex] = MoveTemp(Impl.Pointer[TargetIndex - Count]);
			}

			for (size_t TargetIndex = IndexA; TargetIndex != IndexB; ++TargetIndex)
			{
				Impl.Pointer[TargetIndex] = *First++;
			}

			for (size_t TargetIndex = IndexB; TargetIndex != IndexC; ++TargetIndex)
			{
				new (Impl.Pointer + TargetIndex) FElementType(*First++);
			}

			check(First == Last);

			Impl.ArrayNum = Num() + Count;

			return FIterator(this, Impl.Pointer + InsertIndex);
		}
		else
		{
			TArray Temp(MoveTemp(First), MoveTemp(Last));
			return Insert(Iter, MakeMoveIterator(Temp.Begin()), MakeMoveSentinel(Temp.End()));
		}
	}

	/** Inserts elements from range before 'Iter'. */
	template <CInputRange R> requires (CConstructibleFrom<T, TRangeReference<R>> && CAssignableFrom<T&, TRangeReference<R>> && CMovable<T>)
	FORCEINLINE FIterator Insert(FConstIterator Iter, R&& Range)
	{
		return Insert(Iter, Range::Begin(Range), Range::End(Range));
	}

	/** Inserts elements from initializer list before 'Iter' in the container. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, initializer_list<FElementType> IL) requires (CCopyable<T>)
	{
		return Insert(Iter, Range::Begin(IL), Range::End(IL));
	}

	/** Inserts a new element into the container directly before 'Iter'. */
	template <typename... Ts> requires (CConstructibleFrom<T, Ts...> && CMovable<T>)
	FIterator Emplace(FConstIterator Iter, Ts&&... Args)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) FElementType(Forward<Ts>(Args)...);
			Memory::MoveConstruct<FElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return FIterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) FElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = FElementType(Forward<Ts>(Args)...);
		}
		else new (Impl.Pointer + Num()) FElementType(Forward<Ts>(Args)...);

		Impl.ArrayNum = Num() + 1;

		return FIterator(this, Impl.Pointer + InsertIndex);
	}

	/** Removes the element at 'Iter' in the container. Without changing the order of elements. */
	FORCEINLINE FIterator StableErase(FConstIterator Iter, bool bAllowShrinking = true) requires (CMovable<T>)
	{
		checkf(IsValidIterator(Iter) && Iter != End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return StableErase(Iter, Iter + 1, bAllowShrinking);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. Without changing the order of elements. */
	FIterator StableErase(FConstIterator First, FConstIterator Last, bool bAllowShrinking = true) requires (CMovable<T>)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t EraseIndex = First - Begin();
		const size_t EraseCount = Last - First;

		if (EraseCount == 0) return FIterator(this, Impl.Pointer + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Impl->CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() - EraseCount;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, EraseIndex);
			Memory::MoveConstruct<FElementType>(Impl.Pointer + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return FIterator(this, Impl.Pointer + EraseIndex);
		}

		for (size_t Index = EraseIndex + EraseCount; Index != Num(); ++Index)
		{
			Impl.Pointer[Index - EraseCount] = MoveTemp(Impl.Pointer[Index]);
		}

		Memory::Destruct(Impl.Pointer + Num() - EraseCount, EraseCount);

		Impl.ArrayNum = Num() - EraseCount;

		return FIterator(this, Impl.Pointer + EraseIndex);
	}

	/** Removes the element at 'Iter' in the container. But it may change the order of elements. */
	FORCEINLINE FIterator Erase(FConstIterator Iter, bool bAllowShrinking = true) requires (CMovable<T>)
	{
		checkf(IsValidIterator(Iter) && Iter != End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return Erase(Iter, Iter + 1, bAllowShrinking);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. But it may change the order of elements. */
	FIterator Erase(FConstIterator First, FConstIterator Last, bool bAllowShrinking = true) requires (CMovable<T>)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t EraseIndex = First - Begin();
		const size_t EraseCount = Last - First;

		if (EraseCount == 0) return FIterator(this, Impl.Pointer + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Impl->CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() - EraseCount;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, EraseIndex);
			Memory::MoveConstruct<FElementType>(Impl.Pointer + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return FIterator(this, Impl.Pointer + EraseIndex);
		}

		for (size_t Index = 0; Index != EraseCount; ++Index)
		{
			if (EraseIndex + Index >= Num() - EraseCount) break;

			Impl.Pointer[EraseIndex + Index] = MoveTemp(Impl.Pointer[Num() - Index - 1]);
		}

		Memory::Destruct(Impl.Pointer + Num() - EraseCount, EraseCount);

		Impl.ArrayNum = Num() - EraseCount;

		return FIterator(this, Impl.Pointer + EraseIndex);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(const FElementType& InValue) requires (CCopyable<T>)
	{
		EmplaceBack(InValue);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(FElementType&& InValue) requires (CMovable<T>)
	{
		EmplaceBack(MoveTemp(InValue));
	}

	/** Appends a new element to the end of the container. */
	template <typename... Ts> requires (CConstructibleFrom<T, Ts...> && CMovable<T>)
	FElementType& EmplaceBack(Ts&&... Args)
	{
		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, Num() - 1);
			new (Impl.Pointer + Num() - 1) FElementType(Forward<Ts>(Args)...);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Impl.Pointer[Num() - 1];
		}

		new (Impl.Pointer + Num()) FElementType(Forward<Ts>(Args)...);

		Impl.ArrayNum = Num() + 1;

		return Impl.Pointer[Num() - 1];
	}

	/** Removes the last element of the container. The array cannot be empty. */
	FORCEINLINE void PopBack(bool bAllowShrinking = true) requires (CMovable<T>)
	{
		Erase(End() - 1, bAllowShrinking);
	}

	/** Resizes the container to contain 'Count' elements. Additional default elements are appended. */
	void SetNum(size_t Count, bool bAllowShrinking = true) requires (CDefaultConstructible<T> && CMovable<T>)
	{
		size_t NumToAllocate = Count;

		NumToAllocate = NumToAllocate > Max()                    ? Impl->CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Impl->CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Count;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, NumToDestruct);
				Memory::DefaultConstruct<FElementType>(Impl.Pointer + NumToDestruct, Num() - NumToDestruct);
			}
			else
			{
				Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, Num());
			}

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return;
		}

		if (Count <= Num())
		{
			Memory::Destruct(Impl.Pointer + Count, Num() - Count);
		}
		else if (Count <= Max())
		{
			Memory::DefaultConstruct<FElementType>(Impl.Pointer + Num(), Count - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = Count;
	}

	/** Resizes the container to contain 'Count' elements. Additional copies of 'InValue' are appended. */
	void SetNum(size_t Count, const FElementType& InValue, bool bAllowShrinking = true) requires (CCopyConstructible<T> && CMovable<T>)
	{
		size_t NumToAllocate = Count;

		NumToAllocate = NumToAllocate > Max()                    ? Impl->CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Impl->CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			FElementType* OldAllocation = Impl.Pointer;
			const size_t  NumToDestruct = Num();

			Impl.ArrayNum = Count;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, NumToDestruct);

				for (size_t Index = NumToDestruct; Index != Num(); ++Index)
				{
					new (Impl.Pointer + Index) FElementType(InValue);
				}
			}
			else
			{
				Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, Num());
			}

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return;
		}

		if (Count <= Num())
		{
			Memory::Destruct(Impl.Pointer + Count, Num() - Count);
		}
		else if (Count <= Max())
		{
			for (size_t Index = Num(); Index != Count; ++Index)
			{
				new (Impl.Pointer + Index) FElementType(InValue);
			}
		}
		else check_no_entry();

		Impl.ArrayNum = Count;
	}

	/** Increase the max capacity of the array to a value that's greater or equal to 'Count'. */
	void Reserve(size_t Count) requires (CMovable<T>)
	{
		if (Count <= Max()) return;

		const size_t  NumToAllocate = Impl->CalculateSlackReserve(Count);
		FElementType* OldAllocation = Impl.Pointer;

		check(NumToAllocate > Max());

		Impl.ArrayMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, Num());

		Memory::Destruct(OldAllocation, Num());
		Impl->Deallocate(OldAllocation);
	}

	/** Requests the removal of unused capacity. */
	void Shrink()
	{
		size_t NumToAllocate = Impl->CalculateSlackReserve(Num());

		check(NumToAllocate <= Max());

		if (NumToAllocate == Max()) return;

		FElementType* OldAllocation = Impl.Pointer;

		Impl.ArrayMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::MoveConstruct<FElementType>(Impl.Pointer, OldAllocation, Num());
		Memory::Destruct(OldAllocation, Num());

		Impl->Deallocate(OldAllocation);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE       FElementType* GetData()       { return Impl.Pointer; }
	NODISCARD FORCEINLINE const FElementType* GetData() const { return Impl.Pointer; }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE      FIterator Begin()       { return      FIterator(this, Impl.Pointer);         }
	NODISCARD FORCEINLINE FConstIterator Begin() const { return FConstIterator(this, Impl.Pointer);         }
	NODISCARD FORCEINLINE      FIterator End()         { return      FIterator(this, Impl.Pointer + Num()); }
	NODISCARD FORCEINLINE FConstIterator End()   const { return FConstIterator(this, Impl.Pointer + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE      FReverseIterator RBegin()       { return      FReverseIterator(End());   }
	NODISCARD FORCEINLINE FConstReverseIterator RBegin() const { return FConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      FReverseIterator REnd()         { return      FReverseIterator(Begin()); }
	NODISCARD FORCEINLINE FConstReverseIterator REnd()   const { return FConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE size_t Num() const { return Impl.ArrayNum; }

	/** @return The number of elements that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t Max() const { return Impl.ArrayMax; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(FConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE       FElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Impl.Pointer[Index]; }
	NODISCARD FORCEINLINE const FElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Impl.Pointer[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE       FElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const FElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       FElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const FElementType& Back()  const { return *(End() - 1); }

	/** Erases all elements from the container. After this call, Num() returns zero. */
	void Reset(bool bAllowShrinking = true)
	{
		const size_t NumToAllocate = Impl->CalculateSlackReserve(0);

		if (bAllowShrinking && NumToAllocate != Max())
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = 0;
			Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
			Impl.Pointer  = Impl->Allocate(Max());

			return;
		}

		Memory::Destruct(Impl.Pointer, Num());
		Impl.ArrayNum = 0;
	}

	/** Overloads the GetTypeHash algorithm for TArray. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TArray& A) requires (CHashable<T>)
	{
		size_t Result = 0;

		for (FConstIterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	/** Overloads the Swap algorithm for TArray. */
	friend void Swap(TArray& A, TArray& B) requires (CMovable<T>)
	{
		const bool bIsTransferable =
			A.Impl->IsTransferable(A.Impl.Pointer) &&
			B.Impl->IsTransferable(B.Impl.Pointer);

		if (bIsTransferable)
		{
			Swap(A.Impl.ArrayNum, B.Impl.ArrayNum);
			Swap(A.Impl.ArrayMax, B.Impl.ArrayMax);
			Swap(A.Impl.Pointer,  B.Impl.Pointer);

			return;
		}

		TArray Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	ALLOCATOR_WRAPPER_BEGIN(FAllocatorType, FElementType, Impl)
	{
		size_t ArrayNum;
		size_t ArrayMax;
		FElementType* Pointer;
	}
	ALLOCATOR_WRAPPER_END(FAllocatorType, FElementType, Impl)

private:

	template <bool bConst, typename U>
	class TIteratorImpl final
	{
	public:

		using FElementType = T;

		FORCEINLINE TIteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer)
		{ }
#		else
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer)
		{ }
#		endif

		FORCEINLINE TIteratorImpl(const TIteratorImpl&)            = default;
		FORCEINLINE TIteratorImpl(TIteratorImpl&&)                 = default;
		FORCEINLINE TIteratorImpl& operator=(const TIteratorImpl&) = default;
		FORCEINLINE TIteratorImpl& operator=(TIteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE bool operator==(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD friend FORCEINLINE strong_ordering operator<=>(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { return LHS.Pointer <=> RHS.Pointer; }

		NODISCARD FORCEINLINE U& operator*()  const { CheckThis(true ); return *Pointer; }
		NODISCARD FORCEINLINE U* operator->() const { CheckThis(false); return  Pointer; }

		NODISCARD FORCEINLINE U& operator[](ptrdiff Index) const { TIteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE TIteratorImpl& operator++() { ++Pointer; CheckThis(); return *this; }
		FORCEINLINE TIteratorImpl& operator--() { --Pointer; CheckThis(); return *this; }

		FORCEINLINE TIteratorImpl operator++(int) { TIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE TIteratorImpl operator--(int) { TIteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE TIteratorImpl& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
		FORCEINLINE TIteratorImpl& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE TIteratorImpl operator+(TIteratorImpl Iter, ptrdiff Offset) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE TIteratorImpl operator+(ptrdiff Offset, TIteratorImpl Iter) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE TIteratorImpl operator-(ptrdiff Offset) const { TIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE ptrdiff operator-(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	private:

#		if DO_CHECK
		const TArray* Owner = nullptr;
#		endif

		U* Pointer = nullptr;

#		if DO_CHECK
		FORCEINLINE TIteratorImpl(const TArray* InContainer, U* InPointer)
			: Owner(InContainer), Pointer(InPointer)
		{ }
#		else
		FORCEINLINE TIteratorImpl(const TArray* InContainer, U* InPointer)
			: Pointer(InPointer)
		{ }
#		endif

		FORCEINLINE void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool, typename> friend class TIteratorImpl;

		friend TArray;

	};

};

template <typename I, typename S>
TArray(I, S) -> TArray<TIteratorElement<I>>;

template <typename R>
TArray(R) -> TArray<TRangeElement<R>>;

template <typename T>
TArray(initializer_list<T>) -> TArray<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
