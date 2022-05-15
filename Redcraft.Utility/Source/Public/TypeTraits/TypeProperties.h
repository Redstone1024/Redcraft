#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/Miscellaneous.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> struct TIsConst                        : TBoolConstant<NAMESPACE_STD::is_const_v<T>>                          { };
template <typename T> struct TIsVolatile                     : TBoolConstant<NAMESPACE_STD::is_volatile_v<T>>                       { };
template <typename T> struct TIsTrivial                      : TBoolConstant<NAMESPACE_STD::is_trivial_v<T>>                        { };
template <typename T> struct TIsTriviallyCopyable            : TBoolConstant<NAMESPACE_STD::is_trivially_copyable_v<T>>             { };
template <typename T> struct TIsStandardLayout               : TBoolConstant<NAMESPACE_STD::is_standard_layout_v<T>>                { };
template <typename T> struct THasUniqueObjectRepresentations : TBoolConstant<NAMESPACE_STD::has_unique_object_representations_v<T>> { };
template <typename T> struct TIsEmpty                        : TBoolConstant<NAMESPACE_STD::is_empty_v<T>>                          { };
template <typename T> struct TIsPolymorphic                  : TBoolConstant<NAMESPACE_STD::is_polymorphic_v<T>>                    { };
template <typename T> struct TIsAbstract                     : TBoolConstant<NAMESPACE_STD::is_abstract_v<T>>                       { };
template <typename T> struct TIsFinal                        : TBoolConstant<NAMESPACE_STD::is_final_v<T>>                          { };
template <typename T> struct TIsAggregate                    : TBoolConstant<NAMESPACE_STD::is_aggregate_v<T>>                      { };
template <typename T> struct TIsSigned                       : TBoolConstant<NAMESPACE_STD::is_signed_v<T>>                         { };
template <typename T> struct TIsUnsigned                     : TBoolConstant<NAMESPACE_STD::is_unsigned_v<T>>                       { };
template <typename T> struct TIsBoundedArray                 : TBoolConstant<NAMESPACE_STD::is_bounded_array_v<T>>                  { };
template <typename T> struct TIsUnboundedArray               : TBoolConstant<NAMESPACE_STD::is_unbounded_array_v<T>>                { };

template <typename T>
struct TIsScopedEnum : TBoolConstant<CEnum<T> && !TIsConvertible<T, int64>::Value> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
