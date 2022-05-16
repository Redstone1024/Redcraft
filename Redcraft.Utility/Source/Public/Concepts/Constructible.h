#pragma once

#include "CoreTypes.h"
#include "Concepts/Convertible.h"
#include "TypeTraits/TypeTraits.h"
#include "Concepts/Destructible.h"

#include <new>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename... Args>
concept CConstructibleFrom = CDestructible<T> && CConstructible<T, Args...>;

template <typename T>
concept CDefaultInitializable = CConstructibleFrom<T> && requires { T{}; ::new(static_cast<void*>(nullptr)) T; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
