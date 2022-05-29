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
	requires(const TRemoveReference<T>& A, const TRemoveReference<U>& B)
	{
		{ A == B } -> CBooleanTestable;
		{ A != B } -> CBooleanTestable;
		{ B == A } -> CBooleanTestable;
		{ B != A } -> CBooleanTestable;
	};

template <typename T, typename U = T>
concept CEqualityComparable = CWeaklyEqualityComparable<T> && CWeaklyEqualityComparable<U> && CWeaklyEqualityComparable<T, U>
	&& CCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>
	&& CWeaklyEqualityComparable<TCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>>;

template <typename T, typename U = T>
concept CPartiallyOrdered =
	requires(const TRemoveReference<T>& A, const TRemoveReference<U>& B)
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
	&& CPartiallyOrdered<TCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>>
	&& CEqualityComparable<TCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
