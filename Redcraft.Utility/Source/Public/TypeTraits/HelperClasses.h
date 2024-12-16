#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, T InValue>
struct TConstant
{
	using FValueType = T;
	using FType = TConstant;
	static constexpr FValueType Value = InValue;
	FORCEINLINE constexpr operator FValueType() const { return Value; }
	FORCEINLINE constexpr FValueType operator()() const { return Value; }
};

template <bool InValue>
using TBoolConstant = TConstant<bool, InValue>;

using FTrue  = TBoolConstant<true>;
using FFalse = TBoolConstant<false>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
