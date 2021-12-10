#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)
NAMESPACE_BEGIN(TypeTraits)

template <typename T> struct TIsVoid                  : TBoolConstant<NAMESPACE_STD::is_void_v<T>>                    { };
template <typename T> struct TIsNullPointer           : TBoolConstant<NAMESPACE_STD::is_null_pointer_v<T>>            { };
template <typename T> struct TIsIntegral              : TBoolConstant<NAMESPACE_STD::is_integral_v<T>>                { };
template <typename T> struct TIsFloatingPoint         : TBoolConstant<NAMESPACE_STD::is_floating_point_v<T>>          { };
template <typename T> struct TIsArray                 : TBoolConstant<NAMESPACE_STD::is_array_v<T>>                   { };
template <typename T> struct TIsPointer               : TBoolConstant<NAMESPACE_STD::is_pointer_v<T>>                 { };
template <typename T> struct TIsLValueReference       : TBoolConstant<NAMESPACE_STD::is_lvalue_reference_v<T>>        { };
template <typename T> struct TIsRValueReference       : TBoolConstant<NAMESPACE_STD::is_rvalue_reference_v<T>>        { };
template <typename T> struct TIsMemberObjectPointer   : TBoolConstant<NAMESPACE_STD::is_member_object_pointer_v<T>>   { };
template <typename T> struct TIsMemberFunctionPointer : TBoolConstant<NAMESPACE_STD::is_member_function_pointer_v<T>> { };
template <typename T> struct TIsEnum                  : TBoolConstant<NAMESPACE_STD::is_enum_v<T>>                    { };
template <typename T> struct TIsUnion                 : TBoolConstant<NAMESPACE_STD::is_union_v<T>>                   { };
template <typename T> struct TIsClass                 : TBoolConstant<NAMESPACE_STD::is_class_v<T>>                   { };
template <typename T> struct TIsFunction              : TBoolConstant<NAMESPACE_STD::is_function_v<T>>                { };

NAMESPACE_PRIVATE_BEGIN

static char(&Resolve(int))[2];
static char Resolve(...);

template <typename T>
struct TIsEnumConvertibleToInt : TBoolConstant<sizeof(Resolve(T())) - 1> { };

NAMESPACE_PRIVATE_END

template <typename T>
struct TIsEnumClass : TBoolConstant<TAnd<TIsEnum<T>, TNot<NAMESPACE_PRIVATE::TIsEnumConvertibleToInt<T>>>::Value> { };

NAMESPACE_END(TypeTraits)
NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
