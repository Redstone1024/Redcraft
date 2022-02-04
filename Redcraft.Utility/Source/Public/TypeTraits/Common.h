#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

struct FNoopStruct { };

template <typename... Types> concept CCommonType      = requires { typename NAMESPACE_STD::common_type_t<Types...>;      };
template <typename... Types> concept CCommonReference = requires { typename NAMESPACE_STD::common_reference_t<Types...>; };

template <typename... Types> struct TCommonType      { using Type = NAMESPACE_STD::common_type_t<Types...>;      };
template <typename... Types> struct TCommonReference { using Type = NAMESPACE_STD::common_reference_t<Types...>; };

NAMESPACE_PRIVATE_END

template <typename... Types> struct TCommonType      : TConditional<NAMESPACE_PRIVATE::CCommonType<Types...>,      NAMESPACE_PRIVATE::TCommonType<Types...>,      NAMESPACE_PRIVATE::FNoopStruct>::Type { };
template <typename... Types> struct TCommonReference : TConditional<NAMESPACE_PRIVATE::CCommonReference<Types...>, NAMESPACE_PRIVATE::TCommonReference<Types...>, NAMESPACE_PRIVATE::FNoopStruct>::Type { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END