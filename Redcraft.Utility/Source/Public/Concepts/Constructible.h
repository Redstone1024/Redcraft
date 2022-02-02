#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Concepts/Convertible.h"

#include <new>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename... Args>
concept CConstructibleFrom = CDestructible<T> && TIsConstructible<T, Args...>::Value;

template <typename T>
concept CDefaultInitializable = CConstructibleFrom<T> && requires { T{}; ::new(static_cast<void*>(nullptr)) T; };

template <typename T>
concept CMoveConstructible = CConstructibleFrom<T, T> && CConvertibleTo<T, T>;

template <typename T>
concept CCopyConstructible = CMoveConstructible<T> &&
	CConstructibleFrom<T, T&>       && CConvertibleTo<T&, T> &&
	CConstructibleFrom<T, const T&> && CConvertibleTo<const T&, T> &&
	CConstructibleFrom<T, const T>  && CConvertibleTo<const T, T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
