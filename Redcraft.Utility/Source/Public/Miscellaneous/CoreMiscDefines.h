#pragma once

#include "Miscellaneous/CoreDefines.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

constexpr size_t   INDEX_NONE = -1;
constexpr WIDECHAR UNICODE_BOM = 0xfeff;

struct FForceInit { explicit FForceInit() = default; };
struct FNoInit    { explicit FNoInit()    = default; };
struct FInvalid   { explicit FInvalid()   = default; };
struct FInPlace   { explicit FInPlace()   = default; };

inline constexpr FForceInit ForceInit{ };
inline constexpr FNoInit    NoInit{ };
inline constexpr FInvalid   Invalid{ };
inline constexpr FInPlace   InPlace{ };

template <typename T> struct TInPlaceType  { explicit TInPlaceType()  = default; };
template <size_t   I> struct TInPlaceIndex { explicit TInPlaceIndex() = default; };

template <typename T> inline constexpr TInPlaceType<T>  InPlaceType{ };
template <size_t   I> inline constexpr TInPlaceIndex<I> InPlaceIndex{ };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
