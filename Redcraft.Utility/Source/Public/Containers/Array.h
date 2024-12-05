#pragma once

#include "CoreTypes.h"
#include "Memory/Allocator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/Iterator.h"
#include "Miscellaneous/Container.h"
#include "Miscellaneous/AssertionMacros.h"
#include "Miscellaneous/ConstantIterator.h"

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

	using ElementType   = T;
	using AllocatorType = Allocator;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator = TIteratorImpl<false>;
	using ConstIterator = TIteratorImpl<true >;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Default constructor. Constructs an empty container with a default-constructed allocator. */
	FORCEINLINE TArray() : TArray(0) { }

	/** Constructs the container with 'Count' default instances of T. */
	explicit TArray(size_t Count) requires (CDefaultConstructible<ElementType>)
	{
		Impl.ArrayNum = Count;
		Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::DefaultConstruct<ElementType>(Impl.Pointer, Num());
	}

	/** Constructs the container with 'Count' copies of elements with 'InValue'. */
	TArray(size_t Count, const ElementType& InValue) requires (CCopyConstructible<ElementType>)
		: TArray(MakeCountedConstantIterator(InValue, Count), DefaultSentinel)
	{ }

	/** Constructs the container with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>> && CMovable<ElementType>)
	TArray(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t Count = Iteration::Distance(First, Last);

			Impl.ArrayNum = Count;
			Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
			Impl.Pointer  = Impl->Allocate(Max());

			for (size_t Index = 0; Index != Count; ++Index)
			{
				new (Impl.Pointer + Index) ElementType(*First++);
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

	/** Copy constructor. Constructs the container with the copy of the contents of 'InValue'. */
	TArray(const TArray& InValue) requires (CCopyConstructible<ElementType>)
	{
		Impl.ArrayNum = InValue.Num();
		Impl.ArrayMax = Impl->CalculateSlackReserve(Num());
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::CopyConstruct<ElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());
	}

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	TArray(TArray&& InValue) requires (CMoveConstructible<ElementType>)
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

			Memory::MoveConstruct<ElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());
		}

		InValue.Reset();
	}

	/** Constructs the container with the contents of the initializer list. */
	FORCEINLINE TArray(initializer_list<ElementType> IL) requires (CCopyConstructible<ElementType>) : TArray(Iteration::Begin(IL), Iteration::End(IL)) { }

	/** Destructs the array. The destructors of the elements are called and the used storage is deallocated. */
	~TArray()
	{
		Memory::Destruct(Impl.Pointer,Num());
		Impl->Deallocate(Impl.Pointer);
	}

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	TArray& operator=(const TArray& InValue) requires (CCopyable<ElementType>)
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

			Memory::CopyConstruct<ElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());

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
			Memory::CopyConstruct<ElementType>(Impl.Pointer + Num(), InValue.Impl.Pointer + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = InValue.Num();

		return *this;
	}

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	TArray& operator=(TArray&& InValue) requires (CMovable<ElementType>)
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

			Memory::MoveConstruct<ElementType>(Impl.Pointer, InValue.Impl.Pointer, Num());

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
			Memory::MoveConstruct<ElementType>(Impl.Pointer + Num(), InValue.Impl.Pointer + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = InValue.Num();

		InValue.Reset();

		return *this;
	}

	/** Replaces the contents with those identified by initializer list. */
	TArray& operator=(initializer_list<ElementType> IL) requires (CCopyable<ElementType>)
	{
		size_t NumToAllocate = GetNum(IL);

		NumToAllocate = NumToAllocate > Max() ? Impl->CalculateSlackGrow(GetNum(IL), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Impl->CalculateSlackShrink(GetNum(IL), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Impl.Pointer, Num());
			Impl->Deallocate(Impl.Pointer);

			Impl.ArrayNum = GetNum(IL);
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::CopyConstruct<ElementType>(Impl.Pointer, NAMESPACE_REDCRAFT::GetData(IL), Num());

			return *this;
		}

		if (GetNum(IL) <= Num())
		{
			Memory::CopyAssign(Impl.Pointer, NAMESPACE_REDCRAFT::GetData(IL), GetNum(IL));
			Memory::Destruct(Impl.Pointer + GetNum(IL), Num() - GetNum(IL));
		}
		else if (GetNum(IL) <= Max())
		{
			Memory::CopyAssign(Impl.Pointer, NAMESPACE_REDCRAFT::GetData(IL), Num());
			Memory::CopyConstruct<ElementType>(Impl.Pointer + Num(), NAMESPACE_REDCRAFT::GetData(IL) + Num(), GetNum(IL) - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = GetNum(IL);

		return *this;
	}

	/** Compares the contents of two arrays. */
	NODISCARD friend bool operator==(const TArray& LHS, const TArray& RHS) requires (CWeaklyEqualityComparable<ElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		for (size_t Index = 0; Index < LHS.Num(); ++Index)
		{
			if (LHS[Index] != RHS[Index]) return false;
		}

		return true;
	}

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend auto operator<=>(const TArray& LHS, const TArray& RHS) requires (CSynthThreeWayComparable<ElementType>)
	{
		const size_t NumToCompare = LHS.Num() < RHS.Num() ? LHS.Num() : RHS.Num();

		for (size_t Index = 0; Index < NumToCompare; ++Index)
		{
			if (const auto Result = SynthThreeWayCompare(LHS[Index], RHS[Index]); Result != 0) return Result;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, const ElementType& InValue) requires (CCopyable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) ElementType(InValue);
			Memory::MoveConstruct<ElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Iterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) ElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = InValue;
		}
		else new (Impl.Pointer + Num()) ElementType(InValue);

		Impl.ArrayNum = Num() + 1;

		return Iterator(this, Impl.Pointer + InsertIndex);
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, ElementType&& InValue) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) ElementType(MoveTemp(InValue));
			Memory::MoveConstruct<ElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Iterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) ElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = MoveTemp(InValue);
		}
		else new (Impl.Pointer + Num()) ElementType(MoveTemp(InValue));

		Impl.ArrayNum = Num() + 1;

		return Iterator(this, Impl.Pointer + InsertIndex);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, size_t Count, const ElementType& InValue) requires (CCopyable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Insert(Iter, MakeCountedConstantIterator(InValue, Count), DefaultSentinel);
	}

	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>
		&& CAssignableFrom<ElementType&, TIteratorReferenceType<I>> && CMovable<ElementType>)
	Iterator Insert(ConstIterator Iter, I First, S Last)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t InsertIndex = Iter - Begin();
			const size_t Count = Iteration::Distance(First, Last);

			if (Count == 0) return Iterator(this, Impl.Pointer + InsertIndex);

			const size_t NumToAllocate = Num() + Count > Max() ? Impl->CalculateSlackGrow(Num() + Count, Max()) : Max();

			check(NumToAllocate >= Num() + Count);

			if (NumToAllocate != Max())
			{
				ElementType* OldAllocation = Impl.Pointer;
				const size_t NumToDestruct = Num();

				Impl.ArrayNum = Num() + Count;
				Impl.ArrayMax = NumToAllocate;
				Impl.Pointer  = Impl->Allocate(Max());

				Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, InsertIndex);

				for (size_t Index = InsertIndex; Index != InsertIndex + Count; ++Index)
				{
					new (Impl.Pointer + Index) ElementType(*First++);
				}

				Memory::MoveConstruct<ElementType>(Impl.Pointer + InsertIndex + Count, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

				Memory::Destruct(OldAllocation, NumToDestruct);
				Impl->Deallocate(OldAllocation);

				return Iterator(this, Impl.Pointer + InsertIndex);
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
				new (Impl.Pointer + TargetIndex) ElementType(MoveTemp(Impl.Pointer[TargetIndex - Count]));
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
				new (Impl.Pointer + TargetIndex) ElementType(*First++);
			}

			check(First == Last);

			Impl.ArrayNum = Num() + Count;

			return Iterator(this, Impl.Pointer + InsertIndex);
		}
		else
		{
			TArray Temp(MoveTemp(First), MoveTemp(Last));
			return Insert(Iter, MakeMoveIterator(Temp.Begin()), MakeMoveSentinel(Temp.End()));
		}
	}

	/** Inserts elements from initializer list before 'Iter' in the container. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, initializer_list<ElementType> IL) requires (CCopyable<ElementType>)
	{
		return Insert(Iter, Iteration::Begin(IL), Iteration::End(IL));
	}

	/** Inserts a new element into the container directly before 'Iter'. */
	template <typename... Ts> requires (CConstructibleFrom<ElementType, Ts...> && CMovable<ElementType>)
	Iterator Emplace(ConstIterator Iter, Ts&&... Args)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, InsertIndex);
			new (Impl.Pointer + InsertIndex) ElementType(Forward<Ts>(Args)...);
			Memory::MoveConstruct<ElementType>(Impl.Pointer + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Iterator(this, Impl.Pointer + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Impl.Pointer + Num()) ElementType(MoveTemp(Impl.Pointer[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Impl.Pointer[Index] = MoveTemp(Impl.Pointer[Index - 1]);
			}

			Impl.Pointer[InsertIndex] = ElementType(Forward<Ts>(Args)...);
		}
		else new (Impl.Pointer + Num()) ElementType(Forward<Ts>(Args)...);

		Impl.ArrayNum = Num() + 1;

		return Iterator(this, Impl.Pointer + InsertIndex);
	}

	/** Removes the element at 'Iter' in the container. Without changing the order of elements. */
	FORCEINLINE Iterator StableErase(ConstIterator Iter, bool bAllowShrinking = true) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(Iter) && Iter != End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return StableErase(Iter, Iter + 1, bAllowShrinking);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. Without changing the order of elements. */
	Iterator StableErase(ConstIterator First, ConstIterator Last, bool bAllowShrinking = true) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t EraseIndex = First - Begin();
		const size_t EraseCount = Last - First;

		if (EraseCount == 0) return Iterator(this, Impl.Pointer + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Impl->CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() - EraseCount;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, EraseIndex);
			Memory::MoveConstruct<ElementType>(Impl.Pointer + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Iterator(this, Impl.Pointer + EraseIndex);
		}

		for (size_t Index = EraseIndex + EraseCount; Index != Num(); ++Index)
		{
			Impl.Pointer[Index - EraseCount] = MoveTemp(Impl.Pointer[Index]);
		}

		Memory::Destruct(Impl.Pointer + Num() - EraseCount, EraseCount);

		Impl.ArrayNum = Num() - EraseCount;

		return Iterator(this, Impl.Pointer + EraseIndex);
	}

	/** Removes the element at 'Iter' in the container. But it may change the order of elements. */
	FORCEINLINE Iterator Erase(ConstIterator Iter, bool bAllowShrinking = true) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(Iter) && Iter != End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return Erase(Iter, Iter + 1, bAllowShrinking);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. But it may change the order of elements. */
	Iterator Erase(ConstIterator First, ConstIterator Last, bool bAllowShrinking = true) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t EraseIndex = First - Begin();
		const size_t EraseCount = Last - First;

		if (EraseCount == 0) return Iterator(this, Impl.Pointer + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Impl->CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() - EraseCount;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, EraseIndex);
			Memory::MoveConstruct<ElementType>(Impl.Pointer + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Iterator(this, Impl.Pointer + EraseIndex);
		}

		for (size_t Index = 0; Index != EraseCount; ++Index)
		{
			if (EraseIndex + Index >= Num() - EraseCount) break;

			Impl.Pointer[EraseIndex + Index] = MoveTemp(Impl.Pointer[Num() - Index - 1]);
		}

		Memory::Destruct(Impl.Pointer + Num() - EraseCount, EraseCount);

		Impl.ArrayNum = Num() - EraseCount;

		return Iterator(this, Impl.Pointer + EraseIndex);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(const ElementType& InValue) requires (CCopyable<ElementType>)
	{
		EmplaceBack(InValue);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(ElementType&& InValue) requires (CMovable<ElementType>)
	{
		EmplaceBack(MoveTemp(InValue));
	}

	/** Appends a new element to the end of the container. */
	template <typename... Ts> requires (CConstructibleFrom<ElementType, Ts...> && CMovable<ElementType>)
	ElementType& EmplaceBack(Ts&&... Args)
	{
		const size_t NumToAllocate = Num() + 1 > Max() ? Impl->CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Num() + 1;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, Num() - 1);
			new (Impl.Pointer + Num() - 1) ElementType(Forward<Ts>(Args)...);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Impl->Deallocate(OldAllocation);

			return Impl.Pointer[Num() - 1];
		}

		new (Impl.Pointer + Num()) ElementType(Forward<Ts>(Args)...);

		Impl.ArrayNum = Num() + 1;

		return Impl.Pointer[Num() - 1];
	}

	/** Removes the last element of the container. The array cannot be empty. */
	FORCEINLINE void PopBack(bool bAllowShrinking = true) requires (CMovable<ElementType>)
	{
		Erase(End() - 1, bAllowShrinking);
	}

	/** Resizes the container to contain 'Count' elements. Additional default elements are appended. */
	void SetNum(size_t Count, bool bAllowShrinking = true) requires (CDefaultConstructible<ElementType> && CMovable<ElementType>)
	{
		size_t NumToAllocate = Count;

		NumToAllocate = NumToAllocate > Max()                    ? Impl->CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Impl->CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Count;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, NumToDestruct);
				Memory::DefaultConstruct<ElementType>(Impl.Pointer + NumToDestruct, Num() - NumToDestruct);
			}
			else
			{
				Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, Num());
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
			Memory::DefaultConstruct<ElementType>(Impl.Pointer + Num(), Count - Num());
		}
		else check_no_entry();

		Impl.ArrayNum = Count;
	}

	/** Resizes the container to contain 'Count' elements. Additional copies of 'InValue' are appended. */
	void SetNum(size_t Count, const ElementType& InValue, bool bAllowShrinking = true) requires (CCopyConstructible<ElementType> && CMovable<ElementType>)
	{
		size_t NumToAllocate = Count;

		NumToAllocate = NumToAllocate > Max()                    ? Impl->CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Impl->CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = Num();

			Impl.ArrayNum = Count;
			Impl.ArrayMax = NumToAllocate;
			Impl.Pointer  = Impl->Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, NumToDestruct);

				for (size_t Index = NumToDestruct; Index != Num(); ++Index)
				{
					new (Impl.Pointer + Index) ElementType(InValue);
				}
			}
			else
			{
				Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, Num());
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
				new (Impl.Pointer + Index) ElementType(InValue);
			}
		}
		else check_no_entry();

		Impl.ArrayNum = Count;
	}

	/** Increase the max capacity of the array to a value that's greater or equal to 'Count'. */
	void Reserve(size_t Count) requires (CMovable<ElementType>)
	{
		if (Count <= Max()) return;

		const size_t NumToAllocate = Impl->CalculateSlackReserve(Count);
		ElementType* OldAllocation = Impl.Pointer;

		check(NumToAllocate > Max());

		Impl.ArrayMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, Num());

		Memory::Destruct(OldAllocation, Num());
		Impl->Deallocate(OldAllocation);
	}

	/** Requests the removal of unused capacity. */
	void Shrink()
	{
		size_t NumToAllocate = Impl->CalculateSlackReserve(Num());

		check(NumToAllocate <= Max());

		if (NumToAllocate == Max()) return;

		ElementType* OldAllocation = Impl.Pointer;

		Impl.ArrayMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(Max());

		Memory::MoveConstruct<ElementType>(Impl.Pointer, OldAllocation, Num());
		Memory::Destruct(OldAllocation, Num());

		Impl->Deallocate(OldAllocation);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE       ElementType* GetData()       { return Impl.Pointer; }
	NODISCARD FORCEINLINE const ElementType* GetData() const { return Impl.Pointer; }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE      Iterator Begin()       { return      Iterator(this, Impl.Pointer);         }
	NODISCARD FORCEINLINE ConstIterator Begin() const { return ConstIterator(this, Impl.Pointer);         }
	NODISCARD FORCEINLINE      Iterator End()         { return      Iterator(this, Impl.Pointer + Num()); }
	NODISCARD FORCEINLINE ConstIterator End()   const { return ConstIterator(this, Impl.Pointer + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE size_t Num() const { return Impl.ArrayNum; }

	/** @return The number of elements that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t Max() const { return Impl.ArrayMax; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE       ElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Impl.Pointer[Index]; }
	NODISCARD FORCEINLINE const ElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Impl.Pointer[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE       ElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       ElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const ElementType& Back()  const { return *(End() - 1); }

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
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TArray& A) requires (CHashable<ElementType>)
	{
		size_t Result = 0;

		for (ConstIterator Iter = A.Begin(); Iter != A.End(); ++Iter)
		{
			Result = HashCombine(Result, GetTypeHash(*Iter));
		}

		return Result;
	}

	/** Overloads the Swap algorithm for TArray. */
	friend void Swap(TArray& A, TArray& B) requires (CMovable<ElementType>)
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

	ALLOCATOR_WRAPPER_BEGIN(AllocatorType, ElementType, Impl)
	{
		size_t ArrayNum;
		size_t ArrayMax;
		ElementType* Pointer;
	}
	ALLOCATOR_WRAPPER_END(AllocatorType, ElementType, Impl)

private:

	template <bool bConst, typename U>
	class TIteratorImpl final
	{
	public:

		using ElementType = TRemoveCV<T>;

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
TArray(I, S) -> TArray<TIteratorElementType<I>>;

template <typename T>
TArray(initializer_list<T>) -> TArray<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
