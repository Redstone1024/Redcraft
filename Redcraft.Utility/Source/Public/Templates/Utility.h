#pragma once

#include "CoreTypes.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
FORCEINLINE constexpr const T& AsConst(T& Ref)
{
	return Ref;
}

template <typename T>
void AsConst(const T&& Ref) = delete;

template <typename T>
FORCEINLINE constexpr TRemoveReference<T>&& MoveTemp(T&& Obj)
{
	using CastType = TRemoveReference<T>;
	return static_cast<CastType&&>(Obj);
}

template <typename T>
FORCEINLINE constexpr T CopyTemp(T& Obj)
{
	return const_cast<const T&>(Obj);
}

template <typename T>
FORCEINLINE constexpr T CopyTemp(const T& Obj)
{
	return Obj;
}

template <typename T>
FORCEINLINE constexpr T&& CopyTemp(T&& Obj)
{
	return MoveTemp(Obj);
}

template <typename T>
FORCEINLINE constexpr T&& Forward(TRemoveReference<T>& Obj)
{
	return static_cast<T&&>(Obj);
}

template <typename T>
FORCEINLINE constexpr T&& Forward(TRemoveReference<T>&& Obj)
{
	return static_cast<T&&>(Obj);
}

template <typename T> requires (requires(T& A, T& B) { A.Swap(B); }
	|| (CMoveConstructible<T> && CMoveAssignable<T>))
FORCEINLINE constexpr void Swap(T& A, T& B)
{
	if constexpr (requires(T& A, T& B) { A.Swap(B); })
	{
		A.Swap(B);
	}
	else
	{
		T Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
	}
}

template <typename T, typename U = T> requires (CMoveConstructible<T> && CAssignableFrom<T&, U>)
FORCEINLINE constexpr T Exchange(T& A, U&& B)
{
	T Temp = MoveTemp(A);
	A = Forward<U>(B);
	return Temp;
}

template <typename T>
TAddRValueReference<T> DeclVal();

template <typename T> requires (CObject<T>)
FORCEINLINE constexpr T* AddressOf(T& Object)
{
	return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(Object)));
}

template <typename T> requires (!CObject<T>)
FORCEINLINE constexpr T* AddressOf(T& Object)
{
	return &Object;
}

struct FIgnore
{
	template <typename T>
	FORCEINLINE constexpr void operator=(T&&) const { }
};

inline constexpr FIgnore Ignore;

// This macro is used in place of using type aliases, see Atomic.h, etc
#define STRONG_INHERIT(...) /* BaseClass */        \
	/* struct DerivedClass : */ public __VA_ARGS__ \
	{                                              \
	private:                                       \
		                                           \
		using BaseClassTypedef = __VA_ARGS__;      \
		                                           \
	public:                                        \
		                                           \
		using BaseClassTypedef::BaseClassTypedef;  \
		using BaseClassTypedef::operator=;         \
		                                           \
	}

// TOverloaded Usage Example
// 
//	Visit(TOverloaded {
//		[](auto A)           { ... },
//		[](double A)         { ... },
//		[](const FString& A) { ... },
//	}, Target);
//
template <typename... Ts>
struct TOverloaded : Ts...
{
	using Ts::operator()...;
};

template <typename... Ts>
TOverloaded(Ts...) -> TOverloaded<Ts...>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
