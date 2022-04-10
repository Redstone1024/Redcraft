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

inline constexpr size_t DefaultAlignment = 0;
inline constexpr size_t MinimumAlignment = 8;

#ifdef __cpp_lib_hardware_interference_size

inline constexpr size_t DestructiveInterference = std::hardware_destructive_interference_size;
inline constexpr size_t ConstructiveInterference = std::hardware_constructive_interference_size;

#else

inline constexpr size_t DestructiveInterference = 64;
inline constexpr size_t ConstructiveInterference = 64;

#endif

FORCEINLINE void* Memmove(void* Destination, const void* Source, size_t Count)
{
	return std::memmove(Destination, Source, Count);
}

FORCEINLINE int32 Memcmp(const void* BufferLHS, const void* BufferRHS, size_t Count)
{
	return std::memcmp(BufferLHS, BufferRHS, Count);
}

FORCEINLINE void Memset(void* Destination, uint8 ValueToSet, size_t Count)
{
	std::memset(Destination, ValueToSet, Count);
}

FORCEINLINE void* Memzero(void* Destination, size_t Count)
{
	return std::memset(Destination, 0, Count);
}

FORCEINLINE void* Memcpy(void* Destination, const void* Source, size_t Count)
{
	return std::memcpy(Destination, Source, Count);
}

template <typename T>
FORCEINLINE void Memset(T& Source, uint8 ValueToSet)
{
	static_assert(!TIsPointer<T>::Value, "For pointers use the three parameters function");
	Memset(&Source, ValueToSet, sizeof(T));
}

template <typename T>
FORCEINLINE void Memzero(T& Source)
{
	static_assert(!TIsPointer<T>::Value, "For pointers use the two parameters function");
	Memzero(&Source, sizeof(T));
}

template <typename T>
FORCEINLINE void Memcpy(T& Destination, const T& Source)
{
	static_assert(!TIsPointer<T>::Value, "For pointers use the three parameters function");
	Memcpy(&Destination, &Source, sizeof(T));
}

FORCEINLINE void* SystemMalloc(size_t Count)
{
	return std::malloc(Count);
}

FORCEINLINE void* SystemRealloc(void* Ptr, size_t Count)
{
	return std::realloc(Ptr, Count);
}

FORCEINLINE void SystemFree(void* Ptr)
{
	std::free(Ptr);
}

REDCRAFTUTILITY_API void* Malloc(size_t Count, size_t Alignment = DefaultAlignment);
REDCRAFTUTILITY_API void* Realloc(void* Ptr, size_t Count, size_t Alignment = DefaultAlignment);
REDCRAFTUTILITY_API void Free(void* Ptr);
REDCRAFTUTILITY_API size_t QuantizeSize(size_t Count, size_t Alignment = DefaultAlignment);

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(disable : 28251)

// The global overload operators new/delete do not cross .dll boundaries, and the macros should be placed in the .cpp of each module.
#define REPLACEMENT_OPERATOR_NEW_AND_DELETE                                                                                                                                     \
	void* operator new(std::size_t Count)                             { return NAMESPACE_REDCRAFT::Memory::Malloc(Count);                                                     } \
	void* operator new(std::size_t Count, std::align_val_t Alignment) { return NAMESPACE_REDCRAFT::Memory::Malloc(Count, static_cast<NAMESPACE_REDCRAFT::size_t>(Alignment)); } \
	void operator delete(void* Ptr)                             noexcept { NAMESPACE_REDCRAFT::Memory::Free(Ptr); }                                                             \
	void operator delete(void* Ptr, std::align_val_t Alignment) noexcept { NAMESPACE_REDCRAFT::Memory::Free(Ptr); }
