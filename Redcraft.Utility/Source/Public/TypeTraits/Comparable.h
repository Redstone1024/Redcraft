#pragma once

#include "CoreTypes.h"
#include "TypeTraits/Common.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/BooleanTestable.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename U = T>
concept CWeaklyEqualityComparable =
	requires(const typename TRemoveReference<T>::Type& A, const typename TRemoveReference<U>::Type& B)
	{
		{ A == B } -> CBooleanTestable;
		{ A != B } -> CBooleanTestable;
		{ B == A } -> CBooleanTestable;
		{ B != A } -> CBooleanTestable;
	};

template <typename T, typename U = T>
concept CEqualityComparable = CWeaklyEqualityComparable<T> && CWeaklyEqualityComparable<U> && CWeaklyEqualityComparable<T, U>
	&& CCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>
	&& CWeaklyEqualityComparable<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type>;

template <typename T, typename U = T>
concept CPartiallyOrdered =
	requires(const typename TRemoveReference<T>::Type& A, const typename TRemoveReference<U>::Type& B)
	{
		{ A <  B } -> CBooleanTestable;
		{ A >  B } -> CBooleanTestable;
		{ A <= B } -> CBooleanTestable;
		{ A >= B } -> CBooleanTestable;
		{ B <  A } -> CBooleanTestable;
		{ B >  A } -> CBooleanTestable;
		{ B <= A } -> CBooleanTestable;
		{ B >= A } -> CBooleanTestable;
	};

template <typename T, typename U = T>
concept CTotallyOrdered =
	CPartiallyOrdered<T> && CPartiallyOrdered<U>
	&& CEqualityComparable<T> && CEqualityComparable<U>
	&& CPartiallyOrdered<T, U> && CEqualityComparable<T, U>
	&& CPartiallyOrdered<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type>
	&& CEqualityComparable<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
