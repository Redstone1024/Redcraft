#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/Allocator.h"
#include "Memory/MemoryOperator.h"
#include "Iterator/Utility.h"
#include "Iterator/BasicIterator.h"
#include "Iterator/Sentinel.h"
#include "Iterator/ReverseIterator.h"
#include "Range/Utility.h"
#include "Range/Factory.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CAllocatableObject T, CMultipleAllocator<T> Allocator = FHeapAllocator>
class TList
{
private:

	struct FNode;

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

	static_assert(CBidirectionalIterator<     FIterator>);
	static_assert(CBidirectionalIterator<FConstIterator>);

	/** Default constructor. Constructs an empty container with a default-constructed allocator. */
	TList()
	{
		Impl.HeadNode = Impl->Allocate(1);
		Impl.HeadNode->PrevNode = Impl.HeadNode;
		Impl.HeadNode->NextNode = Impl.HeadNode;

		Impl.ListNum = 0;
	}

	/** Constructs the container with 'Count' default instances of T. */
	explicit TList(size_t Count) requires (CDefaultConstructible<FElementType>) : TList()
	{
		FNode* EndNode = Impl.HeadNode->PrevNode;

		while (Count > Impl.ListNum)
		{
			FNode* Node = new (Impl->Allocate(1)) FNode;

			EndNode->NextNode = Node;
			Node->PrevNode = EndNode;

			++Impl.ListNum;

			EndNode = Node;
		}

		EndNode->NextNode = Impl.HeadNode;
		Impl.HeadNode->PrevNode = EndNode;
	}

	/** Constructs the container with 'Count' copies of elements with 'InValue'. */
	TList(size_t Count, const FElementType& InValue) requires (CCopyable<FElementType>)
		: TList(Range::Repeat(InValue, Count))
	{ }

	/** Constructs the container with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	TList(I First, S Last) : TList()
	{
		FNode* EndNode = Impl.HeadNode->PrevNode;

		for (; First != Last; ++First)
		{
			FNode* Node = new (Impl->Allocate(1)) FNode(InPlace, *First);

			EndNode->NextNode = Node;
			Node->PrevNode = EndNode;

			++Impl.ListNum;

			EndNode = Node;
		}

		EndNode->NextNode = Impl.HeadNode;
		Impl.HeadNode->PrevNode = EndNode;
	}

	/** Constructs the container with the contents of the range. */
	template <CInputRange R> requires (!CSameAs<TRemoveCVRef<R>, TList> && CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE explicit TList(R&& Range) : TList(Range::Begin(Range), Range::End(Range)) { }

	/** Copy constructor. Constructs the container with the copy of the contents of 'InValue'. */
	FORCEINLINE TList(const TList& InValue) requires (CCopyConstructible<FElementType>) : TList(InValue.Begin(), InValue.End()) { }

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TList(TList&& InValue) : TList() { Swap(*this, InValue); }

	/** Constructs the container with the contents of the initializer list. */
	FORCEINLINE TList(initializer_list<FElementType> IL) requires (CCopyConstructible<FElementType>) : TList(Range::Begin(IL), Range::End(IL)) { }

	/** Destructs the list. The destructors of the elements are called and the used storage is deallocated. */
	~TList()
	{
		FNode* NodeToDeallocate = Impl.HeadNode->NextNode;

		Impl->Deallocate(NodeToDeallocate->PrevNode);

		for (size_t Index = 0; Index != Impl.ListNum; ++Index)
		{
			FNode* NextNode = NodeToDeallocate->NextNode;

			Memory::Destruct(NodeToDeallocate);
			Impl->Deallocate(NodeToDeallocate);

			NodeToDeallocate = NextNode;
		}
	}

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	TList& operator=(const TList& InValue) requires (CCopyable<FElementType>)
	{
		if (&InValue == this) UNLIKELY return *this;

		     FIterator ThisIter  =         Begin();
		FConstIterator OtherIter = InValue.Begin();

		while (ThisIter != End() && OtherIter != InValue.End())
		{
			*ThisIter = *OtherIter;

			++ThisIter;
			++OtherIter;
		}

		if (ThisIter == End())
		{
			while (OtherIter != InValue.End())
			{
				EmplaceBack(*OtherIter);
				++OtherIter;
			}
		}
		else if (OtherIter == InValue.End())
		{
			Erase(ThisIter, End());
		}

		Impl.ListNum = InValue.Num();

		return *this;
	}

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TList& operator=(TList&& InValue) { Swap(*this, InValue); InValue.Reset(); return *this; }

	/** Replaces the contents with those identified by initializer list. */
	TList& operator=(initializer_list<FElementType> IL) requires (CCopyable<FElementType>)
	{
		      FIterator     ThisIter  =        Begin();
		const FElementType* OtherIter = Range::Begin(IL);

		while (ThisIter != End() && OtherIter != Range::End(IL))
		{
			*ThisIter = *OtherIter;

			++ThisIter;
			++OtherIter;
		}

		if (ThisIter == End())
		{
			while (OtherIter != Range::End(IL))
			{
				EmplaceBack(*OtherIter);
				++OtherIter;
			}
		}
		else if (OtherIter == Range::End(IL))
		{
			Erase(ThisIter, End());
		}

		Impl.ListNum = Range::Num(IL);

		return *this;
	}

	/** Compares the contents of two lists. */
	NODISCARD friend bool operator==(const TList& LHS, const TList& RHS) requires (CWeaklyEqualityComparable<FElementType>)
	{
		if (LHS.Num() != RHS.Num()) return false;

		FConstIterator LHSIter = LHS.Begin();
		FConstIterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End())
		{
			if (*LHSIter != *RHSIter) return false;

			++LHSIter;
			++RHSIter;
		}

		check(RHSIter == RHS.End());

		return true;
	}

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend auto operator<=>(const TList& LHS, const TList& RHS) requires (CSynthThreeWayComparable<FElementType>)
	{
		FConstIterator LHSIter = LHS.Begin();
		FConstIterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End() && RHSIter != RHS.End())
		{
			if (const auto Result = SynthThreeWayCompare(*LHSIter, *RHSIter); Result != 0) return Result;

			++LHSIter;
			++RHSIter;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, const FElementType& InValue) requires (CCopyConstructible<FElementType>) { return Emplace(Iter, InValue); }

	/** Inserts 'InValue' before 'Iter' in the container. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, FElementType&& InValue) requires (CMoveConstructible<FElementType>) { return Emplace(Iter, MoveTemp(InValue)); }

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the container. */
	FIterator Insert(FConstIterator Iter, size_t Count, const FElementType& InValue) requires (CCopyConstructible<FElementType>)
	{
		return Insert(Iter, Range::Repeat(InValue, Count));
	}

	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	FIterator Insert(FConstIterator Iter, I First, S Last)
	{
		if (First == Last) return FIterator(Iter.Pointer);

		FNode* InsertNode = Iter.Pointer->PrevNode;

		const auto InsertOnce = [&]() -> FNode*
		{
			FNode* Node = new (Impl->Allocate(1)) FNode(InPlace, *First);

			InsertNode->NextNode = Node;
			Node->PrevNode = InsertNode;

			++Impl.ListNum;

			InsertNode = Node;

			return Node;
		};

		FNode* FirstNode = InsertOnce();

		for (++First; First != Last; ++First)
		{
			InsertOnce();
		}

		InsertNode->NextNode = Iter.Pointer;
		Iter.Pointer->PrevNode = InsertNode;

		return FIterator(FirstNode);
	}

	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputRange R> requires (CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE FIterator Insert(FConstIterator Iter, R&& Range) { return Insert(Iter, Range::Begin(Range), Range::End(Range)); }

	/** Inserts elements from initializer list before 'Iter' in the container. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, initializer_list<FElementType> IL) requires (CCopyConstructible<FElementType>) { return Insert(Iter, Range::Begin(IL), Range::End(IL)); }

	/** Inserts a new element into the container directly before 'Iter'. */
	template <typename... Ts> requires (CConstructibleFrom<FElementType, Ts...>)
	FIterator Emplace(FConstIterator Iter, Ts&&... Args)
	{
		FNode* Node = new (Impl->Allocate(1)) FNode(InPlace, Forward<Ts>(Args)...);

		++Impl.ListNum;

		Node->PrevNode = Iter.Pointer->PrevNode;
		Node->NextNode = Iter.Pointer;

		Node->PrevNode->NextNode = Node;
		Node->NextNode->PrevNode = Node;

		return FIterator(Node);
	}

	/** Removes the element at 'Iter' in the container. */
	FIterator Erase(FConstIterator Iter)
	{
		FNode* NodeToErase = Iter.Pointer;

		checkf(NodeToErase->NextNode != NodeToErase, TEXT("Read access violation. Please check Iter != End()."));

		NodeToErase->PrevNode->NextNode = NodeToErase->NextNode;
		NodeToErase->NextNode->PrevNode = NodeToErase->PrevNode;

		FNode* NextNode = NodeToErase->NextNode;

		Memory::Destruct(NodeToErase);
		Impl->Deallocate(NodeToErase);

		--Impl.ListNum;

		return FIterator(NextNode);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. */
	FIterator Erase(FConstIterator First, FConstIterator Last)
	{
		FNode* FirstToErase = First.Pointer;
		FNode* LastToErase  = Last.Pointer;

		FirstToErase->PrevNode->NextNode = LastToErase;
		LastToErase->PrevNode = FirstToErase->PrevNode;

		while (FirstToErase != LastToErase)
		{
			FNode* NextNode = FirstToErase->NextNode;

			Memory::Destruct(FirstToErase);
			Impl->Deallocate(FirstToErase);

			--Impl.ListNum;

			FirstToErase = NextNode;
		}

		return FIterator(LastToErase);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(const FElementType& InValue) requires (CCopyConstructible<FElementType>) { EmplaceBack(InValue); }

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(FElementType&& InValue) requires (CMoveConstructible<FElementType>) { EmplaceBack(MoveTemp(InValue)); }

	/** Appends a new element to the end of the container. */
	template <typename... Ts> requires (CConstructibleFrom<FElementType, Ts...>)
	FORCEINLINE FReference EmplaceBack(Ts&&... Args) { return *Emplace(End(), Forward<Ts>(Args)...); }

	/** Removes the last element of the container. The list cannot be empty. */
	FORCEINLINE void PopBack() { Erase(--End()); }

	/** Prepends the given element value to the beginning of the container. */
	FORCEINLINE void PushFront(const FElementType& InValue) requires (CCopyConstructible<FElementType>) { EmplaceFront(InValue); }

	/** Prepends the given element value to the beginning of the container. */
	FORCEINLINE void PushFront(FElementType&& InValue) requires (CMoveConstructible<FElementType>) { EmplaceFront(MoveTemp(InValue)); }

	/** Prepends a new element to the beginning of the container. */
	template <typename... Ts> requires (CConstructibleFrom<FElementType, Ts...>)
	FORCEINLINE FReference EmplaceFront(Ts&&... Args) { return *Emplace(Begin(), Forward<Ts>(Args)...); }

	/** Removes the first element of the container. The list cannot be empty. */
	FORCEINLINE void PopFront() { Erase(Begin()); }

	/** Resizes the container to contain 'Count' elements. Additional default elements are appended. */
	void SetNum(size_t Count) requires (CDefaultConstructible<FElementType>)
	{
		if (Count == Impl.ListNum) return;

		if (Count < Impl.ListNum)
		{
			FIterator First = End();

			for (size_t Index = 0; Index != Impl.ListNum - Count; ++Index) --First;

			Erase(First, End());

			Impl.ListNum = Count;

			return;
		}

		FNode* EndNode = Impl.HeadNode->PrevNode;

		while (Count > Impl.ListNum)
		{
			FNode* Node = new (Impl->Allocate(1)) FNode;

			EndNode->NextNode = Node;
			Node->PrevNode = EndNode;

			++Impl.ListNum;

			EndNode = Node;
		}

		EndNode->NextNode = Impl.HeadNode;
		Impl.HeadNode->PrevNode = EndNode;
	}

	/** Resizes the container to contain 'Count' elements. Additional copies of 'InValue' are appended. */
	void SetNum(size_t Count, const FElementType& InValue) requires (CCopyConstructible<FElementType>)
	{
		if (Count == Impl.ListNum) return;

		if (Count < Impl.ListNum)
		{
			FIterator First = End();

			for (size_t Index = 0; Index != Impl.ListNum - Count; ++Index) --First;

			Erase(First, End());

			Impl.ListNum = Count;

			return;
		}

		FNode* EndNode = Impl.HeadNode->PrevNode;

		while (Count > Impl.ListNum)
		{
			FNode* Node = new (Impl->Allocate(1)) FNode(InPlace, InValue);

			EndNode->NextNode = Node;
			Node->PrevNode = EndNode;

			++Impl.ListNum;

			EndNode = Node;
		}

		EndNode->NextNode = Impl.HeadNode;
		Impl.HeadNode->PrevNode = EndNode;
	}

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE      FIterator Begin()       { return      FIterator(Impl.HeadNode->NextNode); }
	NODISCARD FORCEINLINE FConstIterator Begin() const { return FConstIterator(Impl.HeadNode->NextNode); }
	NODISCARD FORCEINLINE      FIterator End()         { return      FIterator(Impl.HeadNode);           }
	NODISCARD FORCEINLINE FConstIterator End()   const { return FConstIterator(Impl.HeadNode);           }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE      FReverseIterator RBegin()       { return      FReverseIterator(End());   }
	NODISCARD FORCEINLINE FConstReverseIterator RBegin() const { return FConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      FReverseIterator REnd()         { return      FReverseIterator(Begin()); }
	NODISCARD FORCEINLINE FConstReverseIterator REnd()   const { return FConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE size_t Num() const { return Impl.ListNum; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(FConstIterator Iter) const
	{
		FNode* Current = Impl.HeadNode;

		for (size_t Index = 0; Index != Impl.ListNum + 1; ++Index)
		{
			if (Current == Iter.Pointer)
			{
				return true;
			}

			Current = Current->NextNode;
		}

		return false;
	}

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE       FElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const FElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       FElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const FElementType& Back()  const { return *(End() - 1); }

	/** Erases all elements from the container. After this call, Num() returns zero. */
	void Reset()
	{
		FNode* NodeToDeallocate = Impl.HeadNode->NextNode;

		Impl.HeadNode->PrevNode = Impl.HeadNode;
		Impl.HeadNode->NextNode = Impl.HeadNode;

		for (size_t Index = 0; Index != Impl.ListNum; ++Index)
		{
			FNode* NextNode = NodeToDeallocate->NextNode;

			Memory::Destruct(NodeToDeallocate);
			Impl->Deallocate(NodeToDeallocate);

			NodeToDeallocate = NextNode;
		}

		Impl.ListNum = 0;
	}

	/** Overloads the GetTypeHash algorithm for TList. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TList& A) requires (CHashable<FElementType>)
	{
		size_t Result = 0;

		for (const FElementType& Element : A)
		{
			Result = HashCombine(Result, GetTypeHash(Element));
		}

		return Result;
	}

	/** Overloads the Swap algorithm for TList. */
	friend FORCEINLINE void Swap(TList& A, TList& B)
	{
		Swap(A.Impl.HeadNode, B.Impl.HeadNode);
		Swap(A.Impl.ListNum,  B.Impl.ListNum);
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	struct FNode
	{
		FNode* PrevNode;
		FNode* NextNode;
		FElementType Value;

		FORCEINLINE FNode() = default;

		template <typename... Ts>
		FORCEINLINE FNode(FInPlace, Ts&&... Args) : Value(Forward<Ts>(Args)...) { }
	};

	static_assert(CMultipleAllocator<FAllocatorType, FNode>);

	ALLOCATOR_WRAPPER_BEGIN(FAllocatorType, FNode, Impl)
	{
		FNode* HeadNode;
		size_t ListNum;
	}
	ALLOCATOR_WRAPPER_END(FAllocatorType, FNode, Impl)

	template <bool bConst, typename U>
	class TIteratorImpl final
	{
	public:

		using FElementType = TRemoveCV<T>;

		FORCEINLINE TIteratorImpl() = default;

		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer)
		{ }

		FORCEINLINE TIteratorImpl(const TIteratorImpl&)            = default;
		FORCEINLINE TIteratorImpl(TIteratorImpl&&)                 = default;
		FORCEINLINE TIteratorImpl& operator=(const TIteratorImpl&) = default;
		FORCEINLINE TIteratorImpl& operator=(TIteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE bool operator==(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD FORCEINLINE U& operator*()  const { return  Pointer->Value; }
		NODISCARD FORCEINLINE U* operator->() const { return &Pointer->Value; }

		FORCEINLINE TIteratorImpl& operator++() { Pointer = Pointer->NextNode; return *this; }
		FORCEINLINE TIteratorImpl& operator--() { Pointer = Pointer->PrevNode; return *this; }

		FORCEINLINE TIteratorImpl operator++(int) { TIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE TIteratorImpl operator--(int) { TIteratorImpl Temp = *this; --*this; return Temp; }

	private:

		FNode* Pointer = nullptr;

		FORCEINLINE TIteratorImpl(FNode* InPointer)
			: Pointer(InPointer)
		{ }

		template <bool, typename> friend class TIteratorImpl;

		friend TList;

	};

};

template <typename I, typename S>
TList(I, S) -> TList<TIteratorElement<I>>;

template <typename R>
TList(R) -> TList<TRangeElement<R>>;

template <typename T>
TList(initializer_list<T>) -> TList<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
