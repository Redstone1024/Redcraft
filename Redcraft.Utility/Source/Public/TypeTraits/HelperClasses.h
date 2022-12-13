#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename InType, InType InValue>
struct TConstant
{
	using ValueType = InType;
	using Type = TConstant;
	static constexpr ValueType Value = InValue;
	FORCEINLINE constexpr operator ValueType() const { return Value; }
	FORCEINLINE constexpr ValueType operator()() const { return Value; }
};

template <bool InValue>
using TBoolConstant = TConstant<bool, InValue>;

using FTrue  = TBoolConstant<true>;
using FFalse = TBoolConstant<false>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
