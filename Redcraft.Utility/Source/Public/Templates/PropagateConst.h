#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/PointerTraits.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> requires (TPointerTraits<T>::bIsPointer)
class TPropagateConst;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTPropagateConst                     : FFalse { };
template <typename T> struct TIsTPropagateConst<TPropagateConst<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTPropagateConst = NAMESPACE_PRIVATE::TIsTPropagateConst<TRemoveCV<T>>::Value;

/**
 * TPropagateConst is a const-propagating wrapper for pointers and pointer-like objects.
 * It treats the wrapped pointer as a pointer to const when accessed through a const access path, hence the name.
 */
template <typename T> requires (TPointerTraits<T>::bIsPointer)
class TPropagateConst final
{
public:

	using FElementType = TPointerTraits<T>::FElementType;

	/** Constructs an TPropagateConst, default-initializing underlying pointer. */
	FORCEINLINE constexpr TPropagateConst() = default;

	/** Initializes underlying pointer if by direct-non-list-initialization with the expression Forward<U>(InValue). */
	template <typename U> requires (CConstructibleFrom<T, U> && !CTPropagateConst<TRemoveCVRef<U>>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U, T>) TPropagateConst(U&& InValue)
		: Ptr(Forward<U>(InValue))
	{ }

	/** Explicitly defaulted copy/move constructor that copy/move constructs underlying pointer. */
	FORCEINLINE constexpr TPropagateConst(const TPropagateConst&) = default;
	FORCEINLINE constexpr TPropagateConst(TPropagateConst&&)      = default;

	/** Initializes underlying pointer as if by direct-non-list-initialization. */
	template <typename U> requires (CConstructibleFrom<T, const U&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const U&, T>) TPropagateConst(const TPropagateConst<U>& InValue)
		: Ptr(InValue.Ptr)
	{ }

	/** Initializes underlying pointer as if by direct-non-list-initialization. */
	template <typename U> requires (CConstructibleFrom<T, U&&>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&&, T>) TPropagateConst(TPropagateConst<U>&& InValue)
		: Ptr(MoveTemp(InValue.Ptr))
	{ }

	/** Destructs an TPropagateConst, destroying the contained underlying pointer */
	FORCEINLINE constexpr ~TPropagateConst() = default;

	/** Explicitly defaulted copy/move assignment operator that copy/move assigns underlying pointer. */
	FORCEINLINE constexpr TPropagateConst& operator=(const TPropagateConst& InValue) = default;
	FORCEINLINE constexpr TPropagateConst& operator=(TPropagateConst&& InValue)      = default;

	/** Assigns underlying pointer from 'InValue'. */
	template <typename U> requires (CAssignableFrom<T, const U&>)
	FORCEINLINE constexpr TPropagateConst& operator=(const TPropagateConst<T>& InValue)
	{
		Ptr = InValue.Ptr;
		return *this;
	}

	/** Assigns underlying pointer from 'InValue'. */
	template <typename U> requires (CAssignableFrom<T, U&&>)
	FORCEINLINE constexpr TPropagateConst& operator=(TPropagateConst<T>&& InValue)
	{
		Ptr = MoveTemp(InValue.Ptr);
		return *this;
	}

	/** Assigns underlying pointer from 'InValue'. */
	template <typename U> requires (CConvertibleTo<U, T>)
	FORCEINLINE constexpr TPropagateConst& operator=(U&& InValue)
	{
		Ptr = Forward<U>(InValue);
		return *this;
	}

	/** Compares the pointer values of two TPropagateConst. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TPropagateConst& LHS, const TPropagateConst& RHS) requires (CWeaklyEqualityComparable<T>) { return LHS.Ptr == RHS.Ptr; }

	/** Compares the pointer values of two TPropagateConst. */
	NODISCARD friend FORCEINLINE constexpr decltype(auto) operator<=>(const TPropagateConst& LHS, const TPropagateConst& RHS) requires (CSynthThreeWayComparable<T>) { return SynthThreeWayCompare(LHS.Ptr, RHS.Ptr); }

	/** Compares the pointer values with a underlying pointer. */
	template <typename U> requires (CWeaklyEqualityComparable<T, U>)
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Ptr == InPtr; }

	/** Compares the pointer values with a underlying pointer. */
	template <typename U> requires (CSynthThreeWayComparable<T, U>)
	NODISCARD FORCEINLINE constexpr decltype(auto) operator<=>(U InPtr) const& { return SynthThreeWayCompare(Ptr, InPtr); }

	/** @return The pointer to the object pointed to by the wrapped pointer. */
	NODISCARD FORCEINLINE constexpr       FElementType* Get()       { return TPointerTraits<T>::ToAddress(Ptr); }
	NODISCARD FORCEINLINE constexpr const FElementType* Get() const { return TPointerTraits<T>::ToAddress(Ptr); }

	/** @return true if *this owns an object, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	/** @return The a reference or pointer to the object owned by *this, i.e. Get(). */
	NODISCARD FORCEINLINE constexpr       FElementType& operator*()        { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return *Get(); }
	NODISCARD FORCEINLINE constexpr const FElementType& operator*()  const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return *Get(); }
	NODISCARD FORCEINLINE constexpr       FElementType* operator->()       { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return  Get(); }
	NODISCARD FORCEINLINE constexpr const FElementType* operator->() const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return  Get(); }

	/** @return The element at index, i.e. Get()[Index]. */
	NODISCARD FORCEINLINE constexpr       T& operator[](size_t Index)       { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return Get()[Index]; }
	NODISCARD FORCEINLINE constexpr const T& operator[](size_t Index) const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return Get()[Index]; }

	/** @return The pointer to the object pointed to by the wrapped pointer-like object. */
	NODISCARD FORCEINLINE constexpr operator       FElementType*()       requires (CConvertibleTo<T, FElementType*>) { return Ptr; }
	NODISCARD FORCEINLINE constexpr operator const FElementType*() const requires (CConvertibleTo<T, FElementType*>) { return Ptr; }

	/** @return The reference to the pointer-like object stored. */
	NODISCARD FORCEINLINE constexpr       T& GetUnderlying()       { return Ptr; }
	NODISCARD FORCEINLINE constexpr const T& GetUnderlying() const { return Ptr; }

	/** Overloads the GetTypeHash algorithm for TPropagateConst. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TPropagateConst& A) requires (CHashable<T>)
	{
		return GetTypeHash(A.Ptr);
	}

	/** Overloads the Swap algorithm for TPropagateConst. */
	friend FORCEINLINE constexpr void Swap(TPropagateConst& A, TPropagateConst& B) requires (CSwappable<T>)
	{
		Swap(A.Ptr, B.Ptr);
	}

private:

	T Ptr;

};

template <typename T>
TPropagateConst(T) -> TPropagateConst<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
