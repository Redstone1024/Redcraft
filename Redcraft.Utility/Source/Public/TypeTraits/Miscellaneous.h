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

template <typename T>               inline constexpr size_t TRank   =  NAMESPACE_STD::rank_v<T>;
template <typename T, size_t I = 0> inline constexpr size_t TExtent =  NAMESPACE_STD::extent_v<T, I>;

template <typename T, typename U> concept CSameAs        = NAMESPACE_STD::is_same_v<T, U>;
template <typename T, typename U> concept CBaseOf        = NAMESPACE_STD::is_base_of_v<T, U>;
template <typename T, typename U> concept CConvertibleTo = NAMESPACE_STD::is_convertible_v<T, U>;

template <typename T> using TRemoveConst      = NAMESPACE_STD::remove_const_t<T>;
template <typename T> using TRemoveVolatile   = NAMESPACE_STD::remove_volatile_t<T>;
template <typename T> using TRemoveCV         = NAMESPACE_STD::remove_cv_t<T>;
template <typename T> using TRemovePointer    = NAMESPACE_STD::remove_pointer_t<T>;
template <typename T> using TRemoveReference  = NAMESPACE_STD::remove_reference_t<T>;
template <typename T> using TRemoveCVRef      = NAMESPACE_STD::remove_cvref_t<T>;
template <typename T> using TRemoveExtent     = NAMESPACE_STD::remove_extent_t<T>;
template <typename T> using TRemoveAllExtents = NAMESPACE_STD::remove_all_extents_t<T>;

template <typename T> using TAddConst           = NAMESPACE_STD::add_const_t<T>;
template <typename T> using TAddVolatile        = NAMESPACE_STD::add_volatile_t<T>;
template <typename T> using TAddCV              = NAMESPACE_STD::add_cv_t<T>;
template <typename T> using TAddLValueReference = NAMESPACE_STD::add_lvalue_reference_t<T>;
template <typename T> using TAddRValueReference = NAMESPACE_STD::add_rvalue_reference_t<T>;
template <typename T> using TAddPointer         = NAMESPACE_STD::add_pointer_t<T>;

template <typename T> using TMakeSigned   = NAMESPACE_STD::make_signed_t<T>;
template <typename T> using TMakeUnsigned = NAMESPACE_STD::make_unsigned_t<T>;

template <size_t Size, size_t Align = 16> class TAlignedStorage { struct alignas(Align) { uint8 Pad[Size]; } Padding; };
template <size_t Size, typename... Ts>    using TAlignedUnion   = TAlignedStorage<NAMESPACE_PRIVATE::TMaximum<Size, sizeof(Ts)...>::Value, NAMESPACE_PRIVATE::TMaximum<alignof(Ts)...>::Value>;
template <typename T>                     using TDecay          = NAMESPACE_STD::decay_t<T>;
template <bool B, typename T = void>      using TEnableIf       = NAMESPACE_STD::enable_if_t<B, T>;
template <bool B, typename T, typename F> using TConditional    = NAMESPACE_STD::conditional_t<B, T, F>;
template <typename T>                     using TUnderlyingType = NAMESPACE_STD::underlying_type_t<T>;
template <typename... Ts>                 using TVoid           = void;
template <typename T>                     using TIdentity       = T;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
