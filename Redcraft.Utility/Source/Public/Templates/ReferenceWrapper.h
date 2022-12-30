#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/Optional.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * TReferenceWrapper is a class template that wraps a reference object.
 * It is frequently used as a mechanism to store references inside standard
 * containers which cannot normally hold references.
 */
template <typename ReferencedType> requires (CObject<ReferencedType> || CFunction<ReferencedType>)
class TReferenceWrapper final
{
public:

	using Type = ReferencedType;

	/** Constructs a new reference wrapper. */
	template <typename T = ReferencedType> requires (CConvertibleTo<T, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(T&& Object)
	{
		ReferencedType& Reference = Forward<T>(Object);
		Pointer = AddressOf(Reference);
	}

	/** Copies/moves content of other into a new instance. */
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper&) = default;
	FORCEINLINE constexpr TReferenceWrapper(TReferenceWrapper&&)      = default;

	/** Converting copy constructor. */
	template <typename T = ReferencedType> requires (CConvertibleTo<T&, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper<T>& InValue)
		: Pointer(InValue.Pointer)
	{ }

	/** Assign a value to the referenced object. */
	template <typename T = ReferencedType> requires (CAssignableFrom<ReferencedType&, T&&>)
	FORCEINLINE constexpr TReferenceWrapper& operator=(T&& Object) { Get() = Forward<T>(Object); return *this; }

	/** Remove the assignment operator, as rebinding is not allowed. */
	FORCEINLINE constexpr TReferenceWrapper& operator=(const TReferenceWrapper&) = delete;
	FORCEINLINE constexpr TReferenceWrapper& operator=(TReferenceWrapper&&)      = delete;
	
	/** @return The stored reference. */
	FORCEINLINE constexpr ReferencedType& Get()      const { return *Pointer; }
	FORCEINLINE constexpr operator ReferencedType&() const { return *Pointer; }

	/** Calls the Callable object, reference to which is stored. */
	template <typename... Ts>
	FORCEINLINE constexpr TInvokeResult<ReferencedType&, Ts...> operator()(Ts&&... Args) const
	{
		return Invoke(Get(), Forward<Ts>(Args)...);
	}

	/** Overloads the GetTypeHash algorithm for TReferenceWrapper. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TReferenceWrapper A) requires (CHashable<ReferencedType>)
	{
		return GetTypeHash(A.Get());
	}

	/** Overloads the Swap algorithm for TReferenceWrapper. */
	friend FORCEINLINE constexpr void Swap(TReferenceWrapper A, TReferenceWrapper B) requires (CSwappable<ReferencedType>)
	{
		Swap(A.Get(), B.Get());
	}

private:

	ReferencedType* Pointer;

	template <typename T> requires (CObject<T> || CFunction<T>) friend class TReferenceWrapper;

};

template <typename T>
TReferenceWrapper(T&) -> TReferenceWrapper<T>;

template <typename T>
void Ref(const T&&) = delete;

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<T> Ref(T& InValue)
{
	return TReferenceWrapper<T>(InValue);
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<const T> Ref(const T& InValue)
{
	return TReferenceWrapper<const T>(InValue);
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<const T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTReferenceWrapperImpl                       : FFalse { };
template <typename T> struct TIsTReferenceWrapperImpl<TReferenceWrapper<T>> : FTrue  { };

template <typename T> struct TUnwrapReferenceImpl                       { using Type = T;  };
template <typename T> struct TUnwrapReferenceImpl<TReferenceWrapper<T>> { using Type = T&; };

template <typename T> struct TUnwrapRefDecayImpl { using Type = typename TUnwrapReferenceImpl<TDecay<T>>::Type; };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTReferenceWrapper = NAMESPACE_PRIVATE::TIsTReferenceWrapperImpl<TRemoveCV<T>>::Value;

template <typename T>
using TUnwrapReference = typename NAMESPACE_PRIVATE::TUnwrapReferenceImpl<T>::Type;

template <typename T>
using TUnwrapRefDecay = typename NAMESPACE_PRIVATE::TUnwrapRefDecayImpl<T>::Type;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
