#include "HAL/Memory.h"
#include "Templates/TypeTraits.h"
#include "MemoryOps.h"

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

NS_PRIVATE_BEGIN

template <typename DestinationElementType, typename SourceElementType>
struct TCanBitwiseRelocate
{
	enum
	{
		Value =
		TypeTraits::TOr<
			TypeTraits::TIsSame<DestinationElementType, SourceElementType>,
			TypeTraits::TAnd<
				TypeTraits::TIsBitwiseConstructible<DestinationElementType, SourceElementType>,
				TypeTraits::TIsTriviallyDestructible<SourceElementType>
			>
		>::Value
	};
};

NS_PRIVATE_END

template<typename ElementType, typename SizeType>
FORCEINLINE void DefaultConstructItems(void* Address, SizeType Count)
{
	if constexpr (TypeTraits::TIsZeroConstructType<ElementType>::Value)
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

template<typename ElementType, typename SizeType>
FORCEINLINE void DestructItems(ElementType* Element, SizeType Count)
{
	if constexpr (!TypeTraits::TIsTriviallyDestructible<ElementType>::Value)
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

template<typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE void ConstructItems(void* Dest, const SourceElementType* Source, SizeType Count)
{
	if constexpr (TypeTraits::TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value)
	{
		Memory::Memcpy(Dest, Source, sizeof(SourceElementType) * Count);
	}
	else
	{
		while (Count)
		{
			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			++Source;
			--Count;
		}
	}
}

template<typename ElementType, typename SizeType>
FORCEINLINE void CopyAssignItems(ElementType* Dest, const ElementType* Source, SizeType Count)
{
	if constexpr (TypeTraits::TIsTriviallyCopyAssignable<ElementType>::Value)
	{
		Memory::Memcpy(Dest, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			*Dest = *Source;
			++Dest;
			++Source;
			--Count;
		}
	}
}

template<typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE void RelocateConstructItems(void* Dest, const SourceElementType* Source, SizeType Count)
{
	if constexpr (NS_PRIVATE::TCanBitwiseRelocate<DestinationElementType, SourceElementType>::Value)
	{
		Memory::Memmove(Dest, Source, sizeof(SourceElementType) * Count);
	}
	else
	{
		while (Count)
		{
			typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			(Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
			--Count;
		}
	}
}

template<typename ElementType, typename SizeType>
FORCEINLINE void MoveConstructItems(void* Dest, const ElementType* Source, SizeType Count)
{
	if constexpr (TypeTraits::TIsTriviallyCopyConstructible<ElementType>::Value)
	{
		Memory::Memmove(Dest, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			new (Dest) ElementType((ElementType&&)*Source);
			++(ElementType*&)Dest;
			++Source;
			--Count;
		}
	}
}

template<typename ElementType, typename SizeType>
FORCEINLINE void MoveAssignItems(ElementType* Dest, const ElementType* Source, SizeType Count)
{
	if constexpr (TypeTraits::TIsTriviallyCopyAssignable<ElementType>::Value)
	{
		Memory::Memmove(Dest, Source, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			*Dest = (ElementType&&)*Source;
			++Dest;
			++Source;
			--Count;
		}
	}
}

template<typename ElementType, typename SizeType>
FORCEINLINE bool CompareItems(const ElementType* A, const ElementType* B, SizeType Count)
{
	if constexpr (TypeTraits::TCanBitwiseCompare<ElementType>::Value)
	{
		return !Memory::Memcmp(A, B, sizeof(ElementType) * Count);
	}
	else
	{
		while (Count)
		{
			if (!(*A == *B))
			{
				return false;
			}

			++A;
			++B;
			--Count;
		}

		return true;
	}
}

NS_END(Memory)
NS_REDCRAFT_END
