#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename F, typename... Types>
concept CInvocable = requires(F&& Func, Types&&... Args) { Invoke(Forward<F>(Func), Forward<Types>(Args)...); };

template <typename F, typename... Types>
concept CRegularInvocable = CInvocable<F, Types...>;

template <typename F, typename... Types>
concept CPredicate = CRegularInvocable<F, Types...> && CBooleanTestable<typename TInvokeResult<F, Types...>::Type>;

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
