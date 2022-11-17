#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTInPlaceType                  : FFalse { };
template <typename T> struct TIsTInPlaceType<TInPlaceType<T>> : FTrue  { };

template <typename T> struct TIsTInPlaceIndex                   : FFalse { };
template <size_t   I> struct TIsTInPlaceIndex<TInPlaceIndex<I>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTInPlaceType  = NAMESPACE_PRIVATE::TIsTInPlaceType<TRemoveCV<T>>::Value;
template <typename T> concept CTInPlaceIndex = NAMESPACE_PRIVATE::TIsTInPlaceIndex<TRemoveCV<T>>::Value;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
