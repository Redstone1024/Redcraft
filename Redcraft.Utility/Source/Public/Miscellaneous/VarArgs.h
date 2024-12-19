#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

#include <cstdarg>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T>
struct TVarArgsAssert
{
	static_assert(CArithmetic<T> || CEnum<T> || CPointer<T> || CMemberPointer<T> || CClass<T>, "The type must be arithmetic, enum, pointer, member pointer, or class");
	static_assert(!CNullPointer<T>,                    "The 'nullptr_t' is promoted to 'void*' when passed through '...'");
	static_assert(!CSameAs<T, float>,                  "The 'float' is promoted to 'double' when passed through '...'");
	static_assert(!CSameAs<T,  bool>,                  "The 'bool' is promoted to 'int' when passed through '...'");
	static_assert(!CSameAs<T,  char>,                  "The 'char' is promoted to 'int' when passed through '...'");
	static_assert(!CSameAs<T, short>,                  "The 'short' is promoted to 'int' when passed through '...'");
	static_assert(CSameAs<T, TRemoveCV<T>>,            "The 'const' and 'volatile' qualifiers are removed when passed through '...'");
	static_assert(!CEnum<T>  || CScopedEnum<T>,        "The unscoped enum is promoted to 'int' when passed through '...'");
	static_assert(!CClass<T> || CTriviallyCopyable<T>, "The non-trivially copyable class is not supported");
};

template <typename T> inline constexpr TVarArgsAssert<T> VarArgsAssert{ };

NAMESPACE_PRIVATE_END

/** Enables access to variadic function arguments. */
#define VARARGS_ACCESS_BEGIN(ContextName, NamedParam) NAMESPACE_STD::va_list ContextName; va_start(ContextName, NamedParam)

/** Makes a copy of the variadic function arguments. */
#define VARARGS_ACCESS_COPY(ContextName, ContextSource) NAMESPACE_STD::va_list ContextName; va_copy(ContextName, ContextSource)

/** Accesses the next variadic function argument. */
#define VARARGS_ACCESS(ContextName, Type) (NAMESPACE_REDCRAFT::NAMESPACE_PRIVATE::VarArgsAssert<Type>, va_arg(ContextName, Type))

/** Ends traversal of the variadic function arguments. */
#define VARARGS_ACCESS_END(ContextName) va_end(ContextName)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
