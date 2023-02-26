#pragma once

#include "CoreTypes.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/Swappable.h"
#include "Memory/ObserverPointer.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** @return The pointer to the container element storage. */
template <typename T> requires (requires(T&& Container) { { Container.GetData() } -> CTObserverPtr; })
FORCEINLINE constexpr decltype(auto) GetData(T&& Container)
{
	return Container.GetData();
}

/** Overloads the GetData algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr TObserverPtr<      T[]> GetData(      T(&  Container)[N]) { return TObserverPtr<      T[]>(Container); }
template <typename T, size_t N> FORCEINLINE constexpr TObserverPtr<      T[]> GetData(      T(&& Container)[N]) { return TObserverPtr<      T[]>(Container); }
template <typename T, size_t N> FORCEINLINE constexpr TObserverPtr<const T[]> GetData(const T(&  Container)[N]) { return TObserverPtr<const T[]>(Container); }
template <typename T, size_t N> FORCEINLINE constexpr TObserverPtr<const T[]> GetData(const T(&& Container)[N]) { return TObserverPtr<const T[]>(Container); }

/** Overloads the GetData algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr TObserverPtr<const T[]> GetData(initializer_list<T> Container)
{
	return TObserverPtr<const T[]>(Container.begin());
}

/** @return The number of elements in the container. */
template <typename T> requires (requires(T&& Container) { { Container.Num() } -> CConvertibleTo<size_t>; })
FORCEINLINE constexpr decltype(auto) GetNum(T&& Container)
{
	return Container.Num();
}

/** Overloads the GetNum algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr size_t GetNum(      T(&  Container)[N]) { return N; }
template <typename T, size_t N> FORCEINLINE constexpr size_t GetNum(      T(&& Container)[N]) { return N; }
template <typename T, size_t N> FORCEINLINE constexpr size_t GetNum(const T(&  Container)[N]) { return N; }
template <typename T, size_t N> FORCEINLINE constexpr size_t GetNum(const T(&& Container)[N]) { return N; }

/** Overloads the GetNum algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr size_t GetNum(initializer_list<T> Container)
{
	return Container.size();
}

/** Overloads the Swap algorithm for arrays. */
template <typename T, size_t N> requires (CSwappable<TRemoveAllExtents<T>>)
FORCEINLINE constexpr void Swap(T(&A)[N], T(&B)[N])
{
	for (size_t Index = 0; Index < N; ++Index)
	{
		Swap(A[Index], B[Index]);
	}
}

/** Overloads the GetTypeHash algorithm for arrays. */
template <typename T, size_t N> requires (CHashable<TRemoveAllExtents<T>>)
FORCEINLINE constexpr size_t GetTypeHash(T(&A)[N])
{
	size_t Result = 3516520171;

	for (size_t Index = 0; Index < N; ++Index)
	{
		Result = HashCombine(Result, GetTypeHash(A[Index]));
	}
	
	return Result;
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
