#include "HAL/Memory.h"

#include "Templates/Alignment.h"

#ifdef PLATFORM_WINDOWS
#include <corecrt_malloc.h>
#endif

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

void* Malloc(size_t Count, uint32 Alignment)
{
	Alignment = Math::Max(Count >= 16 ? (uint32)16 : (uint32)8, Alignment);

	void* Result = nullptr;

#ifdef PLATFORM_WINDOWS
	if (Count != 0) Result = _aligned_malloc(Count, Alignment);
#else
	void* Ptr = SystemMalloc(Count + Alignment + sizeof(void*) + sizeof(size_t));
	if (Ptr)
	{
		Result = Align((uint8*)Ptr + sizeof(void*) + sizeof(size_t), Alignment);
		*((void**)((uint8*)Result - sizeof(void*))) = Ptr;
		*((size_t*)((uint8*)Result - sizeof(void*) - sizeof(size_t))) = Count;
	}
#endif

	return Result;
}

void* Realloc(void* Ptr, size_t Count, uint32 Alignment)
{
	Alignment = Math::Max(Count >= 16 ? (uint32)16 : (uint32)8, Alignment);

	if (Ptr && Count)
	{
#ifdef PLATFORM_WINDOWS
		return _aligned_realloc(Ptr, Count, Alignment);
#else
		void* Result = Malloc(Count, Alignment);
		size_t PtrSize = *((size_t*)((uint8*)Ptr - sizeof(void*) - sizeof(size_t)));
		Memcpy(Result, Ptr, Math::Min(Count, PtrSize));
		Free(Ptr);
		return Result;
#endif
	}
	else if (Ptr == nullptr)
	{
		return Malloc(Count, Alignment);
	}
	else
	{
		Free(Ptr);
		return nullptr;
	}
}

void Free(void* Ptr)
{
#if PLATFORM_WINDOWS
	_aligned_free(Ptr);
#else
	SystemFree(*((void**)((uint8*)Ptr - sizeof(void*))));
#endif
}

size_t QuantizeSize(size_t Count, uint32 Alignment)
{
	return Count;
}

NS_END(Memory)
NS_REDCRAFT_END
