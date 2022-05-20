#pragma once

#include "CoreTypes.h"
#include "TypeTraits/Miscellaneous.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

struct FNoopStruct { };

template <typename... Types> concept CCommonType      = requires { typename NAMESPACE_STD::common_type_t<Types...>;      };
template <typename... Types> concept CCommonReference = requires { typename NAMESPACE_STD::common_reference_t<Types...>; };

template <typename... Types> struct TCommonType      { using Type = NAMESPACE_STD::common_type_t<Types...>;      };
template <typename... Types> struct TCommonReference { using Type = NAMESPACE_STD::common_reference_t<Types...>; };

NAMESPACE_PRIVATE_END

template <typename... Types> struct TCommonType      : TConditional<NAMESPACE_PRIVATE::CCommonType<Types...>,      NAMESPACE_PRIVATE::TCommonType<Types...>,      NAMESPACE_PRIVATE::FNoopStruct>::Type { };
template <typename... Types> struct TCommonReference : TConditional<NAMESPACE_PRIVATE::CCommonReference<Types...>, NAMESPACE_PRIVATE::TCommonReference<Types...>, NAMESPACE_PRIVATE::FNoopStruct>::Type { };

template <typename T, typename U>
concept CCommonReferenceWith =
	requires
	{
		typename TCommonReference<T, U>::Type;
		typename TCommonReference<U, T>::Type;
	} &&
	CSameAs<typename TCommonReference<T, U>::Type, typename TCommonReference<U, T>::Type> &&
	CConvertibleTo<T, typename TCommonReference<T, U>::Type>&&
	CConvertibleTo<U, typename TCommonReference<T, U>::Type>;

template <typename T, typename U>
concept CCommonWith =
	requires
	{
		typename TCommonType<T, U>::Type;
		typename TCommonType<U, T>::Type;
		requires CSameAs<typename TCommonType<T, U>::Type, typename TCommonType<U, T>::Type>;
		static_cast<TCommonType<T, U>::Type>(DeclVal<T>());
		static_cast<TCommonType<T, U>::Type>(DeclVal<U>());
	} &&
	CCommonReferenceWith<const T&, const U&> &&
	CCommonReferenceWith<typename TCommonType<T, U>::Type&, typename TCommonReference<const T&, const U&>::Type> &&
	CSameAs<typename TCommonReference<T, U>::Type, typename TCommonReference<U, T>::Type>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
