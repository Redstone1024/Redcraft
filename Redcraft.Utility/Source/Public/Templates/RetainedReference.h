#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * A helper class which replaces T& as a function parameter when the reference is
 * intended to be retained by the function (e.g. as a class member).  The benefit
 * of this class is that it is a compile error to pass an rvalue which might otherwise
 * bind to a const reference, which is dangerous when the reference is retained.
 *
 *	struct FRAIIType
 *	{
 *		explicit FRAIIType(TRetainedRef<const FThing> InThing) : Ref(InThing) { }
 *
 *		void DoSomething() { Ref.Something(); }
 *
 *		FThing& Ref;
 *	};
 *
 */
template <typename T>
struct TRetainedRef
{
	/** Retain a non-const lvalue reference. */
	FORCEINLINE constexpr TRetainedRef(T& InRef) : Ref(InRef) { }

	/** Can't retain a rvalue reference. */
	TRetainedRef(const T& ) = delete;
	TRetainedRef(      T&&) = delete;
	TRetainedRef(const T&&) = delete;

	/** @return The managed reference. */
	FORCEINLINE constexpr operator T&() const { return Ref; }
	FORCEINLINE constexpr      T& Get() const { return Ref; }

private:

	T& Ref;

};

template <typename T>
struct TRetainedRef<const T>
{
	/** Retain a non-const lvalue reference. */
	FORCEINLINE constexpr TRetainedRef(T& InRef) : Ref(InRef) { }

	/** Retain a const lvalue reference. */
	FORCEINLINE constexpr TRetainedRef(const T& InRef) : Ref(InRef) { }

	/** Can't retain a rvalue reference. */
	TRetainedRef(      T&&) = delete;
	TRetainedRef(const T&&) = delete;

	/** @return The managed reference. */
	FORCEINLINE constexpr operator const T&() const { return Ref; }
	FORCEINLINE constexpr      const T& Get() const { return Ref; }

private:

	const T& Ref;

};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
