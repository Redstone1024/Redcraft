#pragma once

#include "CoreTypes.h"

NS_REDCRAFT_BEGIN
NS_BEGIN(Math)

template <typename T>
static constexpr FORCEINLINE T Abs(const T A)
{
	return (A >= (T)0) ? A : -A;
}

template <typename T>
static constexpr FORCEINLINE T Sign(const T A)
{
	return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
}

template <typename T>
static constexpr FORCEINLINE T Max(const T A, const T B)
{
	return (A >= B) ? A : B;
}

template <typename T>
static constexpr FORCEINLINE T Min(const T A, const T B)
{
	return (A <= B) ? A : B;
}

NS_END(Math)
NS_REDCRAFT_END
