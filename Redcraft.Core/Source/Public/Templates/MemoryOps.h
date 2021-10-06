#pragma once

#include "CoreTypes.h"

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

template <typename ElementType, typename SizeType>
FORCEINLINE void DefaultConstructItems(void* Address, SizeType Count = 1);

template <typename ElementType, typename SizeType>
FORCEINLINE void DestructItems(ElementType* Element, SizeType Count = 1);

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE void ConstructItems(void* Dest, const SourceElementType* Source, SizeType Count = 1);

template <typename ElementType, typename SizeType>
FORCEINLINE void CopyAssignItems(ElementType* Dest, const ElementType* Source, SizeType Count = 1);

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE void RelocateConstructItems(void* Dest, const SourceElementType* Source, SizeType Count = 1);

template <typename ElementType, typename SizeType>
FORCEINLINE void MoveConstructItems(void* Dest, const ElementType* Source, SizeType Count = 1);

template <typename ElementType, typename SizeType>
FORCEINLINE void MoveAssignItems(ElementType* Dest, const ElementType* Source, SizeType Count = 1);

template <typename ElementType, typename SizeType>
FORCEINLINE bool CompareItems(const ElementType* A, const ElementType* B, SizeType Count = 1);

NS_END(Memory)
NS_REDCRAFT_END

#include "Templates/MemoryOps.inl"
