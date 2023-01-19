#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** The class template provides the standardized way to access certain properties of pointer-like types. */
template <typename T>
struct TPointerTraits
{
	static constexpr bool bIsPointer = false;
};

/** A specialization of TPointerTraits is provided for pointer types T*. */
template <typename T>
struct TPointerTraits<T*>
{
	static constexpr bool bIsPointer = true;

	using PointerType = T*;
	using ElementType = T;
	
	static FORCEINLINE constexpr ElementType* ToAddress(PointerType InPtr)
	{
		return InPtr;
	}
};

/** A specialization of TPointerTraits is provided for array pointer types T(*)[]. */
template <typename T>
struct TPointerTraits<T(*)[]>
{
	static constexpr bool bIsPointer = true;

	using PointerType = T(*)[];
	using ElementType = T;

	static FORCEINLINE constexpr ElementType* ToAddress(PointerType InPtr)
	{
		return InPtr;
	}
};

/** A specialization of TPointerTraits is provided for pointer-like type. */
#define DEFINE_TPointerTraits(TPtr)                                                   \
	template <typename T>                                                             \
	struct TPointerTraits<TPtr<T>>                                                    \
	{                                                                                 \
		static constexpr bool bIsPointer = true;                                      \
		                                                                              \
		using PointerType = TPtr<T>;                                                  \
		using ElementType = TPtr<T>::ElementType;                                     \
		                                                                              \
		static FORCEINLINE constexpr ElementType* ToAddress(const PointerType& InPtr) \
		{                                                                             \
			return InPtr.Get();                                                       \
		}                                                                             \
	};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
