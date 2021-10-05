#include "Math/MathUtility.h"
#include "Templates/TypeTraits.h"

#include <cstring>
#include <cstdlib>

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

void* Memmove(void* Dest, const void* Src, size_t Count)
{
	return std::memmove(Dest, Src, Count);
}

int32 Memcmp(const void* Buf1, const void* Buf2, size_t Count)
{
	return std::memcmp(Buf1, Buf2, Count);
}

void Memset(void* Dest, uint8 ValueToSet, size_t Count)
{
	std::memset(Dest, ValueToSet, Count);
}

void* Memzero(void* Dest, size_t Count)
{
	return std::memset(Dest, 0, Count);
}

void* Memcpy(void* Dest, const void* Src, size_t Count)
{
	return std::memcpy(Dest, Src, Count);
}

template<typename T>
static FORCEINLINE void Memset(T& Src, uint8 ValueToSet)
{
	static_assert(!TypeTraits::TIsPointer<T>::Value, "For pointers use the three parameters function");
	Memset(&Src, ValueToSet, sizeof(T));
}

template<typename T>
static FORCEINLINE void Memzero(T& Src)
{
	static_assert(!TypeTraits::TIsPointer<T>::Value, "For pointers use the two parameters function");
	Memzero(&Src, sizeof(T));
}

template<typename T>
static FORCEINLINE void Memcpy(T& Dest, const T& Src)
{
	static_assert(!TypeTraits::TIsPointer<T>::Value, "For pointers use the three parameters function");
	Memcpy(&Dest, &Src, sizeof(T));
}

void* SystemMalloc(size_t Count)
{
	return std::malloc(Count);
}

void SystemFree(void* Ptr)
{
	std::free(Ptr);
}

NS_END(Memory)
NS_REDCRAFT_END
