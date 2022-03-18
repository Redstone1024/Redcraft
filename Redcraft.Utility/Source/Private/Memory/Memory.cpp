#include "Memory/Memory.h"

#include "Memory/Alignment.h"

#if PLATFORM_WINDOWS
#include <corecrt_malloc.h>
#endif

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

void* Malloc(size_t Count, size_t Alignment)
{
	const size_t MinimumAlignment = Count >= 16 ? 16 : 8;
	Alignment = MinimumAlignment > Alignment ? MinimumAlignment : Alignment;

	void* Result = nullptr;

#if PLATFORM_WINDOWS
	if (Count != 0) Result = _aligned_malloc(Count, Alignment);
#else
	void* Ptr = SystemMalloc(Count + Alignment + sizeof(void*) + sizeof(size_t));
	if (Ptr)
	{
		Result = Align(reinterpret_cast<uint8*>(Ptr) + sizeof(void*) + sizeof(size_t), Alignment);
		*reinterpret_cast<void**>(reinterpret_cast<uint8*>(Result) - sizeof(void*)) = Ptr;
		*reinterpret_cast<size_t*>(reinterpret_cast<uint8*>(Result) - sizeof(void*) - sizeof(size_t)) = Count;
	}
#endif

	return Result;
}

void* Realloc(void* Ptr, size_t Count, size_t Alignment)
{
	const size_t MinimumAlignment = Count >= 16 ? 16 : 8;
	Alignment = MinimumAlignment > Alignment ? MinimumAlignment : Alignment;

	if (Ptr && Count)
	{
#if PLATFORM_WINDOWS
		return _aligned_realloc(Ptr, Count, Alignment);
#else
		void* Result = Malloc(Count, Alignment);
		size_t PtrSize = *reinterpret_cast<size_t*>(reinterpret_cast<uint8*>(Ptr) - sizeof(void*) - sizeof(size_t));
		Memcpy(Result, Ptr, Count < PtrSize ? Count : PtrSize);
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
	SystemFree(*reinterpret_cast<void**>(reinterpret_cast<uint8*>(Ptr) - sizeof(void*)));
#endif
}

size_t QuantizeSize(size_t Count, size_t Alignment)
{
	return Count;
}

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

REPLACEMENT_OPERATOR_NEW_AND_DELETE
