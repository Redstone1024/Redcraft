#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** The class template provides the standardized way to access certain properties of pointer-like types. */
template <typename>
struct TPointerTraits
{
	static constexpr bool bIsPointer = false;
};

/** A specialization of TPointerTraits is provided for pointer types T*. */
template <typename T>
struct TPointerTraits<T*>
{
	static constexpr bool bIsPointer = true;

	using FPointerType = T*;
	using FElementType = T;

	static FORCEINLINE constexpr FElementType* ToAddress(FPointerType InPtr)
	{
		return InPtr;
	}
};

/** A specialization of TPointerTraits is provided for array pointer types T(*)[]. */
template <typename T>
struct TPointerTraits<T(*)[]>
{
	static constexpr bool bIsPointer = true;

	using FPointerType = T(*)[];
	using FElementType = T;

	static FORCEINLINE constexpr FElementType* ToAddress(FPointerType InPtr)
	{
		return InPtr;
	}
};

/** A specialization of TPointerTraits is provided for pointer-like type. */
#define DEFINE_TPointerTraits(TPtr)                                                     \
	template <typename T>                                                               \
	struct TPointerTraits<TPtr<T>>                                                      \
	{                                                                                   \
		static constexpr bool bIsPointer = true;                                        \
		                                                                                \
		using FPointerType = TPtr<T>;                                                   \
		using FElementType = TPtr<T>::FElementType;                                     \
		                                                                                \
		static FORCEINLINE constexpr FElementType* ToAddress(const FPointerType& InPtr) \
		{                                                                               \
			return InPtr.Get();                                                         \
		}                                                                               \
	};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
