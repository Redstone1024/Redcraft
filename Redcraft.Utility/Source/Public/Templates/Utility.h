﻿#pragma once

#include "CoreTypes.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** Forms lvalue reference to const type of 'Ref'. */
template <typename T>
FORCEINLINE constexpr const T& AsConst(T& Ref)
{
	return Ref;
}

/** The const rvalue reference overload is deleted to disallow rvalue arguments. */
template <typename T>
void AsConst(const T&& Ref) = delete;

/** MoveTemp will cast a reference to an rvalue reference. */
template <typename T>
FORCEINLINE constexpr TRemoveReference<T>&& MoveTemp(T&& Obj)
{
	using CastType = TRemoveReference<T>;
	return static_cast<CastType&&>(Obj);
}

/** CopyTemp will enforce the creation of an rvalue which can bind to rvalue reference parameters. */
template <typename T>
FORCEINLINE constexpr T CopyTemp(T& Obj)
{
	return const_cast<const T&>(Obj);
}

/** CopyTemp will enforce the creation of an rvalue which can bind to rvalue reference parameters. */
template <typename T>
FORCEINLINE constexpr T CopyTemp(const T& Obj)
{
	return Obj;
}

/** CopyTemp will enforce the creation of an rvalue which can bind to rvalue reference parameters. */
template <typename T>
FORCEINLINE constexpr T&& CopyTemp(T&& Obj)
{
	// If we already have an rvalue, just return it unchanged, rather than needlessly creating yet another rvalue from it.
	return MoveTemp(Obj);
}

/** Forwards lvalues as either lvalues or as rvalues, depending on T. */
template <typename T>
FORCEINLINE constexpr T&& Forward(TRemoveReference<T>& Obj)
{
	return static_cast<T&&>(Obj);
}

/** Forwards lvalues as either lvalues or as rvalues, depending on T. */
template <typename T>
FORCEINLINE constexpr T&& Forward(TRemoveReference<T>&& Obj)
{
	return static_cast<T&&>(Obj);
}

/** Exchanges the given values. */
template <typename T> requires (CMoveConstructible<T> && CMoveAssignable<T>)
FORCEINLINE constexpr void Swap(T& A, T& B)
{
	T Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

/** Replaces the value of 'A' with 'B' and returns the old value of 'A'. */
template <typename T, typename U = T> requires (CMoveConstructible<T> && CAssignableFrom<T&, U>)
FORCEINLINE constexpr T Exchange(T& A, U&& B)
{
	T Temp = MoveTemp(A);
	A = Forward<U>(B);
	return Temp;
}

/**
 * Converts any type T to a reference type, making it possible to use member functions
 * in decltype expressions without the need to go through constructors.
 */
template <typename T>
TAddRValueReference<T> DeclVal();

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

struct FIgnore final
{
	template <typename T>
	FORCEINLINE constexpr void operator=(T&&) const { }
};

/**
 * An object of unspecified type such that any value can be assigned to it with no effect.
 * Intended for use with Tie when unpacking a TTuple, as placeholders for unused arguments
 * or using Ignore to avoid warnings about unused return values ​​from NODISCARD functions.
 */
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

/**
 * This class is used to create a set of overloaded functions.
 * 
 *	Visit(TOverloaded {
 *		[](auto A)           { ... },
 *		[](double A)         { ... },
 *		[](const FString& A) { ... },
 *	}, Target);
 */
template <typename... Ts>
struct TOverloaded final : Ts...
{
	using Ts::operator()...;
};

template <typename... Ts>
TOverloaded(Ts...) -> TOverloaded<Ts...>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
