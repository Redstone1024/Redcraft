#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FForceInit { explicit FForceInit() = default; };
inline constexpr FForceInit ForceInit{};

struct FNoInit { explicit FNoInit() = default; };
inline constexpr FNoInit NoInit{};

struct FInvalid { explicit FInvalid() = default; };
inline constexpr FInvalid Invalid{};

struct FInPlace { explicit FInPlace() = default; };
inline constexpr FInPlace InPlace{};

template <typename T> struct TInPlaceType { explicit TInPlaceType() = default; };
template <typename T> inline constexpr TInPlaceType<T> InPlaceType{};

template <size_t I> struct TInPlaceIndex { explicit TInPlaceIndex() = default; };
template <size_t I> inline constexpr TInPlaceIndex<I> InPlaceIndex{};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
