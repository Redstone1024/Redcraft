#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Memory)

template <typename T>
FORCEINLINE constexpr T Align(T InValue, size_t Alignment)
{
	static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "Align expects an integer or pointer type");

	return (T)(((uintptr_t)(InValue) + static_cast<uintptr_t>(Alignment) - 1) & ~(static_cast<uintptr_t>(Alignment) - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignDown(T InValue, size_t Alignment)
{
	static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "AlignDown expects an integer or pointer type");

	return (T)((uintptr_t)(InValue) & ~(static_cast<uintptr_t>(Alignment) - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignArbitrary(T InValue, size_t Alignment)
{
	static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uintptr_t)(InValue) + static_cast<uintptr_t>(Alignment) - 1) / static_cast<uintptr_t>(Alignment)) * static_cast<uintptr_t>(Alignment));
}

template <typename T>
FORCEINLINE constexpr bool IsAligned(T InValue, size_t Alignment)
{
	static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "IsAligned expects an integer or pointer type");

	return !((uintptr_t)(InValue) & (static_cast<uintptr_t>(Alignment) - 1));
}

NAMESPACE_END(Memory)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
