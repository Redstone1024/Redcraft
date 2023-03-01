#pragma once

#include "CoreTypes.h"
#include "Memory/Allocator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Memory/ObserverPointer.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename ArrayType, typename T>
class TArrayIterator
{
public:

	using ElementType = T;

	FORCEINLINE TArrayIterator() = default;

#	if DO_CHECK
	FORCEINLINE TArrayIterator(const TArrayIterator<ArrayType, TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Owner(InValue.Owner), Pointer(InValue.Pointer)
	{ }
#	else
	FORCEINLINE TArrayIterator(const TArrayIterator<ArrayType, TRemoveConst<ElementType>>& InValue) requires (CConst<ElementType>)
		: Pointer(InValue.Pointer)
	{ }
#	endif

	FORCEINLINE TArrayIterator(const TArrayIterator&)            = default;
	FORCEINLINE TArrayIterator(TArrayIterator&&)                 = default;
	FORCEINLINE TArrayIterator& operator=(const TArrayIterator&) = default;
	FORCEINLINE TArrayIterator& operator=(TArrayIterator&&)      = default;

	NODISCARD friend FORCEINLINE bool operator==(const TArrayIterator& LHS, const TArrayIterator& RHS) { return LHS.Pointer == RHS.Pointer; }

	NODISCARD friend FORCEINLINE strong_ordering operator<=>(const TArrayIterator & LHS, const TArrayIterator & RHS) { return LHS.Pointer <=> RHS.Pointer; }

	NODISCARD FORCEINLINE ElementType& operator*()  const { CheckThis(true); return *Pointer; }
	NODISCARD FORCEINLINE ElementType* operator->() const { CheckThis(true); return  Pointer; }

	NODISCARD FORCEINLINE ElementType& operator[](ptrdiff Index) const { TArrayIterator Temp = *this + Index; return *Temp; }

	FORCEINLINE TArrayIterator& operator++() { ++Pointer; CheckThis(); return *this; }
	FORCEINLINE TArrayIterator& operator--() { --Pointer; CheckThis(); return *this; }

	FORCEINLINE TArrayIterator operator++(int) { TArrayIterator Temp = *this; ++*this; return Temp; }
	FORCEINLINE TArrayIterator operator--(int) { TArrayIterator Temp = *this; --*this; return Temp; }

	FORCEINLINE TArrayIterator& operator+=(ptrdiff Offset) { Pointer += Offset; CheckThis(); return *this; }
	FORCEINLINE TArrayIterator& operator-=(ptrdiff Offset) { Pointer -= Offset; CheckThis(); return *this; }

	NODISCARD friend FORCEINLINE TArrayIterator operator+(TArrayIterator Iter, ptrdiff Offset) { TArrayIterator Temp = Iter; Temp += Offset; return Temp; }
	NODISCARD friend FORCEINLINE TArrayIterator operator+(ptrdiff Offset, TArrayIterator Iter) { TArrayIterator Temp = Iter; Temp += Offset; return Temp; }

	NODISCARD FORCEINLINE TArrayIterator operator-(ptrdiff Offset) const { TArrayIterator Temp = *this; Temp -= Offset; return Temp; }

	NODISCARD friend FORCEINLINE ptrdiff operator-(const TArrayIterator& LHS, const TArrayIterator& RHS) { LHS.CheckThis(); RHS.CheckThis(); return LHS.Pointer - RHS.Pointer; }

	NODISCARD FORCEINLINE explicit operator TObserverPtr<ElementType[]>() const { CheckThis(); return TObserverPtr<ElementType[]>(Pointer); }

private:

#	if DO_CHECK
	const ArrayType* Owner = nullptr;
#	endif

	ElementType* Pointer = nullptr;

#	if DO_CHECK
	FORCEINLINE TArrayIterator(const ArrayType* InContainer, ElementType* InPointer)
		: Owner(InContainer), Pointer(InPointer)
	{ }
#	else
	FORCEINLINE TArrayIterator(const ArrayType* InContainer, ElementType* InPointer)
		: Pointer(InPointer)
	{ }
#	endif

	FORCEINLINE void CheckThis(bool bExceptEnd = false) const
	{
		checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
		checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
	}

	friend ArrayType;

	template <typename InArrayType, typename InElementType>
	friend class TArrayIterator;

};

NAMESPACE_PRIVATE_END

/** Dynamic array. The elements are stored contiguously, which means that elements can be accessed not only through iterators, but also using offsets to regular pointers to elements. */
template <CElementalObject T, CInstantiableAllocator Allocator = FHeapAllocator> requires (!CConst<T>)
class TArray final
{
public:

	using ElementType   = T;
	using AllocatorType = Allocator;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator = NAMESPACE_PRIVATE::TArrayIterator<TArray,       ElementType>;
	using ConstIterator = NAMESPACE_PRIVATE::TArrayIterator<TArray, const ElementType>;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Default constructor. Constructs an empty container with a default-constructed allocator. */
	FORCEINLINE TArray() : TArray(0) { }

	/** Constructs the container with 'Count' default instances of T. */
	explicit TArray(size_t Count) requires (CDefaultConstructible<ElementType>)
	{
		Storage.GetNum()     = Count;
		Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
		Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

		Memory::DefaultConstruct<ElementType>(Storage.GetPointer(), Num());
	}
	
	/** Constructs the container with 'Count' copies of elements with 'InValue'. */
	TArray(size_t Count, const ElementType& InValue) requires (CCopyConstructible<ElementType>)
	{
		Storage.GetNum()     = Count;
		Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
		Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

		for (size_t Index = 0; Index < Num(); ++Index)
		{
			new (Storage.GetPointer() + Index) ElementType(InValue);
		}
	}

	/** Constructs the container with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>> && CMovable<ElementType>)
	TArray(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last."));

			const size_t Count = Iteration::Distance(First, Last);

			Storage.GetNum()     = Count;
			Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			for (size_t Index = 0; Index != Count; ++Index)
			{
				new (Storage.GetPointer() + Index) ElementType(*First++);
			}
		}
		else
		{
			Storage.GetNum()     = 0;
			Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

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
		Storage.GetNum()     = InValue.Num();
		Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
		Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

		Memory::CopyConstruct<ElementType>(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());
	}

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	TArray(TArray&& InValue) requires (CMoveConstructible<ElementType>)
	{
		Storage.GetNum() = InValue.Num();

		if (InValue.Storage.GetAllocator().IsTransferable(InValue.Storage.GetPointer()))
		{
			Storage.GetMax()     = InValue.Max();
			Storage.GetPointer() = InValue.Storage.GetPointer();

			InValue.Storage.GetNum()     = 0;
			InValue.Storage.GetMax()     = InValue.Storage.GetAllocator().CalculateSlackReserve(InValue.Num());
			InValue.Storage.GetPointer() = InValue.Storage.GetAllocator().Allocate(InValue.Max());
		}
		else
		{
			Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());
		}
	}

	/** Constructs the container with the contents of the initializer list. */
	FORCEINLINE TArray(initializer_list<ElementType> IL) requires (CCopyConstructible<ElementType>) : TArray(Iteration::Begin(IL), Iteration::End(IL)) { }

	/** Destructs the array. The destructors of the elements are called and the used storage is deallocated. */
	~TArray()
	{
		Memory::Destruct(Storage.GetPointer(),Num());
		Storage.GetAllocator().Deallocate(Storage.GetPointer());
	}

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	TArray& operator=(const TArray& InValue) requires (CCopyable<ElementType>)
	{
		if (&InValue == this) UNLIKELY return *this;

		size_t NumToAllocate = InValue.Num();

		NumToAllocate = NumToAllocate > Max() ? Storage.GetAllocator().CalculateSlackGrow(InValue.Num(), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Storage.GetAllocator().CalculateSlackShrink(InValue.Num(), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Storage.GetPointer(), Num());
			Storage.GetAllocator().Deallocate(Storage.GetPointer());
			
			Storage.GetNum()     = InValue.Num();
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::CopyConstruct<ElementType>(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());

			return *this;
		}

		if (InValue.Num() <= Num())
		{
			Memory::CopyAssign(Storage.GetPointer(), InValue.Storage.GetPointer(), InValue.Num());
			Memory::Destruct(Storage.GetPointer() + InValue.Num(), Num() - InValue.Num());
		}
		else if (InValue.Num() <= Max())
		{
			Memory::CopyAssign(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());
			Memory::CopyConstruct<ElementType>(Storage.GetPointer() + Num(), InValue.Storage.GetPointer() + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Storage.GetNum() = InValue.Num();

		return *this;
	}

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	TArray& operator=(TArray&& InValue) requires (CMovable<ElementType>)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (InValue.Storage.GetAllocator().IsTransferable(InValue.Storage.GetPointer()))
		{
			Memory::Destruct(Storage.GetPointer(), Num());
			Storage.GetAllocator().Deallocate(Storage.GetPointer());

			Storage.GetPointer() = InValue.Storage.GetPointer();

			InValue.Storage.GetNum()     = 0;
			InValue.Storage.GetMax()     = InValue.Storage.GetAllocator().CalculateSlackReserve(InValue.Num());
			InValue.Storage.GetPointer() = InValue.Storage.GetAllocator().Allocate(InValue.Max());

			return *this;
		}

		size_t NumToAllocate = InValue.Num();

		NumToAllocate = NumToAllocate > Max() ? Storage.GetAllocator().CalculateSlackGrow(InValue.Num(), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Storage.GetAllocator().CalculateSlackShrink(InValue.Num(), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Storage.GetPointer(), Num());
			Storage.GetAllocator().Deallocate(Storage.GetPointer());
			
			Storage.GetNum()     = InValue.Num();
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());
			
			InValue.Reset();

			return *this;
		}

		if (InValue.Num() <= Num())
		{
			Memory::MoveAssign(Storage.GetPointer(), InValue.Storage.GetPointer(), InValue.Num());
			Memory::Destruct(Storage.GetPointer() + InValue.Num(), Num() - InValue.Num());
		}
		else if (InValue.Num() <= Max())
		{
			Memory::MoveAssign(Storage.GetPointer(), InValue.Storage.GetPointer(), Num());
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + Num(), InValue.Storage.GetPointer() + Num(), InValue.Num() - Num());
		}
		else check_no_entry();

		Storage.GetNum() = InValue.Num();

		InValue.Reset();

		return *this;
	}

	/** Replaces the contents with those identified by initializer list. */
	TArray& operator=(initializer_list<ElementType> IL) requires (CCopyable<ElementType>)
	{
		size_t NumToAllocate = GetNum(IL);

		NumToAllocate = NumToAllocate > Max() ? Storage.GetAllocator().CalculateSlackGrow(GetNum(IL), Max())   : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? Storage.GetAllocator().CalculateSlackShrink(GetNum(IL), Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			Memory::Destruct(Storage.GetPointer(), Num());
			Storage.GetAllocator().Deallocate(Storage.GetPointer());
			
			Storage.GetNum()     = GetNum(IL);
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::CopyConstruct<ElementType>(Storage.GetPointer(), NAMESPACE_REDCRAFT::GetData(IL).Get(), Num());

			return *this;
		}

		if (GetNum(IL) <= Num())
		{
			Memory::CopyAssign(Storage.GetPointer(), NAMESPACE_REDCRAFT::GetData(IL).Get(), GetNum(IL));
			Memory::Destruct(Storage.GetPointer() + GetNum(IL), Num() - GetNum(IL));
		}
		else if (GetNum(IL) <= Max())
		{
			Memory::CopyAssign(Storage.GetPointer(), NAMESPACE_REDCRAFT::GetData(IL).Get(), Num());
			Memory::CopyConstruct<ElementType>(Storage.GetPointer() + Num(), NAMESPACE_REDCRAFT::GetData(IL).Get() + Num(), GetNum(IL) - Num());
		}
		else check_no_entry();

		Storage.GetNum() = GetNum(IL);

		return *this;
	}

	/** Compares the contents of two arrays. */
	NODISCARD friend bool operator==(const TArray& LHS, const TArray& RHS) requires (CWeaklyEqualityComparable<ElementType>)
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
	NODISCARD friend auto operator<=>(const TArray& LHS, const TArray& RHS) requires (CSynthThreeWayComparable<ElementType>)
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
	
	/** Inserts 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, const ElementType& InValue) requires (CCopyable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() + 1;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, InsertIndex);
			new (Storage.GetPointer() + InsertIndex) ElementType(InValue);
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Storage.GetPointer() + Num()) ElementType(MoveTemp(Storage.GetPointer()[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Storage.GetPointer()[Index] = MoveTemp(Storage.GetPointer()[Index - 1]);
			}

			Storage.GetPointer()[InsertIndex] = InValue;
		}
		else new (Storage.GetPointer() + Num()) ElementType(InValue);

		Storage.GetNum() = Num() + 1;

		return Iterator(this, Storage.GetPointer() + InsertIndex);
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, ElementType&& InValue) requires (CMovable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		const size_t NumToAllocate = Num() + 1 > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() + 1;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, InsertIndex);
			new (Storage.GetPointer() + InsertIndex) ElementType(MoveTemp(InValue));
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Storage.GetPointer() + Num()) ElementType(MoveTemp(Storage.GetPointer()[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Storage.GetPointer()[Index] = MoveTemp(Storage.GetPointer()[Index - 1]);
			}

			Storage.GetPointer()[InsertIndex] = MoveTemp(InValue);
		}
		else new (Storage.GetPointer() + Num()) ElementType(MoveTemp(InValue));
		
		Storage.GetNum() = Num() + 1;

		return Iterator(this, Storage.GetPointer() + InsertIndex);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, size_t Count, const ElementType& InValue) requires (CCopyable<ElementType>)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		const size_t InsertIndex = Iter - Begin();

		if (Count == 0) return Iterator(this, Storage.GetPointer() + InsertIndex);

		const size_t NumToAllocate = Num() + Count > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + Count, Max()) : Max();

		check(NumToAllocate >= Num() + Count);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() + Count;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, InsertIndex);

			for (size_t Index = InsertIndex; Index != InsertIndex + Count; ++Index)
			{
				new (Storage.GetPointer() + Index) ElementType(InValue);
			}

			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + InsertIndex + Count, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + InsertIndex);
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
			new (Storage.GetPointer() + TargetIndex) ElementType(MoveTemp(Storage.GetPointer()[TargetIndex - Count]));
		}

		for (size_t TargetIndex = IndexD - 1; TargetIndex != IndexC - 1; --TargetIndex)
		{
			Storage.GetPointer()[TargetIndex] = MoveTemp(Storage.GetPointer()[TargetIndex - Count]);
		}

		for (size_t TargetIndex = IndexA; TargetIndex != IndexB; ++TargetIndex)
		{
			Storage.GetPointer()[TargetIndex] = InValue;
		}

		for (size_t TargetIndex = IndexB; TargetIndex != IndexC; ++TargetIndex)
		{
			new (Storage.GetPointer() + TargetIndex) ElementType(InValue);
		}

		Storage.GetNum() = Num() + Count;

		return Iterator(this, Storage.GetPointer() + InsertIndex);
	}
	
	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>
		&& CAssignableFrom<ElementType&, TIteratorReferenceType<I>> && CMovable<ElementType>)
	Iterator Insert(ConstIterator Iter, I First, S Last)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last."));

			const size_t InsertIndex = Iter - Begin();
			const size_t Count = Iteration::Distance(First, Last);

			if (Count == 0) return Iterator(this, Storage.GetPointer() + InsertIndex);

			const size_t NumToAllocate = Num() + Count > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + Count, Max()) : Max();

			check(NumToAllocate >= Num() + Count);

			if (NumToAllocate != Max())
			{
				ElementType* OldAllocation = Storage.GetPointer();
				const size_t NumToDestruct = Num();

				Storage.GetNum()     = Num() + Count;
				Storage.GetMax()     = NumToAllocate;
				Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

				Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, InsertIndex);

				for (size_t Index = InsertIndex; Index != InsertIndex + Count; ++Index)
				{
					new (Storage.GetPointer() + Index) ElementType(*First++);
				}

				Memory::MoveConstruct<ElementType>(Storage.GetPointer() + InsertIndex + Count, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

				Memory::Destruct(OldAllocation, NumToDestruct);
				Storage.GetAllocator().Deallocate(OldAllocation);

				return Iterator(this, Storage.GetPointer() + InsertIndex);
			}

			const size_t IndexA = InsertIndex;
			const size_t IndexC = InsertIndex + Count;
			const size_t IndexB = Num() > IndexA ? (Num() < IndexC ? Num() : IndexC) : IndexA;
			const size_t IndexD = Num() > IndexC ? Num() : IndexC;
			const size_t IndexO = Num() + Count;

			size_t TargetIndex = Num() + Count - 1;

			for (size_t TargetIndex = IndexO - 1; TargetIndex != IndexD - 1; --TargetIndex)
			{
				new (Storage.GetPointer() + TargetIndex) ElementType(MoveTemp(Storage.GetPointer()[TargetIndex - Count]));
			}

			for (size_t TargetIndex = IndexD - 1; TargetIndex != IndexC - 1; --TargetIndex)
			{
				Storage.GetPointer()[TargetIndex] = MoveTemp(Storage.GetPointer()[TargetIndex - Count]);
			}

			for (size_t TargetIndex = IndexA; TargetIndex != IndexB; ++TargetIndex)
			{
				Storage.GetPointer()[TargetIndex] = *First++;
			}

			for (size_t TargetIndex = IndexB; TargetIndex != IndexC; ++TargetIndex)
			{
				new (Storage.GetPointer() + TargetIndex) ElementType(*First++);
			}

			check(First == Last);

			Storage.GetNum() = Num() + Count;

			return Iterator(this, Storage.GetPointer() + InsertIndex);
		}
		else
		{
			TArray Temp(MoveTemp(First), MoveTemp(Last));
			return Insert(Iter, TMoveIterator(Temp.Begin()), TMoveSentinel(Temp.End()));
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

		const size_t NumToAllocate = Num() + 1 > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() + 1;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, InsertIndex);
			new (Storage.GetPointer() + InsertIndex) ElementType(Forward<Ts>(Args)...);
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + InsertIndex + 1, OldAllocation + InsertIndex, NumToDestruct - InsertIndex);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + InsertIndex);
		}

		if (InsertIndex != Num())
		{
			new (Storage.GetPointer() + Num()) ElementType(MoveTemp(Storage.GetPointer()[Num() - 1]));

			for (size_t Index = Num() - 1; Index != InsertIndex; --Index)
			{
				Storage.GetPointer()[Index] = MoveTemp(Storage.GetPointer()[Index - 1]);
			}

			Storage.GetPointer()[InsertIndex] = ElementType(Forward<Ts>(Args)...);
		}
		else new (Storage.GetPointer() + Num()) ElementType(Forward<Ts>(Args)...);

		Storage.GetNum() = Num() + 1;

		return Iterator(this, Storage.GetPointer() + InsertIndex);
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

		if (EraseCount == 0) return Iterator(this, Storage.GetPointer() + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Storage.GetAllocator().CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() - EraseCount;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, EraseIndex);
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + EraseIndex);
		}

		for (size_t Index = EraseIndex + EraseCount; Index != Num(); ++Index)
		{
			Storage.GetPointer()[Index - EraseCount] = MoveTemp(Storage.GetPointer()[Index]);
		}

		Memory::Destruct(Storage.GetPointer() + Num() - EraseCount, EraseCount);

		Storage.GetNum() = Num() - EraseCount;

		return Iterator(this, Storage.GetPointer() + EraseIndex);
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

		if (EraseCount == 0) return Iterator(this, Storage.GetPointer() + EraseIndex);

		const size_t NumToAllocate = bAllowShrinking ? Storage.GetAllocator().CalculateSlackShrink(Num() - EraseCount, Max()) : Max();

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Num() - EraseCount;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, EraseIndex);
			Memory::MoveConstruct<ElementType>(Storage.GetPointer() + EraseIndex, OldAllocation + EraseIndex + EraseCount, NumToDestruct - EraseIndex - EraseCount);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Iterator(this, Storage.GetPointer() + EraseIndex);
		}

		for (size_t Index = 0; Index != EraseCount; ++Index)
		{
			if (EraseIndex + Index >= Num() - EraseCount) break;

			Storage.GetPointer()[EraseIndex + Index] = MoveTemp(Storage.GetPointer()[Num() - Index - 1]);
		}

		Memory::Destruct(Storage.GetPointer() + Num() - EraseCount, EraseCount);

		Storage.GetNum() = Num() - EraseCount;

		return Iterator(this, Storage.GetPointer() + EraseIndex);
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
		const size_t NumToAllocate = Num() + 1 > Max() ? Storage.GetAllocator().CalculateSlackGrow(Num() + 1, Max()) : Max();

		check(NumToAllocate >= Num() + 1);

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();
			
			Storage.GetNum()     = Num() + 1;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, Num() - 1);
			new (Storage.GetPointer() + Num() - 1) ElementType(Forward<Ts>(Args)...);

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return Storage.GetPointer()[Num() - 1];
		}

		new (Storage.GetPointer() + Num()) ElementType(Forward<Ts>(Args)...);

		Storage.GetNum() = Num() + 1;

		return Storage.GetPointer()[Num() - 1];
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
		
		NumToAllocate = NumToAllocate > Max()                    ? Storage.GetAllocator().CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Storage.GetAllocator().CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Count;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, NumToDestruct);
				Memory::DefaultConstruct<ElementType>(Storage.GetPointer() + NumToDestruct, Num() - NumToDestruct);
			}
			else
			{
				Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, Num());
			}

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return;
		}
		
		if (Count <= Num())
		{
			Memory::Destruct(Storage.GetPointer() + Count, Num() - Count);
		}
		else if (Count <= Max())
		{
			Memory::DefaultConstruct<ElementType>(Storage.GetPointer() + Num(), Count - Num());
		}
		else check_no_entry();

		Storage.GetNum() = Count;
	}

	/** Resizes the container to contain 'Count' elements. Additional copies of 'InValue' are appended. */
	void SetNum(size_t Count, const ElementType& InValue, bool bAllowShrinking = true) requires (CCopyConstructible<ElementType> && CMovable<ElementType>)
	{
		size_t NumToAllocate = Count;
		
		NumToAllocate = NumToAllocate > Max()                    ? Storage.GetAllocator().CalculateSlackGrow(Count, Max())            : NumToAllocate;
		NumToAllocate = NumToAllocate < Max() ? (bAllowShrinking ? Storage.GetAllocator().CalculateSlackShrink(Count, Max()) : Max()) : NumToAllocate;

		if (NumToAllocate != Max())
		{
			ElementType* OldAllocation = Storage.GetPointer();
			const size_t NumToDestruct = Num();

			Storage.GetNum()     = Count;
			Storage.GetMax()     = NumToAllocate;
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			if (NumToDestruct <= Num())
			{
				Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, NumToDestruct);

				for (size_t Index = NumToDestruct; Index != Num(); ++Index)
				{
					new (Storage.GetPointer() + Index) ElementType(InValue);
				}
			}
			else
			{
				Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, Num());
			}

			Memory::Destruct(OldAllocation, NumToDestruct);
			Storage.GetAllocator().Deallocate(OldAllocation);

			return;
		}
		
		if (Count <= Num())
		{
			Memory::Destruct(Storage.GetPointer() + Count, Num() - Count);
		}
		else if (Count <= Max())
		{
			for (size_t Index = Num(); Index != Count; ++Index)
			{
				new (Storage.GetPointer() + Index) ElementType(InValue);
			}
		}
		else check_no_entry();

		Storage.GetNum() = Count;
	}

	/** Increase the max capacity of the array to a value that's greater or equal to 'Count'. */
	void Reserve(size_t Count) requires (CMovable<ElementType>)
	{
		if (Count <= Max()) return;

		const size_t NumToAllocate = Storage.GetAllocator().CalculateSlackReserve(Count);
		ElementType* OldAllocation = Storage.GetPointer();

		check(NumToAllocate > Max());

		Storage.GetMax()     = NumToAllocate;
		Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

		Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, Num());

		Memory::Destruct(OldAllocation, Num());
		Storage.GetAllocator().Deallocate(OldAllocation);
	}

	/** Requests the removal of unused capacity. */
	void Shrink()
	{
		size_t NumToAllocate = Storage.GetAllocator().CalculateSlackReserve(Num());

		check(NumToAllocate <= Max());

		if (NumToAllocate == Max()) return;

		ElementType* OldAllocation = Storage.GetPointer();
		
		Storage.GetMax()     = NumToAllocate;
		Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

		Memory::MoveConstruct<ElementType>(Storage.GetPointer(), OldAllocation, Num());
		Memory::Destruct(OldAllocation, Num());

		Storage.GetAllocator().Deallocate(OldAllocation);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE TObserverPtr<      ElementType[]> GetData()       { return TObserverPtr<      ElementType[]>(Storage.GetPointer()); }
	NODISCARD FORCEINLINE TObserverPtr<const ElementType[]> GetData() const { return TObserverPtr<const ElementType[]>(Storage.GetPointer()); }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE      Iterator Begin()       { return      Iterator(this, Storage.GetPointer());         }
	NODISCARD FORCEINLINE ConstIterator Begin() const { return ConstIterator(this, Storage.GetPointer());         }
	NODISCARD FORCEINLINE      Iterator End()         { return      Iterator(this, Storage.GetPointer() + Num()); }
	NODISCARD FORCEINLINE ConstIterator End()   const { return ConstIterator(this, Storage.GetPointer() + Num()); }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE size_t Num() const { return Storage.GetNum(); }

	/** @return The number of elements that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t Max() const { return Storage.GetMax(); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE       ElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Storage.GetPointer()[Index]; }
	NODISCARD FORCEINLINE const ElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return Storage.GetPointer()[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE       ElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       ElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const ElementType& Back()  const { return *(End() - 1); }

	/** Erases all elements from the container. After this call, Num() returns zero. */
	void Reset(bool bAllowShrinking = true)
	{
		const size_t NumToAllocate = Storage.GetAllocator().CalculateSlackReserve(0);

		if (bAllowShrinking && NumToAllocate != Max())
		{
			Memory::Destruct(Storage.GetPointer(), Num());
			Storage.GetAllocator().Deallocate(Storage.GetPointer());

			Storage.GetNum()     = 0;
			Storage.GetMax()     = Storage.GetAllocator().CalculateSlackReserve(Num());
			Storage.GetPointer() = Storage.GetAllocator().Allocate(Max());

			return;
		}

		Memory::Destruct(Storage.GetPointer(), Num());
		Storage.GetNum() = 0;
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
			A.Storage.GetAllocator().IsTransferable(A.Storage.GetPointer()) &&
			B.Storage.GetAllocator().IsTransferable(B.Storage.GetPointer());

		if (bIsTransferable)
		{
			Swap(A.Storage.GetNum(),     B.Storage.GetNum());
			Swap(A.Storage.GetMax(),     B.Storage.GetMax());
			Swap(A.Storage.GetPointer(), B.Storage.GetPointer());

			return;
		}

		TArray Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	template <typename A, bool = CEmpty<A> && !CFinal<A>>
	class TStorage;

	template <typename A>
	class TStorage<A, true> : private A
	{
	public:

		FORCEINLINE TStorage() = default;

		FORCEINLINE TStorage(const TStorage&)            = delete;
		FORCEINLINE TStorage(TStorage&& InValue)         = delete;
		FORCEINLINE TStorage& operator=(const TStorage&) = delete;
		FORCEINLINE TStorage& operator=(TStorage&&)      = delete;

		FORCEINLINE T*& GetPointer()       { return Pointer; }
		FORCEINLINE T*  GetPointer() const { return Pointer; }

		FORCEINLINE size_t& GetNum()       { return ArrayNum; }
		FORCEINLINE size_t  GetNum() const { return ArrayNum; }
		FORCEINLINE size_t& GetMax()       { return ArrayMax; }
		FORCEINLINE size_t  GetMax() const { return ArrayMax; }

		FORCEINLINE       A& GetAllocator()       { return *this;   }
		FORCEINLINE const A& GetAllocator() const { return *this;   }

	private:

		// NOTE: NO_UNIQUE_ADDRESS is not valid in MSVC, use base class instead of member variable
		//NO_UNIQUE_ADDRESS A Allocator;

		T* Pointer;

		size_t ArrayNum;
		size_t ArrayMax;

	};

	template <typename A>
	class TStorage<A, false>
	{
	public:

		FORCEINLINE TStorage() = default;

		FORCEINLINE TStorage(const TStorage&)            = delete;
		FORCEINLINE TStorage(TStorage&& InValue)         = delete;
		FORCEINLINE TStorage& operator=(const TStorage&) = delete;
		FORCEINLINE TStorage& operator=(TStorage&&)      = delete;

		FORCEINLINE T*& GetPointer()       { return Pointer; }
		FORCEINLINE T*  GetPointer() const { return Pointer; }

		FORCEINLINE size_t& GetNum()       { return ArrayNum; }
		FORCEINLINE size_t  GetNum() const { return ArrayNum; }
		FORCEINLINE size_t& GetMax()       { return ArrayMax; }
		FORCEINLINE size_t  GetMax() const { return ArrayMax; }

		FORCEINLINE       A& GetAllocator()       { return AllocatorInstance; }
		FORCEINLINE const A& GetAllocator() const { return AllocatorInstance; }

	private:

		T* Pointer;

		size_t ArrayNum;
		size_t ArrayMax;

		A AllocatorInstance;

	};

	TStorage<typename AllocatorType::template ForElementType<T>> Storage;

};

template <typename I, typename S>
TArray(I, S) -> TArray<TIteratorElementType<I>>;

template <typename T>
TArray(initializer_list<T>) -> TArray<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
