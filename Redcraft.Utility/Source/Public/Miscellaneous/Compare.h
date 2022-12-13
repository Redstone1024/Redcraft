#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

#include <compare>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// The result of the three-way comparison operator is the built-in type of the compiler, which is directly introduced here

typedef NAMESPACE_STD::partial_ordering partial_ordering;
typedef NAMESPACE_STD::weak_ordering    weak_ordering;
typedef NAMESPACE_STD::strong_ordering  strong_ordering;

NAMESPACE_PRIVATE_BEGIN

template<int32> struct TCommonComparisonCategoryBasic    {                                };
template<>      struct TCommonComparisonCategoryBasic<0> { using Type = strong_ordering;  };
template<>      struct TCommonComparisonCategoryBasic<2> { using Type = partial_ordering; };
template<>      struct TCommonComparisonCategoryBasic<4> { using Type = weak_ordering;    };
template<>      struct TCommonComparisonCategoryBasic<6> { using Type = partial_ordering; };

template <typename... Ts>
struct TCommonComparisonCategoryImpl
	: TCommonComparisonCategoryBasic <(0u | ... |
			(
				CSameAs<Ts, strong_ordering > ? 0u :
				CSameAs<Ts, weak_ordering   > ? 4u :
				CSameAs<Ts, partial_ordering> ? 2u : 1u
			)
		)> 
{ };

NAMESPACE_PRIVATE_END

template <typename... Ts>
using TCommonComparisonCategory = typename NAMESPACE_PRIVATE::TCommonComparisonCategoryImpl<Ts...>::Type;

template <typename T, typename OrderingType>
concept CThreeWayComparesAs = CSameAs<TCommonComparisonCategory<T, OrderingType>, OrderingType>;

template <typename T, typename U = T, typename OrderingType = partial_ordering>
concept CThreeWayComparable = CWeaklyEqualityComparable<T, U> && CPartiallyOrdered<T, U>
	&& CCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>
	&& requires(const TRemoveReference<T>& A, const TRemoveReference<U>& B,
		const TRemoveReference<TCommonReference<const TRemoveReference<T>&, const TRemoveReference<U>&>>& C)
		{
			{ A <=> A } -> CThreeWayComparesAs<OrderingType>;
			{ B <=> B } -> CThreeWayComparesAs<OrderingType>;
			{ A <=> B } -> CThreeWayComparesAs<OrderingType>;
			{ B <=> A } -> CThreeWayComparesAs<OrderingType>;
			{ C <=> C } -> CThreeWayComparesAs<OrderingType>;
		};

template <typename T, typename U = T> requires (CThreeWayComparable<T, U>)
using TCompareThreeWayResult = decltype(DeclVal<const TRemoveReference<T>&>() <=> DeclVal<const TRemoveReference<U>&>());

template <typename T, typename U = T, typename OrderingType = partial_ordering>
concept CSynthThreeWayComparable = CThreeWayComparable<T, U> || CTotallyOrdered<T, U>;

template <typename T, typename U = T> requires (CSynthThreeWayComparable<T, U>)
FORCEINLINE constexpr decltype(auto) SynthThreeWayCompare(T&& LHS, U&& RHS)
{
	if constexpr (CThreeWayComparable<T, U>)
	{
		return Forward<T>(LHS) <=> Forward<U>(RHS);
	}
	else
	{
		return Forward<T>(LHS) < Forward<U>(RHS) ? weak_ordering::less : Forward<U>(RHS) < Forward<T>(LHS) ? weak_ordering::greater : weak_ordering::equivalent;
	}
}

template <typename T, typename U = T> requires (CSynthThreeWayComparable<T, U>)
using TSynthThreeWayResult = decltype(SynthThreeWayCompare(DeclVal<const TRemoveReference<T>&>(), DeclVal<const TRemoveReference<U>&>()));

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
