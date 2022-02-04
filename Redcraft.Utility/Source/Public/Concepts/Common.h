#pragma once

#include "CoreTypes.h"
#include "Concepts/Same.h"
#include "Templates/Templates.h"
#include "Concepts/Convertible.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

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
