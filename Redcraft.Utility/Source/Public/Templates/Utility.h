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
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
	typedef typename TRemoveReference<T>::Type CastType;
	
	static_assert(TIsLValueReference<T>::Value, "MoveTemp called on an rvalue.");
	static_assert(!TIsConst<CastType>::Value, "MoveTemp called on a const object.");

	return (CastType&&)Obj;
}

template <typename T>
FORCEINLINE T CopyTemp(T& Val)
{
	return const_cast<const T&>(Val);
}

template <typename T>
FORCEINLINE T CopyTemp(const T& Val)
{
	return Val;
}

template <typename T>
FORCEINLINE T&& CopyTemp(T&& Val)
{
	return MoveTemp(Val);
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE void Swap(T& A, T& B)
{
	T Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

template <typename T>
FORCEINLINE void Exchange(T& A, T& B)
{
	Swap(A, B);
}

template <typename T>
T&& DeclVal();

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
