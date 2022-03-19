#pragma once

#include "CoreTypes.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Assume that all operands of bitwise operations have the same size

// This type traits is allowed to be specialised.
template <typename T> struct TIsZeroConstructible : TBoolConstant<TIsEnum<T>::Value || TIsArithmetic<T>::Value || TIsPointer<T>::Value> { };

// This type traits is allowed to be specialised.
template <typename T, typename U> struct TIsBitwiseConstructible;

template <typename T, typename U> struct TIsBitwiseConstructible<               T, const          U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<               T,       volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<               T, const volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const          T,                U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const          T, const          U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const          T,       volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const          T, const volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<      volatile T,                U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<      volatile T, const          U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<      volatile T,       volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<      volatile T, const volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const volatile T,                U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const volatile T, const          U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const volatile T,       volatile U> : TIsBitwiseConstructible<T, U> { };
template <typename T, typename U> struct TIsBitwiseConstructible<const volatile T, const volatile U> : TIsBitwiseConstructible<T, U> { };

template <typename T> struct TIsBitwiseConstructible<T,  T> : TIsTriviallyCopyConstructible<T> { };

template <typename T, typename U> struct TIsBitwiseConstructible<T*, U*> : TBoolConstant<TIsSame<typename TRemoveCV<T>::Type, typename TRemoveCV<U>::Type>::Value> { };

template <typename T, typename U> struct TIsBitwiseConstructible : FFalse { };

template <> struct TIsBitwiseConstructible<uint8,   int8>  : FTrue { };
template <> struct TIsBitwiseConstructible< int8,  uint8>  : FTrue { };
template <> struct TIsBitwiseConstructible<uint16,  int16> : FTrue { };
template <> struct TIsBitwiseConstructible< int16, uint16> : FTrue { };
template <> struct TIsBitwiseConstructible<uint32,  int32> : FTrue { };
template <> struct TIsBitwiseConstructible< int32, uint32> : FTrue { };
template <> struct TIsBitwiseConstructible<uint64,  int64> : FTrue { };
template <> struct TIsBitwiseConstructible< int64, uint64> : FTrue { };

// It is usually only necessary to specialize TIsBitwiseConstructible and not recommended to specialize TIsBitwiseRelocatable.
template <typename T, typename U> struct TIsBitwiseRelocatable;

template <typename T, typename U> struct TIsBitwiseRelocatable<               T, const          U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<               T,       volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<               T, const volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const          T,                U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const          T, const          U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const          T,       volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const          T, const volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<      volatile T,                U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<      volatile T, const          U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<      volatile T,       volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<      volatile T, const volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const volatile T,                U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const volatile T, const          U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const volatile T,       volatile U> : TIsBitwiseRelocatable<T, U> { };
template <typename T, typename U> struct TIsBitwiseRelocatable<const volatile T, const volatile U> : TIsBitwiseRelocatable<T, U> { };

template <typename T> struct TIsBitwiseRelocatable<T, T> : FTrue { };

template <typename T, typename U> struct TIsBitwiseRelocatable : TBoolConstant<TIsBitwiseConstructible<T, U>::Value && TIsTriviallyDestructible<U>::Value> { };

// This type traits is allowed to be specialised.
template <typename T> struct TIsBitwiseComparable : TBoolConstant<TIsEnum<T>::Value || TIsArithmetic<T>::Value || TIsPointer<T>::Value> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
