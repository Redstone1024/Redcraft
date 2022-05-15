#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/Miscellaneous.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> concept CConst                     = NAMESPACE_STD::is_const_v<T>;
template <typename T> concept CVolatile                  = NAMESPACE_STD::is_volatile_v<T>;
template <typename T> concept CTrivial                   = NAMESPACE_STD::is_trivial_v<T>;
template <typename T> concept CTriviallyCopyable         = NAMESPACE_STD::is_trivially_copyable_v<T>;
template <typename T> concept CStandardLayout            = NAMESPACE_STD::is_standard_layout_v<T>;
template <typename T> concept CUniqueObjectRepresentible = NAMESPACE_STD::has_unique_object_representations_v<T>;
template <typename T> concept CEmpty                     = NAMESPACE_STD::is_empty_v<T>;
template <typename T> concept CPolymorphic               = NAMESPACE_STD::is_polymorphic_v<T>;
template <typename T> concept CAbstract                  = NAMESPACE_STD::is_abstract_v<T>;
template <typename T> concept CFinal                     = NAMESPACE_STD::is_final_v<T>;
template <typename T> concept CAggregate                 = NAMESPACE_STD::is_aggregate_v<T>;
template <typename T> concept CSigned                    = NAMESPACE_STD::is_signed_v<T>;
template <typename T> concept CUnsigned                  = NAMESPACE_STD::is_unsigned_v<T>;
template <typename T> concept CBoundedArray              = NAMESPACE_STD::is_bounded_array_v<T>;
template <typename T> concept CUnboundedArray            = NAMESPACE_STD::is_unbounded_array_v<T>;
template <typename T> concept CScopedEnum                = CEnum<T> && !TIsConvertible<T, int64>::Value;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
