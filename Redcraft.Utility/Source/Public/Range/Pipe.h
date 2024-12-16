#pragma once

#include "CoreTypes.h"
#include "Range/Utility.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Range)

/**
 * An interface class template for defining a range adaptor closure.
 * When the derived class has a unary operator() with range as a reference and is not itself a range,
 * the derived class is the range adaptor closure type and its objects can participate in pipe operations.
 * Specify, the unary operator() with any reference qualifier or cv-qualifier must be defined and has same effect.
 * Not directly instantiable.
 */
template <CObject D> requires (CSameAs<D, TRemoveCV<D>>)
class IAdaptorClosure { };

/** An adaptor closure helper that wraps a callable object. */
template <CMoveConstructible F>
class TAdaptorClosure : public IAdaptorClosure<TAdaptorClosure<F>>
{
public:

	FORCEINLINE constexpr explicit TAdaptorClosure(F InClosure) : Closure(MoveTemp(InClosure)) { }

	template <typename R> requires (CInvocable<F&, R>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &
	{
		return Invoke(Closure, Forward<R>(Range));
	}

	template <typename R> requires (CInvocable<const F&, R>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&
	{
		return Invoke(Closure, Forward<R>(Range));
	}

	template <typename R> requires (CInvocable<F&&, R>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &&
	{
		return Invoke(MoveTemp(Closure), Forward<R>(Range));
	}

	template <typename R> requires (CInvocable<const F&&, R>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&&
	{
		return Invoke(MoveTemp(Closure), Forward<R>(Range));
	}

private:

	NO_UNIQUE_ADDRESS F Closure;

};

/** A pipe closure that wraps two adaptor closures. */
template <CMoveConstructible T, CMoveConstructible U> requires (CDerivedFrom<T, IAdaptorClosure<T>> && CDerivedFrom<U, IAdaptorClosure<U>>)
class TPipeClosure final : public IAdaptorClosure<TPipeClosure<T, U>>
{
public:

	FORCEINLINE constexpr explicit TPipeClosure(T InLHS, U InRHS) : LHS(MoveTemp(InLHS)), RHS(MoveTemp(InRHS)) { }

	template <typename R> requires (CInvocable<T&, R> && CInvocable<U&, TInvokeResult<T&, R>>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &
	{
		return Forward<R>(Range) | LHS | RHS;
	}

	template <typename R> requires (CInvocable<const T&, R> && CInvocable<const U&, TInvokeResult<const T&, R>>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&
	{
		return Forward<R>(Range) | LHS | RHS;
	}

	template <typename R> requires (CInvocable<T&&, R> && CInvocable<U&&, TInvokeResult<T&&, R>>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &&
	{
		return Forward<R>(Range) | MoveTemp(LHS) | MoveTemp(RHS);
	}

	template <typename R> requires (CInvocable<const T&&, R> && CInvocable<const U&&, TInvokeResult<const T&&, R>>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&&
	{
		return Forward<R>(Range) | MoveTemp(LHS) | MoveTemp(RHS);
	}

private:

	NO_UNIQUE_ADDRESS T LHS;
	NO_UNIQUE_ADDRESS U RHS;
};

/** Apply the range adaptor closure to the range. */
template <CRange R, CInvocable<R> T> requires (CDerivedFrom<TRemoveCVRef<T>, IAdaptorClosure<TRemoveCVRef<T>>>)
NODISCARD FORCEINLINE constexpr auto operator|(R&& Range, T&& Closure)
{
	return Invoke(Forward<T>(Closure), Forward<R>(Range));
}

/** Create a pipe closure that wraps two adaptor closures. */
template <CMoveConstructible T, CMoveConstructible U> requires (CDerivedFrom<T, IAdaptorClosure<T>> && CDerivedFrom<U, IAdaptorClosure<U>>)
NODISCARD FORCEINLINE constexpr auto operator|(T LHS, U RHS)
{
	return TPipeClosure(MoveTemp(LHS), MoveTemp(RHS));
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
