#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename From, typename To> struct TCopyConst                 { using Type =       To; };
template <typename From, typename To> struct TCopyConst<const From, To> { using Type = const To; };

template <typename From, typename To> struct TCopyVolatile                    { using Type =          To; };
template <typename From, typename To> struct TCopyVolatile<volatile From, To> { using Type = volatile To; };

template <typename From, typename To> struct TCopyCV { using Type = typename TCopyConst<From, typename TCopyVolatile<From, To>::Type>::Type; };

template <typename From, typename To> struct TCopyReference               { using Type = To;   };
template <typename From, typename To> struct TCopyReference<From&,  To  > { using Type = To&;  };
template <typename From, typename To> struct TCopyReference<From&&, To  > { using Type = To&&; };

template <typename From, typename To> struct TCopyCVRef               { using Type = typename TCopyCV<From, To>::Type;   };
template <typename From, typename To> struct TCopyCVRef<From,   To& > { using Type = typename TCopyCV<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRef<From,   To&&> { using Type = typename TCopyCV<From, To>::Type&&; };
template <typename From, typename To> struct TCopyCVRef<From&,  To  > { using Type = typename TCopyCV<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRef<From&,  To& > { using Type = typename TCopyCV<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRef<From&,  To&&> { using Type = typename TCopyCV<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRef<From&&, To  > { using Type = typename TCopyCV<From, To>::Type&&; };
template <typename From, typename To> struct TCopyCVRef<From&&, To& > { using Type = typename TCopyCV<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRef<From&&, To&&> { using Type = typename TCopyCV<From, To>::Type&&; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
