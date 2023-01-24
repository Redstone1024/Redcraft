#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FAllocatorInterface;

template <typename T>
concept CInstantiableAllocator = CDerivedFrom<T, FAllocatorInterface> && !CSameAs<T, FAllocatorInterface>;

/**
 * This is the allocator interface, the allocator does not use virtual, this contains the default of
 * the allocator interface functions. Unlike std::allocator, IAllocator should be bound to only a object,
 * such as a container, because there may be side effects between multiple allocations, for example,
 * inline storage cannot be allocated multiple times in TInlineAllocator.
 */
struct FAllocatorInterface
{
	template <CObject T>
	struct ForElementType : private FSingleton
	{
		/** 
		 * Allocates uninitialized storage.
		 * Should be allocated according to the results given by the CalculateSlackReserve() family,
		 * without needing to allocate memory of the same size as the allocated memory,
		 * this is to support special allocators such as TInlineAllocator.
		 * If 'InNum' is zero, return nullptr.
		 */
		NODISCARD FORCEINLINE T* Allocate(size_t InNum) = delete;

		/** Deallocates storage. */
		FORCEINLINE void Deallocate(T* InPtr) = delete;

		/** @return true if allocation can be deallocated by another allocator, otherwise false. */
		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) { return true; }

		/** Calculates the amount of slack to allocate for an array that has just grown to a given number of elements. */
		NODISCARD FORCEINLINE constexpr size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just shrunk to a given number of elements. */
		NODISCARD FORCEINLINE constexpr size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just grown or shrunk to a given number of elements. */
		NODISCARD FORCEINLINE constexpr size_t CalculateSlackReserve(size_t Num) const = delete;

	};
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
