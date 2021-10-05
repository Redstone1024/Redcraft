#pragma once

#pragma once

#include "CoreTypes.h"
#include "Templates/TypeTraits.h"

NS_REDCRAFT_BEGIN
NS_BEGIN(Memory)

template <typename T>
FORCEINLINE constexpr T Align(T Val, uint64 Alignment)
{
	static_assert(TypeTraits::TIsIntegral<T>::Value || TypeTraits::TIsPointer<T>::Value, "Align expects an integer or pointer type");

	return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignDown(T Val, uint64 Alignment)
{
	static_assert(TypeTraits::TIsIntegral<T>::Value || TypeTraits::TIsPointer<T>::Value, "AlignDown expects an integer or pointer type");

	return (T)(((uint64)Val) & ~(Alignment - 1));
}

template <typename T>
FORCEINLINE constexpr bool IsAligned(T Val, uint64 Alignment)
{
	static_assert(TypeTraits::TIsIntegral<T>::Value || TypeTraits::TIsPointer<T>::Value, "IsAligned expects an integer or pointer type");

	return !((uint64)Val & (Alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(TypeTraits::TIsIntegral<T>::Value || TypeTraits::TIsPointer<T>::Value, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

NS_END(Memory)
NS_REDCRAFT_END
