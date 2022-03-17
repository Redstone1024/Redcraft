#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> struct TIsInPlaceTypeSpecialization                  : FFalse { };
template <typename T> struct TIsInPlaceTypeSpecialization<TInPlaceType<T>> : FTrue  { };

template <typename T> struct TIsInPlaceIndexSpecialization                   : FFalse { };
template <size_t   I> struct TIsInPlaceIndexSpecialization<TInPlaceIndex<I>> : FTrue  { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
