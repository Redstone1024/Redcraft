#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
constexpr const T& AsConst(T& Ref)
{
	return Ref;
}

template <typename T>
void AsConst(const T&& Ref) = delete;

template <typename T, size_t N>
constexpr const T(&AsConst(T(&Array)[N]))[N]
{
	return Array;
}

template <typename T>
constexpr typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
	typedef typename TRemoveReference<T>::Type CastType;
	return (CastType&&)Obj;
}

template <typename T>
constexpr T CopyTemp(T& Val)
{
	return const_cast<const T&>(Val);
}

template <typename T>
constexpr T CopyTemp(const T& Val)
{
	return Val;
}

template <typename T>
constexpr T&& CopyTemp(T&& Val)
{
	return MoveTemp(Val);
}

template <typename T>
constexpr T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
	return (T&&)Obj;
}

template <typename T>
constexpr T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
	return (T&&)Obj;
}

template <typename T> requires TIsMoveConstructible<T>::Value && TIsMoveAssignable<T>::Value
constexpr void Swap(T& A, T& B)
{
	T Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

template <typename T, typename U = T> requires TIsMoveConstructible<T>::Value && TIsAssignable<T&, U>::Value
constexpr T Exchange(T& A, U&& B)
{
	T Temp = MoveTemp(A);
	A = Forward<U>(B);
	return Temp;
}

template <typename T>
constexpr T&& DeclVal();

template <typename T> requires TIsObject<T>::Value
constexpr T* AddressOf(T& Object)
{
	return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(Object)));
}

template <typename T> requires (!TIsObject<T>::Value)
constexpr T* AddressOf(T& Object)
{
	return &Object;
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
