#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/Common.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename U = T>
concept CSwappable = CCommonReference<T, U> &&
	requires(T& A, U& B)
	{
		Swap(A, A);
		Swap(B, B);
		Swap(A, B);
		Swap(B, A);
	};

//template <typename T, typename U> concept CNothrowSwappable;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
