#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
class TReferenceWrapper
{
public:

	using Type = T;

	template <typename U> requires (!TIsSame<TReferenceWrapper, typename TRemoveCVRef<U>::Type>::Value)
	constexpr TReferenceWrapper(U&& Object) : Ptr(AddressOf(Forward<U>(Object))) { }

	TReferenceWrapper(const TReferenceWrapper&) = default;
	TReferenceWrapper& operator=(const TReferenceWrapper& x) = default;

	constexpr operator T&() const { return *Ptr; }
	constexpr T& Get() const { return *Ptr; }

	template <typename... Types>
	constexpr TInvokeResult<T&, Types...>::Type operator()(Types&&... Args) const
	{
		return Invoke(Get(), Forward<Types>(Args)...);
	}
	
private:

	T* Ptr;

};

template <typename T>
TReferenceWrapper(T&) -> TReferenceWrapper<T>;

template <typename T>
void Ref(const T&&) = delete;

template <typename T>
constexpr TReferenceWrapper<T> Ref(T& InValue)
{
	return TReferenceWrapper<T>(InValue);
}

template <typename T>
constexpr TReferenceWrapper<T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

template <typename T>
constexpr TReferenceWrapper<const T> Ref(const T& InValue)
{
	return TReferenceWrapper<const T>(InValue);
}

template <typename T>
constexpr TReferenceWrapper<const T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

template <typename T> struct TIsTReferenceWrapper                       : FFalse { };
template <typename T> struct TIsTReferenceWrapper<TReferenceWrapper<T>> : FTrue  { };

template <typename T> struct TUnwrapReference                       { using Type = T;  };
template <typename T> struct TUnwrapReference<TReferenceWrapper<T>> { using Type = T&; };

template <typename T> struct TUnwrapRefDecay { using Type = typename TUnwrapReference<typename TDecay<T>::Type>::Type; };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
