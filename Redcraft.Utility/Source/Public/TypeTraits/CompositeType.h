#pragma once

#include "CoreTypes.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/TypeProperties.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> concept CReference     = NAMESPACE_STD::is_reference_v<T>;
template <typename T> concept CArithmetic    = NAMESPACE_STD::is_arithmetic_v<T>;
template <typename T> concept CFundamental   = NAMESPACE_STD::is_fundamental_v<T>;
template <typename T> concept CObject        = NAMESPACE_STD::is_object_v<T>;
template <typename T> concept CScalar        = NAMESPACE_STD::is_scalar_v<T>;
template <typename T> concept CCompound      = NAMESPACE_STD::is_compound_v<T>;
template <typename T> concept CMemberPointer = NAMESPACE_STD::is_member_pointer_v<T>;

template <typename T> concept CSignedIntegral   = CIntegral<T> && TIsSigned<T>::Value;
template <typename T> concept CUnsignedIntegral = CIntegral<T> && TIsUnsigned<T>::Value;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
