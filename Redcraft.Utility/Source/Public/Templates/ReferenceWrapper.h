#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/Optional.h"
#include "Templates/TypeHash.h"
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

	using FType = ReferencedType;

	/** Constructs a new reference wrapper. */
	template <typename T = ReferencedType&> requires (CConvertibleTo<T&&, ReferencedType&> && !CSameAs<TReferenceWrapper, TRemoveCVRef<T>>)
	FORCEINLINE constexpr TReferenceWrapper(T&& Object)
		: Pointer(AddressOf(static_cast<ReferencedType&>(Forward<T>(Object))))
	{ }

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper&) = default;

	/** Converting copy constructor. */
	template <typename T = ReferencedType> requires (CConvertibleTo<T&, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper<T>& InValue)
		: Pointer(InValue.Pointer)
	{ }

	/** Assigns by copying the content of others. */
	FORCEINLINE constexpr TReferenceWrapper& operator=(const TReferenceWrapper&) = default;

	/** Assigns by copying the content of others. */
	template <typename T = ReferencedType> requires (CConvertibleTo<T&, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper& operator=(const TReferenceWrapper<T>& InValue)
	{
		Pointer = InValue.Pointer;

		return *this;
	}

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

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTReferenceWrapperImpl                       : FFalse { };
template <typename T> struct TIsTReferenceWrapperImpl<TReferenceWrapper<T>> : FTrue  { };

template <typename T> struct TUnwrapReferenceImpl                       { using FType = T;  };
template <typename T> struct TUnwrapReferenceImpl<TReferenceWrapper<T>> { using FType = T&; };

template <typename T> struct TUnwrapRefDecayImpl { using FType = typename TUnwrapReferenceImpl<TDecay<T>>::FType; };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTReferenceWrapper = NAMESPACE_PRIVATE::TIsTReferenceWrapperImpl<TRemoveCV<T>>::Value;

template <typename T>
using TUnwrapReference = typename NAMESPACE_PRIVATE::TUnwrapReferenceImpl<T>::FType;

template <typename T>
using TUnwrapRefDecay = typename NAMESPACE_PRIVATE::TUnwrapRefDecayImpl<T>::FType;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
