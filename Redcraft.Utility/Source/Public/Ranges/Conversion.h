#pragma once

#include "CoreTypes.h"
#include "Ranges/View.h"
#include "Ranges/Utility.h"
#include "Ranges/AllView.h"
#include "Templates/Utility.h"
#include "Ranges/TransformView.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// NOTE: In the STL, use std::from_range_t as a disambiguation tag that resolves ambiguity
// introduced by the list-initialization. For example, for the following code:
//
//	R RangeOfInts = /* ... */;
//	static_assert(CRange<R> and CSameAs<TRangeElement<R>, int>);
//
//	TArray Arr(RangeOfInts);
//	TArray Brr{RangeOfInts};
//
// If R is TArray<int> than decltype(Arr) is TArray<int> and decltype(Brr) is TArray<int>,
// otherwise,               decltype(Arr) is TArray<int> and decltype(Brr) is TArray<R>.
//
// But Redcraft can't use the std::from_range_t tag because list-initialization is discouraged.

/** A concept specifies a container that can reserve size. */
template <typename C>
concept CReservableContainer = CSizedRange<C>
	&& requires (C& Container, size_t N)
	{
		  Container.Reserve(N);
		{ Container.Num() } -> CSameAs<size_t>;
		{ Container.Max() } -> CSameAs<size_t>;
	};

/** A concept specifies a container that can append elements. */
template <typename C, typename T>
concept CAppendableContainer =
	requires (C& Container, T&& Object)
	{
		requires
		(
			requires { Container.EmplaceBack             (Forward<T>(Object)); } ||
			requires { Container.PushBack                (Forward<T>(Object)); } ||
			requires { Container.Emplace(Container.End(), Forward<T>(Object)); } ||
			requires { Container.Insert (Container.End(), Forward<T>(Object)); }
		);
	};

NAMESPACE_BEGIN(Ranges)

template <typename T, CAppendableContainer<T> C>
FORCEINLINE constexpr void AppendTo(C& Container, T&& Object)
{
	if constexpr (requires { Container.EmplaceBack(Forward<T>(Object)); })
	{
		Container.EmplaceBack(Forward<T>(Object));
	}

	else if constexpr (requires { Container.PushBack(Forward<T>(Object)); })
	{
		Container.PushBack(Forward<T>(Object));
	}

	else if constexpr (requires { Container.Emplace(Container.End(), Forward<T>(Object)); })
	{
		Container.Emplace(Container.End(), Forward<T>(Object));
	}

	else /* if constexpr (requires { Container.Insert(Container.End(), Forward<T>(Object)); }) */
	{
		Container.Insert(Container.End(), Forward<T>(Object));
	}
}

/** Constructs a non-view object from the elements of the range. */
template <typename C, CInputRange R, typename... Ts> requires (!CView<C>)
NODISCARD FORCEINLINE constexpr auto To(R&& Range, Ts&&... Args)
{
	if constexpr (!CInputRange<C> || CConvertibleTo<TRangeReference<R>, TRangeElement<C>>)
	{
		if constexpr (CConstructibleFrom<C, R, Ts...>)
		{
			return C(Forward<R>(Range), Forward<Ts>(Args)...);
		}

		else if constexpr (CCommonRange<R> && CInputRange<R> && CConstructibleFrom<C, TRangeIterator<R>, TRangeSentinel<R>, Ts...>)
		{
			return C(Ranges::Begin(Range), Ranges::End(Range), Forward<Ts>(Args)...);
		}

		else if constexpr (CConstructibleFrom<C, Ts...> && CAppendableContainer<C, TRangeReference<R>>)
		{
			C Result(Forward<Ts>(Args)...);

			if constexpr (CSizedRange<R> && CReservableContainer<C>)
			{
				Result.Reserve(Ranges::Num(Range));
			}

			for (TRangeReference<R> Element : Range)
			{
				Ranges::AppendTo(Result, Forward<TRangeReference<R>>(Element));
			}

			return Result;
		}

		else static_assert(sizeof(R) == -1, "The container type is not constructible from a range");
	}
	else
	{
		if constexpr (CInputRange<TRangeReference<C>>)
		{
			return Ranges::To<C>(Ranges::All(Range) | Ranges::Transform([]<typename T>(T&& Element) { return Ranges::To<TRangeElement<C>>(Forward<T>(Element)); }), Forward<Args>(Args)...);
		}

		else static_assert(sizeof(R) == -1, "The container type is not constructible from a range");
	}
}

/** Constructs a non-view object from the elements of the range. */
template <template <typename...> typename C, CInputRange R, typename... Ts>
NODISCARD FORCEINLINE constexpr auto To(R&& Range, Ts&&... Args)
{
	if constexpr (requires { C(DeclVal<R>(), DeclVal<Ts>()...); })
	{
		return Ranges::To<decltype(C(DeclVal<R>(), DeclVal<Ts>()...))>(Forward<R>(Range), Forward<Ts>(Args)...);
	}

	else if constexpr (requires { C(DeclVal<TRangeIterator<R>>(), DeclVal<TRangeSentinel<R>>(), DeclVal<Args>()...); })
	{
		return Ranges::To<decltype(C(DeclVal<TRangeIterator<R>>(), DeclVal<TRangeSentinel<R>>(), DeclVal<Args>()...))>(Forward<R>(Range), Forward<Ts>(Args)...);
	}

	else static_assert(sizeof(R) == -1, "The container type is not constructible from a range");
}

/** Constructs a non-view object from the elements of the range. */
template <typename C, typename... Ts> requires (!CView<C>)
NODISCARD FORCEINLINE constexpr auto To(Ts&&... Args)
{
	using FClosure = decltype([]<CInputRange R, typename... Us> requires (requires { Ranges::To<C>(DeclVal<R>(), DeclVal<Us>()...); }) (R&& Range, Us&&... Args)
	{
		return Ranges::To<C>(Forward<R>(Range), Forward<Us>(Args)...);
	});

	return TAdaptorClosure<FClosure, TDecay<Ts>...>(Forward<Ts>(Args)...);
}

/** Constructs a non-view object from the elements of the range. */
template <template <typename...> typename C, typename... Ts>
NODISCARD FORCEINLINE constexpr auto To(Ts&&... Args)
{
	using FClosure = decltype([]<CInputRange R, typename... Us> requires (requires { Ranges::To<C>(DeclVal<R>(), DeclVal<Us>()...); }) (R&& Range, Us&&... Args)
	{
		return Ranges::To<C>(Forward<R>(Range), Forward<Us>(Args)...);
	});

	return TAdaptorClosure<FClosure, TDecay<Ts>...>(Forward<Ts>(Args)...);
}

NAMESPACE_END(Ranges)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
