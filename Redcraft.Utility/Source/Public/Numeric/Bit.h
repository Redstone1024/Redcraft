#pragma once

#include "CoreTypes.h"
#include "Numeric/Limits.h"
#include "TypeTraits/TypeTraits.h"

#include <bit>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Math)

/** @return The reinterpreted value of the given value. */
template <CTriviallyCopyable T, CTriviallyCopyable U> requires (sizeof(T) == sizeof(U))
FORCEINLINE constexpr T BitCast(const U& Value)
{
	return __builtin_bit_cast(T, Value);
}

/** @return The value of reversed byte order of the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T ByteSwap(T Value)
{
	static_assert(sizeof(T) <= 16, "ByteSwap only works with T up to 128 bits");

	if constexpr (sizeof(T) == 1) return Value;

#	if PLATFORM_COMPILER_MSVC
	{
		if constexpr (sizeof(T) == 2) return _byteswap_ushort(Value);
		if constexpr (sizeof(T) == 4) return _byteswap_ulong(Value);
		if constexpr (sizeof(T) == 8) return _byteswap_uint64(Value);
	}
#	elif PLATFORM_COMPILER_CLANG || PLATFORM_COMPILER_GCC
	{
		if constexpr (sizeof(T) == 2) return __builtin_bswap16(Value);
		if constexpr (sizeof(T) == 4) return __builtin_bswap32(Value);
		if constexpr (sizeof(T) == 8) return __builtin_bswap64(Value);
	}
#	else
	{
		if constexpr (sizeof(T) == 2) return (Value << 8) | (Value >> 8); // AB -> BA

		if constexpr (sizeof(T) == 4)
		{
			T Result = 0;

			Result = ((Result << 8) & 0xFF00FF00u32) | ((Result >> 8) & 0x00FF00FFu32); // ABCD -> BADC

			return (Result << 16) | (Result >> 16);
		}

		if constexpr (sizeof(T) == 8)
		{
			T Result = Value;

			Result = ((Result <<  8) & 0xFF00FF00FF00FF00u64) | ((Result >>  8) & 0x00FF00FF00FF00FFu64); // ABCDEFGH -> BADCFEHG
			Result = ((Result << 16) & 0xFFFF0000FFFF0000u64) | ((Result >> 16) & 0x0000FFFF0000FFFFu64); // BADCFEHG -> DCBAHGFE

			return (Result << 32) | (Result >> 32);
		}
	}
#	endif

#	if PLATFORM_HAS_INT128
	{
		if constexpr (sizeof(T) == 16)
		{
			T Result = Value;

			Result = ((Result <<  8) & 0xFF00FF00FF00FF00FF00FF00FF00FF00u128) | ((Result >> 8)  & 0x00FF00FF00FF00FF00FF00FF00FF00FFu128); // ABCDEFGHIJKLMNOP -> BADCFEHGJILKMONP
			Result = ((Result << 16) & 0xFFFF0000FFFF0000FFFF0000FFFF0000u128) | ((Result >> 16) & 0x0000FFFF0000FFFF0000FFFF0000FFFFu128); // BADCFEHGJILKMONP -> DCBAHGFEJIKLNOPM
			Result = ((Result << 32) & 0xFFFFFFFF00000000FFFFFFFF00000000u128) | ((Result >> 32) & 0x00000000FFFFFFFF00000000FFFFFFFFu128); // DCBAHGFEJIKLNOPM -> HGFEDCBAKJILMOPN

			return (Result << 64) | (Result >> 64);
		}
	}
#	endif

	return 0;
}

/** @return true if the given value is power of two, false otherwise. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr bool IsSingleBit(T Value)
{
	if constexpr (CSameAs<T, bool>) return Value;

	else return Value && !(Value & (Value - 1));
}

/** @return The number of all zeros in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountAllZero(T Value)
{
	if constexpr (CSameAs<T, bool>) return Value ? 0 : 1;

	else return static_cast<uint>(TNumericLimits<T>::Digits - NAMESPACE_STD::popcount(Value));
}

/** @return The number of all ones in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountAllOne(T Value)
{
	if constexpr (CSameAs<T, bool>) return Value ? 1 : 0;

	else return static_cast<uint>(NAMESPACE_STD::popcount(Value));
}

/** @return The number of leading zeros in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountLeftZero(T Value)
{
	if constexpr (CSameAs<T, bool>) return Value ? 0 : 1;

	else return static_cast<uint>(NAMESPACE_STD::countl_zero(Value));
}

/** @return The number of leading ones in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountLeftOne(T Value)
{
	return Math::CountLeftZero<T>(~Value);
}

/** @return The number of trailing zeros in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountRightZero(T Value)
{
	if constexpr (CSameAs<T, bool>) return Value ? 0 : 1;

	else return static_cast<uint>(NAMESPACE_STD::countr_zero(Value));
}

/** @return The number of trailing ones in the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr uint CountRightOne(T Value)
{
	return Math::CountRightZero<T>(~Value);
}

/** @return The smallest number of bits that can represent the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T BitWidth(T Value)
{
	return TNumericLimits<T>::Digits - Math::CountLeftZero(Value);
}

/** @return The smallest integral power of two not less than the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T BitCeil(T Value)
{
	if (Value <= 1u) return static_cast<T>(1);

	return static_cast<T>(1) << Math::BitWidth(static_cast<T>(Value - 1));
}

/** @return The largest integral power of two not greater than the given value. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T BitFloor(T Value)
{
	if (Value == 0u) return static_cast<T>(0);

	return static_cast<T>(1) << (Math::BitWidth(static_cast<T>(Value)) - 1);
}

template <CUnsignedIntegral T> FORCEINLINE constexpr T RotateLeft (T Value, int Offset);
template <CUnsignedIntegral T> FORCEINLINE constexpr T RotateRight(T Value, int Offset);

/** @return The value bitwise left-rotation by the given offset. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T RotateLeft(T Value, int Offset)
{
	if constexpr (CSameAs<T, bool>) return Value;

	else
	{
		if (Offset >= 0)
		{
			const auto Remainder = Offset % TNumericLimits<T>::Digits;

			return static_cast<T>((Value << Remainder) | (Value >> (TNumericLimits<T>::Digits - Remainder)));
		}

		return Math::RotateRight(Value, -Offset);
	}
}

/** @return The value bitwise right-rotation by the given offset. */
template <CUnsignedIntegral T>
FORCEINLINE constexpr T RotateRight(T Value, int Offset)
{
	if constexpr (CSameAs<T, bool>) return Value;

	else
	{
		if (Offset >= 0)
		{
			const auto Remainder = Offset % TNumericLimits<T>::Digits;

			return static_cast<T>((Value >> Remainder) | (Value << (TNumericLimits<T>::Digits - Remainder)));
		}
		return Math::RotateLeft(Value, -Offset);
	}
}

/** The enum indicates the endianness of scalar types. */
enum class EEndian
{
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)

	Little = __ORDER_LITTLE_ENDIAN__,
	Big    = __ORDER_BIG_ENDIAN__,
	Native = __BYTE_ORDER__,

#elif PLATFORM_LITTLE_ENDIAN

	Little,
	Big,
	Native = Little,

#elif PLATFORM_BIG_ENDIAN

	Little,
	Big,
	Native = Big,

#else

	Little,
	Big,
	Native,

#endif
};

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
