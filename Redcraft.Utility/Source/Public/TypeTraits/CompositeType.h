#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)
NAMESPACE_BEGIN(TypeTraits)

template <typename T> struct TIsReference     : TBoolConstant<NAMESPACE_STD::is_reference_v<T>>      { };
template <typename T> struct TIsArithmetic    : TBoolConstant<NAMESPACE_STD::is_arithmetic_v<T>>     { };
template <typename T> struct TIsFundamental   : TBoolConstant<NAMESPACE_STD::is_fundamental_v<T>>    { };
template <typename T> struct TIsObject        : TBoolConstant<NAMESPACE_STD::is_object_v<T>>         { };
template <typename T> struct TIsScalar        : TBoolConstant<NAMESPACE_STD::is_scalar_v<T>>         { };
template <typename T> struct TIsCompound      : TBoolConstant<NAMESPACE_STD::is_compound_v<T>>       { };
template <typename T> struct TIsMemberPointer : TBoolConstant<NAMESPACE_STD::is_member_pointer_v<T>> { };

NAMESPACE_END(TypeTraits)
NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
