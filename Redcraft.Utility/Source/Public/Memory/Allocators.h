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
concept CAllocatableObject = CObject<T> && !CConst<T> && !CVolatile<T> && CDestructible<T>;

template <typename A, typename T = int>
concept CAllocator = !CSameAs<A, FAllocatorInterface> && CAllocatableObject<T>
	&& requires (typename A::template TForElementType<T>& Allocator, T* InPtr, size_t Num, size_t NumAllocated)
	{
		{         Allocator.Allocate(Num)          } -> CSameAs<T*>;
		{         Allocator.Deallocate(InPtr)      } -> CSameAs<void>;
		{ AsConst(Allocator).IsTransferable(InPtr) } -> CBooleanTestable;
		{ AsConst(Allocator).CalculateSlackGrow(Num, NumAllocated)   } -> CSameAs<size_t>;
		{ AsConst(Allocator).CalculateSlackShrink(Num, NumAllocated) } -> CSameAs<size_t>;
		{ AsConst(Allocator).CalculateSlackReserve(Num)              } -> CSameAs<size_t>;
	};

template <typename A, typename T = int>
concept CMultipleAllocator = CAllocator<A, T> && A::bSupportsMultipleAllocation;

/**
 * This is the allocator interface, the allocator does not use virtual, this contains the default of
 * the allocator interface functions. Unlike std::allocator, IAllocator should be bound to only a object,
 * such as a container, because there may be side effects between multiple allocations, for example,
 * inline storage cannot be allocated multiple times in TInlineAllocator.
 */
struct FAllocatorInterface
{
	/**
	 * If this flag is false, it is possible to allocate an address that has already been allocated.
	 * Should be allocated according to the results given by the CalculateSlackReserve() family,
	 * without needing to allocate memory of the same size as the allocated memory,
	 * this is to support special allocators such as TInlineAllocator.
	 */
	static constexpr bool bSupportsMultipleAllocation = true;

	template <CAllocatableObject T>
	class TForElementType /*: private FSingleton*/
	{
	public:

		TForElementType()                                  = default;
		TForElementType(const TForElementType&)            = delete;
		TForElementType(TForElementType&&)                 = delete;
		TForElementType& operator=(const TForElementType&) = delete;
		TForElementType& operator=(TForElementType&&)      = delete;

		/** Allocates uninitialized storage. If 'InNum' is zero, return nullptr. */
		NODISCARD FORCEINLINE T* Allocate(size_t InNum) = delete;

		/** Deallocates storage. */
		FORCEINLINE void Deallocate(T* InPtr) = delete;

		/** @return true if allocation can be deallocated by another allocator, otherwise false. always return true when bSupportsMultipleAllocation is true. */
		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const { return true; }

		/** Calculates the amount of slack to allocate for an array that has just grown to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just shrunk to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const = delete;

		/** Calculates the amount of slack to allocate for an array that has just grown or shrunk to a given number of elements. */
		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const = delete;

	};
};

#define ALLOCATOR_WRAPPER_BEGIN(Allocator, Type, Name)     \
	                                                       \
	struct PREPROCESSOR_JOIN(F, Name) /*: private FSingleton*/

#define ALLOCATOR_WRAPPER_END(Allocator, Type, Name) ;                                             \
	                                                                                               \
	template <typename A, bool = CEmpty<A> && !CFinal<A>>                                          \
	struct PREPROCESSOR_JOIN(T, Name);                                                             \
	                                                                                               \
	template <typename A>                                                                          \
	struct PREPROCESSOR_JOIN(T, Name)<A, true> : public PREPROCESSOR_JOIN(F, Name), private A      \
	{                                                                                              \
		NODISCARD FORCEINLINE       A& operator*()        { return *this; }                        \
		NODISCARD FORCEINLINE const A& operator*()  const { return *this; }                        \
		NODISCARD FORCEINLINE       A* operator->()       { return  this; }                        \
		NODISCARD FORCEINLINE const A* operator->() const { return  this; }                        \
	};                                                                                             \
	                                                                                               \
	template <typename A>                                                                          \
	struct PREPROCESSOR_JOIN(T, Name)<A, false> : public PREPROCESSOR_JOIN(F, Name)                \
	{                                                                                              \
		NODISCARD FORCEINLINE       A& operator*()        { return  AllocatorInstance; }           \
		NODISCARD FORCEINLINE const A& operator*()  const { return  AllocatorInstance; }           \
		NODISCARD FORCEINLINE       A* operator->()       { return &AllocatorInstance; }           \
		NODISCARD FORCEINLINE const A* operator->() const { return &AllocatorInstance; }           \
		                                                                                           \
	private:                                                                                       \
		                                                                                           \
		A AllocatorInstance;                                                                       \
		                                                                                           \
	};                                                                                             \
	                                                                                               \
	PREPROCESSOR_JOIN(T, Name)<typename Allocator::template TForElementType<Type>> Name;

/** This is heap allocator that calls Memory::Malloc() directly for memory allocation. */
struct FHeapAllocator
{
	static constexpr bool bSupportsMultipleAllocation = true;

	template <CAllocatableObject T>
	class TForElementType /*: private FSingleton*/
	{
	public:

		TForElementType()                                  = default;
		TForElementType(const TForElementType&)            = delete;
		TForElementType(TForElementType&&)                 = delete;
		TForElementType& operator=(const TForElementType&) = delete;
		TForElementType& operator=(TForElementType&&)      = delete;

		NODISCARD FORCEINLINE T* Allocate(size_t InNum)
		{
			return InNum != 0 ? static_cast<T*>(Memory::Malloc(Memory::QuantizeSize(InNum * sizeof(T)), alignof(T))) : nullptr;
		}

		FORCEINLINE void Deallocate(T* InPtr)
		{
			Memory::Free(InPtr);
		}

		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const { return true; }

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
template <size_t NumInline, CAllocator SecondaryAllocator = FHeapAllocator>
struct TInlineAllocator
{
	static constexpr bool bSupportsMultipleAllocation = false;

	template <CAllocatableObject T>
	class TForElementType /*: private FSingleton*/
	{
	public:

		TForElementType()                                  = default;
		TForElementType(const TForElementType&)            = delete;
		TForElementType(TForElementType&&)                 = delete;
		TForElementType& operator=(const TForElementType&) = delete;
		TForElementType& operator=(TForElementType&&)      = delete;

		NODISCARD FORCEINLINE T* Allocate(size_t InNum)
		{
			if (InNum == 0) return nullptr;

			check(InNum >= NumInline);

			if (InNum == NumInline) return Impl.GetInline();

			return Impl->Allocate(InNum);
		}

		FORCEINLINE void Deallocate(T* InPtr)
		{
			if (InPtr == Impl.GetInline()) return;

			Impl->Deallocate(InPtr);
		}

		NODISCARD FORCEINLINE bool IsTransferable(T* InPtr) const
		{
			if (InPtr == Impl.GetInline()) return false;

			return Impl->IsTransferable(InPtr);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackGrow(size_t Num, size_t NumAllocated) const
		{
			check(Num > NumAllocated);
			check(NumAllocated >= NumInline);

			if (Num <= NumInline) return NumInline;

			return Impl->CalculateSlackGrow(Num, NumAllocated <= NumInline ? 0 : NumAllocated);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackShrink(size_t Num, size_t NumAllocated) const
		{
			check(Num < NumAllocated);
			check(NumAllocated >= NumInline);

			if (Num <= NumInline) return NumInline;

			return Impl->CalculateSlackShrink(Num, NumAllocated);
		}

		NODISCARD FORCEINLINE size_t CalculateSlackReserve(size_t Num) const
		{
			if (Num <= NumInline) return NumInline;

			return Impl->CalculateSlackReserve(Num);
		}

	private:

		ALLOCATOR_WRAPPER_BEGIN(SecondaryAllocator, T, Impl)
		{
			TAlignedStorage<sizeof(T), alignof(T)> InlineStorage[NumInline];

			NODISCARD FORCEINLINE       T* GetInline()       { return reinterpret_cast<      T*>(&InlineStorage); }
			NODISCARD FORCEINLINE const T* GetInline() const { return reinterpret_cast<const T*>(&InlineStorage); }
		}
		ALLOCATOR_WRAPPER_END(SecondaryAllocator, T, Impl)

	};
};

/** This is a null allocator for which all operations are illegal. */
struct FNullAllocator
{
	static constexpr bool bSupportsMultipleAllocation = true;

	template <CAllocatableObject T>
	class TForElementType /*: private FSingleton*/
	{
	public:

		TForElementType()                                  = default;
		TForElementType(const TForElementType&)            = delete;
		TForElementType(TForElementType&&)                 = delete;
		TForElementType& operator=(const TForElementType&) = delete;
		TForElementType& operator=(TForElementType&&)      = delete;

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
