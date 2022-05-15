#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> concept CVoid                  = NAMESPACE_STD::is_void_v<T>;
template <typename T> concept CNullPointer           = NAMESPACE_STD::is_null_pointer_v<T>;
template <typename T> concept CIntegral              = NAMESPACE_STD::is_integral_v<T>;
template <typename T> concept CFloatingPoint         = NAMESPACE_STD::is_floating_point_v<T>;
template <typename T> concept CArray                 = NAMESPACE_STD::is_array_v<T>;
template <typename T> concept CPointer               = NAMESPACE_STD::is_pointer_v<T>;
template <typename T> concept CLValueReference       = NAMESPACE_STD::is_lvalue_reference_v<T>;
template <typename T> concept CRValueReference       = NAMESPACE_STD::is_rvalue_reference_v<T>;
template <typename T> concept CMemberObjectPointer   = NAMESPACE_STD::is_member_object_pointer_v<T>;
template <typename T> concept CMemberFunctionPointer = NAMESPACE_STD::is_member_function_pointer_v<T>;
template <typename T> concept CEnum                  = NAMESPACE_STD::is_enum_v<T>;
template <typename T> concept CUnion                 = NAMESPACE_STD::is_union_v<T>;
template <typename T> concept CClass                 = NAMESPACE_STD::is_class_v<T>;
template <typename T> concept CFunction              = NAMESPACE_STD::is_function_v<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
