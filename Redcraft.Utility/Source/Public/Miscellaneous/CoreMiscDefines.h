#pragma once

#include "Miscellaneous/CoreDefines.h"

#include <typeinfo>
#include <initializer_list>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// ReSharper disable CppInconsistentNaming

#define NORETURN               [[noreturn]]
#define CARRIES_DEPENDENCY     [[carries_dependency]]
#define DEPRECATED(Message)    [[deprecated(Message)]]
#define FALLTHROUGH            [[fallthrough]]
#define NODISCARD              [[nodiscard]]
#define MAYBE_UNUSED           [[maybe_unused]]
#define LIKELY                 [[likely]]
#define UNLIKELY               [[unlikely]]
#define NO_UNIQUE_ADDRESS      [[no_unique_address]]

constexpr size_t INDEX_NONE = -1;

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

using type_info        = NAMESPACE_STD::type_info;

template <typename T>
using initializer_list = NAMESPACE_STD::initializer_list<T>;

// ReSharper restore CppInconsistentNaming

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
