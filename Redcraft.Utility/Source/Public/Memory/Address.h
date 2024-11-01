#pragma once

#include "CoreTypes.h"
#include "Memory/PointerTraits.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** Obtains a raw pointer from a pointer-like type. */
template <typename T> requires (TPointerTraits<T>::bIsPointer || requires(const T& Ptr) { Ptr.operator->(); })
constexpr auto ToAddress(const T& Ptr) noexcept
{
	if constexpr (TPointerTraits<T>::bIsPointer) {
		return TPointerTraits<T>::ToAddress(Ptr);
	}
	else {
		return ToAddress(Ptr.operator->());
	}
}

/** Obtains the actual address of the object or function arg, even in presence of overloaded operator&. */
template <typename T> requires (CObject<T>)
FORCEINLINE constexpr T* AddressOf(T& Object)
{
	return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(Object)));
}

/** Obtains the actual address of the object or function arg, even in presence of overloaded operator&. */
template <typename T> requires (!CObject<T>)
FORCEINLINE constexpr T* AddressOf(T& Object)
{
	return &Object;
}

/** Rvalue overload is deleted to prevent taking the address of const rvalues. */
template <typename T>
const T* AddressOf(const T&&) = delete;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
