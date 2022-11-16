#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

template <CDefaultConstructible ElementType>
FORCEINLINE void DefaultConstruct(void* Address, size_t Count = 1)
{
	if constexpr (!CTriviallyDefaultConstructible<ElementType>)
	{
		while (Count)
		{
			new (Address) ElementType;
			++reinterpret_cast<ElementType*&>(Address);
			--Count;
		}
	}
}

template <typename DestinationElementType, typename SourceElementType = DestinationElementType>
	requires (CConstructibleFrom<DestinationElementType, const SourceElementType&>)
FORCEINLINE void Construct(void* Destination, const SourceElementType* Source, size_t Count = 1)
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
			++reinterpret_cast<DestinationElementType*&>(Destination);
			++Source;
			--Count;
		}
	}
}

template <CCopyConstructible ElementType>
FORCEINLINE void CopyConstruct(void* Destination, const ElementType* Source, size_t Count = 1)
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
			++reinterpret_cast<ElementType*&>(Destination);
			++Source;
			--Count;
		}
	}
}

template <CMoveConstructible ElementType>
FORCEINLINE void MoveConstruct(void* Destination, ElementType* Source, size_t Count = 1)
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
			++reinterpret_cast<ElementType*&>(Destination);
			++Source;
			--Count;
		}
	}
}

template <CCopyAssignable ElementType>
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

template <CMoveAssignable ElementType>
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
			++Destination;
			++Source;
			--Count;
		}
	}
}

template <CDestructible ElementType>
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
