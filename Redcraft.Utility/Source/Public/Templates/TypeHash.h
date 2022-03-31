#pragma once

#include "CoreTypes.h"
#include "Concepts/Same.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/Miscellaneous.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

constexpr size_t HashCombine(size_t A, size_t C)
{
	size_t B = 0x9E3779B97F4A7C16;
	A += B;

	A -= B; A -= C; A ^= (C >> 13);
	B -= C; B -= A; B ^= (A <<  8);
	C -= A; C -= B; C ^= (B >> 13);
	A -= B; A -= C; A ^= (C >> 12);
	B -= C; B -= A; B ^= (A << 16);
	C -= A; C -= B; C ^= (B >>  5);
	A -= B; A -= C; A ^= (C >>  3);
	B -= C; B -= A; B ^= (A << 10);
	C -= A; C -= B; C ^= (B >> 15);

	return C;
}

template <typename T> requires TIsIntegral<T>::Value
constexpr size_t GetTypeHash(T A)
{
	static_assert(sizeof(T) <= 16, "GetTypeHash only works with T up to 128 bits.");

	if constexpr (sizeof(T) <=  8) return static_cast<size_t>(A);
	if constexpr (sizeof(T) == 16) return static_cast<size_t>(A ^ (A >> 64));
	else return INDEX_NONE;
}

template <typename T> requires TIsFloatingPoint<T>::Value
constexpr size_t GetTypeHash(T A)
{
	static_assert(sizeof(T) <= 16, "GetTypeHash only works with T up to 128 bits.");

	if constexpr (sizeof(T) ==  1) return GetTypeHash(*reinterpret_cast<uint8 *>(&A));
	if constexpr (sizeof(T) ==  2) return GetTypeHash(*reinterpret_cast<uint16*>(&A));
	if constexpr (sizeof(T) ==  4) return GetTypeHash(*reinterpret_cast<uint32*>(&A));
	if constexpr (sizeof(T) ==  8) return GetTypeHash(*reinterpret_cast<uint64*>(&A));
	if constexpr (sizeof(T) == 16) return GetTypeHash(*reinterpret_cast<uint64*>(&A) + *(reinterpret_cast<uint64*>(&A) + 1));
	else return INDEX_NONE;
}

template <typename T> requires TIsEnum<T>::Value
constexpr size_t GetTypeHash(T A)
{
	return GetTypeHash(static_cast<typename TUnderlyingType<T>::Type>(A));
}

constexpr size_t GetTypeHash(nullptr_t)
{
	return GetTypeHash(2.7182818284590452353602874713527);
}

constexpr size_t GetTypeHash(const void* A)
{
	return GetTypeHash(reinterpret_cast<intptr_t>(A));
}

template <typename T> requires requires(T&& A) { { GetTypeHash(A.GetTypeHash()) } -> CSameAs<size_t>; }
constexpr size_t GetTypeHash(T&& A)
{
	return GetTypeHash(A.GetTypeHash());
}

template <typename T>
concept CHashable = requires(T&& A) { { GetTypeHash(A) } -> CSameAs<size_t>; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
