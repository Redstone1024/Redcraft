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
	class ForElementType : private FSingleton
	{
	public:

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
		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const { return true; }

		/** Calculates the amount of slack to allocate for an array that has just grown to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just shrunk to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just grown or shrunk to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const = delete;

	};
};

/** This is heap allocator that calls Memory::Malloc() directly for memory allocation. */
struct FHeapAllocator : public FAllocatorInterface
{
	template <CObject T>
	class ForElementType : public FAllocatorInterface::ForElementType<T>
	{
	public:

		NODISCARD FORCEINLINE T* Allocate(size_t InNum)
		{
			return InNum != 0 ? static_cast<T*>(Memory::Malloc(Memory::QuantizeSize(InNum * sizeof(T)), alignof(T))) : nullptr;
		}

		FORCEINLINE void Deallocate(T* InPtr)
		{
			Memory::Free(InPtr);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const
		{
			const size_t FirstGrow    = 4;
			const size_t ConstantGrow = 16;

			size_t Result;

			check(Num > NumAllocated);

			Result = (NumAllocated != 0) ? (Num + 3 * Num / 8 + ConstantGrow) : (Num > FirstGrow ? Num : FirstGrow);

			Result = Memory::QuantizeSize(Result * sizeof(T), alignof(T)) / sizeof(T);

			return Result;
		}

		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const
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

		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const
		{
			return Num != 0 ? Memory::QuantizeSize(Num * sizeof(T), alignof(T)) / sizeof(T) : 0;
		}

	};
};

/**
 * The inline allocator allocates up to a specified number of elements in the same allocation as the container.
 * Any allocation needed beyond that causes all data to be moved into an indirect allocation.
 */
template <size_t NumInline, CInstantiableAllocator SecondaryAllocator = FHeapAllocator>
struct TInlineAllocator : public FAllocatorInterface
{
	template <CObject T>
	class ForElementType : public FAllocatorInterface::ForElementType<T>
	{
	public:

		NODISCARD FORCEINLINE T* Allocate(size_t InNum)
		{
			if (InNum == 0) return nullptr;

			check(InNum >= NumInline);

			if (InNum == NumInline) return reinterpret_cast<T*>(&InlineStorage);

			return Secondary.Allocate(InNum);
		}

		FORCEINLINE void Deallocate(T* InPtr)
		{
			if (InPtr == reinterpret_cast<T*>(&InlineStorage)) return;

			Secondary.Deallocate(InPtr);
		}

		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const
		{
			if (InPtr == reinterpret_cast<const T*>(&InlineStorage)) return false;

			return Secondary.IsTransferable(InPtr);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const
		{
			check(Num > NumAllocated);
			check(NumAllocated >= NumInline);

			if (Num <= NumInline) return NumInline;

			return Secondary.CalculateSlackGrow(Num, NumAllocated <= NumInline ? 0 : NumAllocated);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const
		{
			check(Num < NumAllocated);
			check(NumAllocated >= NumInline);

			if (Num <= NumInline) return NumInline;

			return Secondary.CalculateSlackShrink(Num, NumAllocated);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const
		{
			if (Num <= NumInline) return NumInline;

			return Secondary.CalculateSlackReserve(Num);
		}

	private:

		TAlignedStorage<sizeof(T), alignof(T)> InlineStorage[NumInline];

		typename SecondaryAllocator::template ForElementType<T> Secondary;

	};
};

/** This is a null allocator for which all operations are illegal. */
struct FNullAllocator : public FAllocatorInterface
{
	template <CObject T>
	class ForElementType : public FAllocatorInterface::ForElementType<T>
	{
	public:

		NODISCARD FORCEINLINE T* Allocate(size_t InNum) { check_no_entry(); return nullptr; }

		FORCEINLINE void Deallocate(T* InPtr) { check_no_entry(); }

		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const { check_no_entry(); return false; }

		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const { check_no_entry(); return 0; }

		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const { check_no_entry(); return 0; }

		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const { check_no_entry(); return 0; }

	};
};

template <size_t Num>
using TFixedAllocator = TInlineAllocator<Num, FNullAllocator>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
