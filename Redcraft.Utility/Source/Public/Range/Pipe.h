#pragma once

#include "CoreTypes.h"
#include "Range/Utility.h"
#include "Templates/Tuple.h"
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
template <CDefaultConstructible F, CMoveConstructible... Ts> requires (CEmpty<F> && ... && CSameAs<TDecay<Ts>, Ts>)
class TAdaptorClosure : public IAdaptorClosure<TAdaptorClosure<F, Ts...>>
{
public:

	template <typename... Us> requires (CConstructibleFrom<TTuple<Ts...>, Us...> && ... && CSameAs<TDecay<Us>, Ts>)
	FORCEINLINE constexpr explicit TAdaptorClosure(Us&&... InArgs) : Args(Forward<Us>(InArgs)...) { }

	template <typename R> requires (CInvocable<F, R, Ts&...>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &
	{
		return [this, &Range]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			return Invoke(F(), Forward<R>(Range), Args.template GetValue<Indices>()...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

	template <typename R> requires (CInvocable<F, R, const Ts&...>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&
	{
		return [this, &Range]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			return Invoke(F(), Forward<R>(Range), Args.template GetValue<Indices>()...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

	template <typename R> requires (CInvocable<F, R, Ts&&...>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) &&
	{
		return [this, &Range]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			return Invoke(F(), Forward<R>(Range), MoveTemp(Args).template GetValue<Indices>()...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

	template <typename R> requires (CInvocable<F, R, const Ts&&...>)
	NODISCARD FORCEINLINE constexpr auto operator()(R&& Range) const&&
	{
		return [this, &Range]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			return Invoke(F(), Forward<R>(Range), MoveTemp(Args).template GetValue<Indices>()...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

private:

	NO_UNIQUE_ADDRESS TTuple<Ts...> Args;

};

/** A pipe closure that wraps two adaptor closures. */
template <CMoveConstructible T, CMoveConstructible U>
	requires (CSameAs<TRemoveCVRef<T>, T> && CDerivedFrom<T, IAdaptorClosure<T>>
	       && CSameAs<TRemoveCVRef<U>, U> && CDerivedFrom<U, IAdaptorClosure<U>>)
class TPipeClosure final : public IAdaptorClosure<TPipeClosure<T, U>>
{
public:

	template <typename V, typename W>
		requires (CSameAs<TRemoveCVRef<V>, T> && CConstructibleFrom<T, V>
		       && CSameAs<TRemoveCVRef<W>, U> && CConstructibleFrom<U, W>)
	FORCEINLINE constexpr explicit TPipeClosure(V InLHS, W InRHS)
		: LHS(Forward<V>(InLHS)), RHS(Forward<W>(InRHS))
	{ }

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
template <CMoveConstructible T, CMoveConstructible U>
	requires (CDerivedFrom<TRemoveCVRef<T>, IAdaptorClosure<TRemoveCVRef<T>>>
	       && CDerivedFrom<TRemoveCVRef<U>, IAdaptorClosure<TRemoveCVRef<U>>>)
NODISCARD FORCEINLINE constexpr auto operator|(T&& LHS, U&& RHS)
{
	return TPipeClosure<TRemoveCVRef<T>, TRemoveCVRef<U>>(Forward<T>(LHS), Forward<U>(RHS));
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
