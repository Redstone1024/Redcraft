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
#include "Miscellaneous/ConstantIterator.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CAllocatableObject T, CMultipleAllocator<T> Allocator = FHeapAllocator>
class TList final
{
private:

	struct FNode;

	template <bool bConst>
	class IteratorImpl;

public:

	using ElementType   = T;
	using AllocatorType = Allocator;

	using      Reference =       T&;
	using ConstReference = const T&;

	using      Iterator = IteratorImpl<false>;
	using ConstIterator = IteratorImpl<true >;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CBidirectionalIterator<     Iterator>);
	static_assert(CBidirectionalIterator<ConstIterator>);

	/** Default constructor. Constructs an empty container with a default-constructed allocator. */
	TList()
	{
		Impl.HeadNode = Impl->Allocate(1);
		Impl.HeadNode->PrevNode = Impl.HeadNode;
		Impl.HeadNode->NextNode = Impl.HeadNode;

		Impl.ListNum = 0;
	}

	/** Constructs the container with 'Count' default instances of T. */
	explicit TList(size_t Count) requires (CDefaultConstructible<ElementType>) : TList()
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
	TList(size_t Count, const ElementType& InValue) requires (CCopyable<ElementType>)
		: TList(MakeCountedConstantIterator(InValue, Count), DefaultSentinel)
	{ }

	/** Constructs the container with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
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

	/** Copy constructor. Constructs the container with the copy of the contents of 'InValue'. */
	FORCEINLINE TList(const TList& InValue) requires (CCopyConstructible<ElementType>) : TList(InValue.Begin(), InValue.End()) { }

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TList(TList&& InValue) : TList() { Swap(*this, InValue); }

	/** Constructs the container with the contents of the initializer list. */
	FORCEINLINE TList(initializer_list<ElementType> IL) requires (CCopyConstructible<ElementType>) : TList(Iteration::Begin(IL), Iteration::End(IL)) { }

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
	TList& operator=(const TList& InValue) requires (CCopyable<ElementType>)
	{
		if (&InValue == this) UNLIKELY return *this;

		     Iterator ThisIter  =         Begin();
		ConstIterator OtherIter = InValue.Begin();

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
	TList& operator=(initializer_list<ElementType> IL) requires (CCopyable<ElementType>)
	{
		      Iterator     ThisIter  =            Begin();
		const ElementType* OtherIter = Iteration::Begin(IL);

		while (ThisIter != End() && OtherIter != Iteration::End(IL))
		{
			*ThisIter = *OtherIter;

			++ThisIter;
			++OtherIter;
		}

		if (ThisIter == End())
		{
			while (OtherIter != Iteration::End(IL))
			{
				EmplaceBack(*OtherIter);
				++OtherIter;
			}
		}
		else if (OtherIter == Iteration::End(IL))
		{
			Erase(ThisIter, End());
		}

		Impl.ListNum = GetNum(IL);

		return *this;
	}

	/** Compares the contents of two lists. */
	NODISCARD friend bool operator==(const TList& LHS, const TList& RHS) requires (CWeaklyEqualityComparable<ElementType>)
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

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend auto operator<=>(const TList& LHS, const TList& RHS) requires (CSynthThreeWayComparable<ElementType>)
	{
		ConstIterator LHSIter = LHS.Begin();
		ConstIterator RHSIter = RHS.Begin();

		while (LHSIter != LHS.End() && RHSIter != RHS.End())
		{
			if (const auto Result = SynthThreeWayCompare(*LHSIter, *RHSIter); Result != 0) return Result;

			++LHSIter;
			++RHSIter;
		}

		return LHS.Num() <=> RHS.Num();
	}

	/** Inserts 'InValue' before 'Iter' in the container. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, const ElementType& InValue) requires (CCopyConstructible<ElementType>) { return Emplace(Iter, InValue); }

	/** Inserts 'InValue' before 'Iter' in the container. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, ElementType&& InValue) requires (CMoveConstructible<ElementType>) { return Emplace(Iter, MoveTemp(InValue)); }

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the container. */
	Iterator Insert(ConstIterator Iter, size_t Count, const ElementType& InValue) requires (CCopyConstructible<ElementType>)
	{
		return Insert(Iter, MakeCountedConstantIterator(InValue, Count), DefaultSentinel);
	}

	/** Inserts elements from range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	Iterator Insert(ConstIterator Iter, I First, S Last)
	{
		if (First == Last) return Iterator(Iter.Pointer);

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

		return Iterator(FirstNode);
	}

	/** Inserts elements from initializer list before 'Iter' in the container. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, initializer_list<ElementType> IL) requires (CCopyConstructible<ElementType>) { return Insert(Iter, Iteration::Begin(IL), Iteration::End(IL)); }

	/** Inserts a new element into the container directly before 'Iter'. */
	template <typename... Ts> requires (CConstructibleFrom<ElementType, Ts...>)
	Iterator Emplace(ConstIterator Iter, Ts&&... Args)
	{
		FNode* Node = new (Impl->Allocate(1)) FNode(InPlace, Forward<Ts>(Args)...);

		++Impl.ListNum;

		Node->PrevNode = Iter.Pointer->PrevNode;
		Node->NextNode = Iter.Pointer;

		Node->PrevNode->NextNode = Node;
		Node->NextNode->PrevNode = Node;

		return Iterator(Node);
	}

	/** Removes the element at 'Iter' in the container. */
	Iterator Erase(ConstIterator Iter)
	{
		FNode* NodeToErase = Iter.Pointer;

		checkf(NodeToErase->NextNode != NodeToErase, TEXT("Read access violation. Please check Iter != End()."));

		NodeToErase->PrevNode->NextNode = NodeToErase->NextNode;
		NodeToErase->NextNode->PrevNode = NodeToErase->PrevNode;

		FNode* NextNode = NodeToErase->NextNode;

		Memory::Destruct(NodeToErase);
		Impl->Deallocate(NodeToErase);

		--Impl.ListNum;

		return Iterator(NextNode);
	}

	/** Removes the elements in the range ['First', 'Last') in the container. */
	Iterator Erase(ConstIterator First, ConstIterator Last)
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

		return Iterator(LastToErase);
	}

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(const ElementType& InValue) requires (CCopyConstructible<ElementType>) { EmplaceBack(InValue); }

	/** Appends the given element value to the end of the container. */
	FORCEINLINE void PushBack(ElementType&& InValue) requires (CMoveConstructible<ElementType>) { EmplaceBack(MoveTemp(InValue)); }

	/** Appends a new element to the end of the container. */
	template <typename... Ts> requires (CConstructibleFrom<ElementType, Ts...>)
	FORCEINLINE Reference EmplaceBack(Ts&&... Args) { return *Emplace(End(), Forward<Ts>(Args)...); }

	/** Removes the last element of the container. The list cannot be empty. */
	FORCEINLINE void PopBack() { Erase(--End()); }

	/** Prepends the given element value to the beginning of the container. */
	FORCEINLINE void PushFront(const ElementType& InValue) requires (CCopyConstructible<ElementType>) { EmplaceFront(InValue); }

	/** Prepends the given element value to the beginning of the container. */
	FORCEINLINE void PushFront(ElementType&& InValue) requires (CMoveConstructible<ElementType>) { EmplaceFront(MoveTemp(InValue)); }

	/** Prepends a new element to the beginning of the container. */
	template <typename... Ts> requires (CConstructibleFrom<ElementType, Ts...>)
	FORCEINLINE Reference EmplaceFront(Ts&&... Args) { return *Emplace(Begin(), Forward<Ts>(Args)...); }

	/** Removes the first element of the container. The list cannot be empty. */
	FORCEINLINE void PopFront() { Erase(Begin()); }

	/** Resizes the container to contain 'Count' elements. Additional default elements are appended. */
	void SetNum(size_t Count) requires (CDefaultConstructible<ElementType>)
	{
		if (Count == Impl.ListNum) return;

		if (Count < Impl.ListNum)
		{
			Iterator First = End();

			Iteration::Advance(First, Count - Impl.ListNum);

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
	void SetNum(size_t Count, const ElementType& InValue) requires (CCopyConstructible<ElementType>)
	{
		if (Count == Impl.ListNum) return;

		if (Count < Impl.ListNum)
		{
			Iterator First = End();

			Iteration::Advance(First, Count - Impl.ListNum);

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
	NODISCARD FORCEINLINE      Iterator Begin()       { return      Iterator(Impl.HeadNode->NextNode); }
	NODISCARD FORCEINLINE ConstIterator Begin() const { return ConstIterator(Impl.HeadNode->NextNode); }
	NODISCARD FORCEINLINE      Iterator End()         { return      Iterator(Impl.HeadNode);           }
	NODISCARD FORCEINLINE ConstIterator End()   const { return ConstIterator(Impl.HeadNode);           }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE size_t Num() const { return Impl.ListNum; }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(ConstIterator Iter) const
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
	NODISCARD FORCEINLINE       ElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       ElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const ElementType& Back()  const { return *(End() - 1); }

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
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TList& A) requires (CHashable<ElementType>)
	{
		size_t Result = 0;

		for (const ElementType& Element : A)
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
		ElementType Value;

		FORCEINLINE FNode() = default;

		template <typename... Ts>
		FORCEINLINE FNode(FInPlace, Ts&&... Args) : Value(Forward<Ts>(Args)...) { }
	};

	static_assert(CMultipleAllocator<AllocatorType, FNode>);

	ALLOCATOR_WRAPPER_BEGIN(AllocatorType, FNode, Impl)
	{
		FNode* HeadNode;
		size_t ListNum;
	}
	ALLOCATOR_WRAPPER_END(AllocatorType, FNode, Impl)

	template <bool bConst>
	class IteratorImpl
	{
	public:

		using ElementType = TConditional<bConst, const T, T>;

		FORCEINLINE IteratorImpl() = default;

		FORCEINLINE IteratorImpl(const IteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer)
		{ }

		FORCEINLINE IteratorImpl(const IteratorImpl&)            = default;
		FORCEINLINE IteratorImpl(IteratorImpl&&)                 = default;
		FORCEINLINE IteratorImpl& operator=(const IteratorImpl&) = default;
		FORCEINLINE IteratorImpl& operator=(IteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE bool operator==(const IteratorImpl& LHS, const IteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer; }

		NODISCARD FORCEINLINE ElementType& operator*()  const { return  Pointer->Value; }
		NODISCARD FORCEINLINE ElementType* operator->() const { return &Pointer->Value; }

		FORCEINLINE IteratorImpl& operator++() { Pointer = Pointer->NextNode; return *this; }
		FORCEINLINE IteratorImpl& operator--() { Pointer = Pointer->PrevNode; return *this; }

		FORCEINLINE IteratorImpl operator++(int) { IteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE IteratorImpl operator--(int) { IteratorImpl Temp = *this; --*this; return Temp; }

	private:

		FNode* Pointer = nullptr;

		FORCEINLINE IteratorImpl(FNode* InPointer)
			: Pointer(InPointer)
		{ }

		template <bool> friend class IteratorImpl;

		friend TList;

	};

};

template <typename I, typename S>
TList(I, S) -> TList<TIteratorElementType<I>>;

template <typename T>
TList(initializer_list<T>) -> TList<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
