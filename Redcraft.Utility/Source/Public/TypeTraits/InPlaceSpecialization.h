#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> struct TIsTInPlaceType                  : FFalse { };
template <typename T> struct TIsTInPlaceType<TInPlaceType<T>> : FTrue  { };

template <typename T> struct TIsTInPlaceIndex                   : FFalse { };
template <size_t   I> struct TIsTInPlaceIndex<TInPlaceIndex<I>> : FTrue  { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
