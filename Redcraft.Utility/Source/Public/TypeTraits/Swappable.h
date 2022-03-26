#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/HelperClasses.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T, typename U>
concept CSwappableWith =
	requires(T&& A, U&& B)
	{
		Swap(A, B);
		Swap(B, A);
	};

NAMESPACE_PRIVATE_END

template <typename T>             struct TIsSwappable     : TBoolConstant<NAMESPACE_PRIVATE::CSwappableWith<T&, T&>> { };
template <typename T, typename U> struct TIsSwappableWith : TBoolConstant<NAMESPACE_PRIVATE::CSwappableWith<T , U >> { };

//template <typename T>             struct TIsNothrowSwappable;
//template <typename T, typename U> struct TIsNothrowSwappableWith;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
