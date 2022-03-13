#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> requires requires(T Container) { Container.GetData(); }
constexpr auto GetData(T&& Container)
{
	return Container.GetData();
}

template <typename T, size_t N> constexpr       T* GetData(      T(&  Container)[N]) { return Container; }
template <typename T, size_t N> constexpr       T* GetData(      T(&& Container)[N]) { return Container; }
template <typename T, size_t N> constexpr const T* GetData(const T(&  Container)[N]) { return Container; }
template <typename T, size_t N> constexpr const T* GetData(const T(&& Container)[N]) { return Container; }

template <typename T> requires requires(T Container) { Container.data(); }
constexpr auto GetData(T&& Container)
{
	return Container.data();
}

template <typename T> requires requires(T Container) { Container.Num(); }
constexpr auto GetNum(T&& Container)
{
	return Container.Num();
}

template <typename T, size_t N> constexpr size_t GetNum(      T(&  Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(      T(&& Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(const T(&  Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(const T(&& Container)[N]) { return N; }

template <typename T> requires requires(T Container) { Container.size(); }
constexpr auto GetNum(T&& Container)
{
	return Container.size();
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
