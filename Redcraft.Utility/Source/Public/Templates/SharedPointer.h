#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Atomic.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Memory/MemoryOperator.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/PrimaryType.h"
#include "Templates/PointerTraits.h"
#include "Templates/UniquePointer.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/TypeProperties.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T> requires (CClass<T>)
class TSharedFromThis;

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TSharedRef;

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TSharedPtr;

template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TWeakPtr;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTSharedRef                : FFalse { };
template <typename T> struct TIsTSharedRef<TSharedRef<T>> : FTrue  { };

template <typename T> struct TIsTSharedPtr                : FFalse { };
template <typename T> struct TIsTSharedPtr<TSharedPtr<T>> : FTrue  { };

template <typename T> struct TIsTWeakPtr              : FFalse { };
template <typename T> struct TIsTWeakPtr<TWeakPtr<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTSharedRef = NAMESPACE_PRIVATE::TIsTSharedRef<TRemoveCV<T>>::Value;

template <typename T>
concept CTSharedPtr = NAMESPACE_PRIVATE::TIsTSharedPtr<TRemoveCV<T>>::Value;

template <typename T>
concept CTWeakPtr = NAMESPACE_PRIVATE::TIsTWeakPtr<TRemoveCV<T>>::Value;

NAMESPACE_PRIVATE_BEGIN

// This is the base object for TSharedPtr and uses constructive interference alignment for performance.
class alignas(Memory::ConstructiveInterference) FSharedController : private FSingleton
{
private:

	using RefCounter = TAtomic<size_t>;

	// Ensure that counters are lock-free for performance.
	static_assert(RefCounter::bIsAlwaysLockFree);

	// When this count is zero the object is destroyed.
	// This count is the number of TSharedRef and TSharedPtr.
	RefCounter SharedReferenceCount;

	// When this count is zero the controller is destroyed.
	// If SharedCounter is not zero this count is one more than the number of TWeakPtr.
	RefCounter WeakReferenceCount;

public:

	// The initialization count is one because TSharedPtr already existed when this controller was constructed.
	FORCEINLINE FSharedController() : SharedReferenceCount(1) , WeakReferenceCount(1) { }

	// The controller is a polymorphic class in order to customize the type of erasure of the deleter.
	virtual ~FSharedController() { }

	// Destructor object.
	virtual void DestroyObject() = 0;

	// Destructor this controller.
	virtual void DestroyThis() { delete this; }

	// Get shared reference count, no definite operation order.
	FORCEINLINE RefCounter::ValueType GetSharedReferenceCount()
	{
		// Get the shared reference count as EMemoryOrder::Relaxed,
		// since this count is for reference only and has no guarantees,
		// where EMemoryOrder::Relaxed only determines the atomicity of this operation and not the order.
		return SharedReferenceCount.Load(EMemoryOrder::Relaxed);
	}

	// Increases the shared reference count, ensuring that the shared reference count is non-zero before calling.
	FORCEINLINE void AddSharedReference()
	{
		// The check was removed in the release version, so you can use the default EMemoryOrder.
		check(SharedReferenceCount.Load() != 0);

		// We assume a non-zero reference count, which can be incremented directly with EMemoryOrder::Relaxed,
		// where EMemoryOrder::Relaxed only determines the atomicity of this operation and not the order.
		SharedReferenceCount.FetchAdd(1, EMemoryOrder::Relaxed);
	}

	// increment the shared reference count, do not need to ensure that the shared reference count is zero,
	// if the shared reference count is zero return false.
	bool AddSharedReferenceIfUnexpired()
	{
		RefCounter::ValueType OldSharedReferenceCount = GetSharedReferenceCount();

		// We need to make sure we don't increase the reference count from zero to one.
		while (true)
		{
			// Never add a shared reference if the pointer has already expired.
			if (OldSharedReferenceCount == 0) return false;

			// Attempt to increment the reference count.
			// We do a weak read here because we require a loop where the loop only happens in very unusual cases.
			if (SharedReferenceCount.CompareExchange(OldSharedReferenceCount, OldSharedReferenceCount + 1, EMemoryOrder::Relaxed, true)) return true;
		}
	}

	// Release the shared reference count, make sure the shared reference count is not zero before,
	// and destroy the object when the shared reference count is released to zero.
	void ReleaseSharedReference()
	{
		// Decrement with EMemoryOrder::Release and get the old value,
		// where EMemoryOrder::Release ensures that the side effects of all operations
		// on the shared reference count of all threads are visible to this thread,
		// preventing the shared reference count from actually going to zero.
		RefCounter::ValueType OldSharedReferenceCount = SharedReferenceCount.FetchSub(1, EMemoryOrder::Release);

		// Make sure the shared reference count is not zero before.
		check(OldSharedReferenceCount != 0);

		// Destroy the object when the reference count is released to zero.
		if (OldSharedReferenceCount == 1)
		{
			// Use EMemoryOrder::Acquire to ensure visibility of the side effects of the decrement to any other threads.
			AtomicThreadFence(EMemoryOrder::Acquire);

			// Destroy objects using the type-erase deleter.
			DestroyObject();

			// Release a weak reference count to indicate that no TSharedRef and TSharedPtr are referencing this controller.
			ReleaseWeakReference();
		}
	}

	// Increases the weak reference count, ensuring that the weak reference count is non-zero before calling.
	FORCEINLINE void AddWeakReference()
	{
		// The use of EMemoryOrder is the same as in AddSharedReference().

		check(WeakReferenceCount.Load() != 0);

		WeakReferenceCount.FetchAdd(1, EMemoryOrder::Relaxed);
	}

	// Release the weak reference count, make sure the weak reference count is not zero before,
	// and destroy the controller when the weak reference count is released to zero.
	void ReleaseWeakReference()
	{
		// The use of EMemoryOrder is the same as in ReleaseSharedReference().

		RefCounter::ValueType OldWeakReferenceCount = WeakReferenceCount.FetchSub(1, EMemoryOrder::Release);

		check(OldWeakReferenceCount != 0);

		if (OldWeakReferenceCount == 1)
		{
			AtomicThreadFence(EMemoryOrder::Acquire);
			DestroyThis(); // Destroy this controller.
		}
	}

};

template <typename T, typename E, bool = CEmpty<E> && !CFinal<E>>
class TSharedControllerWithDeleter;

template <typename T, typename E>
class TSharedControllerWithDeleter<T, E, true> final : public FSharedController, private E
{
public:

	TSharedControllerWithDeleter() = delete;

	FORCEINLINE TSharedControllerWithDeleter(T* InPtr) : E(), Pointer(InPtr) { }

	template<typename U>
	FORCEINLINE TSharedControllerWithDeleter(T* InPtr, U&& InDeleter) : E(Forward<U>(InDeleter)), Pointer(InPtr) { }

	virtual ~TSharedControllerWithDeleter() = default;

	virtual void DestroyObject() final { Invoke(GetDeleter(), Pointer); }

	FORCEINLINE T* GetPointer() { return Pointer; }
	FORCEINLINE E& GetDeleter() { return *this;   }

private:

	// NOTE: NO_UNIQUE_ADDRESS is not valid in MSVC, use base class instead of member variable
	//NO_UNIQUE_ADDRESS E Deleter;

	T* Pointer;

};

template <typename T, typename E>
class TSharedControllerWithDeleter<T, E, false> final : public FSharedController
{
public:

	TSharedControllerWithDeleter() = delete;

	FORCEINLINE TSharedControllerWithDeleter(T* InPtr) : Pointer(InPtr), Deleter() { }

	template<typename U>
	FORCEINLINE TSharedControllerWithDeleter(T* InPtr, U&& InDeleter) : Pointer(InPtr), Deleter(Forward<U>(InDeleter)) { }

	virtual ~TSharedControllerWithDeleter() = default;

	virtual void DestroyObject() final { Invoke(GetDeleter(), Pointer); }

	FORCEINLINE T* GetPointer() { return Pointer; }
	FORCEINLINE E& GetDeleter() { return Deleter; }

private:

	T* Pointer;
	E  Deleter;

};

template <typename T>
class TSharedControllerWithObject final : public FSharedController
{
public:

	FORCEINLINE explicit TSharedControllerWithObject(FNoInit) requires (!CConstructibleFrom<T, FNoInit>) { new (&Storage) T; }

	template <typename... Ts> requires (CConstructibleFrom<T, Ts...>)
	FORCEINLINE explicit TSharedControllerWithObject(Ts&&... Args) { new (&Storage) T(Forward<Ts>(Args)...); }

	virtual ~TSharedControllerWithObject() = default;

	virtual void DestroyObject() final { GetPointer()->~T(); }

	FORCEINLINE T* GetPointer() const { return reinterpret_cast<T*>(&Storage); }

private:

	mutable TAlignedStorage<sizeof(T), alignof(T)> Storage;

};

template <typename T>
class TSharedControllerWithArray final : public FSharedController
{
public:

	static TSharedControllerWithArray* New(size_t N, FNoInit)
	{
		void* Buffer = Memory::Malloc(sizeof(TSharedControllerWithArray) + sizeof(T) * (N - 1), alignof(TSharedControllerWithArray));
		const auto Controller = new (Buffer) TSharedControllerWithArray(N);
		const T* ElementPtr = new (Controller->GetPointer()) T[N];
		check(ElementPtr == Controller->GetPointer());
		return Controller;
	}

	static TSharedControllerWithArray* New(size_t N)
	{
		void* Buffer = Memory::Malloc(sizeof(TSharedControllerWithArray) + sizeof(T) * (N - 1), alignof(TSharedControllerWithArray));
		const auto Controller = new (Buffer) TSharedControllerWithArray(N);
		const T* ElementPtr = new (Controller->GetPointer()) T[N]();
		check(ElementPtr == Controller->GetPointer());
		return Controller;
	}

	virtual ~TSharedControllerWithArray() = default;

	virtual void DestroyObject() final { Memory::Destruct(GetPointer(), Num); }

	virtual void DestroyThis() final
	{
		this->~TSharedControllerWithArray();
		Memory::Free(this);
	}

	FORCEINLINE T* GetPointer() const { return reinterpret_cast<T*>(&Storage); }

private:

	size_t Num;

	mutable TAlignedStorage<sizeof(T), alignof(T)> Storage;

	FORCEINLINE explicit TSharedControllerWithArray(size_t N) : Num(N) { }

};

struct FSharedHelper
{
	template <typename T, typename U> requires (CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>
		&& (CTSharedRef<T> || CTSharedPtr<T>) && (CTSharedRef<U> || CTSharedPtr<U>))
	static T& CopySharedReference(T& This, const U& InValue)
	{
		if (This.Controller == InValue.Controller)
		{
			This.Pointer = InValue.Pointer;
			return This;
		}

		if constexpr (CTSharedRef<T>)
		{
			This.Controller->ReleaseSharedReference();
		}
		else if (This.Controller != nullptr)
		{
			This.Controller->ReleaseSharedReference();
		}

		This.Pointer    = InValue.Pointer;
		This.Controller = InValue.Controller;

		if constexpr (CTSharedRef<T> || CTSharedRef<U>)
		{
			This.Controller->AddSharedReference();
		}
		else if (This.Controller != nullptr)
		{
			This.Controller->AddSharedReference();
		}

		return This;
	}

	template <typename T, typename U> requires (CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>
		&& (CTSharedRef<T> || CTSharedPtr<T>) && (CTSharedRef<U> || CTSharedPtr<U>))
	static T& MoveSharedReference(T& This, U&& InValue)
	{
		if constexpr (CTSharedRef<T>)
		{
			Swap(This.Pointer,    InValue.Pointer);
			Swap(This.Controller, InValue.Controller);
		}
		else if constexpr (CTSharedPtr<T> && CTSharedPtr<U>)
		{
			if (&InValue == &This) UNLIKELY return This;

			if (This.Controller != nullptr)
			{
				This.Controller->ReleaseSharedReference();
			}

			This.Pointer    = Exchange(InValue.Pointer,    nullptr);
			This.Controller = Exchange(InValue.Controller, nullptr);
		}
		else
		{
			CopySharedReference(This, InValue);
		}

		return This;
	}

	template <typename T, typename U> requires (CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>
		&& CTWeakPtr<T> && (CTSharedRef<U> || CTSharedPtr<U> || CTWeakPtr<U>))
	static T& CopyWeakReference(T& This, const U& InValue)
	{
		if constexpr (CTWeakPtr<T> && CTWeakPtr<U>)
		{
			if (This.Controller == InValue.Controller)
			{
				This.Pointer = InValue.Pointer;
				return This;
			}
		}

		if (This.Controller != nullptr)
		{
			This.Controller->ReleaseWeakReference();
		}

		This.Pointer    = InValue.Pointer;
		This.Controller = InValue.Controller;

		if constexpr (CTSharedRef<U>)
		{
			This.Controller->AddWeakReference();
		}
		else if (This.Controller != nullptr)
		{
			This.Controller->AddWeakReference();
		}

		return This;
	}

	template <typename T, typename U> requires (CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>
		&& CTWeakPtr<T> && (CTSharedRef<U> || CTSharedPtr<U> || CTWeakPtr<U>))
	static T& MoveWeakReference(T& This, U&& InValue)
	{
		if constexpr (CTWeakPtr<T> && CTWeakPtr<U>)
		{
			if (&InValue == &This) UNLIKELY return This;
			
			if (This.Controller != nullptr)
			{
				This.Controller->ReleaseWeakReference();
			}
			
			This.Pointer    = Exchange(InValue.Pointer,    nullptr);
			This.Controller = Exchange(InValue.Controller, nullptr);
		}
		else
		{
			CopyWeakReference(This, InValue);
		}

		return This;
	}

};

template <typename T>
class TSharedProxy : private FSingleton
{
public:

	FORCEINLINE TSharedProxy(TRemoveExtent<T>* InPtr, FSharedController* InController)
		: Pointer(InPtr), Controller(InController)
	{ }

	template <typename U> requires (CArray<T> == CArray<U> && ((!CArray<U> && CConvertibleTo<T(*)[], U(*)[]>)
		|| (CArray<U> && CConvertibleTo<TRemoveExtent<T>(*)[], TRemoveExtent<U>(*)[]>)))
	NODISCARD FORCEINLINE operator TSharedRef<U>() &&
	{
		check_code({ return TSharedRef<U>(Pointer, Exchange(Controller, nullptr)); });
		return TSharedRef<U>(Pointer, Controller);
	}

	template <typename U> requires (CArray<T> == CArray<U> && ((!CArray<U> && CConvertibleTo<T(*)[], U(*)[]>)
		|| (CArray<U> && CConvertibleTo<TRemoveExtent<T>(*)[], TRemoveExtent<U>(*)[]>)))
	NODISCARD FORCEINLINE operator TSharedPtr<U>() &&
	{
		check_code({ return TSharedPtr<U>(Pointer, Exchange(Controller, nullptr)); });
		return TSharedPtr<U>(Pointer, Controller);
	}

#	if DO_CHECK

	FORCEINLINE ~TSharedProxy() { checkf(Controller == nullptr, TEXT("The return value from MakeShared() is incorrectly ignored.")); }

#	endif

private:

	TRemoveExtent<T>*  Pointer;
	FSharedController* Controller;

};

struct FSharedPtrConstructor { explicit FSharedPtrConstructor() = default; };

inline constexpr FSharedPtrConstructor SharedPtrConstructor{ };

NAMESPACE_PRIVATE_END

/**
 * Derive your class from TSharedFromThis to enable access to a TSharedRef directly from an object instance
 * that's already been allocated. Note that when your class is managed indirectly rather than directly,
 * it is NOT enable access to a TSharedRef, for example managed by the array version of the shared pointer.
 */
template <typename T> requires (CClass<T>)
class TSharedFromThis
{
public:

	/**
	 * Provides access to a shared reference to this object.
	 * Note that is only valid to call this after a shared pointer to the object has already been created.
	 * Also note that it is illegal to call this in the object's destructor.
	 */
	FORCEINLINE TSharedRef<T> AsShared()
	{
		TSharedPtr<T> SharedThis(AsWeak().Lock());
		checkf(SharedThis.Get() == this, TEXT("Your class is now not directly managed. Please check DoesSharedInstanceExist()."));
		return MoveTemp(SharedThis).ToSharedRef();
	}

	/**
	 * Provides access to a shared reference to this object.
	 * Note that is only valid to call this after a shared pointer to the object has already been created.
	 * Also note that it is illegal to call this in the object's destructor.
	 */
	FORCEINLINE TSharedRef<const T> AsShared() const
	{
		TSharedPtr<const T> SharedThis(AsWeak().Lock());
		checkf(SharedThis.Get() == this, TEXT("Your class is now not directly managed. Please check DoesSharedInstanceExist()."));
		return MoveTemp(SharedThis).ToSharedRef();
	}

	/** Provides access to a weak reference to this object. */
	FORCEINLINE TWeakPtr<T> AsWeak()
	{
		return WeakThis;
	}

	/** Provides access to a weak reference to this object. */
	FORCEINLINE TWeakPtr<const T> AsWeak() const
	{
		return WeakThis;
	}

	/** Checks whether our referenced instance is valid, i.e. whether it's safe to call AsShared() or AsWeak(). */
	FORCEINLINE bool DoesSharedInstanceExist() const
	{
		return !WeakThis.Expired();
	}

protected:

	FORCEINLINE constexpr TSharedFromThis() : WeakThis() { }

	FORCEINLINE TSharedFromThis(const TSharedFromThis&) : TSharedFromThis() { }

	FORCEINLINE TSharedFromThis& operator=(const TSharedFromThis&) { return *this; }

	FORCEINLINE ~TSharedFromThis() = default;

private:

	using SharedFromThisType = TSharedFromThis;

	// Here it is updated by the private constructor of TSharedRef or TSharedPtr.
	mutable TWeakPtr<T> WeakThis;

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

};

/** Shared-ownership non-nullable smart pointer. Use this when you need an object's lifetime to be managed by a shared smart pointer. */
template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TSharedRef final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;
	using WeakType    = TWeakPtr<T>;

	/** TSharedRef cannot be initialized by nullptr. */
	TSharedRef() = delete;

	/** TSharedRef cannot be initialized by nullptr. */
	TSharedRef(nullptr_t) = delete;

	/** Constructs a shared reference that owns the specified object. Must not be nullptr. */
	FORCEINLINE explicit TSharedRef(T* InPtr) : TSharedRef(InPtr, TDefaultDelete<T>()) { }

	/** Constructs a shared reference that owns the specified object with a deleter. Must not be nullptr. */
	template <typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE TSharedRef(T* InPtr, E&& InDeleter)
		: TSharedRef(InPtr, new NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, TDecay<E>>(InPtr, Forward<E>(InDeleter)))
	{
		checkf(InPtr != nullptr, TEXT("TSharedRef cannot be initialized by nullptr. Please use TSharedPtr."));
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared object, but pointing to a different object, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The object pointer to use (instead of the incoming shared pointer's object).
	 */
	template <typename U>
	FORCEINLINE TSharedRef(const TSharedRef<U>& InValue, T* InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(InPtr != nullptr, TEXT("TSharedRef cannot be initialized by nullptr. Please use TSharedPtr."));

		Controller->AddSharedReference();
	}
	
	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared object, but pointing to a different object, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The object pointer to use (instead of the incoming shared pointer's object).
	 */
	template <typename U>
	FORCEINLINE TSharedRef(TSharedRef<U>&& InValue, T* InPtr): TSharedRef(InValue, InPtr) { }

	/** Constructs a TSharedRef which shares ownership of the object managed by 'InValue'. */
	FORCEINLINE TSharedRef(const TSharedRef& InValue) : TSharedRef(InValue, InValue.Get()) { }

	/** Constructs a TSharedRef which shares ownership of the object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedRef(const TSharedRef<U>& InValue) : TSharedRef(InValue, InValue.Get()) { }

	/** Constructs a TSharedRef which shares ownership of the object managed by 'InValue'. */
	FORCEINLINE TSharedRef(TSharedRef&& InValue) : TSharedRef(InValue) { }

	/** Constructs a TSharedRef which shares ownership of the object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedRef(TSharedRef<U>&& InValue) : TSharedRef(InValue) { }

	/** If this owns an object and it is the last TSharedRef owning it, the object is destroyed through the owned deleter. */
	FORCEINLINE ~TSharedRef() { Controller->ReleaseSharedReference(); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TSharedRef& operator=(const TSharedRef& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedRef& operator=(const TSharedRef<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TSharedRef& operator=(TSharedRef&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedRef& operator=(TSharedRef<U>&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Compares the pointer values of two TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TSharedRef& LHS, const TSharedRef& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TSharedRef& LHS, const TSharedRef& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr bool operator==(T* InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(T* InPtr) const& { return Get() <=> InPtr; }

	/** TSharedRef cannot be initialized by nullptr. */
	void Reset(nullptr_t) = delete;

	/** Replaces the managed object. */
	FORCEINLINE void Reset(T* InPtr) { *this = MoveTemp(TSharedRef(InPtr)); }

	/** TSharedRef cannot be initialized by nullptr. */
	template <typename E>
	void Reset(nullptr_t, E&&) = delete;

	/** Replaces the managed object with a deleter. */
	template <typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE void Reset(T* InPtr, E&& InDeleter) { *this = MoveTemp(TSharedRef(InPtr, Forward<E>(InDeleter))); }

	/** @return The pointer to the managed object. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	/** @return The pointer to the owned deleter or nullptr. */
	template <typename E> requires (CInvocable<TDecay<E>, TRemoveExtent<T>*> && (CDestructible<E> || CLValueReference<E>))
	NODISCARD FORCEINLINE E* GetDeleter() const
	{
		const auto ControllerWithDeleter = dynamic_cast<NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, E>*>(Controller);
		return ControllerWithDeleter != nullptr ? &ControllerWithDeleter->GetDeleter() : nullptr;
	}

	/** @return The a reference or pointer to the object owned by *this, i.e. Get(). */
	NODISCARD FORCEINLINE constexpr T& operator*()  const { return *Get(); }
	NODISCARD FORCEINLINE constexpr T* operator->() const { return  Get(); }

	/**
	 * Returns the number of shared references to this object (including this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current object.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller->GetSharedReferenceCount(); }

	/**
	 * Checks if this is the only instance managing the current object, i.e. whether GetSharedReferenceCount() == 1.
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return true if *this is the only shared_ptr instance managing the current object, false otherwise.
	 */
	NODISCARD FORCEINLINE bool IsUnique() const { return GetSharedReferenceCount() == 1; }

	/**
	 * Checks whether this TSharedRef precedes other in implementation defined owner-based (as opposed to value-based) order.
	 * This ensures that the ordering of TSharedRef constructed by the aliasing constructor is not affected by object pointer.
	 *
	 * @return The ordering of the addresses of the control blocks.
	 */
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedRef<U>& InValue) const { return Controller <=> InValue.Controller; }
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedPtr<U>& InValue) const { return Controller <=> InValue.Controller; }

	/** Overloads the GetTypeHash algorithm for TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TSharedRef& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TSharedRef. */
	friend FORCEINLINE constexpr void Swap(TSharedRef& A, TSharedRef& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;

	FORCEINLINE TSharedRef(NAMESPACE_PRIVATE::FSharedPtrConstructor, const TSharedPtr<T>& InValue)
		: Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		Controller->AddSharedReference();
	}

	FORCEINLINE TSharedRef(NAMESPACE_PRIVATE::FSharedPtrConstructor, TSharedPtr<T>&& InValue)
		: Pointer(Exchange(InValue.Pointer, nullptr))
		, Controller(Exchange(InValue.Controller, nullptr))
	{ }

	FORCEINLINE TSharedRef(T* InPtr, NAMESPACE_PRIVATE::FSharedController* InController)
		: Pointer(InPtr), Controller(InController)
	{
		check(!((Pointer == nullptr) ^ (Controller == nullptr)));

		if constexpr (CClass<T> && !CVolatile<T> && requires { typename T::SharedFromThisType; })
		{
			using SharedFromThisType = T::SharedFromThisType;

			if constexpr (CDerivedFrom<T, SharedFromThisType>)
			{
				if (Pointer != nullptr)
				{
					const SharedFromThisType& SharedFromThis = *Pointer;
					checkf(!SharedFromThis.DoesSharedInstanceExist(), TEXT("This object is incorrectly managed by multiple TSharedRef or TSharedPtr."));
					SharedFromThis.WeakThis = ConstCast<TRemoveCV<T>>(*this);
				}
			}
		}
	}

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	template <typename U> friend class NAMESPACE_PRIVATE::TSharedProxy;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** Shared-ownership non-nullable smart pointer. Use this when you need an array's lifetime to be managed by a shared smart pointer. */
template <typename T>
class TSharedRef<T[]> final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;
	using WeakType    = TWeakPtr<T>;

	/** TSharedRef cannot be initialized by nullptr. */
	TSharedRef() = delete;

	/** TSharedRef cannot be initialized by nullptr. */
	TSharedRef(nullptr_t) = delete;

	/** Constructs a shared reference that owns the specified array. Must not be nullptr. */
	template <typename U = T*> requires (CPointer<U>&& CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE explicit TSharedRef(U InPtr) : TSharedRef(InPtr, TDefaultDelete<T[]>()) { }

	/** Constructs a shared reference that owns the specified array with a deleter. Must not be nullptr. */
	template <typename U = T*, typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>
		&& (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)))
	FORCEINLINE TSharedRef(U InPtr, E&& InDeleter)
		: TSharedRef(InPtr, new NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, TDecay<E>>(InPtr, Forward<E>(InDeleter)))
	{
		checkf(InPtr != nullptr, TEXT("TSharedRef cannot be initialized by nullptr. Please use TSharedPtr."));
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared array, but pointing to a different array, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The array pointer to use (instead of the incoming shared pointer's array).
	 */
	template <typename U, typename V = T*> requires (CNullPointer<V> || (CPointer<V> && CConvertibleTo<TRemovePointer<V>(*)[], T(*)[]>))
	FORCEINLINE TSharedRef(const TSharedRef<U>& InValue, V InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(InPtr != nullptr, TEXT("TSharedRef cannot be initialized by nullptr. Please use TSharedPtr."));

		Controller->AddSharedReference();
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared array, but pointing to a different array, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The array pointer to use (instead of the incoming shared pointer's array).
	 */
	template <typename U, typename V = T*> requires (CNullPointer<V> || (CPointer<V> && CConvertibleTo<TRemovePointer<V>(*)[], T(*)[]>))
	FORCEINLINE TSharedRef(TSharedRef<U>&& InValue, V InPtr) : TSharedRef(InValue, InPtr) { }

	/** Constructs a TSharedRef which shares ownership of the array managed by 'InValue'. */
	FORCEINLINE TSharedRef(const TSharedRef& InValue) : TSharedRef(InValue, InValue.Get()) { }

	/** Constructs a TSharedRef which shares ownership of the array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedRef(const TSharedRef<U>& InValue) : TSharedRef(InValue, InValue.Get()) { }

	/** Constructs a TSharedRef which shares ownership of the array managed by 'InValue'. */
	FORCEINLINE TSharedRef(TSharedRef&& InValue) : TSharedRef(InValue) { }

	/** Constructs a TSharedRef which shares ownership of the array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedRef(TSharedRef<U>&& InValue) : TSharedRef(InValue) { }

	/** If this owns an array and it is the last TSharedRef owning it, the array is destroyed through the owned deleter. */
	FORCEINLINE ~TSharedRef() { Controller->ReleaseSharedReference(); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TSharedRef& operator=(const TSharedRef& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedRef& operator=(const TSharedRef<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TSharedRef& operator=(TSharedRef&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedRef& operator=(TSharedRef<U>&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Compares the pointer values of two TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TSharedRef& LHS, const TSharedRef& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TSharedRef& LHS, const TSharedRef& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(U InPtr) const& { return Get() <=> InPtr; }

	/** TSharedRef cannot be initialized by nullptr. */
	void Reset(nullptr_t) = delete;

	/** Replaces the managed array. */
	template <typename U> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE void Reset(U InPtr) { *this = MoveTemp(TSharedRef(InPtr)); }

	/** TSharedRef cannot be initialized by nullptr. */
	template <typename E>
	void Reset(nullptr_t, E&&) = delete;

	/** Replaces the managed array with a deleter. */
	template <typename U, typename E> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>
		&& CConstructibleFrom<TDecay<E>, E> && CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE void Reset(U InPtr, E&& InDeleter) { *this = MoveTemp(TSharedRef(InPtr, Forward<E>(InDeleter))); }

	/** @return The pointer to the managed array. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	/** @return The pointer to the owned deleter or nullptr. */
	template <typename E> requires (CInvocable<TDecay<E>, TRemoveExtent<T>*> && (CDestructible<E> || CLValueReference<E>))
	NODISCARD FORCEINLINE E* GetDeleter() const
	{
		const auto ControllerWithDeleter = dynamic_cast<NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, E>*>(Controller);
		return ControllerWithDeleter != nullptr ? &ControllerWithDeleter->GetDeleter() : nullptr;
	}

	/** @return The element at index, i.e. Get()[Index]. */
	NODISCARD FORCEINLINE constexpr T& operator[](size_t Index) const { return Get()[Index]; }

	/**
	 * Returns the number of shared references to this array (including this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current array.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller->GetSharedReferenceCount(); }

	/**
	 * Checks if this is the only instance managing the current array, i.e. whether GetSharedReferenceCount() == 1.
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return true if *this is the only shared_ptr instance managing the current array, false otherwise.
	 */
	NODISCARD FORCEINLINE bool IsUnique() const { return GetSharedReferenceCount() == 1; }

	/**
	 * Checks whether this TSharedRef precedes other in implementation defined owner-based (as opposed to value-based) order.
	 * This ensures that the ordering of TSharedRef constructed by the aliasing constructor is not affected by array pointer.
	 *
	 * @return The ordering of the addresses of the control blocks.
	 */
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedRef<U>& InValue) const { return Controller <=> InValue.Controller; }
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedPtr<U>& InValue) const { return Controller <=> InValue.Controller; }

	/** Overloads the GetTypeHash algorithm for TSharedRef. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TSharedRef& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TSharedRef. */
	friend FORCEINLINE constexpr void Swap(TSharedRef& A, TSharedRef& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;

	FORCEINLINE TSharedRef(NAMESPACE_PRIVATE::FSharedPtrConstructor, const TSharedPtr<T[]>& InValue)
		: Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		Controller->AddSharedReference();
	}

	FORCEINLINE TSharedRef(NAMESPACE_PRIVATE::FSharedPtrConstructor, TSharedPtr<T[]>&& InValue)
		: Pointer(Exchange(InValue.Pointer, nullptr))
		, Controller(Exchange(InValue.Controller, nullptr))
	{ }

	FORCEINLINE TSharedRef(T* InPtr, NAMESPACE_PRIVATE::FSharedController* InController)
		: Pointer(InPtr), Controller(InController)
	{
		check(!((Pointer == nullptr) ^ (Controller == nullptr)));
	}

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	template <typename U> friend class NAMESPACE_PRIVATE::TSharedProxy;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** Shared-ownership smart pointer. Use this when you need an object's lifetime to be managed by a shared smart pointer. */
template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TSharedPtr final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;
	using WeakType    = TWeakPtr<T>;
	
	/** Constructs an empty shared pointer. */
	FORCEINLINE constexpr TSharedPtr() : Pointer(nullptr), Controller(nullptr) { }

	/** Constructs an empty shared pointer. */
	FORCEINLINE constexpr TSharedPtr(nullptr_t) : TSharedPtr() { }

	/** Constructs a shared pointer that owns the specified object. Note that nullptr is not managed like std. */
	FORCEINLINE explicit TSharedPtr(T* InPtr) : TSharedPtr(InPtr, TDefaultDelete<T>()) { }

	/** Constructs a shared pointer that owns the specified object with a deleter. Note that nullptr is not managed like std. */
	template <typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE TSharedPtr(T* InPtr, E&& InDeleter)
		: TSharedPtr(InPtr, InPtr != nullptr ? new NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, TDecay<E>>(InPtr, Forward<E>(InDeleter)) : nullptr)
	{ }

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared object, but pointing to a different object, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The object pointer to use (instead of the incoming shared pointer's object).
	 */
	template <typename U>
	FORCEINLINE TSharedPtr(const TSharedPtr<U>& InValue, T* InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(!((Pointer == nullptr) ^ (Controller == nullptr)), TEXT("TSharedPtr's aliasing constructor cannot be initialized by nullptr."));

		if (Controller != nullptr)
		{
			Controller->AddSharedReference();
		}
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared object, but pointing to a different object, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The object pointer to use (instead of the incoming shared pointer's object).
	 */
	template <typename U>
	FORCEINLINE TSharedPtr(const TSharedRef<U>& InValue, T* InPtr)
	{
		new (this) TSharedRef<T>(InValue, InPtr);
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared object, but pointing to a different object, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The object pointer to use (instead of the incoming shared pointer's object).
	 */
	template <typename U>
	FORCEINLINE TSharedPtr(TSharedPtr<U>&& InValue, T* InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(!((Pointer == nullptr) ^ (Controller == nullptr)), TEXT("TSharedPtr's aliasing constructor cannot be initialized by nullptr."));

		InValue.Pointer    = nullptr;
		InValue.Controller = nullptr;
	}

	/** Constructs a TSharedPtr which shares ownership of the object managed by 'InValue'. */
	FORCEINLINE TSharedPtr(const TSharedPtr& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr(const TSharedPtr<U>& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr(const TSharedRef<U>& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the object managed by 'InValue'. */
	FORCEINLINE TSharedPtr(TSharedPtr&& InValue) : TSharedPtr(MoveTemp(InValue), InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr(TSharedPtr<U>&& InValue) : TSharedPtr(MoveTemp(InValue), InValue.Get()) { }

	/** Constructs a TSharedPtr which gets ownership of the object managed by 'InValue'. */
	template <typename U, typename E> requires (CConvertibleTo<U*, T*> && !CArray<U> && (CDestructible<E> || CLValueReference<E>))
	FORCEINLINE TSharedPtr(TUniquePtr<U, E>&& InValue) : TSharedPtr(InValue.Release(), Forward<E>(InValue.GetDeleter())) { }

	/** If this owns an object and it is the last TSharedPtr owning it, the object is destroyed through the owned deleter. */
	FORCEINLINE ~TSharedPtr() { if (Controller != nullptr) Controller->ReleaseSharedReference(); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TSharedPtr& operator=(const TSharedPtr& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr& operator=(const TSharedPtr<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr& operator=(const TSharedRef<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TSharedPtr& operator=(TSharedPtr&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TSharedPtr& operator=(TSharedPtr<U>&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U, typename E> requires (CConvertibleTo<U*, T*> && !CArray<U> && (CDestructible<E> || CLValueReference<E>))
	FORCEINLINE TSharedPtr& operator=(TUniquePtr<U, E>&& InValue) { return Swap(*this, TSharedPtr(MoveTemp(InValue))); }

	/** Effectively the same as calling Reset(). */
	FORCEINLINE TSharedPtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Compares the pointer values of two TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TSharedPtr& LHS, const TSharedPtr& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TSharedPtr& LHS, const TSharedPtr& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr bool operator==(T* InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(T* InPtr) const& { return Get() <=> InPtr; }

	/** Converts a shared pointer to a shared reference. The pointer MUST be valid or an assertion will trigger. */
	FORCEINLINE TSharedRef<T> ToSharedRef() const&
	{
		checkf(IsValid(), TEXT("TSharedRef cannot be initialized by nullptr."));
		return TSharedRef<T>(NAMESPACE_PRIVATE::SharedPtrConstructor, *this);
	}

	/** Converts a shared pointer to a shared reference. The pointer MUST be valid or an assertion will trigger. */
	FORCEINLINE TSharedRef<T> ToSharedRef() &&
	{
		checkf(IsValid(), TEXT("TSharedRef cannot be initialized by nullptr."));
		return TSharedRef<T>(NAMESPACE_PRIVATE::SharedPtrConstructor, *this);
	}

	/** Replaces the managed object. */
	FORCEINLINE void Reset(T* InPtr = nullptr) { *this = MoveTemp(TSharedPtr(InPtr)); }

	/** Replaces the managed object with a deleter. */
	template <typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE void Reset(T* InPtr, E&& InDeleter) { *this = MoveTemp(TSharedPtr(InPtr, Forward<E>(InDeleter))); }

	/** @return The pointer to the managed object. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	/** @return The pointer to the owned deleter or nullptr. */
	template <typename E> requires (CInvocable<TDecay<E>, TRemoveExtent<T>*> && (CDestructible<E> || CLValueReference<E>))
	NODISCARD FORCEINLINE E* GetDeleter() const
	{
		const auto ControllerWithDeleter = dynamic_cast<NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, E>*>(Controller);
		return ControllerWithDeleter != nullptr ? &ControllerWithDeleter->GetDeleter() : nullptr;
	}

	/** @return true if *this owns an object, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	/** @return The a reference or pointer to the object owned by *this, i.e. Get(). */
	NODISCARD FORCEINLINE constexpr T& operator*()  const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return *Get(); }
	NODISCARD FORCEINLINE constexpr T* operator->() const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return  Get(); }

	/**
	 * Returns the number of shared references to this object (including this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current object.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller != nullptr ? Controller->GetSharedReferenceCount() : 0; }

	/**
	 * Checks if this is the only instance managing the current object, i.e. whether GetSharedReferenceCount() == 1.
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return true if *this is the only shared_ptr instance managing the current object, false otherwise.
	 */
	NODISCARD FORCEINLINE bool IsUnique() const { return GetSharedReferenceCount() == 1; }

	/**
	 * Checks whether this TSharedPtr precedes other in implementation defined owner-based (as opposed to value-based) order.
	 * This ensures that the ordering of TSharedPtr constructed by the aliasing constructor is not affected by object pointer.
	 *
	 * @return The ordering of the addresses of the control blocks.
	 */
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedRef<U>& InValue) const { return Controller <=> InValue.Controller; }
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedPtr<U>& InValue) const { return Controller <=> InValue.Controller; }

	/** Overloads the GetTypeHash algorithm for TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TSharedPtr& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TSharedPtr. */
	friend FORCEINLINE constexpr void Swap(TSharedPtr& A, TSharedPtr& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;
	
	FORCEINLINE TSharedPtr(const TWeakPtr<T>& InValue)
	{
		const bool bIsUnexpired = InValue.Controller != nullptr && InValue.Controller->AddSharedReferenceIfUnexpired();

		Pointer    = bIsUnexpired ? InValue.Pointer    : nullptr;
		Controller = bIsUnexpired ? InValue.Controller : nullptr;
	
	}

	FORCEINLINE TSharedPtr(T* InPtr, NAMESPACE_PRIVATE::FSharedController* InController)
		: Pointer(InPtr), Controller(InController)
	{
		check(!((Pointer == nullptr) ^ (Controller == nullptr)));

		if constexpr (CClass<T> && !CVolatile<T> && requires { typename T::SharedFromThisType; })
		{
			using SharedFromThisType = T::SharedFromThisType;

			if constexpr (CDerivedFrom<T, SharedFromThisType>)
			{
				if (Pointer != nullptr)
				{
					const SharedFromThisType& SharedFromThis = *Pointer;
					checkf(!SharedFromThis.DoesSharedInstanceExist(), TEXT("This object is incorrectly managed by multiple TSharedRef or TSharedPtr."));
					SharedFromThis.WeakThis = ConstCast<TRemoveCV<T>>(*this);
				}
			}
		}
	}

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	template <typename U> friend class NAMESPACE_PRIVATE::TSharedProxy;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** Shared-ownership smart pointer. Use this when you need an array's lifetime to be managed by a shared smart pointer. */
template <typename T>
class TSharedPtr<T[]> final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;
	using WeakType    = TWeakPtr<T>;

	/** Constructs an empty shared pointer. */
	FORCEINLINE constexpr TSharedPtr() : Pointer(nullptr), Controller(nullptr) { }

	/** Constructs an empty shared pointer. */
	FORCEINLINE constexpr TSharedPtr(nullptr_t) : TSharedPtr() { }

	/** Constructs a shared pointer that owns the specified array. Note that nullptr is not managed like std. */
	template <typename U = T*> requires (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
	FORCEINLINE explicit TSharedPtr(U InPtr) : TSharedPtr(InPtr, TDefaultDelete<T[]>()) { }

	/** Constructs a shared pointer that owns the specified array with a deleter. Note that nullptr is not managed like std. */
	template <typename U = T*, typename E> requires (CConstructibleFrom<TDecay<E>, E>
		&& CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>
		&& (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)))
	FORCEINLINE TSharedPtr(U InPtr, E&& InDeleter)
		: TSharedPtr(InPtr, InPtr != nullptr ? new NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, TDecay<E>>(InPtr, Forward<E>(InDeleter)) : nullptr)
	{ }

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared array, but pointing to a different array, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The array pointer to use (instead of the incoming shared pointer's array).
	 */
	template <typename U, typename V = T*> requires (CNullPointer<V> || (CPointer<V> && CConvertibleTo<TRemovePointer<V>(*)[], T(*)[]>))
	FORCEINLINE TSharedPtr(const TSharedPtr<U>& InValue, V InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(!((Pointer == nullptr) ^ (Controller == nullptr)), TEXT("TSharedPtr's aliasing constructor cannot be initialized by nullptr."));

		if (Controller != nullptr)
		{
			Controller->AddSharedReference();
		}
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared array, but pointing to a different array, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The array pointer to use (instead of the incoming shared pointer's array).
	 */
	template <typename U, typename V = T*> requires (CNullPointer<V> || (CPointer<V> && CConvertibleTo<TRemovePointer<V>(*)[], T(*)[]>))
	FORCEINLINE TSharedPtr(const TSharedRef<U>& InValue, V InPtr)
	{
		new (this) TSharedRef<T[]>(InValue, InPtr);
	}

	/**
	 * Aliasing constructor used to create a shared reference which shares its reference count with
	 * another shared array, but pointing to a different array, typically a subobject.
	 * Must not be nullptr.
	 *
	 * @param  InValue - The shared reference whose reference count should be shared.
	 * @param  InPtr   - The array pointer to use (instead of the incoming shared pointer's array).
	 */
	template <typename U, typename V = T*> requires (CNullPointer<V> || (CPointer<V> && CConvertibleTo<TRemovePointer<V>(*)[], T(*)[]>))
	FORCEINLINE TSharedPtr(TSharedPtr<U>&& InValue, V InPtr)
		: Pointer(InPtr), Controller(InValue.Controller)
	{
		checkf(!((Pointer == nullptr) ^ (Controller == nullptr)), TEXT("TSharedPtr's aliasing constructor cannot be initialized by nullptr."));

		InValue.Pointer    = nullptr;
		InValue.Controller = nullptr;
	}

	/** Constructs a TSharedPtr which shares ownership of the array managed by 'InValue'. */
	FORCEINLINE TSharedPtr(const TSharedPtr& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr(const TSharedPtr<U>& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr(const TSharedRef<U>& InValue) : TSharedPtr(InValue, InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the array managed by 'InValue'. */
	FORCEINLINE TSharedPtr(TSharedPtr&& InValue) : TSharedPtr(MoveTemp(InValue), InValue.Get()) { }

	/** Constructs a TSharedPtr which shares ownership of the array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr(TSharedPtr<U>&& InValue) : TSharedPtr(MoveTemp(InValue), InValue.Get()) { }

	/** Constructs a TSharedPtr which gets ownership of the array managed by 'InValue'. */
	template <typename U, typename E> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U> && (CDestructible<E> || CLValueReference<E>))
	FORCEINLINE TSharedPtr(TUniquePtr<U, E>&& InValue) : TSharedPtr(InValue.Release(), Forward<E>(InValue.GetDeleter())) { }

	/** If this owns an array and it is the last TSharedPtr owning it, the array is destroyed through the owned deleter. */
	FORCEINLINE ~TSharedPtr() { if (Controller != nullptr) Controller->ReleaseSharedReference(); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TSharedPtr& operator=(const TSharedPtr& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr& operator=(const TSharedPtr<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr& operator=(const TSharedRef<U>& InValue) { return Helper::CopySharedReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TSharedPtr& operator=(TSharedPtr&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TSharedPtr& operator=(TSharedPtr<U>&& InValue) { return Helper::MoveSharedReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U, typename E> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U> && (CDestructible<E> || CLValueReference<E>))
	FORCEINLINE TSharedPtr& operator=(TUniquePtr<U, E>&& InValue) { return Swap(*this, TSharedPtr(MoveTemp(InValue))); }

	/** Effectively the same as calling Reset(). */
	FORCEINLINE TSharedPtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Compares the pointer values of two TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TSharedPtr& LHS, const TSharedPtr& RHS) { return LHS.Get() == RHS.Get(); }

	/** Compares the pointer values of two TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TSharedPtr& LHS, const TSharedPtr& RHS) { return LHS.Get() <=> RHS.Get(); }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr bool operator==(U InPtr) const& { return Get() == InPtr; }

	/** Compares the pointer values with a raw pointer. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	NODISCARD FORCEINLINE constexpr strong_ordering operator<=>(U InPtr) const& { return Get() <=> InPtr; }

	/** Converts a shared pointer to a shared reference. The pointer MUST be valid or an assertion will trigger. */
	FORCEINLINE TSharedRef<T[]> ToSharedRef() const&
	{
		checkf(IsValid(), TEXT("TSharedRef cannot be initialized by nullptr."));
		return TSharedRef<T[]>(NAMESPACE_PRIVATE::SharedPtrConstructor, *this);
	}

	/** Converts a shared pointer to a shared reference. The pointer MUST be valid or an assertion will trigger. */
	FORCEINLINE TSharedRef<T[]> ToSharedRef() &&
	{
		checkf(IsValid(), TEXT("TSharedRef cannot be initialized by nullptr."));
		return TSharedRef<T[]>(NAMESPACE_PRIVATE::SharedPtrConstructor, *this);
	}

	/** Replaces the managed array. */
	template <typename U = T*> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>))
	FORCEINLINE void Reset(U InPtr = nullptr) { *this = MoveTemp(TSharedPtr(InPtr)); }

	/** Replaces the managed array with a deleter. */
	template <typename U, typename E> requires (CNullPointer<U> || (CPointer<U> && CConvertibleTo<TRemovePointer<U>(*)[], T(*)[]>)
		&& CConstructibleFrom<TDecay<E>, E> && CInvocable<TDecay<E>, TRemoveExtent<T>*> && CDestructible<TDecay<E>>)
	FORCEINLINE void Reset(U InPtr, E&& InDeleter) { *this = MoveTemp(TSharedPtr(InPtr, Forward<E>(InDeleter))); }

	/** @return The pointer to the managed array. */
	NODISCARD FORCEINLINE constexpr T* Get() const { return Pointer; }

	/** @return The pointer to the owned deleter or nullptr. */
	template <typename E> requires (CInvocable<TDecay<E>, TRemoveExtent<T>*> && (CDestructible<E> || CLValueReference<E>))
	NODISCARD FORCEINLINE E* GetDeleter() const
	{
		const auto ControllerWithDeleter = dynamic_cast<NAMESPACE_PRIVATE::TSharedControllerWithDeleter<T, E>*>(Controller);
		return ControllerWithDeleter != nullptr ? &ControllerWithDeleter->GetDeleter() : nullptr;
	}

	/** @return true if *this owns an array, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Get() != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Get() != nullptr; }

	/** @return The element at index, i.e. Get()[Index]. */
	NODISCARD FORCEINLINE constexpr T& operator[](size_t Index) const { checkf(IsValid(), TEXT("Read access violation. Please check IsValid().")); return Get()[Index]; }

	/**
	 * Returns the number of shared references to this array (including this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current array.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller != nullptr ? Controller->GetSharedReferenceCount() : 0; }

	/**
	 * Checks if this is the only instance managing the current array, i.e. whether GetSharedReferenceCount() == 1.
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return true if *this is the only shared_ptr instance managing the current array, false otherwise.
	 */
	NODISCARD FORCEINLINE bool IsUnique() const { return GetSharedReferenceCount() == 1; }

	/**
	 * Checks whether this TSharedPtr precedes other in implementation defined owner-based (as opposed to value-based) order.
	 * This ensures that the ordering of TSharedPtr constructed by the aliasing constructor is not affected by array pointer.
	 *
	 * @return The ordering of the addresses of the control blocks.
	 */
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedRef<U>& InValue) const { return Controller <=> InValue.Controller; }
	template <typename U = T> NODISCARD FORCEINLINE strong_ordering OwnerCompare(const TSharedPtr<U>& InValue) const { return Controller <=> InValue.Controller; }

	/** Overloads the GetTypeHash algorithm for TSharedPtr. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TSharedPtr& A) { return GetTypeHash(A.Get()); }

	/** Overloads the Swap algorithm for TSharedPtr. */
	friend FORCEINLINE constexpr void Swap(TSharedPtr& A, TSharedPtr& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;

	FORCEINLINE TSharedPtr(const TWeakPtr<T[]>& InValue)
	{
		const bool bIsUnexpired = InValue.Controller != nullptr && InValue.Controller->AddSharedReferenceIfUnexpired();

		Pointer    = bIsUnexpired ? InValue.Pointer    : nullptr;
		Controller = bIsUnexpired ? InValue.Controller : nullptr;
	
	}

	FORCEINLINE TSharedPtr(T* InPtr, NAMESPACE_PRIVATE::FSharedController* InController)
		: Pointer(InPtr), Controller(InController)
	{
		check(!((Pointer == nullptr) ^ (Controller == nullptr)));
	}

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	template <typename U> friend class NAMESPACE_PRIVATE::TSharedProxy;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** TWeakPtr is a smart pointer that holds a non-owning ("weak") reference to an object that is managed by shared pointer. */
template <typename T> requires (CObject<T> && !CBoundedArray<T>)
class TWeakPtr final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;

	/** Constructs an empty TWeakPtr */
	FORCEINLINE constexpr TWeakPtr() : Pointer(nullptr), Controller(nullptr) { }

	/** Constructs an empty TWeakPtr */
	FORCEINLINE constexpr TWeakPtr(nullptr_t) : TWeakPtr() { }

	/** Constructs new TWeakPtr which shares an object managed by 'InValue'. */
	FORCEINLINE TWeakPtr(const TWeakPtr& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Constructs new TWeakPtr which shares an object managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TWeakPtr<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Move constructors. Moves a TWeakPtr instance from 'InValue' into this. */
	FORCEINLINE TWeakPtr(TWeakPtr&& InValue) : Pointer(Exchange(InValue.Pointer, nullptr)), Controller(Exchange(InValue.Controller, nullptr)) { }

	/** Move constructors. Moves a TWeakPtr instance from 'InValue' into this. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE constexpr TWeakPtr(TWeakPtr<U>&& InValue) : Pointer(Exchange(InValue.Pointer, nullptr)), Controller(Exchange(InValue.Controller, nullptr)) { }
	
	/** Constructs a weak pointer from a shared reference. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TSharedRef<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		Controller->AddWeakReference();
	}

	/** Constructs a weak pointer from a shared pointer. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TSharedPtr<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Destroys the TWeakPtr object. Results in no effect to the managed object. */
	FORCEINLINE ~TWeakPtr() { if (Controller != nullptr) Controller->ReleaseWeakReference(); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TWeakPtr& operator=(const TWeakPtr& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TWeakPtr<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	FORCEINLINE TWeakPtr& operator=(TWeakPtr&& InValue) { return Helper::MoveWeakReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed object with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TWeakPtr& operator=(TWeakPtr<U>&& InValue) { return Helper::MoveWeakReference(*this, MoveTemp(InValue)); }

	/** Assignment operator sets this weak pointer from a shared reference. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TSharedRef<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Assignment operator sets this weak pointer from a shared pointer. */
	template <typename U> requires (CConvertibleTo<U*, T*> && !CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TSharedPtr<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Effectively the same as calling Reset(). */
	FORCEINLINE TWeakPtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Resets this weak pointer, removing a weak reference to the object. */
	FORCEINLINE void Reset()
	{
		if (Controller != nullptr)
		{
			Controller->ReleaseWeakReference();
			Pointer    = nullptr;
			Controller = nullptr;
		}
	}

	/**
	 * Returns the number of shared references to this object (Excluding this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current object.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller != nullptr ? Controller->GetSharedReferenceCount() : 0; }

	/** @return true if the weak pointer is expired and a lock operator would have failed. */
	NODISCARD FORCEINLINE bool Expired() const { return GetSharedReferenceCount() == 0; }

	/** A TSharedPtr which shares ownership of the owned object if Expired() returns false. Else returns nullptr. */
	NODISCARD FORCEINLINE TSharedPtr<T> Lock() const { return *this; }

	/** Overloads the Swap algorithm for TWeakPtr. */
	friend FORCEINLINE constexpr void Swap(TWeakPtr& A, TWeakPtr& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** TWeakPtr is a smart pointer that holds a non-owning ("weak") reference to an array that is managed by shared pointer. */
template <typename T>
class TWeakPtr<T[]> final
{
private:

	using Helper = NAMESPACE_PRIVATE::FSharedHelper;

public:

	using ElementType = T;

	/** Constructs an empty TWeakPtr */
	FORCEINLINE constexpr TWeakPtr() : Pointer(nullptr), Controller(nullptr) { }

	/** Constructs an empty TWeakPtr */
	FORCEINLINE constexpr TWeakPtr(nullptr_t) : TWeakPtr() { }

	/** Constructs new TWeakPtr which shares an array managed by 'InValue'. */
	FORCEINLINE TWeakPtr(const TWeakPtr& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Constructs new TWeakPtr which shares an array managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TWeakPtr<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Move constructors. Moves a TWeakPtr instance from 'InValue' into this. */
	FORCEINLINE TWeakPtr(TWeakPtr&& InValue) : Pointer(Exchange(InValue.Pointer, nullptr)), Controller(Exchange(InValue.Controller, nullptr)) { }

	/** Move constructors. Moves a TWeakPtr instance from 'InValue' into this. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE constexpr TWeakPtr(TWeakPtr<U>&& InValue) : Pointer(Exchange(InValue.Pointer, nullptr)), Controller(Exchange(InValue.Controller, nullptr)) { }

	/** Constructs a weak pointer from a shared reference. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TSharedRef<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		Controller->AddWeakReference();
	}

	/** Constructs a weak pointer from a shared pointer. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE constexpr TWeakPtr(const TSharedPtr<U>& InValue) : Pointer(InValue.Pointer), Controller(InValue.Controller)
	{
		if (Controller != nullptr)
		{
			Controller->AddWeakReference();
		}
	}

	/** Destroys the TWeakPtr array. Results in no effect to the managed array. */
	FORCEINLINE ~TWeakPtr() { if (Controller != nullptr) Controller->ReleaseWeakReference(); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TWeakPtr& operator=(const TWeakPtr& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TWeakPtr<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	FORCEINLINE TWeakPtr& operator=(TWeakPtr&& InValue) { return Helper::MoveWeakReference(*this, MoveTemp(InValue)); }

	/** Replaces the managed array with the one managed by 'InValue'. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TWeakPtr& operator=(TWeakPtr<U>&& InValue) { return Helper::MoveWeakReference(*this, MoveTemp(InValue)); }

	/** Assignment operator sets this weak pointer from a shared reference. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TSharedRef<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Assignment operator sets this weak pointer from a shared pointer. */
	template <typename U> requires (CConvertibleTo<TRemoveExtent<U>(*)[], T(*)[]> && CArray<U>)
	FORCEINLINE TWeakPtr& operator=(const TSharedPtr<U>& InValue) { return Helper::CopyWeakReference(*this, InValue); }

	/** Effectively the same as calling Reset(). */
	FORCEINLINE TWeakPtr& operator=(nullptr_t) { Reset(); return *this; }

	/** Resets this weak pointer, removing a weak reference to the array. */
	FORCEINLINE void Reset()
	{
		if (Controller != nullptr)
		{
			Controller->ReleaseWeakReference();
			Pointer    = nullptr;
			Controller = nullptr;
		}
	}

	/**
	 * Returns the number of shared references to this array (Excluding this reference.)
	 * IMPORTANT: With multi-threading this is only an estimate.
	 *
	 * @return The number of instances managing the current array.
	 */
	NODISCARD FORCEINLINE size_t GetSharedReferenceCount() const { return Controller != nullptr ? Controller->GetSharedReferenceCount() : 0; }

	/** @return true if the weak pointer is expired and a lock operator would have failed. */
	NODISCARD FORCEINLINE bool Expired() const { return GetSharedReferenceCount() == 0; }

	/** A TSharedPtr which shares ownership of the owned array if Expired() returns false. Else returns nullptr. */
	NODISCARD FORCEINLINE TSharedPtr<T[]> Lock() const { return *this; }

	/** Overloads the Swap algorithm for TWeakPtr. */
	friend FORCEINLINE constexpr void Swap(TWeakPtr& A, TWeakPtr& B)
	{
		Swap(A.Pointer, B.Pointer);
		Swap(A.Controller, B.Controller);
	}

private:

	T* Pointer;
	NAMESPACE_PRIVATE::FSharedController* Controller;

	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedRef;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TSharedPtr;
	template <typename U> requires (CObject<U> && !CBoundedArray<U>) friend class TWeakPtr;

	friend struct NAMESPACE_PRIVATE::FSharedHelper;

};

/** Constructs an object of type T and wraps it in a TSharedRef or TSharedPtr. Without initialization. */
template <typename T> requires (CObject<T> && !CArray<T> && !CConstructibleFrom<T, FNoInit> && CDestructible<T>)
NODISCARD FORCEINLINE NAMESPACE_PRIVATE::TSharedProxy<T> MakeShared(FNoInit)
{
	const auto Controller = new NAMESPACE_PRIVATE::TSharedControllerWithObject<T>(NoInit);
	return NAMESPACE_PRIVATE::TSharedProxy<T>(Controller->GetPointer(), Controller);
}

/** Constructs an object of type T and wraps it in a TSharedRef or TSharedPtr. */
template <typename T, typename... Ts> requires (CObject<T> && !CArray<T> && CConstructibleFrom<T, Ts...> && CDestructible<T>)
NODISCARD FORCEINLINE NAMESPACE_PRIVATE::TSharedProxy<T> MakeShared(Ts&&... Args)
{
	const auto Controller = new NAMESPACE_PRIVATE::TSharedControllerWithObject<T>(Forward<Ts>(Args)...);
	return NAMESPACE_PRIVATE::TSharedProxy<T>(Controller->GetPointer(), Controller);
}

/** Constructs an array of type T and wraps it in a TSharedRef or TSharedPtr. Without initialization. */
template <typename T> requires (CUnboundedArray<T> && CDefaultConstructible<TRemoveExtent<T>> && CDestructible<TRemoveExtent<T>>)
NODISCARD FORCEINLINE NAMESPACE_PRIVATE::TSharedProxy<T> MakeShared(size_t N, FNoInit)
{
	const auto Controller = NAMESPACE_PRIVATE::TSharedControllerWithArray<TRemoveExtent<T>>::New(N, NoInit);
	return NAMESPACE_PRIVATE::TSharedProxy<T>(Controller->GetPointer(), Controller);
}

/** Constructs an array of type T and wraps it in a TSharedRef or TSharedPtr. */
template <typename T> requires (CUnboundedArray<T> && CDefaultConstructible<TRemoveExtent<T>> && CDestructible<TRemoveExtent<T>>)
NODISCARD FORCEINLINE NAMESPACE_PRIVATE::TSharedProxy<T> MakeShared(size_t N)
{
	const auto Controller = NAMESPACE_PRIVATE::TSharedControllerWithArray<TRemoveExtent<T>>::New(N);
	return NAMESPACE_PRIVATE::TSharedProxy<T>(Controller->GetPointer(), Controller);
}

/** Construction of arrays of known bound is disallowed. */
template <typename T, typename... Ts> requires (CBoundedArray<T>)
void MakeShared(Ts&&...) = delete;

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a static_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { static_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> StaticCast(const TSharedRef<U>& InValue)
{
	return TSharedRef<T>(InValue, static_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a static_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { static_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> StaticCast(TSharedRef<U>&& InValue)
{
	return TSharedRef<T>(MoveTemp(InValue), static_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a dynamic_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { dynamic_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> DynamicCast(const TSharedRef<U>& InValue)
{
	return TSharedRef<T>(InValue, dynamic_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a dynamic_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { dynamic_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> DynamicCast(TSharedRef<U>&& InValue)
{
	return TSharedRef<T>(MoveTemp(InValue), dynamic_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a const_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { const_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> ConstCast(const TSharedRef<U>& InValue)
{
	return TSharedRef<T>(InValue, const_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a const_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { const_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> ConstCast(TSharedRef<U>&& InValue)
{
	return TSharedRef<T>(MoveTemp(InValue), const_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a reinterpret_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { reinterpret_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> ReinterpretCast(const TSharedRef<U>& InValue)
{
	return TSharedRef<T>(InValue, reinterpret_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedRef whose stored pointer is obtained from stored pointer of 'InValue' using a reinterpret_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { reinterpret_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedRef<T> ReinterpretCast(TSharedRef<U>&& InValue)
{
	return TSharedRef<T>(MoveTemp(InValue), reinterpret_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a static_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { static_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> StaticCast(const TSharedPtr<U>& InValue)
{
	return TSharedPtr<T>(InValue, static_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a static_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { static_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> StaticCast(TSharedPtr<U>&& InValue)
{
	return TSharedPtr<T>(MoveTemp(InValue), static_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a dynamic_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { dynamic_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> DynamicCast(const TSharedPtr<U>& InValue)
{
	return TSharedPtr<T>(InValue, dynamic_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a dynamic_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { dynamic_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> DynamicCast(TSharedPtr<U>&& InValue)
{
	return TSharedPtr<T>(MoveTemp(InValue), dynamic_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a const_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { const_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> ConstCast(const TSharedPtr<U>& InValue)
{
	return TSharedPtr<T>(InValue, const_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a const_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { const_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> ConstCast(TSharedPtr<U>&& InValue)
{
	return TSharedPtr<T>(MoveTemp(InValue), const_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a reinterpret_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { reinterpret_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> ReinterpretCast(const TSharedPtr<U>& InValue)
{
	return TSharedPtr<T>(InValue, reinterpret_cast<T*>(InValue.Get()));
}

/** Creates a new instance of TSharedPtr whose stored pointer is obtained from stored pointer of 'InValue' using a reinterpret_cast expression. */
template <typename T, typename U> requires (requires(U* InPtr) { reinterpret_cast<T*>(InPtr); })
NODISCARD FORCEINLINE TSharedPtr<T> ReinterpretCast(TSharedPtr<U>&& InValue)
{
	return TSharedPtr<T>(MoveTemp(InValue), reinterpret_cast<T*>(InValue.Get()));
}

DEFINE_TPointerTraits(TSharedRef);
DEFINE_TPointerTraits(TSharedPtr);

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
