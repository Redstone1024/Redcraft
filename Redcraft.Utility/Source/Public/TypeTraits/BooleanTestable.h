#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/Miscellaneous.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CBooleanTestable = CConvertibleTo<T, bool> &&
	requires(T && B) { { !Forward<T>(B) } -> CConvertibleTo<bool>; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
