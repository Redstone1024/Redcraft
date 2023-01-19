#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/PointerTraits.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T, typename E, bool = CEmpty<E> && !CFinal<E>>
class TUniqueStorage;

template <typename T, typename E>
class TUniqueStorage<T, E, true> : private E
{
public:

	FORCEINLINE constexpr TUniqueStorage() = delete;

	FORCEINLINE constexpr TUniqueStorage(T* InPtr) : E(), Pointer(InPtr) { }

	template<typename U>
	FORCEINLINE constexpr TUniqueStorage(T* InPtr, U&& InDeleter) : E(Forward<U>(InDeleter)), Pointer(InPtr) { }

	FORCEINLINE constexpr TUniqueStorage(const TUniqueStorage&)            = delete;
	FORCEINLINE constexpr TUniqueStorage(TUniqueStorage&& InValue)         = default;
	FORCEINLINE constexpr TUniqueStorage& operator=(const TUniqueStorage&) = delete;
	FORCEINLINE constexpr TUniqueStorage& operator=(TUniqueStorage&&)      = default;

	FORCEINLINE constexpr       T*& GetPointer()       { return Pointer; }
	FORCEINLINE constexpr       T*  GetPointer() const { return Pointer; }
	FORCEINLINE constexpr       E&  GetDeleter()       { return *this;   }
	FORCEINLINE constexpr const E&  GetDeleter() const { return *this;   }

private:

	// NOTE: NO_UNIQUE_ADDRESS is not valid in MSVC, use base class instead of member variable
	//NO_UNIQUE_ADDRESS E Deleter;

	T* Pointer;

};

template <typename T, typename E>
class TUniqueStorage<T, E, false>
{
public:

	FORCEINLINE constexpr TUniqueStorage() = delete;

	FORCEINLINE constexpr TUniqueStorage(T* InPtr) : E(), Pointer(InPtr) { }

	template<typename U>
	FORCEINLINE constexpr TUniqueStorage(T* InPtr, U&& InDeleter) : Pointer(InPtr), Deleter(Forward<U>(InDeleter)) { }

	FORCEINLINE constexpr TUniqueStorage(const TUniqueStorage&)            = delete;
	FORCEINLINE constexpr TUniqueStorage(TUniqueStorage&& InValue)         = default;
	FORCEINLINE constexpr TUniqueStorage& operator=(const TUniqueStorage&) = delete;
	FORCEINLINE constexpr TUniqueStorage& operator=(TUniqueStorage&&)      = default;

	FORCEINLINE constexpr       T*& GetPointer()       { return Pointer; }
	FORCEINLINE constexpr       T*  GetPointer() const { return Pointer; }
	FORCEINLINE constexpr       E&  GetDeleter()       { return Deleter; }
	FORCEINLINE constexpr const E&  GetDeleter() const { return Deleter; }

private:

	T* Pointer;
	E  Deleter;

};

NAMESPACE_PRIVATE_END

/** TDefaultDelete is the default destruction policy used by TUniquePtr when no deleter is specified. */
template <typename T> requires ((!CArray<T> && requires(T* Ptr) { delete Ptr; })
	|| (CArray<T> && requires(TRemoveExtent<T>* Ptr) { delete [] Ptr; }))
struct TDefaultDelete
{
	/** Constructs a TDefaultDelete object. */
	FORCEINLINE constexpr TDefaultDelete() = default;

	/** Constructs a TDefaultDelete object from another TDefaultDelete object. */
	template <typename U> requires (CConvertibleTo<U*, T*>)
	FORCEINLINE constexpr TDefaultDelete(TDefaultDelete<U>) { }

	/** Calls delete on ptr. */
	FORCEINLINE constexpr void operator()(T* Ptr) const
	{
		static_assert(!CVoid<T>,     "Can't delete pointer to incomplete type");
		static_assert(sizeof(T) > 0, "Can't delete pointer to incomplete type");
		delete Ptr;
	}

};

/** TDefaultDelete is the default destruction policy used by TUniquePtr when no deleter is specified. */
template <typename T>
struct TDefaultDelete<T[]>
{
	/** Constructs a TDefaultDelete object. */
	FORCEINLINE constexpr TDefaultDelete() = default;

	/** Constructs a TDefaultDelete object from another TDefaultDelete object. */
	template <typename U> requires (CConvertibleTo<U(*)[], T(*)[]>)
	FORCEINLINE constexpr TDefaultDelete(TDefaultDelete<U[]>) { }

	/** Calls delete [] on ptr. */
	FORCEINLINE constexpr void operator()(T* Ptr) const
	{
		static_assert(!CVoid<T>,     "Can't delete pointer to incomplete type");
		static_assert(sizeof(T) > 0, "Can't delete pointer to incomplete type");
		delete [] Ptr;
	}

};

template <typename T, CInvocable<TRemoveExtent<T>*> E = TDefaultDelete<T>> requires (CObject<T> && !CBoundedArray<T> && (CDestructible<E> || CLValueReference<E>))
class TUniqueRef;

template <typename T, CInvocable<TRemoveExtent<T>*> E = TDefaultDelete<T>> requires (CObject<T> && !CBoundedArray<T> && (CDestructible<E> || CLValueReference<E>))
class TUniquePtr;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTUniqueRef                : FFalse { };
template <typename T> struct TIsTUniqueRef<TUniqueRef<T>> : FTrue  { };

template <typename T> struct TIsTUniquePtr                : FFalse { };
template <typename T> struct TIsTUniquePtr<TUniquePtr<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTUniqueRef = NAMESPACE_PRIVATE::TIsTUniqueRef<TRemoveCV<T>>::Value;

template <typename T>
concept CTUniquePtr = NAMESPACE_PRIVATE::TIsTUniquePtr<TRemoveCV<T>>::Value;

/** This is essentially a reference version of TUniquePtr. */
template <typename T, CInvocable<TRemoveExtent<T>*> E> requires (CObject<T> && !CBoundedArray<T> && (CDestructible<E> || CLValueReference<E>))
class TUniqueRef final : private FSingleton
{
public:

	using ElementType = T;
	using DeleterType = E;

	/** TUniqueRef cannot be initialized by nullptr. */
	TUniqueRef() = delete;

	/** TUniqueRef cannot be initialized by nullptr. */
	TUniqueRef(nullptr_t) = delete;

	/** Constructs a TUniqueRef which owns 'InPtr', initializing the stored pointer with 'InPtr' and value-initializing the stored deleter. */
	FORCEINLINE constexpr explicit TUniqueRef(T* InPtr) requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
	}

	/** Constructs a TUniqueRef object which owns 'InPtr', initializing the stored pointer with 'InPtr' and initializing a deleter 'InDeleter' */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr TUniqueRef(T* InPtr, InE&& InDeleter) : Storage(InPtr, Forward<InE>(InDeleter))
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
	}

	/** Destroy the owned object. */
	FORCEINLINE constexpr ~TUniqueRef() { Invoke(GetDeleter(), Get()); }

	/** Compares the pointer values of two TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TUniqueRef& LHS, const TUniqueRef& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TUniqueRef& LHS, const TUniqueRef& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr bool operator==(T* InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(T* InPtr) const& { return Get() <=> InPtr; }

	/** TUniqueRef cannot be reset to nullptr. */
	void Reset(nullptr_t) = delete;

	/** Replaces the managed object. */
	FORCEINLINE constexpr void Reset(T* InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
		Invoke(GetDeleter(), Get());
		Storage.GetPointer() = InPtr;
	}

	/** TUniqueRef cannot be reset to nullptr. */
	template <typename InE>
	void Reset(nullptr_t, InE&&) = delete;

	/** Replaces the managed object. */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr void Reset(T* InPtr, InE&& InDeleter)
	{
		Reset(InPtr);
		GetDeleter() = Forward<InE>(InDeleter);
	}

	/** TUniqueRef cannot be reset to nullptr. */
	T* ReleaseAndReset(nullptr_t) = delete;

	/** Equivalent to Release() then Reset(InPtr). */
	FORCEINLINE constexpr T* ReleaseAndReset(T* InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
		return Exchange(Storage.GetPointer(), InPtr);
	}

	/** TUniqueRef cannot be reset to nullptr. */
	template <typename InE>
	T* ReleaseAndReset(nullptr_t, InE&&) = delete;

	/** Equivalent to Release() then Reset(InPtr, Forward<InE>(InDeleter)). */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr T* ReleaseAndReset(T* InPtr, InE&& InDeleter)
	{
		GetDeleter() = Forward<InE>(InDeleter);
		return ReleaseAndReset(InPtr);
	}

	/** @return The pointer to the managed object. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Storage.GetPointer(); }

	/** @return The deleter that is used for destruction of the managed object. */
	NODISCARD FORCEINLINE constexpr       E& GetDeleter()       { return Storage.GetDeleter(); }
	NODISCARD FORCEINLINE constexpr const E& GetDeleter() const { return Storage.GetDeleter(); }

	/** @return The a reference or pointer to the object owned by *this, i.e. Get(). */
	NODISCARD FORCEINLINE constexpr T& operator*()  const { return *Get(); }
	NODISCARD FORCEINLINE constexpr T* operator->() const { return  Get(); }

	/** Overloads the GetTypeHash algorithm for TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TUniqueRef& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TUniqueRef. */
	friend FORCEINLINE constexpr void Swap(TUniqueRef& A, TUniqueRef& B) requires (!CReference<E> && CSwappable<E>)
	{
		Swap(A.Storage.GetPointer(), B.Storage.GetPointer());
		Swap(A.Storage.GetDeleter(), B.Storage.GetDeleter());
	}

private:

	NAMESPACE_PRIVATE::TUniqueStorage<T, E> Storage;

};

template <typename T>
TUniqueRef(T*) -> TUniqueRef<T>;

/** This is essentially a reference version of TUniquePtr. */
template <typename T, typename E>
class TUniqueRef<T[], E> final : private FSingleton
{
public:

	using ElementType = T;
	using DeleterType = E;

	/** TUniqueRef cannot be initialized by nullptr. */
	TUniqueRef() = delete;

	/** TUniqueRef cannot be initialized by nullptr. */
	TUniqueRef(nullptr_t) = delete;

	/** Constructs a TUniqueRef which owns 'InPtr', initializing the stored pointer with 'InPtr' and value-initializing the stored deleter. */
	template <typename U = T> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE constexpr explicit TUniqueRef(U InPtr) requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
	}

	/** Constructs a TUniqueRef object which owns 'InPtr', initializing the stored pointer with 'InPtr' and initializing a deleter 'InDeleter' */
	template <typename U = T, typename InE> requires (CConvertibleTo<InE, E>
		&& (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr TUniqueRef(U InPtr, InE&& InDeleter) : Storage(InPtr, Forward<InE>(InDeleter))
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
	}

	/** Destroy the owned array. */
	FORCEINLINE constexpr ~TUniqueRef() { Invoke(GetDeleter(), Get()); }

	/** Compares the pointer values of two TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TUniqueRef& LHS, const TUniqueRef& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TUniqueRef& LHS, const TUniqueRef& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(U InPtr) const& { return Get() <=> InPtr; }

	/** TUniqueRef cannot be reset to nullptr. */
	void Reset(nullptr_t) = delete;

	/** Replaces the managed array. */
	template <typename U> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE constexpr void Reset(U InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
		Invoke(GetDeleter(), Get());
		Storage.GetPointer() = InPtr;
	}

	/** TUniqueRef cannot be reset to nullptr. */
	template <typename InE>
	void Reset(nullptr_t, InE&&) = delete;

	/** Replaces the managed array. */
	template <typename U, typename InE> requires (CConvertibleTo<InE, E>
		&& (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr void Reset(U InPtr, InE&& InDeleter)
	{
		Reset(InPtr);
		GetDeleter() = Forward<InE>(InDeleter);
	}

	/** TUniqueRef cannot be reset to nullptr. */
	T* ReleaseAndReset(nullptr_t) = delete;

	/** Equivalent to Release() then Reset(InPtr). */
	template <typename U> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE constexpr T* ReleaseAndReset(U InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TUniqueRef cannot be initialized by nullptr. Please use TUniquePtr."));
		return Exchange(Storage.GetPointer(), InPtr);
	}

	/** TUniqueRef cannot be reset to nullptr. */
	template <typename InE>
	T* ReleaseAndReset(nullptr_t, InE&&) = delete;

	/** Equivalent to Release() then Reset(InPtr, Forward<InE>(InDeleter)). */
	template <typename U, typename InE> requires (CConvertibleTo<InE, E>
		&& (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr T* ReleaseAndReset(U InPtr, InE&& InDeleter)
	{
		GetDeleter() = Forward<InE>(InDeleter);
		return ReleaseAndReset(InPtr);
	}

	/** @return The pointer to the managed array. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Storage.GetPointer(); }

	/** @return The deleter that is used for destruction of the managed array. */
	NODISCARD FORCEINLINE constexpr       E& GetDeleter()       { return Storage.GetDeleter(); }
	NODISCARD FORCEINLINE constexpr const E& GetDeleter() const { return Storage.GetDeleter(); }

	/** @return The element at index, i.e. Get()[Index]. */
	NODISCARD FORCEINLINE constexpr T& operator[](size_t Index) const { return Get()[Index]; }

	/** Overloads the GetTypeHash algorithm for TUniqueRef. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TUniqueRef& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TUniqueRef. */
	friend FORCEINLINE constexpr void Swap(TUniqueRef& A, TUniqueRef& B) requires (!CReference<E> && CSwappable<E>)
	{
		Swap(A.Storage.GetPointer(), B.Storage.GetPointer());
		Swap(A.Storage.GetDeleter(), B.Storage.GetDeleter());
	}

private:

	NAMESPACE_PRIVATE::TUniqueStorage<T, E> Storage;

};

/** Single-ownership smart pointer. Use this when you need an object's lifetime to be strictly bound to the lifetime of a single smart pointer. */
template <typename T, CInvocable<TRemoveExtent<T>*> E> requires (CObject<T> && !CBoundedArray<T> && (CDestructible<E> || CLValueReference<E>))
class TUniquePtr final : private FNoncopyable
{
public:

	using ElementType = T;
	using DeleterType = E;

	/** Constructs a TUniquePtr that owns nothing. Value-initializes the stored pointer and the stored deleter. */
	FORCEINLINE constexpr TUniquePtr() requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(nullptr) { }

	/** Constructs a TUniquePtr that owns nothing. Value-initializes the stored pointer and the stored deleter. */
	FORCEINLINE constexpr TUniquePtr(nullptr_t) requires(CDefaultConstructible<E> && !CPointer<E>) : TUniquePtr() { }

	/** Constructs a TUniquePtr which owns 'InPtr', initializing the stored pointer with 'InPtr' and value-initializing the stored deleter. */
	FORCEINLINE constexpr explicit TUniquePtr(T* InPtr) requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(InPtr) { }

	/** Constructs a TUniquePtr object which owns 'InPtr', initializing the stored pointer with 'InPtr' and initializing a deleter 'InDeleter' */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr TUniquePtr(T* InPtr, InE&& InDeleter) : Storage(InPtr, Forward<InE>(InDeleter)) { }

	/** Constructs a TUniquePtr by transferring ownership from 'InValue' to *this and stores the nullptr in 'InValue'. */
	FORCEINLINE constexpr TUniquePtr(TUniquePtr&& InValue) : Storage(InValue.Release(), Forward<E>(InValue.GetDeleter())) { }

	/** Constructs a TUniquePtr by transferring ownership from 'InValue' to *this and stores the nullptr in 'InValue'. */
	template <typename U, typename InE> requires (CConvertibleTo<U*, T*> && !CArray<U>
		&& ((CReference<E> && CSameAs<InE, E>) || (!CReference<E> && CConvertibleTo<InE, E>)))
	FORCEINLINE constexpr TUniquePtr(TUniquePtr<U, InE>&& InValue) : Storage(InValue.Release(), Forward<InE>(InValue.GetDeleter())) { }

	/** If !IsValid() there are no effects. Otherwise, the owned object is destroyed. */
	FORCEINLINE constexpr ~TUniquePtr() { if (IsValid()) Invoke(GetDeleter(), Get()); }

	/** Move assignment operator. Transfers ownership from 'InValue' to *this. */
	FORCEINLINE constexpr TUniquePtr& operator=(TUniquePtr&& InValue) requires (!CReference<E> && CAssignableFrom<E&, E&&>)
	{
		Reset(InValue.Release());
		GetDeleter() = Forward<E>(InValue.GetDeleter());
		return *this;
	}

	/** Move assignment operator. Transfers ownership from 'InValue' to *this. */
	template <typename U, typename InE> requires (CConvertibleTo<U*, T*>
		&& !CArray<U> && !CReference<E> && CAssignableFrom<E&, InE&&>)
	FORCEINLINE constexpr TUniquePtr& operator=(TUniquePtr<U, InE>&& InValue)
	{
		Reset(InValue.Release());
		GetDeleter() = Forward<InE>(InValue.GetDeleter());
		return *this;
	}

	/** Effectively the same as calling Reset(). */
	FORCEINLINE constexpr TUniquePtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Compares the pointer values of two TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TUniquePtr& LHS, const TUniquePtr& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TUniquePtr& LHS, const TUniquePtr& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr bool operator==(T* InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(T* InPtr) const& { return Get() <=> InPtr; }

	/** Returns a pointer to the managed object and releases the ownership. */
	NODISCARD FORCEINLINE constexpr T* Release() { return Exchange(Storage.GetPointer(), nullptr); }

	/** Replaces the managed object. */
	FORCEINLINE constexpr void Reset(T* InPtr = nullptr)
	{
		if (IsValid()) Invoke(GetDeleter(), Get());
		Storage.GetPointer() = InPtr;
	}

	/** Replaces the managed object. */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr void Reset(T* InPtr, InE&& InDeleter)
	{
		Reset(InPtr);
		GetDeleter() = Forward<InE>(InDeleter);
	}

	/** Equivalent to Release() then Reset(InPtr). */
	FORCEINLINE constexpr T* ReleaseAndReset(T* InPtr = nullptr)
	{
		return Exchange(Storage.GetPointer(), InPtr);
	}

	/** Equivalent to Release() then Reset(InPtr, Forward<InE>(InDeleter)). */
	template <typename InE> requires (CConvertibleTo<InE, E>)
	FORCEINLINE constexpr T* ReleaseAndReset(T* InPtr, InE&& InDeleter)
	{
		GetDeleter() = Forward<InE>(InDeleter);
		return ReleaseAndReset(InPtr);
	}

	/** @return The pointer to the managed object or nullptr if no object is owned. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Storage.GetPointer(); }

	/** @return The deleter that is used for destruction of the managed object. */
	NODISCARD FORCEINLINE constexpr       E& GetDeleter()       { return Storage.GetDeleter(); }
	NODISCARD FORCEINLINE constexpr const E& GetDeleter() const { return Storage.GetDeleter(); }

	/** @return true if *this owns an object, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	/** @return The a reference or pointer to the object owned by *this, i.e. Get(). */
	NODISCARD FORCEINLINE constexpr T& operator*()  const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return *Get(); }
	NODISCARD FORCEINLINE constexpr T* operator->() const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return  Get(); }

	/** Overloads the GetTypeHash algorithm for TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TUniquePtr& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TUniquePtr. */
	friend FORCEINLINE constexpr void Swap(TUniquePtr& A, TUniquePtr& B) requires (!CReference<E> && CSwappable<E>)
	{
		Swap(A.Storage.GetPointer(), B.Storage.GetPointer());
		Swap(A.Storage.GetDeleter(), B.Storage.GetDeleter());
	}

private:

	NAMESPACE_PRIVATE::TUniqueStorage<T, E> Storage;

	template <typename OtherT, CInvocable<TRemoveExtent<OtherT>*>  OtherE> requires (CObject<OtherT> && !CBoundedArray<OtherT> && (CDestructible<OtherE> || CLValueReference<OtherE>))
	friend class TUniquePtr;

};

template <typename T>
TUniquePtr(T*) -> TUniquePtr<T>;

/** Single-ownership smart pointer. Use this when you need an array's lifetime to be strictly bound to the lifetime of a single smart pointer. */
template <typename T, typename E>
class TUniquePtr<T[], E> final : private FNoncopyable
{
public:

	using ElementType = T;
	using DeleterType = E;

	/** Constructs a TUniquePtr that owns nothing. Value-initializes the stored pointer and the stored deleter. */
	FORCEINLINE constexpr TUniquePtr() requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(nullptr) { }

	/** Constructs a TUniquePtr that owns nothing. Value-initializes the stored pointer and the stored deleter. */
	FORCEINLINE constexpr TUniquePtr(nullptr_t) requires(CDefaultConstructible<E> && !CPointer<E>) : TUniquePtr() { }

	/** Constructs a TUniquePtr which owns 'InPtr', initializing the stored pointer with 'InPtr' and value-initializing the stored deleter. */
	template <typename U = T*> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE constexpr explicit TUniquePtr(U InPtr) requires(CDefaultConstructible<E> && !CPointer<E>) : Storage(InPtr) { }

	/** Constructs a TUniquePtr object which owns 'InPtr', initializing the stored pointer with 'InPtr' and initializing a deleter 'InDeleter' */
	template <typename U = T*, typename InE> requires (CConvertibleTo<InE, E>
		&& (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)))
	FORCEINLINE constexpr TUniquePtr(U InPtr, InE&& InDeleter) : Storage(InPtr, Forward<InE>(InDeleter)) { }

	/** Constructs a TUniquePtr by transferring ownership from 'InValue' to *this and stores the nullptr in 'InValue'. */
	FORCEINLINE constexpr TUniquePtr(TUniquePtr&& InValue) : Storage(InValue.Release(), Forward<E>(InValue.GetDeleter())) { }

	/** Constructs a TUniquePtr by transferring ownership from 'InValue' to *this and stores the nullptr in 'InValue'. */
	template <typename U, typename InE> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>
		&& ((CReference<E> && CSameAs<InE, E>) || (!CReference<E> && CConvertibleTo<InE, E>)))
	FORCEINLINE constexpr TUniquePtr(TUniquePtr<U, InE>&& InValue) : Storage(InValue.Release(), Forward<InE>(InValue.GetDeleter())) { }

	/** If !IsValid() there are no effects. Otherwise, the owned array is destroyed. */
	FORCEINLINE constexpr ~TUniquePtr() { if (IsValid()) Invoke(GetDeleter(), Get()); }

	/** Move assignment operator. Transfers ownership from 'InValue' to *this. */
	FORCEINLINE constexpr TUniquePtr& operator=(TUniquePtr&& InValue) requires (!CReference<E> && CAssignableFrom<E&, E&&>)
	{
		Reset(InValue.Release());
		GetDeleter() = Forward<E>(InValue.GetDeleter());
		return *this;
	}

	/** Move assignment operator. Transfers ownership from 'InValue' to *this. */
	template <typename U, typename InE> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]>
		&& CArray<U> && !CReference<E> && CAssignableFrom<E&, InE&&>)
	FORCEINLINE constexpr TUniquePtr& operator=(TUniquePtr<U, InE>&& InValue)
	{
		Reset(InValue.Release());
		GetDeleter() = Forward<InE>(InValue.GetDeleter());
		return *this;
	}

	/** Effectively the same as calling Reset(). */
	FORCEINLINE constexpr TUniquePtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Compares the pointer values of two TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TUniquePtr& LHS, const TUniquePtr& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TUniquePtr& LHS, const TUniquePtr& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(U InPtr) const& { return Get() <=> InPtr; }

	/** Returns a pointer to the managed array and releases the ownership. */
	NODISCARD FORCEINLINE constexpr T* Release() { return Exchange(Storage.GetPointer(), nullptr); }

	/** Replaces the managed array. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr void Reset(U InPtr = nullptr)
	{
		if (IsValid()) Invoke(GetDeleter(), Get());
		Storage.GetPointer() = InPtr;
	}

	/** Replaces the managed array. */
	template <typename U = T*, typename InE> requires (CConvertibleTo<InE, E>
		&& CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr void Reset(U InPtr, InE&& InDeleter)
	{
		Reset(InPtr);
		GetDeleter() = Forward<InE>(InDeleter);
	}

	/** Equivalent to Release() then Reset(InPtr). */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr T* ReleaseAndReset(U InPtr = nullptr)
	{
		return Exchange(Storage.GetPointer(), InPtr);
	}

	/** Equivalent to Release() then Reset(InPtr, Forward<InE>(InDeleter)). */
	template <typename U = T*, typename InE> requires (CConvertibleTo<InE, E>
		&& CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE constexpr T* ReleaseAndReset(U InPtr, InE&& InDeleter)
	{
		GetDeleter() = Forward<InE>(InDeleter);
		return ReleaseAndReset(InPtr);
	}

	/** @return The pointer to the managed array or nullptr if no array is owned. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Storage.GetPointer(); }

	/** @return The deleter that is used for destruction of the managed array. */
	NODISCARD FORCEINLINE constexpr       E& GetDeleter()       { return Storage.GetDeleter(); }
	NODISCARD FORCEINLINE constexpr const E& GetDeleter() const { return Storage.GetDeleter(); }

	/** @return true if *this owns an array, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	/** @return The element at index, i.e. Get()[Index]. */
	NODISCARD FORCEINLINE constexpr T& operator[](size_t Index) const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return Get()[Index]; }

	/** Overloads the GetTypeHash algorithm for TUniquePtr. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TUniquePtr& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TUniquePtr. */
	friend FORCEINLINE constexpr void Swap(TUniquePtr& A, TUniquePtr& B) requires (!CReference<E> && CSwappable<E>)
	{
		Swap(A.Storage.GetPointer(), B.Storage.GetPointer());
		Swap(A.Storage.GetDeleter(), B.Storage.GetDeleter());
	}

private:

	NAMESPACE_PRIVATE::TUniqueStorage<T, E> Storage;
	
	template <typename OtherT, CInvocable<TRemoveExtent<OtherT>*>  OtherE> requires (CObject<OtherT> && !CBoundedArray<OtherT> && (CDestructible<OtherE> || CLValueReference<OtherE>))
	friend class TUniquePtr;

};

/** Constructs an object of type T and wraps it in a TUniquePtr. Without initialization. */
template <typename T> requires (CObject<T> && !CArray<T> && !CConstructibleFrom<T, FNoInit> && CDestructible<T>)
NODISCARD FORCEINLINE constexpr TUniquePtr<T> MakeUnique(FNoInit) { return TUniquePtr<T>(new T); }

/** Constructs an object of type T and wraps it in a TUniquePtr. */
template <typename T, typename... Ts> requires (CObject<T> && !CArray<T> && CConstructibleFrom<T, Ts...> && CDestructible<T>)
NODISCARD FORCEINLINE constexpr TUniquePtr<T> MakeUnique(Ts&&... Args) { return TUniquePtr<T>(new T(Forward<Ts>(Args)...)); }

/** Constructs an array of type T and wraps it in a TUniquePtr. Without initialization. */
template <typename T> requires (CUnboundedArray<T> && CDefaultConstructible<TRemoveExtent<T>> && CDestructible<TRemoveExtent<T>>)
NODISCARD FORCEINLINE constexpr TUniquePtr<T> MakeUnique(size_t N, FNoInit) { return TUniquePtr<T>(new TRemoveExtent<T>[N]); }

/** Constructs an array of type T and wraps it in a TUniquePtr. */
template <typename T> requires (CUnboundedArray<T> && CDefaultConstructible<TRemoveExtent<T>> && CDestructible<TRemoveExtent<T>>)
NODISCARD FORCEINLINE constexpr TUniquePtr<T> MakeUnique(size_t N) { return TUniquePtr<T>(new TRemoveExtent<T>[N]()); }

/** Construction of arrays of known bound is disallowed. */
template <typename T, typename... Ts> requires (CBoundedArray<T>)
void MakeUnique(Ts&&...) = delete;

static_assert(sizeof(TUniqueRef<int32>) == sizeof(int32*), "The byte size of TUniqueRef is unexpected");
static_assert(sizeof(TUniquePtr<int32>) == sizeof(int32*), "The byte size of TUniquePtr is unexpected");

DEFINE_TPointerTraits(TUniqueRef);
DEFINE_TPointerTraits(TUniquePtr);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
