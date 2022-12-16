#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/Optional.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename ReferencedType> requires (CObject<ReferencedType> || CFunction<ReferencedType>)
class TReferenceWrapper
{
public:

	using Type = ReferencedType;

	template <typename T = ReferencedType> requires (CConvertibleTo<T, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(T&& Object)
	{
		ReferencedType& Reference = Forward<T>(Object);
		Pointer = AddressOf(Reference);
	}

	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper&) = default;
	FORCEINLINE constexpr TReferenceWrapper(TReferenceWrapper&&)      = default;
	
	template <typename T = ReferencedType> requires (CConvertibleTo<T&, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper<T>& InValue)
		: Pointer(InValue.Pointer)
	{ }

	template <typename T = ReferencedType> requires (CAssignableFrom<ReferencedType&, T&&>)
	FORCEINLINE constexpr TReferenceWrapper& operator=(T&& Object) { Get() = Forward<T>(Object); return *this; }

	FORCEINLINE constexpr TReferenceWrapper& operator=(const TReferenceWrapper&) = delete;
	FORCEINLINE constexpr TReferenceWrapper& operator=(TReferenceWrapper&&)      = delete;
	
	FORCEINLINE constexpr operator ReferencedType&() const { return *Pointer; }
	FORCEINLINE constexpr ReferencedType& Get()      const { return *Pointer; }

	template <typename... Ts>
	FORCEINLINE constexpr TInvokeResult<ReferencedType&, Ts...> operator()(Ts&&... Args) const
	{
		return Invoke(Get(), Forward<Ts>(Args)...);
	}

	FORCEINLINE constexpr size_t GetTypeHash() const requires (CHashable<ReferencedType>)
	{
		return NAMESPACE_REDCRAFT::GetTypeHash(Get());
	}
	
	FORCEINLINE constexpr void Swap(TReferenceWrapper& InValue)
	{
		ReferencedType* Temp = Pointer;
		Pointer = InValue.Pointer;
		InValue.Pointer = Temp;
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
