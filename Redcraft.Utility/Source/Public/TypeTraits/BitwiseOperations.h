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

// Specialize these template classes for user-defined types
template <typename T>             struct TIsZeroConstructible;
template <typename T, typename U> struct TIsBitwiseConstructible;
template <typename T, typename U> struct TIsBitwiseRelocatable;
template <typename T>             struct TIsBitwiseComparable;

// Normal use of these concepts
template <typename T>                 concept CZeroConstructible    = TIsZeroConstructible<T>::Value;
template <typename T, typename U = T> concept CBitwiseConstructible = TIsBitwiseConstructible<T, U>::Value;
template <typename T, typename U = T> concept CBitwiseRelocatable   = TIsBitwiseRelocatable<T, U>::Value;
template <typename T>                 concept CBitwiseComparable    = TIsBitwiseComparable<T>::Value;

// Default constructible enum, arithmetic and pointer are zero constructible
template <typename T> struct TIsZeroConstructible : TBoolConstant<CDefaultConstructible<T> && (CEnum<T> || CArithmetic<T> || CPointer<T>)> { };

// Constructing a const T is the same as constructing a T
template <typename T> struct TIsZeroConstructible<const T> : TIsZeroConstructible<T> { };

// T can always be bitwise constructed from itself if it is trivially copy constructible
template <typename T, typename U> struct TIsBitwiseConstructible : TBoolConstant<CSameAs<T, U> ? CTriviallyCopyConstructible<T> : false> { };

// Constructing a const T is the same as constructing a T
template <typename T, typename U> struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U> { };

// Const pointers can be bitwise constructed from non-const pointers
template <typename T> struct TIsBitwiseConstructible<const T*, T*> : FTrue { };

// Unsigned types can be bitwise converted to their signed equivalents, and vice versa
template <> struct TIsBitwiseConstructible<uint8,   int8>  : FTrue { };
template <> struct TIsBitwiseConstructible< int8,  uint8>  : FTrue { };
template <> struct TIsBitwiseConstructible<uint16,  int16> : FTrue { };
template <> struct TIsBitwiseConstructible< int16, uint16> : FTrue { };
template <> struct TIsBitwiseConstructible<uint32,  int32> : FTrue { };
template <> struct TIsBitwiseConstructible< int32, uint32> : FTrue { };
template <> struct TIsBitwiseConstructible<uint64,  int64> : FTrue { };
template <> struct TIsBitwiseConstructible< int64, uint64> : FTrue { };

// WARNING: T is bitwise relocatable from itself by default
// T is bitwise relocatable from U, if U is trivially destructible and can be constructed bitwise to T
template <typename T, typename U> struct TIsBitwiseRelocatable : TBoolConstant<CSameAs<T, U> ? true : CTriviallyDestructible<U> && CBitwiseConstructible<T, U>> { };

// Constructing a const T is the same as constructing a T
template <typename T, typename U> struct TIsBitwiseRelocatable<const T, U> : TIsBitwiseRelocatable<T, U> { };

// Enum, arithmetic and pointer are zero constructible
template <typename T> struct TIsBitwiseComparable : TBoolConstant<CEnum<T> || CArithmetic<T> || CPointer<T>> { };

// Constructing a const T is the same as constructing a T
template <typename T> struct TIsBitwiseComparable<const T> : TIsBitwiseComparable<T> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
