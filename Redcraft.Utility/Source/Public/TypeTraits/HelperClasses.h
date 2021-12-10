#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)
NAMESPACE_BEGIN(TypeTraits)

template <typename InType, InType InValue>
struct TConstant
{
	using Type = InType;
	static constexpr Type Value = InValue;
	constexpr operator Type() const { return Value; }
	constexpr Type operator()() const { return Value; }
};

template <bool InValue>
using TBoolConstant = TConstant<bool, InValue>;

template <int32 InValue>
using TIntegralConstant = TConstant<int32, InValue>;

using FTrue  = TBoolConstant<true>;
using FFalse = TBoolConstant<false>;

template <typename... Types> 
struct TAnd;

template <typename LHS, typename... RHS>
struct TAnd<LHS, RHS...> : TBoolConstant<LHS::Value&& TAnd<RHS...>::Value> { };

template <>
struct TAnd<> : FTrue { };
 
template <typename... Types>
struct TOr;

template <typename LHS, typename... RHS>
struct TOr<LHS, RHS...> : TBoolConstant<LHS::Value || TOr<RHS...>::Value> { };

template <>
struct TOr<> : FFalse { };
 
template <typename Type>
struct TNot : TBoolConstant<!Type::Value> { };

NAMESPACE_END(TypeTraits)
NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
