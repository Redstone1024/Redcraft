#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <size_t... Values>
struct TMaximum;

template <>
struct TMaximum<> : TConstant<size_t, 0> { };

template <size_t Value>
struct TMaximum<Value> : TConstant<size_t, Value> { };

template <size_t First, size_t Second, size_t... Others>
struct TMaximum<First, Second, Others...> : TMaximum<(First < Second ? Second : First), Others...> { };

NAMESPACE_PRIVATE_END

template <typename T>               inline constexpr size_t ArrayRank   =  NAMESPACE_STD::rank_v<T>;
template <typename T, size_t I = 0> inline constexpr size_t ArrayExtent =  NAMESPACE_STD::extent_v<T, I>;

template <typename T, typename U>                   concept CSameAs          = NAMESPACE_STD::is_same_v<T, U>;
template <typename T, typename U>                   concept CBaseOf          = NAMESPACE_STD::is_base_of_v<T, U>;
template <typename T, typename U>                   concept CConvertibleTo   = NAMESPACE_STD::is_convertible_v<T, U>;

template <typename T> struct TRemoveConst      { using Type = NAMESPACE_STD::remove_const_t<T>;       };
template <typename T> struct TRemoveVolatile   { using Type = NAMESPACE_STD::remove_volatile_t<T>;    };
template <typename T> struct TRemoveCV         { using Type = NAMESPACE_STD::remove_cv_t<T>;          };
template <typename T> struct TRemovePointer    { using Type = NAMESPACE_STD::remove_pointer_t<T>;     };
template <typename T> struct TRemoveReference  { using Type = NAMESPACE_STD::remove_reference_t<T>;   };
template <typename T> struct TRemoveCVRef      { using Type = NAMESPACE_STD::remove_cvref_t<T>;       };
template <typename T> struct TRemoveExtent     { using Type = NAMESPACE_STD::remove_extent_t<T>;      };
template <typename T> struct TRemoveAllExtents { using Type = NAMESPACE_STD::remove_all_extents_t<T>; };

template <typename T> struct TMakeSigned   { using Type = NAMESPACE_STD::make_signed_t<T>;   };
template <typename T> struct TMakeUnsigned { using Type = NAMESPACE_STD::make_unsigned_t<T>; };

template <size_t Size, size_t Align>      struct TAlignedStorage  { class Type { struct alignas(Align) { uint8 Pad[Size]; } Padding; }; };
template <size_t Size, typename... Types> struct TAlignedUnion    { using Type = TAlignedStorage<NAMESPACE_PRIVATE::TMaximum<Size, sizeof(Types)...>::Value, NAMESPACE_PRIVATE::TMaximum<alignof(Types)...>::Value>::Type; };
template <typename T>                     struct TDecay           { using Type = NAMESPACE_STD::decay_t<T>;                      };
template <bool B, typename T = void>      struct TEnableIf        { using Type = NAMESPACE_STD::enable_if_t<B, T>;               };
template <bool B, typename T, typename F> struct TConditional     { using Type = NAMESPACE_STD::conditional_t<B, T, F>;          };
template <typename T>                     struct TUnderlyingType  { using Type = NAMESPACE_STD::underlying_type_t<T>;            };
template <typename... Types>              struct TVoid            { using Type = void;                                           };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
