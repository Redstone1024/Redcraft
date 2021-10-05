#pragma once

#include "CoreTypes.h"

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

constexpr uint32 DEFAULT_ALIGNMENT = 0;
constexpr uint32 MIN_ALIGNMENT     = 8;

FORCEINLINE void* Memmove(void* Dest, const void* Src, size_t Count);
FORCEINLINE int32 Memcmp(const void* Buf1, const void* Buf2, size_t Count);
FORCEINLINE void Memset(void* Dest, uint8 ValueToSet, size_t Count);
FORCEINLINE void* Memzero(void* Dest, size_t Count);
FORCEINLINE void* Memcpy(void* Dest, const void* Src, size_t Count);

template<typename T>
static FORCEINLINE void Memset(T& Src, uint8 ValueToSet);

template<typename T>
static FORCEINLINE void Memzero(T& Src);

template<typename T>
static FORCEINLINE void Memcpy(T& Dest, const T& Src);

FORCEINLINE void* SystemMalloc(size_t Count);
FORCEINLINE void SystemFree(void* Ptr);

REDCRAFTCORE_API void* Malloc(size_t Count, uint32 Alignment = DEFAULT_ALIGNMENT);
REDCRAFTCORE_API void* Realloc(void* Ptr, size_t Count, uint32 Alignment = DEFAULT_ALIGNMENT);
REDCRAFTCORE_API void Free(void* Ptr);
REDCRAFTCORE_API size_t QuantizeSize(size_t Count, uint32 Alignment = DEFAULT_ALIGNMENT);

NS_END(Memory)
NS_REDCRAFT_END

#include "HAL/Memory.inl"

void* operator new(std::size_t Count) { return NS_REDCRAFT::Memory::Malloc(Count); };
void* operator new(std::size_t Count, std::align_val_t Alignment) { return NS_REDCRAFT::Memory::Malloc(Count, (NS_REDCRAFT::uint32)Alignment); };

void operator delete(void* Ptr) noexcept { NS_REDCRAFT::Memory::Free(Ptr); }
void operator delete(void* Ptr, std::align_val_t Alignment) noexcept { NS_REDCRAFT::Memory::Free(Ptr); }
