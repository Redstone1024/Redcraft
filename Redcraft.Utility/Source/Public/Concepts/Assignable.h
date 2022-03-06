#pragma once

#include "CoreTypes.h"
#include "Concepts/Common.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename U>
concept CAssignableFrom =
	TIsLValueReference<T>::Value &&
	CCommonReferenceWith<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&> &&
	requires(T A, U&& B)
	{
		{ A = Forward<U>(B) } -> CSameAs<T>;
	};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
