#pragma once

#include "CoreTypes.h"
#include "TypeTraits/BooleanTestable.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename F, typename... Args>             concept CInvocable       = NAMESPACE_STD::is_invocable_v<F, Args...>;
template <typename R, typename F, typename... Args> concept CInvocableResult = NAMESPACE_STD::is_invocable_r_v<R, F, Args...>; // FIXME: The result for char(&())[2] is wrong on MSVC

template <typename F, typename... Args> using TInvokeResult = NAMESPACE_STD::invoke_result_t<F, Args...>; // FIXME: The result for char(&())[2] is wrong on MSVC

template <typename F, typename... Ts>
concept CRegularInvocable = CInvocable<F, Ts...>;

template <typename F, typename... Ts>
concept CPredicate = CRegularInvocable<F, Ts...> && CBooleanTestable<TInvokeResult<F, Ts...>>;

template <typename R, typename T, typename U>
concept CRelation =
	CPredicate<R, T, T> && CPredicate<R, U, U> &&
	CPredicate<R, T, U> && CPredicate<R, U, T>;

template <typename R, typename T, typename U>
concept CEquivalenceRelation = CRelation<R, T, U>;

template <typename R, typename T, typename U>
concept CStrictWeakOrder = CRelation<R, T, U>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
