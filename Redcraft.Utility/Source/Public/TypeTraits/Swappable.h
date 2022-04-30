#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/HelperClasses.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T>
concept CSwappable = requires(T & A, T & B) { Swap(A, B); };

template <typename T, typename U>
concept CSwappableWith =
	requires(T&& A, U&& B)
	{
		Swap(Forward<T>(A), Forward<T>(A));
		Swap(Forward<U>(B), Forward<U>(B));
		Swap(Forward<T>(A), Forward<U>(B));
		Swap(Forward<U>(B), Forward<T>(A));
	};

NAMESPACE_PRIVATE_END

template <typename T>             struct TIsSwappable     : TBoolConstant<NAMESPACE_PRIVATE::CSwappable<T>>        { };
template <typename T, typename U> struct TIsSwappableWith : TBoolConstant<NAMESPACE_PRIVATE::CSwappableWith<T, U>> { };

//template <typename T>             struct TIsNothrowSwappable;
//template <typename T, typename U> struct TIsNothrowSwappableWith;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
