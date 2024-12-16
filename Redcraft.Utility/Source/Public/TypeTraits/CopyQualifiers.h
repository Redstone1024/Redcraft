#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename From, typename To> struct TCopyConstImpl                 { using FType =       To; };
template <typename From, typename To> struct TCopyConstImpl<const From, To> { using FType = const To; };

template <typename From, typename To> struct TCopyVolatileImpl                    { using FType =          To; };
template <typename From, typename To> struct TCopyVolatileImpl<volatile From, To> { using FType = volatile To; };

template <typename From, typename To> struct TCopyCVImpl { using FType = typename TCopyConstImpl<From, typename TCopyVolatileImpl<From, To>::FType>::FType; };

template <typename From, typename To> struct TCopyReferenceImpl             { using FType = To;   };
template <typename From, typename To> struct TCopyReferenceImpl<From&,  To> { using FType = To&;  };
template <typename From, typename To> struct TCopyReferenceImpl<From&&, To> { using FType = To&&; };

template <typename From, typename To> struct TCopyCVRefImpl               { using FType = typename TCopyCVImpl<From, To>::FType;   };
template <typename From, typename To> struct TCopyCVRefImpl<From,   To& > { using FType = typename TCopyCVImpl<From, To>::FType&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From,   To&&> { using FType = typename TCopyCVImpl<From, To>::FType&&; };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To  > { using FType = typename TCopyCVImpl<From, To>::FType&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To& > { using FType = typename TCopyCVImpl<From, To>::FType&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&,  To&&> { using FType = typename TCopyCVImpl<From, To>::FType&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To  > { using FType = typename TCopyCVImpl<From, To>::FType&&; };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To& > { using FType = typename TCopyCVImpl<From, To>::FType&;  };
template <typename From, typename To> struct TCopyCVRefImpl<From&&, To&&> { using FType = typename TCopyCVImpl<From, To>::FType&&; };

NAMESPACE_PRIVATE_END

template <typename From, typename To> using TCopyConst     = typename NAMESPACE_PRIVATE::TCopyConstImpl    <From, To>::FType;
template <typename From, typename To> using TCopyVolatile  = typename NAMESPACE_PRIVATE::TCopyVolatileImpl <From, To>::FType;
template <typename From, typename To> using TCopyCV        = typename NAMESPACE_PRIVATE::TCopyCVImpl       <From, To>::FType;
template <typename From, typename To> using TCopyReference = typename NAMESPACE_PRIVATE::TCopyReferenceImpl<From, To>::FType;
template <typename From, typename To> using TCopyCVRef     = typename NAMESPACE_PRIVATE::TCopyCVRefImpl    <From, To>::FType;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
