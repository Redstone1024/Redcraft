#pragma once

#include "CoreTypes.h"
#include "Templates/Tuple.h"
#include "Templates/Utility.h"
#include "Memory/PointerTraits.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename RT, typename ST>
using TRawPointer = TConditional<CVoid<RT>, typename TPointerTraits<TRemoveCVRef<ST>>::FElementType*, RT>;

template <bool bEnableInput, typename RT, typename ST, typename... Ts>
class FInOutPtr final : private FSingleton
{
public:

	explicit FInOutPtr(ST& InPtr, Ts&&... Args) requires (!bEnableInput)
		: SPointer(InPtr), Tuple(nullptr, Forward<Ts>(Args)...)
	{ }

	explicit FInOutPtr(ST& InPtr, Ts&&... Args) requires (bEnableInput)
		: SPointer(InPtr), Tuple(InPtr.Release(), Forward<Ts>(Args)...)
	{ }

	~FInOutPtr()
	{
		if constexpr (requires(RT* RPtr, ST& SPtr, Ts&&... Args) { SPtr.Reset(RPtr, Forward<Ts>(Args)...); })
		{
			Tuple.Apply([this](Ts&&... Args) { SPointer.Reset(Forward<Ts>(Args)...); });
		}
		else if constexpr (CConstructibleFrom<ST, RT, Ts...> && CMoveAssignable<ST>)
		{
			SPointer = Tuple.template Construct<ST>();
		}
		else check_no_entry();
	}

	operator    RT*()                                { return &Tuple.First; }
	operator void**() requires (!CSameAs<void*, RT>) { return &Tuple.First; }

private:

	ST& SPointer;
	TTuple< RT, Ts&&...> Tuple;

};

NAMESPACE_PRIVATE_END

template <typename RT = void, typename ST, typename... Ts> requires ((CVoid<RT>) || (CPointer<RT>)
	&& (requires(NAMESPACE_PRIVATE::TRawPointer<RT, ST>* RPtr, ST& SPtr, Ts&&... Args) { SPtr.Reset(RPtr, Forward<Ts>(Args)...); })
	|| (CConstructibleFrom<ST, NAMESPACE_PRIVATE::TRawPointer<RT, ST>, Ts...> && CMoveAssignable<ST>)
	&& requires { typename TPointerTraits<TRemoveCV<ST>>::FElementType; })
auto OutPtr(ST& InPtr, Ts&&... Args)
{
	return NAMESPACE_PRIVATE::FInOutPtr<false, NAMESPACE_PRIVATE::TRawPointer<RT, ST>, ST, Ts...>(InPtr, Forward<Ts>(Args)...);
}

template <typename RT = void, typename ST, typename... Ts> requires ((CVoid<RT>) || (CPointer<RT>)
	&& (requires(NAMESPACE_PRIVATE::TRawPointer<RT, ST>* RPtr, ST& SPtr, Ts&&... Args) { SPtr.Reset(RPtr, Forward<Ts>(Args)...); })
	|| (CConstructibleFrom<ST, NAMESPACE_PRIVATE::TRawPointer<RT, ST>, Ts...> && CMoveAssignable<ST>)
	&& requires(ST& SPtr) { { SPtr.Release() } -> CConvertibleTo<NAMESPACE_PRIVATE::TRawPointer<RT, ST>>; }
	&& requires { typename TPointerTraits<TRemoveCV<ST>>::FElementType; })
auto InOutPtr(ST& InPtr, Ts&&... Args)
{
	return NAMESPACE_PRIVATE::FInOutPtr<true, NAMESPACE_PRIVATE::TRawPointer<RT, ST>, ST, Ts...>(InPtr, Forward<Ts>(Args)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
