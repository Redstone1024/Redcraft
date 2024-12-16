#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Container.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T, T... Ints>
struct TIntegerSequence
{
	using FValueType = T;
	FORCEINLINE static constexpr size_t   Num()     { return sizeof...(Ints);                          }
	FORCEINLINE static constexpr const T* GetData() { return NAMESPACE_REDCRAFT::GetData({ Ints... }); }
};

NAMESPACE_PRIVATE_BEGIN

#ifdef _MSC_VER

template <unsigned N, typename T>
struct TMakeIntegerSequenceImpl
{
	using FType = typename __make_integer_seq<TIntegerSequence, T, N>;
};

#elif __has_builtin(__make_integer_seq)

template <unsigned N, typename T>
struct TMakeIntegerSequenceImpl
{
	using FType = typename __make_integer_seq<TIntegerSequence, T, N>;
};

#else

template <unsigned N, typename T, T... Ints>
struct TMakeIntegerSequenceImpl
{
	using FType = typename TMakeIntegerSequenceImpl<N - 1, T, T(N - 1), Ints...>::FType;
};

template <typename T, T... Ints>
struct TMakeIntegerSequenceImpl<0, T, Ints...>
{
	using FType = TIntegerSequence<T, Ints...>;
};

#endif

NAMESPACE_PRIVATE_END

template <size_t... Ints>
using TIndexSequence = TIntegerSequence<size_t, Ints...>;

template <typename T, T N>
using TMakeIntegerSequence = typename NAMESPACE_PRIVATE::TMakeIntegerSequenceImpl<N, T>::FType;

template <size_t N>
using TMakeIndexSequence = TMakeIntegerSequence<size_t, N>;

template <typename... Ts>
using TIndexSequenceFor = TMakeIndexSequence<sizeof...(Ts)>;

template <typename... Ts>
struct TTypeSequence { };

NAMESPACE_PRIVATE_BEGIN

template <typename T           > struct TIsTIntegerSequence                               : FFalse { };
template <typename T, T... Ints> struct TIsTIntegerSequence<TIntegerSequence<T, Ints...>> : FTrue  { };

template <typename  T   > struct TIsTIndexSequence                          : FFalse { };
template <size_t... Ints> struct TIsTIndexSequence<TIndexSequence<Ints...>> : FTrue  { };

template <typename    T > struct TIsTTypeSequence                       : FFalse { };
template <typename... Ts> struct TIsTTypeSequence<TTypeSequence<Ts...>> : FTrue  { };

NAMESPACE_PRIVATE_END

// Unlike other places such as CTTuple, cv-qualifiers are not ignored here
// This is done to simplify the logic and make it easier to use it

template <typename T>
concept CTIntegerSequence = NAMESPACE_PRIVATE::TIsTIntegerSequence<T>::Value;

template <typename T>
concept CTIndexSequence = NAMESPACE_PRIVATE::TIsTIndexSequence<T>::Value;

template <typename T>
concept CTTypeSequence = NAMESPACE_PRIVATE::TIsTTypeSequence<T>::Value;

NAMESPACE_BEGIN(Meta)

NAMESPACE_PRIVATE_BEGIN

template <typename TSequence>
struct TFrontImpl;

template <typename T, typename... Ts>
struct TFrontImpl<TTypeSequence<T, Ts...>>
{
	using FType = T;
};

template <typename TSequence>
struct TPopImpl;

template <typename T, typename... Ts>
struct TPopImpl<TTypeSequence<T, Ts...>>
{
	using FType = TTypeSequence<Ts...>;
};

template <typename T, typename TSequence>
struct TPushImpl;

template <typename T, typename... Ts>
struct TPushImpl<T, TTypeSequence<Ts...>>
{
	using FType = TTypeSequence<T, Ts...>;
};

NAMESPACE_PRIVATE_END

template <CTTypeSequence TSequence>
using TFront = typename NAMESPACE_PRIVATE::TFrontImpl<TSequence>::FType;

template <CTTypeSequence TSequence>
using TPop = typename NAMESPACE_PRIVATE::TPopImpl<TSequence>::FType;

template <typename T, typename TSequence>
using TPush = typename NAMESPACE_PRIVATE::TPushImpl<T, TSequence>::FType;

NAMESPACE_PRIVATE_BEGIN

template <typename T, typename TSequence>
struct TTypeCountImpl
{
	static constexpr size_t Value = (CSameAs<T, TFront<TSequence>> ? 1 : 0) + TTypeCountImpl<T, TPop<TSequence>>::Value;
};

template <typename T>
struct TTypeCountImpl<T, TTypeSequence<>>
{
	static constexpr size_t Value = 0;
};

NAMESPACE_PRIVATE_END

template <typename T, CTTypeSequence TSequence>
inline constexpr size_t TTypeCount = NAMESPACE_PRIVATE::TTypeCountImpl<T, TSequence>::Value;

template <typename T, typename TSequence>
concept CExistentType = CTTypeSequence<TSequence> && TTypeCount<T, TSequence> > 0;

template <typename T, typename TSequence>
concept CDuplicateType = CTTypeSequence<TSequence> && TTypeCount<T, TSequence> > 1;

NAMESPACE_PRIVATE_BEGIN

template <typename TSequence>
struct TSizeImpl;

template <typename... Ts>
struct TSizeImpl<TTypeSequence<Ts...>>
{
	static constexpr size_t Value = sizeof...(Ts);
};

template <size_t I, typename T, typename TSequence>
struct TIndexImpl
{
	static constexpr size_t Value = CSameAs<T, TFront<TSequence>> ? I : TIndexImpl<I + 1, T, TPop<TSequence>>::Value;
};

template <size_t I, typename T>
struct TIndexImpl<I, T, TTypeSequence<>>
{
	static constexpr size_t Value = INDEX_NONE;
};

template <typename T, typename TSequence>
struct TIndexAssert
{
	static_assert( Meta::CExistentType< T, TSequence>, "T is non-existent types in type sequence");
	static_assert(!Meta::CDuplicateType<T, TSequence>, "T is duplicate type in type sequence"    );
	static constexpr size_t Value = TIndexImpl<0, T, TSequence>::Value;
};

template <size_t I, typename TSequence>
struct TTypeImpl
{
	using FType = typename TTypeImpl<I - 1, TPop<TSequence>>::FType;
};

template <typename TSequence>
struct TTypeImpl<0, TSequence>
{
	using FType = TFront<TSequence>;
};

template <typename TSequence>
struct TTypeImpl<INDEX_NONE, TSequence>
{
	using FType = void;
};

template <size_t I, typename TSequence>
struct TTypeAssert
{
	static_assert(I < TSizeImpl<TSequence>::Value, "I is invalid index in type sequence");
	static constexpr size_t SafeIndex = I < TSizeImpl<TSequence>::Value ? I : INDEX_NONE;
	using FType = TCopyCV<TSequence, typename TTypeImpl<SafeIndex, TSequence>::FType>;
};

NAMESPACE_PRIVATE_END

template <CTTypeSequence TSequence>
inline constexpr size_t TSize = NAMESPACE_PRIVATE::TSizeImpl<TSequence>::Value;

template <typename T, CTTypeSequence TSequence>
inline constexpr size_t TIndex = NAMESPACE_PRIVATE::TIndexAssert<T, TSequence>::Value;

template <size_t I, CTTypeSequence TSequence>
using TType = typename NAMESPACE_PRIVATE::TTypeAssert<I, TSequence>::FType;

NAMESPACE_PRIVATE_BEGIN

template <typename TSequence>
struct TUniqueTypeSequenceImpl
{
	using FFrontType = TFront<TSequence>;
	using FNextSequence = TPop<TSequence>;
	using FNextUniqueSequence = typename TUniqueTypeSequenceImpl<FNextSequence>::FType;
	using FType = TConditional<!CExistentType<FFrontType, FNextSequence>, TPush<FFrontType, FNextUniqueSequence>, FNextUniqueSequence>;
};

template <>
struct TUniqueTypeSequenceImpl<TTypeSequence<>>
{
	using FType = TTypeSequence<>;
};

NAMESPACE_PRIVATE_END

template <CTTypeSequence TSequence>
using TUniqueTypeSequence = typename NAMESPACE_PRIVATE::TUniqueTypeSequenceImpl<TSequence>::FType;

NAMESPACE_PRIVATE_BEGIN

template <typename T>
struct TOverload
{
	using F = T(*)(T);
	operator F() const { return nullptr; }
};

template <typename TSequence>
struct TOverloadSetImpl;

template <typename... Ts>
struct TOverloadSetImpl<TTypeSequence<Ts...>> : public TOverload<Ts>... { };

template <typename TSequence>
struct TOverloadSet : public TOverloadSetImpl<TUniqueTypeSequence<TSequence>> { };

NAMESPACE_PRIVATE_END

template <typename T, typename TSequence>
using TOverloadResolution = decltype(DeclVal<NAMESPACE_PRIVATE::TOverloadSet<TSequence>>()(DeclVal<T>()));

NAMESPACE_END(Meta)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
