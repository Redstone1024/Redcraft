#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

template <typename T> requires TIsIntegral<T>::Value || TIsPointer<T>::Value
FORCEINLINE constexpr T Align(T InValue, size_t Alignment)
{
	return (T)(((uint64)(InValue) + static_cast<uint64>(Alignment) - 1) & ~(static_cast<uint64>(Alignment) - 1));
}

template <typename T> requires TIsIntegral<T>::Value || TIsPointer<T>::Value
FORCEINLINE constexpr T AlignDown(T InValue, size_t Alignment)
{
	return (T)((uint64)(InValue) & ~(static_cast<uint64>(Alignment) - 1));
}

template <typename T> requires TIsIntegral<T>::Value || TIsPointer<T>::Value
FORCEINLINE constexpr T AlignArbitrary(T InValue, size_t Alignment)
{
	return (T)((((uint64)(InValue) + static_cast<uint64>(Alignment) - 1) / static_cast<uint64>(Alignment)) * static_cast<uint64>(Alignment));
}

template <typename T> requires TIsIntegral<T>::Value || TIsPointer<T>::Value
FORCEINLINE constexpr bool IsAligned(T InValue, size_t Alignment)
{
	return !((uint64)(InValue) & (static_cast<uint64>(Alignment) - 1));
}

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
