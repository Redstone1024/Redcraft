#pragma once

#include "CoreTypes.h"
#include "Concepts/Common.h"
#include "TypeTraits/TypeTraits.h"
#include "Concepts/BooleanTestable.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, typename U>
concept CWeaklyEqualityComparableWith =
	requires(const TRemoveReference<T>::Type & A, const TRemoveReference<U>::Type & B)
	{
		{ A == B } -> CBooleanTestable;
		{ A != B } -> CBooleanTestable;
		{ B == A } -> CBooleanTestable;
		{ B != A } -> CBooleanTestable;
	};

template <typename T>
concept CEqualityComparable = CWeaklyEqualityComparableWith<T, T>;

template <typename T, typename U>
concept CEqualityComparableWith =
	CEqualityComparable<T> &&
	CEqualityComparable<U> &&
	CWeaklyEqualityComparableWith<T, U> &&
	CCommonReferenceWith<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&> &&
	CEqualityComparable<typename TCommonReference<const typename TRemoveReference<T>::Type&,  const typename TRemoveReference<U>::Type&>::Type>;

template <typename T, typename U>
concept CPartiallyOrderedWith =
	requires(const TRemoveReference<T>::Type& A, const TRemoveReference<U>::Type& B)
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

template <typename T>
concept CTotallyOrdered = CEqualityComparable<T> && CPartiallyOrderedWith<T, T>;

template <typename T, typename U>
concept CTotallyOrderedWith =
	CTotallyOrdered<T> && CTotallyOrdered<U> &&
	CPartiallyOrderedWith<T, U> &&
	CEqualityComparableWith<T, U> &&
	CTotallyOrdered<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
