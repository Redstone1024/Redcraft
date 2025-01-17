#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#define ENABLE_ENUM_CLASS_BITWISE_OPERATIONS(Enum)                                                                                                                                         \
	NODISCARD FORCEINLINE constexpr Enum  operator| (Enum  LHS, Enum RHS) { return static_cast<Enum>(static_cast<TUnderlyingType<Enum>>(LHS) | static_cast<TUnderlyingType<Enum>>(RHS)); } \
	NODISCARD FORCEINLINE constexpr Enum  operator& (Enum  LHS, Enum RHS) { return static_cast<Enum>(static_cast<TUnderlyingType<Enum>>(LHS) & static_cast<TUnderlyingType<Enum>>(RHS)); } \
	NODISCARD FORCEINLINE constexpr Enum  operator^ (Enum  LHS, Enum RHS) { return static_cast<Enum>(static_cast<TUnderlyingType<Enum>>(LHS) ^ static_cast<TUnderlyingType<Enum>>(RHS)); } \
	          FORCEINLINE constexpr Enum& operator|=(Enum& LHS, Enum RHS) { LHS = LHS | RHS; return LHS; }                                                                                 \
	          FORCEINLINE constexpr Enum& operator&=(Enum& LHS, Enum RHS) { LHS = LHS & RHS; return LHS; }                                                                                 \
	          FORCEINLINE constexpr Enum& operator^=(Enum& LHS, Enum RHS) { LHS = LHS ^ RHS; return LHS; }                                                                                 \
	NODISCARD FORCEINLINE constexpr bool  operator! (Enum  E            ) { return                   !static_cast<TUnderlyingType<Enum>>(E);  }                                            \
	NODISCARD FORCEINLINE constexpr Enum  operator~ (Enum  E            ) { return static_cast<Enum>(~static_cast<TUnderlyingType<Enum>>(E)); }

#define FRIEND_ENUM_CLASS_BITWISE_OPERATIONS(Enum)  \
	friend constexpr Enum  operator| (Enum , Enum); \
	friend constexpr Enum  operator& (Enum , Enum); \
	friend constexpr Enum  operator^ (Enum , Enum); \
	friend constexpr Enum& operator|=(Enum&, Enum); \
	friend constexpr Enum& operator&=(Enum&, Enum); \
	friend constexpr Enum& operator^=(Enum&, Enum); \
	friend constexpr bool  operator! (Enum );       \
	friend constexpr Enum  operator~ (Enum );

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
