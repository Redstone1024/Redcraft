#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Concepts/Convertible.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename U>
concept CDerivedFrom = TIsBaseOf<U, T>::Value && CConvertibleTo<const volatile T*, const volatile U*>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
