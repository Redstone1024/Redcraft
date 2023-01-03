#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/Invocable.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** The class template is a general-purpose scope guard intended to call its callback function when a scope is exited. */
template <CInvocable F> requires (CDestructible<F>)
class TScopeCallback final : private FNoncopyable
{
public:

	/** Initializes the callback function with a function or function object. */
	template <typename InF> requires (!CSameAs<TRemoveCVRef<InF>, TScopeCallback> && CConstructibleFrom<F, InF>)
	FORCEINLINE constexpr explicit TScopeCallback(InF&& Func) : Storage(Forward<InF>(Func)), bIsActive(true) { }

	/** Move constructor. Initializes the stored object with the one in other.  */
	FORCEINLINE constexpr TScopeCallback(TScopeCallback&& InValue) requires (CMoveConstructible<F>)
		: Storage(MoveTemp(InValue.Storage)), bIsActive(InValue.bIsActive)
	{
		InValue.Release();
	}

	/** Calls the callback function if the TScopeCallback is active, then destroys the stored object. */
	FORCEINLINE constexpr ~TScopeCallback()
	{
		if (bIsActive) Storage();
	}

	/** Makes the TScopeCallback inactive. */
	FORCEINLINE constexpr void Release() { bIsActive = false; }

	/** @return a const reference to the stored object. */
	FORCEINLINE constexpr const F& Get() const { return Storage; }

private:

	F Storage;
	bool bIsActive;

};

template <typename F>
TScopeCallback(F) -> TScopeCallback<F>;

/** The class template is a general-purpose scope guard intended to make sure a value is restored when a scope is exited. */
template <typename T> requires (CCopyConstructible<T> && CCopyAssignable<T> && CMoveAssignable<T> && CDestructible<T>)
class TGuardValue final : private FNoncopyable
{
public:

	/** Initializes the TGuardValue with a reference. */
	FORCEINLINE constexpr TGuardValue(T& InReference) : Reference(InReference), OldValue(InReference), bIsActive(true) { }

	/** Initializes the TGuardValue with a reference and assign a new value to the reference. */
	template <typename U> requires (CAssignableFrom<T&, U>)
	FORCEINLINE constexpr TGuardValue(T& InReference, U&& InValue) : Reference(InReference), OldValue(InReference), bIsActive(true) { InReference = InValue; }

	/** Move constructor. Initializes the referenced value and old value with the one in other.  */
	FORCEINLINE constexpr TGuardValue(TGuardValue&& InValue) requires (CMoveConstructible<T>)
		: Reference(InValue.Reference), OldValue(MoveTemp(InValue.OldValue)), bIsActive(InValue.bIsActive)
	{
		InValue.Release();
	}

	/** Restore the referenced value if the TGuardValue is active, then destroys the old value. */
	FORCEINLINE constexpr ~TGuardValue()
	{
		if (bIsActive) Reference = MoveTemp(OldValue);
	}

	/** Makes the TGuardValue inactive. */
	FORCEINLINE constexpr void Release() { bIsActive = false; }

	/** @return a const reference to the old value. */
	FORCEINLINE constexpr const T& Get() const { return OldValue; }

private:

	T& Reference;
	T OldValue;
	bool bIsActive;

};

template <typename T>
TGuardValue(T&) -> TGuardValue<T>;

template <typename T, typename U>
TGuardValue(T&, U&&) -> TGuardValue<T>;

/** Commonly used to make sure a value is incremented, and then decremented when a scope is exited. */
template <typename T> requires (requires(T& Value) { ++Value; --Value; })
class TScopeCounter final : private FNoncopyable
{
public:

	/** Initializes the TScopeCounter with a reference and increments it. */
	FORCEINLINE constexpr TScopeCounter(T& InReference)
		: Reference(InReference)
	{
		++Reference;
	}

	/** Decrements the referenced value. */
	FORCEINLINE constexpr ~TScopeCounter()
	{
		--Reference;
	}

	/** @return a const reference to the value. */
	FORCEINLINE constexpr const T& Get() const { return Reference; }

private:

	T& Reference;

};

template <typename T>
TScopeCounter(T&) -> TScopeCounter<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
