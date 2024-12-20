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
		{ Container.Max() } -> CSameAs<size_t>;
	};

/** A concept specifies a container that can append elements. */
template <typename C, typename Ref>
concept CAppendableContainer =
	requires (C& Container, Ref&& Reference)
	{
		requires
		(
			requires { Container.EmplaceBack             (Forward<Ref>(Reference)); } ||
			requires { Container.PushBack                (Forward<Ref>(Reference)); } ||
			requires { Container.Emplace(Container.End(), Forward<Ref>(Reference)); } ||
			requires { Container.Insert (Container.End(), Forward<Ref>(Reference)); }
		);
	};

NAMESPACE_BEGIN(Range)

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
			return C(Range::Begin(Range), Range::End(Range), Forward<Ts>(Args)...);
		}

		else if constexpr (CConstructibleFrom<C, Ts...> && CAppendableContainer<C, TRangeReference<R>>)
		{
			C Result(Forward<Ts>(Args)...);

			if constexpr (CSizedRange<R> && CReservableContainer<C>)
			{
				Result.Reserve(Range::Num(Range));
			}

			for (TRangeReference<R> Element : Range)
			{
				if constexpr (requires { Result.EmplaceBack(DeclVal<TRangeReference<R>>()); })
				{
					Result.EmplaceBack(Forward<TRangeReference<R>>(Element));
				}

				else if constexpr (requires { Result.PushBack(DeclVal<TRangeReference<R>>()); })
				{
					Result.PushBack(Forward<TRangeReference<R>>(Element));
				}

				else if constexpr (requires { Result.Emplace(Result.End(), DeclVal<TRangeReference<R>>()); })
				{
					Result.Emplace(Result.End(), Forward<TRangeReference<R>>(Element));
				}

				else /* if constexpr (requires { Result.Insert(Result.End(), DeclVal<TRangeReference<R>>()); }) */
				{
					Result.Insert(Result.End(), Forward<TRangeReference<R>>(Element));
				}
			}

			return Result;
		}

		else static_assert(sizeof(R) == -1, "The container type is not constructible from a range");
	}
	else
	{
		if constexpr (CInputRange<TRangeReference<C>>)
		{
			return Range::To<C>(Range::All(Range) | Range::Transform([]<typename T>(T&& Element) { return Range::To<TRangeElement<C>>(Forward<T>(Element)); }), Forward<Args>(Args)...);
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
		return Range::To<decltype(C(DeclVal<R>(), DeclVal<Ts>()...))>(Forward<R>(Range), Forward<Ts>(Args)...);
	}

	else if constexpr (requires { C(DeclVal<TRangeIterator<R>>(), DeclVal<TRangeSentinel<R>>(), DeclVal<Args>()...); })
	{
		return Range::To<decltype(C(DeclVal<TRangeIterator<R>>(), DeclVal<TRangeSentinel<R>>(), DeclVal<Args>()...))>(Forward<R>(Range), Forward<Ts>(Args)...);
	}

	else static_assert(sizeof(R) == -1, "The container type is not constructible from a range");
}

/** Constructs a non-view object from the elements of the range. */
template <typename C, typename... Ts> requires (!CView<C>)
NODISCARD FORCEINLINE constexpr auto To(Ts&&... Args)
{
	using FClosure = decltype([]<CInputRange R, typename... Us> requires (requires { Range::To<C>(DeclVal<R>(), DeclVal<Us>()...); }) (R&& Range, Us&&... Args)
	{
		return Range::To<C>(Forward<R>(Range), Forward<Us>(Args)...);
	});

	return TAdaptorClosure<FClosure, TDecay<Ts>...>(Forward<Ts>(Args)...);
}

/** Constructs a non-view object from the elements of the range. */
template <template <typename...> typename C, typename... Ts>
NODISCARD FORCEINLINE constexpr auto To(Ts&&... Args)
{
	using FClosure = decltype([]<CInputRange R, typename... Us> requires (requires { Range::To<C>(DeclVal<R>(), DeclVal<Us>()...); }) (R&& Range, Us&&... Args)
	{
		return Range::To<C>(Forward<R>(Range), Forward<Us>(Args)...);
	});

	return TAdaptorClosure<FClosure, TDecay<Ts>...>(Forward<Ts>(Args)...);
}

NAMESPACE_END(Range)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
