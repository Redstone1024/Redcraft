#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> concept CIntegral           = TIsIntegral<T>::Value;
template <typename T> concept CSignedIntegral     = CIntegral<T> && TIsSigned<T>::Value;
template <typename T> concept CUnsignedIntegral   = CIntegral<T> && TIsUnsigned<T>::Value;
template <typename T> concept CNonBooleanIntegral = CIntegral<T> && !TIsSame<typename TRemoveCVRef<T>::Type, bool>::Value;
template <typename T> concept CFloatingPoint      = TIsFloatingPoint<T>::Value;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
