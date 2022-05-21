#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

#include <compare>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// The result of the three-way comparison operator is the built-in type of the compiler, which is directly introduced here.

typedef NAMESPACE_STD::partial_ordering partial_ordering;
typedef NAMESPACE_STD::weak_ordering    weak_ordering;
typedef NAMESPACE_STD::strong_ordering  strong_ordering;

NAMESPACE_PRIVATE_BEGIN

template<int32> struct TCommonComparisonCategory    { using Type = void;             };
template<>      struct TCommonComparisonCategory<0> { using Type = strong_ordering;  };
template<>      struct TCommonComparisonCategory<2> { using Type = partial_ordering; };
template<>      struct TCommonComparisonCategory<4> { using Type = weak_ordering;    };
template<>      struct TCommonComparisonCategory<6> { using Type = partial_ordering; };

NAMESPACE_PRIVATE_END

template <typename... Types>
struct TCommonComparisonCategory
	: NAMESPACE_PRIVATE::TCommonComparisonCategory<(0u | ... |
			(
				CSameAs<Types, strong_ordering > ? 0u :
				CSameAs<Types, weak_ordering   > ? 4u :
				CSameAs<Types, partial_ordering> ? 2u : 1u
			)
		)> 
{ };

template <typename T, typename OrderingType>
concept CThreeWayComparesAs = CSameAs<typename TCommonComparisonCategory<T, OrderingType>::Type, OrderingType>;

template <typename T, typename U = T, typename OrderingType = partial_ordering>
concept CThreeWayComparable = CWeaklyEqualityComparable<T, U> && CPartiallyOrdered<T, U>
	&& CCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>
	&& requires(const typename TRemoveReference<T>::Type& A, const typename TRemoveReference<U>::Type& B,
		const typename TRemoveReference<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type>::Type& C)
		{
			{ A <=> A } -> CThreeWayComparesAs<OrderingType>;
			{ B <=> B } -> CThreeWayComparesAs<OrderingType>;
			{ A <=> B } -> CThreeWayComparesAs<OrderingType>;
			{ B <=> A } -> CThreeWayComparesAs<OrderingType>;
			{ C <=> C } -> CThreeWayComparesAs<OrderingType>;
		};

template <typename T, typename U = T>
struct TCompareThreeWayResult { };

template <typename T, typename U> requires CThreeWayComparable<T, U>
struct TCompareThreeWayResult<T, U>
{
	using Type = decltype(DeclVal<const typename TRemoveReference<T>::Type&>() <=> DeclVal<const typename TRemoveReference<U>::Type&>());
};

template <typename T, typename U = T, typename OrderingType = partial_ordering>
concept CSynthThreeWayComparable = CThreeWayComparable<T, U> || CTotallyOrdered<T, U>;

template <typename T, typename U = T> requires CSynthThreeWayComparable<T, U>
constexpr decltype(auto) SynthThreeWayCompare(T&& LHS, U&& RHS)
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

template <typename T, typename U = T>
struct TSynthThreeWayResult
{
	using Type = decltype(SynthThreeWayCompare(DeclVal<const typename TRemoveReference<T>::Type&>(), DeclVal<const typename TRemoveReference<U>::Type&>()));
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
