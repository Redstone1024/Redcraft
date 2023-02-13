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

/** This is heap allocator that calls Memory::Malloc() directly for memory allocation. */
struct FHeapAllocator : public FAllocatorInterface
{
	template <CObject T>
	struct ForElementType : public FAllocatorInterface::ForElementType<T>
	{
		NODISCARD FORCEINLINE T* Allocate(size_t InNum)
		{
			return InNum != 0 ? static_cast<T*>(Memory::Malloc(Memory::QuantizeSize(InNum * sizeof(T)), alignof(T))) : nullptr;
		}

		FORCEINLINE void Deallocate(T* InPtr)
		{
			Memory::Free(InPtr);
		}

		NODISCARD FORCEINLINE constexpr size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const
		{
			const size_t FirstGrow    = 4;
			const size_t ConstantGrow = 16;

			size_t Result;

			check(Num > NumAllocated);

			Result = (NumAllocated != 0) ? (Num + 3 * Num / 8 + ConstantGrow) : (Num > FirstGrow ? Num : FirstGrow);

			Result = Memory::QuantizeSize(Result * sizeof(T), alignof(T)) / sizeof(T);

			return Result;
		}

		NODISCARD FORCEINLINE constexpr size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const
		{
			size_t Result;

			check(Num < NumAllocated);

			const bool bTooManySlackBytes    = (NumAllocated - Num) * sizeof(T) >= 16 * 1024;
			const bool bTooManySlackElements = 3 * Num < 2 * NumAllocated;

			const bool bNeedToShrink = (bTooManySlackBytes || bTooManySlackElements) && (NumAllocated - Num > 64 || Num == 0);

			if (bNeedToShrink)
			{
				Result = Num != 0 ? Memory::QuantizeSize(Num * sizeof(T), alignof(T)) / sizeof(T) : 0;
			}
			else
			{
				Result = NumAllocated;
			}

			return Result;
		}

		NODISCARD FORCEINLINE constexpr size_t CalculateSlackReserve(size_t Num) const
		{
			return Num != 0 ? Memory::QuantizeSize(Num * sizeof(T), alignof(T)) / sizeof(T) : 0;
		}

	};
};

using FDefaultAllocator = FHeapAllocator;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
