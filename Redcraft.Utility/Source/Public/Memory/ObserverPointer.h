#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/PointerTraits.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TObserverPtr;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTObserverPtr                  : FFalse { };
template <typename T> struct TIsTObserverPtr<TObserverPtr<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTObserverPtr = NAMESPACE_PRIVATE::TIsTObserverPtr<TRemoveCV<T>>::Value;

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TObserverPtr
{
public:

	using ElementType = T;

	FORCEINLINE constexpr TObserverPtr() : Pointer(nullptr) { }

	FORCEINLINE constexpr TObserverPtr(nullptr_t) : TObserverPtr() { }

	FORCEINLINE constexpr explicit TObserverPtr(T* InPtr) : Pointer(InPtr) { }

	FORCEINLINE constexpr TObserverPtr(const TObserverPtr&) = default;
	FORCEINLINE constexpr TObserverPtr(TObserverPtr&&)      = default;

	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE constexpr TObserverPtr(TObserverPtr<U> InValue) : Pointer(InValue.Pointer) { }

	FORCEINLINE constexpr ~TObserverPtr() = default;

	FORCEINLINE constexpr TObserverPtr& operator=(const TObserverPtr&) = default;
	FORCEINLINE constexpr TObserverPtr& operator=(TObserverPtr&&)      = default;

	NODISCARD friend FORCEINLINE constexpr bool operator==(const TObserverPtr& LHS, const TObserverPtr& RHS) { return LHS.Get() == RHS.Get(); }

	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TObserverPtr& LHS, const TObserverPtr& RHS) { return LHS.Get() <=> RHS.Get(); }

	NODISCARD FORCEINLINE constexpr bool operator==(T* InPtr) const& { return Get() == InPtr; }

	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(T* InPtr) const& { return Get() <=> InPtr; }

	NODISCARD FORCEINLINE constexpr T* Release() { return Exchange(Pointer, nullptr); }

	FORCEINLINE constexpr void Reset(T* InPtr = nullptr) { Pointer = InPtr; }

	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	NODISCARD FORCEINLINE constexpr T& operator*()  const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return *Get(); }
	NODISCARD FORCEINLINE constexpr T* operator->() const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return  Get(); }

	NODISCARD FORCEINLINE constexpr operator       ElementType*()       { return Get(); }
	NODISCARD FORCEINLINE constexpr operator const ElementType*() const { return Get(); }

	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TObserverPtr& A) { return GetTypeHash(A.Get()); }

	friend FORCEINLINE constexpr void Swap(TObserverPtr& A, TObserverPtr& B) { Swap(A.Pointer, B.Pointer); }

private:

	T* Pointer;

};

template <typename T>
class TObserverPtr<T[]>
{
public:

	using ElementType = T;

	FORCEINLINE constexpr TObserverPtr() : Pointer(nullptr) { }

	FORCEINLINE constexpr TObserverPtr(nullptr_t) : TObserverPtr() { }
	
	template <typename U = T*> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE constexpr explicit TObserverPtr(U InPtr) : Pointer(InPtr) { }

	FORCEINLINE constexpr TObserverPtr(const TObserverPtr&) = default;
	FORCEINLINE constexpr TObserverPtr(TObserverPtr&&)      = default;
	
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE constexpr TObserverPtr(TObserverPtr<U> InValue) : Pointer(InValue.Pointer) { }

	FORCEINLINE constexpr ~TObserverPtr() = default;

	FORCEINLINE constexpr TObserverPtr& operator=(const TObserverPtr&) = default;
	FORCEINLINE constexpr TObserverPtr& operator=(TObserverPtr&&)      = default;

	NODISCARD friend FORCEINLINE constexpr bool operator==(const TObserverPtr& LHS, const TObserverPtr& RHS) { return LHS.Get() == RHS.Get(); }

	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TObserverPtr& LHS, const TObserverPtr& RHS) { return LHS.Get() <=> RHS.Get(); }
	
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Get() == InPtr; }

	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(U InPtr) const& { return Get() <=> InPtr; }

	NODISCARD FORCEINLINE constexpr T* Release() { return Exchange(Pointer, nullptr); }

	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr void Reset(U InPtr = nullptr) { Pointer = InPtr; }

	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	NODISCARD FORCEINLINE constexpr T& operator[](size_t Index) const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return Get()[Index]; }

	NODISCARD FORCEINLINE constexpr operator       ElementType*()       { return Get(); }
	NODISCARD FORCEINLINE constexpr operator const ElementType*() const { return Get(); }

	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TObserverPtr& A) { return GetTypeHash(A.Get()); }

	friend FORCEINLINE constexpr void Swap(TObserverPtr& A, TObserverPtr& B) { Swap(A.Pointer, B.Pointer); }

private:

	T* Pointer;

};

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
NODISCARD FORCEINLINE constexpr TObserverPtr<T> MakeObserver(TRemoveExtent<T>* InPtr) { return TObserverPtr<T>(InPtr); }

DEFINE_TPointerTraits(TObserverPtr);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
