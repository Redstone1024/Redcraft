#pragma once

#include "CoreTypes.h"
#include "TypeTraits/Swappable.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** @return The pointer to the container element storage. */
template <typename T> requires (requires(T&& Container) { { Container.GetData() } -> CPointer; })
FORCEINLINE constexpr decltype(auto) GetData(T&& Container)
{
	return Container.GetData();
}

/** Overloads the GetData algorithm for arrays. */
template <typename T, size_t N> FORCEINLINE constexpr       T* GetData(      T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr       T* GetData(      T(&& Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* GetData(const T(&  Container)[N]) { return Container; }
template <typename T, size_t N> FORCEINLINE constexpr const T* GetData(const T(&& Container)[N]) { return Container; }

/** Overloads the GetData algorithm for T::data(). */
template <typename T> requires (requires(T&& Container) { { Container.data() } -> CPointer; })
FORCEINLINE constexpr decltype(auto) GetData(T&& Container)
{
	return Container.data();
}

/** Overloads the GetData algorithm for initializer_list. */
template <typename T>
FORCEINLINE constexpr const T* GetData(initializer_list<T> Container)
{
	return Container.begin();
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

/** Overloads the GetNum algorithm for T::size(). */
template <typename T> requires (requires(T&& Container) { { Container.size() } -> CConvertibleTo<size_t>; })
FORCEINLINE constexpr decltype(auto) GetNum(T&& Container)
{
	return Container.size();
}

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

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
