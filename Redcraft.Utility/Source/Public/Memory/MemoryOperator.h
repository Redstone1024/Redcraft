#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Utility.h"
#include "Concepts/Comparable.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

template <typename ElementType, typename SizeType = int32>
	requires (TIsDefaultConstructible<ElementType>::Value || TIsZeroConstructible<ElementType>::Value)
FORCEINLINE void DefaultConstruct(ElementType* Address, SizeType Count = 1)
{
	if constexpr (TIsZeroConstructible<ElementType>::Value)
	{
		Memory::Memset(Address, 0, sizeof(ElementType) * Count);
	}
	else
	{
		ElementType* Element = (ElementType*)Address;
		while (Count)
		{
			new (Element) ElementType;
			++Element;
			--Count;
		}
	}
}

template <typename DestinationElementType, typename SourceElementType, typename SizeType = int32>
	requires (TIsConstructible<DestinationElementType, const SourceElementType&>::Value || TIsBitwiseConstructible<DestinationElementType, const SourceElementType&>::Value)
FORCEINLINE void Construct(DestinationElementType* Destination, const SourceElementType* Source, SizeType Count = 1)
{
	if constexpr (TIsBitwiseConstructible<DestinationElementType, const SourceElementType>::Value)
	{
		Memory::Memcpy(Destination, Source, sizeof(SourceElementType) * Count);
	}
	else
	{
		while (Count)
		{
			new (Destination) DestinationElementType(*Source);
			++(DestinationElementType*&)Destination;
			++Source;
			--Count;
		}
	}
}

template <typename ElementType, typename SizeType = int32>
	requires (TIsMoveConstructible<ElementType>::Value)
FORCEINLINE void MoveConstruct(ElementType* Destination, ElementType* Source, SizeType Count = 1)
{
	if constexpr (TIsTriviallyMoveConstructible<ElementType>::Value)
	{
		Memory::Memmove(Destination, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			new (Destination) ElementType(MoveTemp(*Source));
			++(ElementType*&)Destination;
			++Source;
			--Count;
		}
	}
}

template <typename DestinationElementType, typename SourceElementType, typename SizeType = int32>
	requires ((TIsConstructible<DestinationElementType, SourceElementType&&>::Value && TIsDestructible<SourceElementType>::Value) || TIsBitwiseRelocatable<DestinationElementType, SourceElementType>::Value)
FORCEINLINE void RelocateConstruct(DestinationElementType* Destination, SourceElementType* Source, SizeType Count = 1)
{
	if constexpr (TIsBitwiseRelocatable<DestinationElementType, SourceElementType>::Value)
	{
		Memory::Memmove(Destination, Source, sizeof(SourceElementType) * Count);
	}
	else
	{
		while (Count)
		{
			typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

			new (Destination) DestinationElementType(MoveTemp(*Source));
			++(DestinationElementType*&)Destination;
			(Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
			--Count;
		}
	}
}

template <typename ElementType, typename SizeType = int32>
	requires (TIsDestructible<ElementType>::Value)
FORCEINLINE void Destruct(ElementType* Element, SizeType Count = 1)
{
	if constexpr (!TIsTriviallyDestructible<ElementType>::Value)
	{
		while (Count)
		{
			typedef ElementType DestructItemsElementTypeTypedef;

			Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			++Element;
			--Count;
		}
	}
}

template <typename ElementType, typename SizeType = int32>
	requires (TIsCopyAssignable<ElementType>::Value)
FORCEINLINE void CopyAssign(ElementType* Destination, const ElementType* Source, SizeType Count = 1)
{
	if constexpr (TIsTriviallyCopyAssignable<ElementType>::Value)
	{
		Memory::Memcpy(Destination, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			*Destination = *Source;
			++Destination;
			++Source;
			--Count;
		}
	}
}

template <typename ElementType, typename SizeType = int32>
	requires (TIsMoveAssignable<ElementType>::Value)
FORCEINLINE void MoveAssign(ElementType* Destination, ElementType* Source, SizeType Count = 1)
{
	if constexpr (TIsTriviallyCopyConstructible<ElementType>::Value)
	{
		Memory::Memmove(Destination, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			*Destination = MoveTemp(*Source);
			++(ElementType*&)Destination;
			++Source;
			--Count;
		}
	}
}

template <typename ElementType, typename SizeType = int32>
	requires (CEqualityComparable<ElementType> || TIsBitwiseComparable<ElementType>::Value)
FORCEINLINE bool Compare(const ElementType* LHS, const ElementType* RHS, SizeType Count = 1)
{
	if constexpr (TIsBitwiseComparable<ElementType>::Value)
	{
		return !Memory::Memcmp(LHS, RHS, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			if (!(*LHS == *RHS))
			{
				return false;
			}

			++LHS;
			++RHS;
			--Count;
		}

		return true;
	}
}

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END