#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

template <typename ElementType>
	requires CDefaultConstructible<ElementType>
FORCEINLINE void DefaultConstruct(ElementType* Address, size_t Count = 1)
{
	if constexpr (!CTriviallyDefaultConstructible<ElementType>)
	{
		ElementType* Element = Address;
		while (Count)
		{
			new (Element) ElementType;
			++Element;
			--Count;
		}
	}
}

template <typename DestinationElementType, typename SourceElementType = DestinationElementType>
	requires CConstructibleFrom<DestinationElementType, const SourceElementType&>
FORCEINLINE void Construct(DestinationElementType* Destination, const SourceElementType* Source, size_t Count = 1)
{
	if constexpr (CTriviallyConstructibleFrom<DestinationElementType, const SourceElementType> && sizeof(DestinationElementType) == sizeof(SourceElementType))
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
	if constexpr (CTriviallyMoveAssignable<ElementType>)
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

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
