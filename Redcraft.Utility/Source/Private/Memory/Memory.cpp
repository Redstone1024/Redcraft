#include "Memory/Memory.h"

#include "Memory/Alignment.h"
#include "Templates/Atomic.h"
#include "Templates/ScopeHelper.h"
#include "Miscellaneous/AssertionMacros.h"

#if PLATFORM_WINDOWS
#include <corecrt_malloc.h>
#endif

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

#if DO_CHECK

class FMemoryLeakChecker
{
private:

	TAtomic<size_t> MemoryAllocationCount;

public:

	FORCEINLINE constexpr FMemoryLeakChecker()
		: MemoryAllocationCount(0)
	{ }

	FORCEINLINE ~FMemoryLeakChecker()
	{
		checkf(MemoryAllocationCount.Load() == 0, TEXT("There is unfree memory. Please check for memory leaks."));
	}

	FORCEINLINE void AddMemoryAllocationCount()
	{
		MemoryAllocationCount.FetchAdd(1, EMemoryOrder::Relaxed);
	}

	FORCEINLINE void ReleaseMemoryAllocationCount()
	{
		MemoryAllocationCount.FetchSub(1, EMemoryOrder::Relaxed);
	}

};

FMemoryLeakChecker MemoryLeakChecker;

#endif

void* Malloc(size_t Count, size_t Alignment)
{
	checkf(IsValidAlignment(Alignment), TEXT("The alignment value must be an integer power of 2."));

	Count = Count != 0 ? Count : 1; // Treat zero-byte allocation as one-byte allocation.

	const size_t MinimumAlignment = Count >= 16 ? 16 : 8;
	Alignment = MinimumAlignment > Alignment ? MinimumAlignment : Alignment;

	void* Result = nullptr;

#	if PLATFORM_WINDOWS
	{
		Result = _aligned_malloc(Count, Alignment);
	}
#	else
	{
		void* Ptr = SystemMalloc(Count + Alignment + sizeof(void*) + sizeof(size_t));

		if (Ptr != nullptr)
		{
			Result = Align(reinterpret_cast<uint8*>(Ptr) + sizeof(void*) + sizeof(size_t), Alignment);
			*reinterpret_cast<void**>(reinterpret_cast<uint8*>(Result) - sizeof(void*)) = Ptr;
			*reinterpret_cast<size_t*>(reinterpret_cast<uint8*>(Result) - sizeof(void*) - sizeof(size_t)) = Count;
		}
	}
#	endif

	check(Result != nullptr);

	check_code({ MemoryLeakChecker.AddMemoryAllocationCount(); });

	return Result;
}

void* Realloc(void* Ptr, size_t Count, size_t Alignment)
{
	checkf(IsValidAlignment(Alignment), TEXT("The alignment value must be an integer power of 2."));

	Count = Count != 0 ? Count : 1; // Treat zero-byte allocation as one-byte allocation.

	const size_t MinimumAlignment = Count >= 16 ? 16 : 8;
	Alignment = MinimumAlignment > Alignment ? MinimumAlignment : Alignment;

	void* Result = nullptr;

	if (Ptr != nullptr)
	{
#		if PLATFORM_WINDOWS
		{
			Result = _aligned_realloc(Ptr, Count, Alignment);
		}
#		else
		{
			Result = Malloc(Count, Alignment);

			if (Result != nullptr)
			{
				size_t PtrSize = *reinterpret_cast<size_t*>(reinterpret_cast<uint8*>(Ptr) - sizeof(void*) - sizeof(size_t));
				Memcpy(Result, Ptr, Count < PtrSize ? Count : PtrSize);
				Free(Ptr);
			}
		}
#		endif
	}
	else
	{
		Result = Malloc(Count, Alignment);
	}

	check(Result != nullptr);

	return Result;
}

void Free(void* Ptr)
{
#	if PLATFORM_WINDOWS
	{
		_aligned_free(Ptr);
	}
#	else
	{
		SystemFree(*reinterpret_cast<void**>(reinterpret_cast<uint8*>(Ptr) - sizeof(void*)));
	}
#	endif

	check_code({ if (Ptr != nullptr) MemoryLeakChecker.ReleaseMemoryAllocationCount(); });
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
