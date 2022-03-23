#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, T... Ints>
struct TIntegerSequence 
{
	using ValueType = T;
	static constexpr size_t Size() { return sizeof...(Ints); }
};

NAMESPACE_PRIVATE_BEGIN

#ifdef _MSC_VER

template <unsigned N, typename T>
struct TMakeIntegerSequence
{
	using Type = typename __make_integer_seq<TIntegerSequence, T, N>;
};

#elif __has_builtin(__make_integer_seq)

template <unsigned N, typename T>
struct TMakeIntegerSequence
{
	using Type = typename __make_integer_seq<TIntegerSequence, T, N>;
};

#else

template <unsigned N, typename T, T... Ints>
struct TMakeIntegerSequence
{
	using Type = typename TMakeIntegerSequence<N - 1, T, T(N - 1), Ints...>::Type;
};

template <typename T, T... Ints>
struct TMakeIntegerSequence<0, T, Ints...>
{
	using Type = TIntegerSequence<T, Ints...>;
};

#endif

NAMESPACE_PRIVATE_END

template <size_t... Ints>
using TIndexSequence = TIntegerSequence<size_t, Ints...>;

template<typename T, T N>
using TMakeIntegerSequence = typename NAMESPACE_PRIVATE::TMakeIntegerSequence<N, T>::Type;

template<size_t N>
using TMakeIndexSequence = TMakeIntegerSequence<size_t, N>;

template<typename... T>
using TIndexSequenceFor = TMakeIndexSequence<sizeof...(T)>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
