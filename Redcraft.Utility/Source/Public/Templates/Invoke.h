#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

struct InvokeFunction
{
	template <typename F, typename... Types>
	static auto Invoke(F&& Object, Types&&... Args)
	{
		return Object(Forward<Types>(Args)...);
	}
};

struct InvokeMemberFunction
{
	template <typename F, typename ObjectType, typename... Types>
	static auto Invoke(F&& Func, ObjectType&& Object, Types&&... Args) ->
		decltype((Object->*Func)(Forward<Types>(Args)...))
	{
		return (Object->*Func)(Forward<Types>(Args)...);
	}

	template <typename F, typename ObjectType, typename... Types>
	static auto Invoke(F&& Func, ObjectType&& Object, Types&&... Args) ->
		decltype((Object.*Func)(Forward<Types>(Args)...))
	{
		return (Object.*Func)(Forward<Types>(Args)...);
	}
};

struct InvokeMemberObject
{
	template <typename F, typename ObjectType>
	static auto Invoke(F&& Func, ObjectType&& Object) ->
		decltype(Object->*Func)
	{
		return (Object->*Func);
	}

	template <typename F, typename ObjectType>
	static auto Invoke(F&& Func, ObjectType&& Object) ->
		decltype(Object.*Func)
	{
		return (Object.*Func);
	}
};

template <typename F,
	typename T,
	typename Decayed = typename TDecay<F>::Type,
	bool IsMemberFunction = TIsMemberFunctionPointer<Decayed>::Value,
	bool IsMemberObject = TIsMemberObjectPointer<Decayed>::Value>
	struct InvokeMember;

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed,  true, false> : InvokeMemberFunction { };

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed, false,  true> : InvokeMemberObject { };

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed, false, false> : InvokeFunction { };

template <typename F, typename... Types>
struct InvokeImpl;

template <typename F>
struct InvokeImpl<F> : InvokeFunction { };

template <typename F, typename T, typename... Types>
struct InvokeImpl<F, T, Types...> : InvokeMember<F, T> { };

NAMESPACE_PRIVATE_END

template <typename F, typename... Types> requires TIsInvocable<F, Types...>::Value
constexpr auto Invoke(F&& Func, Types&&... Args)
{
	return NAMESPACE_PRIVATE::InvokeImpl<F, Types...>::Invoke(Forward<F>(Func), Forward<Types>(Args)...);
}

template <typename R, typename F, typename... Types> requires TIsInvocableResult<R, F, Types...>::Value
constexpr R InvokeResult(F&& Func, Types&&... Args)
{
	if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), Forward<Types>(Args)...);
	else                      return Invoke(Forward<F>(Func), Forward<Types>(Args)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
