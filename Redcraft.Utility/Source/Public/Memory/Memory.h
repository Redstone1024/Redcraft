#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

#include <new>
#include <cstring>
#include <cstdlib>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

/**
 * Default allocator alignment.
 * Blocks >= 16 bytes will be 16-byte-aligned, Blocks < 16 will be 8-byte aligned. If the allocator does
 * not support allocation alignment, the alignment will be ignored.
 */ 
inline constexpr size_t DefaultAlignment = 0;

/**
 * Minimum allocator alignment.
 */
inline constexpr size_t MinimumAlignment = 8;

#ifdef __cpp_lib_hardware_interference_size

/**
 * Minimum offset between two objects to avoid false sharing.
 *
 *	struct FTwoCacheLiner { // occupies two cache lines
 *		alignas(DestructiveInterference) TAtomic<uint64> X;
 *		alignas(DestructiveInterference) TAtomic<uint64> Y;
 *	};
 *
 */
inline constexpr size_t DestructiveInterference = NAMESPACE_STD::hardware_destructive_interference_size;

/**
 * Maximum size of contiguous memory to promote true sharing.
 *
 *	struct alignas(ConstructiveInterference) FOneCacheLiner { // occupies one cache line
 *		TAtomic<uint64> X;
 *		TAtomic<uint64> Y;
 *	};
 *
 */
inline constexpr size_t ConstructiveInterference = NAMESPACE_STD::hardware_constructive_interference_size;

#else

/**
 * Minimum offset between two objects to avoid false sharing.
 * 
 *	struct FTwoCacheLiner { // occupies two cache lines
 *		alignas(DestructiveInterference) TAtomic<uint64> X;
 *		alignas(DestructiveInterference) TAtomic<uint64> Y;
 *	};
 * 
 */
inline constexpr size_t DestructiveInterference  = 64;

/**
 * Maximum size of contiguous memory to promote true sharing.
 *
 *	struct alignas(ConstructiveInterference) FOneCacheLiner { // occupies one cache line
 *		TAtomic<uint64> X;
 *		TAtomic<uint64> Y;
 *	};
 *
 */
inline constexpr size_t ConstructiveInterference = 64;

#endif

/**
 * Copies 'Count' bytes from the buffer pointed to by 'Source' to the buffer pointed to by 'Destination'.
 * The buffers may overlap, copying takes place as if the characters were copied to a temporary character
 * array and then the characters were copied from the array to 'Destination'.
 *
 * @param  Destination - The pointer to the memory location to copy to.
 * @param  Source      - The pointer to the memory location to copy from.
 * @param  Count       - The number of bytes to copy.
 * 
 * @return Destination
 */
FORCEINLINE void* Memmove(void* Destination, const void* Source, size_t Count)
{
	return NAMESPACE_STD::memmove(Destination, Source, Count);
}

/**
 * Reinterprets the buffers pointed to by 'BufferLHS' and 'BufferRHS' as arrays of unsigned char and
 * compares the first 'Count' characters of these arrays. The comparison is done lexicographically.
 * The sign of the result is the sign of the difference between the values of the first pair of bytes
 * (both interpreted as unsigned char) that differ in the objects being compared.
 *
 * @param  BufferLHS - The pointers to the memory buffers to compare.
 * @param  BufferRHS - The pointers to the memory buffers to compare.
 * @param  Count     - The number of bytes to examine.
 * 
 * @return Negative value if the first differing byte in 'BufferLHS' is less than the corresponding byte in 'BufferRHS'.
 *         0 if all 'Count' bytes of 'BufferLHS' and 'BufferRHS' are equal.
 *         Positive value if the first differing byte in 'BufferLHS' is greater than the corresponding byte in 'BufferRHS'.
 */
FORCEINLINE int32 Memcmp(const void* BufferLHS, const void* BufferRHS, size_t Count)
{
	return NAMESPACE_STD::memcmp(BufferLHS, BufferRHS, Count);
}

/**
 * Copies 'ValueToSet' into each of the first 'Count' characters of the buffer pointed to by 'Destination'.
 *
 * @param  Destination - The pointer to the buffer to fill.
 * @param  ValueToSet  - The fill byte.
 * @param  Count       - The number of bytes to fill.
 *
 * @return Destination
 */
FORCEINLINE void* Memset(void* Destination, uint8 ValueToSet, size_t Count)
{
	return NAMESPACE_STD::memset(Destination, ValueToSet, Count);
}

/**
 * Copies 0 into each of the first 'Count' characters of the buffer pointed to by 'Destination'.
 *
 * @param  Destination - The pointer to the buffer to fill.
 * @param  Count       - The number of bytes to fill.
 *
 * @return Destination
 */
FORCEINLINE void* Memzero(void* Destination, size_t Count)
{
	return NAMESPACE_STD::memset(Destination, 0, Count);
}

/**
 * Copies 'Count' bytes from the buffer pointed to by 'Source' to the buffer pointed to by 'Destination'.
 * If the buffers overlap, the behavior is undefined.
 *
 * @param  Destination - The pointer to the memory location to copy to.
 * @param  Source      - The pointer to the memory location to copy from.
 * @param  Count       - The number of bytes to copy.
 *
 * @return Destination
 */
FORCEINLINE void* Memcpy(void* Destination, const void* Source, size_t Count)
{
	return NAMESPACE_STD::memcpy(Destination, Source, Count);
}

/**
 * Copies the object referenced to by 'Source' to the object referenced to by 'Destination'.
 * The objects may overlap, copying takes place as if the characters were copied to a temporary character
 * array and then the characters were copied from the array to 'Destination'.
 *
 * @param  Destination - The reference to the object to copy to.
 * @param  Source      - The reference to the object to copy from.
 */
template <typename T> requires (!CPointer<T>)
FORCEINLINE void Memmove(T& Destination, const T& Source)
{
	Memmove(&Destination, &Source, sizeof(T));
}

/**
 * Reinterprets the objects referenced to by 'BufferLHS' and 'BufferRHS' as arrays of unsigned char and
 * compares the all characters of these arrays. The comparison is done lexicographically.
 * The sign of the result is the sign of the difference between the values of the first pair of bytes
 * (both interpreted as unsigned char) that differ in the objects being compared.
 *
 * @param  BufferLHS - The reference to the object to compare.
 * @param  BufferRHS - The reference to the object to compare.
 *
 * @return Negative value if the first differing byte in 'BufferLHS' is less than the corresponding byte in 'BufferRHS'.
 *         0 if all bytes of 'BufferLHS' and 'BufferRHS' are equal.
 *         Positive value if the first differing byte in 'BufferLHS' is greater than the corresponding byte in 'BufferRHS'.
 */
template <typename T> requires (!CPointer<T>)
FORCEINLINE int32 Memcmp(const T& BufferLHS, const T& BufferRHS)
{
	return Memcmp(&BufferLHS, &BufferRHS, sizeof(T));
}

/**
 * Copies 'ValueToSet' into each of the all characters of the object referenced to by 'Destination'.
 *
 * @param  Destination - The reference to the object to fill.
 * @param  ValueToSet  - The fill byte.
 */
template <typename T> requires (!CPointer<T>)
FORCEINLINE void Memset(T& Destination, uint8 ValueToSet)
{
	Memset(&Destination, ValueToSet, sizeof(T));
}

/**
 * Copies 0 into each of the all characters of the object referenced to by 'Destination'.
 *
 * @param  Destination - The reference to the object to fill.
 * @param  ValueToSet  - The fill byte.
 */
template <typename T> requires (!CPointer<T>)
FORCEINLINE void Memzero(T& Destination)
{
	Memzero(&Destination, sizeof(T));
}

/**
 * Copies the object referenced to by 'Source' to the object referenced to by 'Destination'.
 * If the objects overlap, the behavior is undefined.
 *
 * @param  Destination - The reference to the object to copy to.
 * @param  Source      - The reference to the object to copy from.
 */
template <typename T> requires (!CPointer<T>)
FORCEINLINE void Memcpy(T& Destination, const T& Source)
{
	Memcpy(&Destination, &Source, sizeof(T));
}

/** Fallback to std::malloc(). */
NODISCARD FORCEINLINE void* SystemMalloc(size_t Count)
{
	return NAMESPACE_STD::malloc(Count);
}

/** Fallback to std::realloc(). */
NODISCARD FORCEINLINE void* SystemRealloc(void* Ptr, size_t Count)
{
	return NAMESPACE_STD::realloc(Ptr, Count);
}

/** Fallback to std::free(). */
FORCEINLINE void SystemFree(void* Ptr)
{
	NAMESPACE_STD::free(Ptr);
}

/**
 * Allocates 'Count' bytes of uninitialized storage with 'Alignment'.
 * 
 * @param  Count     - The number of bytes to allocate.
 * @param  Alignment - The alignment value, must be a power of 2.
 * 
 * @return The non-null pointer to the beginning of newly allocated memory. To avoid a memory leak,
 *         the returned pointer must be deallocated with Free() or Realloc().
 * 
 * @see DefaultAlignment
 */
NODISCARD REDCRAFTUTILITY_API void* Malloc(size_t Count, size_t Alignment = DefaultAlignment);

/**
 * Reallocates the given area of memory. It must be previously allocated by Malloc() or Realloc().
 * If 'Ptr' is a nullptr, effectively the same as calling Malloc().
 *
 * @param  Ptr       - The pointer to the memory area to be reallocated.
 * @param  Count     - The number of bytes to allocate.
 * @param  Alignment - The alignment value, must be a power of 2.
 *
 * @return The non-null pointer to the beginning of newly allocated memory. To avoid a memory leak,
 *         the returned pointer must be deallocated with Free() or Realloc().
 *
 * @see DefaultAlignment
 */
NODISCARD REDCRAFTUTILITY_API void* Realloc(void* Ptr, size_t Count, size_t Alignment = DefaultAlignment);

/**
 * Deallocates the space previously allocated by Malloc() or Realloc().
 * If 'Ptr' is a nullptr, the function does nothing.
 * 
 * @param  Ptr - The pointer to the memory to deallocate.
 */
REDCRAFTUTILITY_API void Free(void* Ptr);

/**
 * For some allocators this will return the actual size that should be requested to eliminate
 * internal fragmentation. The return value will always be >= 'Count'. This can be used to grow
 * and shrink containers to optimal sizes.
 * This call is always fast and threadsafe with no locking.
 *
 * @param  Count     - The number of bytes to allocate.
 * @param  Alignment - The alignment value, must be a power of 2.
 *
 * @return The optimized new size.
 */
NODISCARD REDCRAFTUTILITY_API size_t QuantizeSize(size_t Count, size_t Alignment = DefaultAlignment);

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(disable : 28251)

// The global overload operators new/delete do not cross .dll boundaries, and the macros should be placed in the .cpp of each module.
#define REPLACEMENT_OPERATOR_NEW_AND_DELETE                                                                                                                                                                   \
	NODISCARD void* operator new(NAMESPACE_STD::size_t Count)                                       { return NAMESPACE_REDCRAFT::Memory::Malloc(Count, __STDCPP_DEFAULT_NEW_ALIGNMENT__);                   } \
	NODISCARD void* operator new(NAMESPACE_STD::size_t Count, NAMESPACE_STD::align_val_t Alignment) { return NAMESPACE_REDCRAFT::Memory::Malloc(Count, static_cast<NAMESPACE_REDCRAFT::size_t>(Alignment)); } \
	          void operator delete(void* Ptr)                                       noexcept        { NAMESPACE_REDCRAFT::Memory::Free(Ptr); }                                                                \
	          void operator delete(void* Ptr, NAMESPACE_STD::align_val_t Alignment) noexcept        { NAMESPACE_REDCRAFT::Memory::Free(Ptr); }
