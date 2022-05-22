#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename From, typename To> struct TCopyConstImpl                 { using Type =       To; };
template <typename From, typename To> struct TCopyConstImpl<const From, To> { using Type = const To; };

template <typename From, typename To> struct TCopyVolatileImpl                    { using Type =          To; };
template <typename From, typename To> struct TCopyVolatileImpl<volatile From, To> { using Type = volatile To; };

template <typename From, typename To> struct TCopyCVImpl { using Type = typename TCopyConstImpl<From, typename TCopyVolatileImpl<From, To>::Type>::Type; };

template <typename From, typename To> struct TCopyReferenceImpl             { using Type = To;   };
template <typename From, typename To> struct TCopyReferenceImpl<From&,  To> { using Type = To&;  };
template <typename From, typename To> struct TCopyReferenceImpl<From&&, To> { using Type = To&&; };

template <typename From, typename To> struct TCopyCVRefImpl               { using Type = typename TCopyCVImpl<From, To>::Type;   };
template <typename From, typename To> struct TCopyCVRefImpl<From,   To& > { using Type = typename TCopyCVImpl<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From,   To&&> { using Type = typename TCopyCVImpl<From, To>::Type&&; };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To  > { using Type = typename TCopyCVImpl<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To& > { using Type = typename TCopyCVImpl<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To&&> { using Type = typename TCopyCVImpl<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To  > { using Type = typename TCopyCVImpl<From, To>::Type&&; };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To& > { using Type = typename TCopyCVImpl<From, To>::Type&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To&&> { using Type = typename TCopyCVImpl<From, To>::Type&&; };

NAMESPACE_PRIVATE_END

template <typename From, typename To> using TCopyConst     = typename NAMESPACE_PRIVATE::TCopyConstImpl<From, To>::Type;
template <typename From, typename To> using TCopyVolatile  = typename NAMESPACE_PRIVATE::TCopyVolatileImpl<From, To>::Type;
template <typename From, typename To> using TCopyCV        = typename NAMESPACE_PRIVATE::TCopyCVImpl<From, To>::Type;
template <typename From, typename To> using TCopyReference = typename NAMESPACE_PRIVATE::TCopyReferenceImpl<From, To>::Type;
template <typename From, typename To> using TCopyCVRef     = typename NAMESPACE_PRIVATE::TCopyCVRefImpl<From, To>::Type;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
