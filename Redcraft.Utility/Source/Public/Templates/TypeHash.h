#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/Miscellaneous.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

FORCEINLINE constexpr size_t HashCombine()
{
	return 0;
}

FORCEINLINE constexpr size_t HashCombine(size_t A)
{
	return A;
}

FORCEINLINE constexpr size_t HashCombine(size_t A, size_t C)
{
	
	size_t B = static_cast<size_t>(0x9E3779B97F4A7C16);

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

template <typename... Ts> requires (true && ... && CConvertibleTo<Ts, size_t>)
FORCEINLINE constexpr size_t HashCombine(size_t A, size_t C, Ts... InOther)
{
	size_t B = HashCombine(A, C);
	return HashCombine(B, InOther...);
}

template <CIntegral T>
FORCEINLINE constexpr size_t GetTypeHash(T A)
{
	static_assert(sizeof(T) <= 16, "GetTypeHash only works with T up to 128 bits.");

	if constexpr (sizeof(T) <= sizeof(size_t)) return static_cast<size_t>(A);
	if constexpr (sizeof(T) ==  8) return GetTypeHash(static_cast<uint32>(A)) ^ GetTypeHash(static_cast<uint32>(A >> 32));
	if constexpr (sizeof(T) == 16) return GetTypeHash(static_cast<uint64>(A)) ^ GetTypeHash(static_cast<uint64>(A >> 64));
	else check_no_entry();
	
	return INDEX_NONE;
}

template <CFloatingPoint T>
FORCEINLINE constexpr size_t GetTypeHash(T A)
{
	static_assert(sizeof(T) <= 16, "GetTypeHash only works with T up to 128 bits.");

	if constexpr (sizeof(T) ==  1) return GetTypeHash(*reinterpret_cast<uint8 *>(&A));
	if constexpr (sizeof(T) ==  2) return GetTypeHash(*reinterpret_cast<uint16*>(&A));
	if constexpr (sizeof(T) ==  4) return GetTypeHash(*reinterpret_cast<uint32*>(&A));
	if constexpr (sizeof(T) ==  8) return GetTypeHash(*reinterpret_cast<uint64*>(&A));
	if constexpr (sizeof(T) == 16) return GetTypeHash(*reinterpret_cast<uint64*>(&A) + *(reinterpret_cast<uint64*>(&A) + 1));
	else check_no_entry();
	
	return INDEX_NONE;
}

template <CEnum T>
FORCEINLINE constexpr size_t GetTypeHash(T A)
{
	return GetTypeHash(static_cast<TUnderlyingType<T>>(A));
}

template <typename T> requires (CPointer<T> || CSameAs<T, nullptr_t>)
FORCEINLINE constexpr size_t GetTypeHash(T A)
{
	return GetTypeHash(reinterpret_cast<intptr>(A));
}

template <typename T> requires (requires(const T& A) { { GetTypeHash(A.hash_code()) } -> CSameAs<size_t>; })
FORCEINLINE constexpr size_t GetTypeHash(const T& A)
{
	return GetTypeHash(A.hash_code());
}

template <typename T>
concept CHashable = requires(const T& A) { { GetTypeHash(A) } -> CSameAs<size_t>; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
