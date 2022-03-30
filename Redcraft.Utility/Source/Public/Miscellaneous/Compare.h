#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Concepts/Concepts.h"
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
				TIsSame<Types, strong_ordering >::Value ? 0u :
				TIsSame<Types, weak_ordering   >::Value ? 4u :
				TIsSame<Types, partial_ordering>::Value ? 2u : 1u
			)
		)> 
{ };

template <typename T, typename OrderingType>
concept CThreeWayComparesAs = CSameAs<typename TCommonComparisonCategory<T, OrderingType>::Type, OrderingType>;

template <typename T, typename OrderingType = partial_ordering>
concept CThreeWayComparable = CWeaklyEqualityComparableWith<T, T> && CPartiallyOrderedWith<T, T> &&
	requires(const TRemoveReference<T>::Type& A, const TRemoveReference<T>::Type& B)
	{
		{ A <=> B } -> CThreeWayComparesAs<OrderingType>;
	};

template <typename T, typename U, typename OrderingType = partial_ordering>
concept CThreeWayComparableWith = CWeaklyEqualityComparableWith<T, U> && CPartiallyOrderedWith<T, U> &&
	CThreeWayComparable<T, OrderingType> &&	CThreeWayComparable<U, OrderingType> &&
	CCommonReferenceWith<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&> &&
	CThreeWayComparable<typename TCommonReference<const typename TRemoveReference<T>::Type&, const typename TRemoveReference<U>::Type&>::Type, OrderingType> &&
	requires(const TRemoveReference<T>::Type& A, const TRemoveReference<U>::Type& B)
	{
		{ A <=> B } -> CThreeWayComparesAs<OrderingType>;
		{ B <=> A } -> CThreeWayComparesAs<OrderingType>;
	};

template <typename T, typename U = T>
struct TCompareThreeWayResult { };

template <typename T, typename U> requires CThreeWayComparableWith<T, U>
struct TCompareThreeWayResult<T, U>
{
	using Type = decltype(DeclVal<const typename TRemoveReference<T>::Type&>() <=> DeclVal<const typename TRemoveReference<U>::Type&>());
};

template <typename T = void> requires (CSameAs<T, void> || CThreeWayComparable<T>)
struct TCompareThreeWay
{
	constexpr auto operator()(T&& LHS, T&& RHS) const
	{
		return Forward<T>(LHS) <=> Forward<T>(RHS);
	}
};

template <>
struct TCompareThreeWay<void>
{
	template <typename T, typename U> requires CThreeWayComparableWith<T, U>
	constexpr auto operator()(T&& LHS, U&& RHS) const
	{
		return Forward<T>(LHS) <=> Forward<U>(RHS);
	}
};

template <typename T, typename OrderingType = partial_ordering>
concept CSynthThreeWayComparable = CThreeWayComparable<T> ||
	requires(const TRemoveReference<T>::Type& A, const TRemoveReference<T>::Type& B)
	{
		{ A < B } -> CBooleanTestable;
		{ B < A } -> CBooleanTestable;
	};

template <typename T, typename U, typename OrderingType = partial_ordering>
concept CSynthThreeWayComparableWith = CThreeWayComparableWith<T, U> ||
	requires(const TRemoveReference<T>::Type& A, const TRemoveReference<U>::Type& B)
	{
		{ A < B } -> CBooleanTestable;
		{ B < A } -> CBooleanTestable;
	};

template <typename T = void> requires (CSameAs<T, void> || CSynthThreeWayComparable<T>)
struct TSynthThreeWay
{
	constexpr auto operator()(T&& LHS, T&& RHS) const
	{
		if constexpr (CThreeWayComparable<T>)
		{
			return Forward<T>(LHS) <=> Forward<T>(RHS);
		}
		else
		{
			return Forward<T>(LHS) < Forward<T>(RHS) ? weak_ordering::less : Forward<T>(RHS) < Forward<T>(LHS) ? weak_ordering::greater : weak_ordering::equivalent;
		}
	}
};

template <>
struct TSynthThreeWay<void>
{
	template <typename T, typename U> requires CSynthThreeWayComparableWith<T, U>
	constexpr auto operator()(T&& LHS, U&& RHS) const
	{
		if constexpr (CThreeWayComparableWith<T, U>)
		{
			return Forward<T>(LHS) <=> Forward<U>(RHS);
		}
		else
		{
			return Forward<T>(LHS) < Forward<U>(RHS) ? weak_ordering::less : Forward<U>(RHS) < Forward<T>(LHS) ? weak_ordering::greater : weak_ordering::equivalent;
		}
	}
};

template <typename T, typename U = T>
struct TSynthThreeWayResult
{
	using Type = decltype(TSynthThreeWay{}(DeclVal<const typename TRemoveReference<T>::Type&>(), DeclVal<const typename TRemoveReference<U>::Type&>()));
};

NAMESPACE_UNNAMED_BEGIN

inline constexpr decltype(NAMESPACE_STD::strong_order)                   StrongOrder;
inline constexpr decltype(NAMESPACE_STD::weak_order)                     WeakOrder;
inline constexpr decltype(NAMESPACE_STD::partial_order)                  PartialOrder;
inline constexpr decltype(NAMESPACE_STD::compare_strong_order_fallback)  CompareStrongOrderFallback;
inline constexpr decltype(NAMESPACE_STD::compare_weak_order_fallback)    CompareWeakOrderFallback;
inline constexpr decltype(NAMESPACE_STD::compare_partial_order_fallback) ComparePartialOrderFallback;

NAMESPACE_UNNAMED_END

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
