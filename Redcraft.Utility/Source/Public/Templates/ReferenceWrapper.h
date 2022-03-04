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

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
