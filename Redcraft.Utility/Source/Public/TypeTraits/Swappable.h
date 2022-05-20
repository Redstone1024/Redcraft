#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/Common.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CSwappable = requires(T& A, T& B) { Swap(A, B); };

template <typename T, typename U>
concept CSwappableWith = CCommonReferenceWith<T, U> &&
	requires(T&& A, U&& B)
	{
		Swap(Forward<T>(A), Forward<T>(A));
		Swap(Forward<U>(B), Forward<U>(B));
		Swap(Forward<T>(A), Forward<U>(B));
		Swap(Forward<U>(B), Forward<T>(A));
	};

//template <typename T>             concept CNothrowSwappable;
//template <typename T, typename U> concept CNothrowSwappableWith;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
