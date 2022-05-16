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

template <typename ElementType>
	requires CDefaultConstructible<ElementType>
FORCEINLINE void DefaultConstruct(ElementType* Address, size_t Count = 1)
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

template <typename DestinationElementType, typename SourceElementType>
	requires CConstructible<DestinationElementType, const SourceElementType&>
FORCEINLINE void Construct(DestinationElementType* Destination, const SourceElementType* Source, size_t Count = 1)
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

template <typename ElementType>
	requires CCopyConstructible<ElementType>
FORCEINLINE void CopyConstruct(ElementType* Destination, const ElementType* Source, size_t Count = 1)
{
	if constexpr (CTriviallyCopyConstructible<ElementType>)
	{
		Memory::Memcpy(Destination, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			new (Destination) ElementType(*Source);
			++(ElementType*&)Destination;
			++Source;
			--Count;
		}
	}
}

template <typename ElementType>
	requires CMoveConstructible<ElementType>
FORCEINLINE void MoveConstruct(ElementType* Destination, ElementType* Source, size_t Count = 1)
{
	if constexpr (CTriviallyMoveConstructible<ElementType>)
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

template <typename DestinationElementType, typename SourceElementType>
	requires CConstructible<DestinationElementType, SourceElementType&&> && CDestructible<SourceElementType>
FORCEINLINE void RelocateConstruct(DestinationElementType* Destination, SourceElementType* Source, size_t Count = 1)
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

template <typename ElementType>
	requires CDestructible<ElementType>
FORCEINLINE void Destruct(ElementType* Element, size_t Count = 1)
{
	if constexpr (!CTriviallyDestructible<ElementType>)
	{
		while (Count)
		{
			Element->~ElementType();
			++Element;
			--Count;
		}
	}
}

template <typename ElementType>
	requires CCopyAssignable<ElementType>
FORCEINLINE void CopyAssign(ElementType* Destination, const ElementType* Source, size_t Count = 1)
{
	if constexpr (CTriviallyCopyAssignable<ElementType>)
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

template <typename ElementType>
	requires CMoveAssignable<ElementType>
FORCEINLINE void MoveAssign(ElementType* Destination, ElementType* Source, size_t Count = 1)
{
	if constexpr (CTriviallyCopyConstructible<ElementType>)
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

template <typename ElementType>
	requires CEqualityComparable<ElementType>
FORCEINLINE bool Compare(const ElementType* LHS, const ElementType* RHS, size_t Count = 1)
{
	if constexpr (TIsBitwiseComparable<ElementType>::Value)
	{
		return !Memory::Memcmp(LHS, RHS, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			if (!(*LHS == *RHS)) return false;

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
