#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

struct InvokeFunction
{
	template <typename F, typename... Ts>
	static auto Invoke(F&& Object, Ts&&... Args)
		-> decltype(Forward<F>(Object)(Forward<Ts>(Args)...))
	{
		return Forward<F>(Object)(Forward<Ts>(Args)...);
	}
};

struct InvokeMemberFunction
{
	template <typename F, typename ObjectType, typename... Ts>
	static auto Invoke(F&& Func, ObjectType&& Object, Ts&&... Args)
		-> decltype((Forward<ObjectType>(Object)->*Func)(Forward<Ts>(Args)...))
	{
		return (Forward<ObjectType>(Object)->*Func)(Forward<Ts>(Args)...);
	}

	template <typename F, typename ObjectType, typename... Ts>
	static auto Invoke(F&& Func, ObjectType&& Object, Ts&&... Args)
		-> decltype((Forward<ObjectType>(Object).*Func)(Forward<Ts>(Args)...))
	{
		return (Forward<ObjectType>(Object).*Func)(Forward<Ts>(Args)...);
	}
};

struct InvokeMemberObject
{
	template <typename F, typename ObjectType>
	static auto Invoke(F&& Func, ObjectType&& Object)
		-> decltype(Forward<ObjectType>(Object)->*Func)
	{
		return (Forward<ObjectType>(Object)->*Func);
	}

	template <typename F, typename ObjectType>
	static auto Invoke(F&& Func, ObjectType&& Object)
		-> decltype(Forward<ObjectType>(Object).*Func)
	{
		return (Forward<ObjectType>(Object).*Func);
	}
};

template <typename F,
	typename T,
	typename Decayed = TDecay<F>,
	bool IsMemberFunction = CMemberFunctionPointer<Decayed>,
	bool IsMemberObject = CMemberObjectPointer<Decayed>>
	struct InvokeMember;

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed,  true, false> : InvokeMemberFunction { };

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed, false,  true> : InvokeMemberObject { };

template <typename F, typename T, typename Decayed>
struct InvokeMember<F, T, Decayed, false, false> : InvokeFunction { };

template <typename F, typename... Ts>
struct InvokeImpl;

template <typename F>
struct InvokeImpl<F> : InvokeFunction { };

template <typename F, typename T, typename... Ts>
struct InvokeImpl<F, T, Ts...> : InvokeMember<F, T> { };

NAMESPACE_PRIVATE_END

template <typename F, typename... Ts> requires (CInvocable<F, Ts...>)
constexpr auto Invoke(F&& Func, Ts&&... Args)
	-> decltype(NAMESPACE_PRIVATE::InvokeImpl<F, Ts...>::Invoke(Forward<F>(Func), Forward<Ts>(Args)...))
{
	return NAMESPACE_PRIVATE::InvokeImpl<F, Ts...>::Invoke(Forward<F>(Func), Forward<Ts>(Args)...);
}

template <typename R, typename F, typename... Ts> requires (CInvocableResult<R, F, Ts...>)
constexpr R InvokeResult(F&& Func, Ts&&... Args)
{
	if constexpr (CVoid<R>) Invoke(Forward<F>(Func), Forward<Ts>(Args)...);
	else             return Invoke(Forward<F>(Func), Forward<Ts>(Args)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
