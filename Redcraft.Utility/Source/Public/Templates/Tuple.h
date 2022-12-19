#pragma once

#include "CoreTypes.h"
#include "Templates/Meta.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/ReferenceWrapper.h"

#include <tuple>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#define RS_TUPLE_ELEMENT_STATIC_ALIAS 1

template <typename... Ts>
class TTuple;

NAMESPACE_PRIVATE_BEGIN

template <typename    T > struct TIsTTuple                : FFalse { };
template <typename... Ts> struct TIsTTuple<TTuple<Ts...>> : FTrue  { };

struct FForwardingConstructor { explicit FForwardingConstructor() = default; };
struct FOtherTupleConstructor { explicit FOtherTupleConstructor() = default; };

inline constexpr FForwardingConstructor ForwardingConstructor{ };
inline constexpr FOtherTupleConstructor OtherTupleConstructor{ };

template <typename TupleType>
struct TTupleArityImpl;

template <typename... Ts>
struct TTupleArityImpl<TTuple<Ts...>> : TConstant<size_t, Meta::TSize<TTypeSequence<Ts...>>> { };

template <typename T, typename TupleType>
struct TTupleIndexImpl;

template <typename T, typename... Ts>
struct TTupleIndexImpl<T, TTuple<Ts...>> : TConstant<size_t, Meta::TIndex<T, TTypeSequence<Ts...>>> { };

template <size_t I, typename TupleType>
struct TTupleElementImpl;

template <size_t I, typename... Ts>
struct TTupleElementImpl<I, TTuple<Ts...>>
{
	using Type = Meta::TType<I, TTypeSequence<Ts...>>;
};

template <bool bTrue, typename... Ts>
struct TTupleConvertCopy : FTrue { };

template <typename T, typename U>
struct TTupleConvertCopy<false, T, U> 
	: TBoolConstant<!(CConvertibleTo<const TTuple<U>&, T>
		|| CConstructibleFrom<T, const TTuple<U>&>
		|| CSameAs<T, U>)>
{ };

template <bool bTrue, typename... Ts>
struct TTupleConvertMove : FTrue { };

template <typename T, typename U>
struct TTupleConvertMove<false, T, U>
	: TBoolConstant<!(CConvertibleTo<TTuple<U>&&, T>
		|| CConstructibleFrom<T, TTuple<U>&&>
		|| CSameAs<T, U>)>
{ };

template <typename T, size_t Index>
struct TTupleBasicElement
{
private:

	using ValueType = T;
	ValueType Value;

public:

	template <typename Type>
	FORCEINLINE constexpr TTupleBasicElement(Type&& Arg)
		: Value(Forward<Type>(Arg))
	{ }

	FORCEINLINE constexpr TTupleBasicElement()                                     = default;
	FORCEINLINE constexpr TTupleBasicElement(const TTupleBasicElement&)            = default;
	FORCEINLINE constexpr TTupleBasicElement(TTupleBasicElement&&)                 = default;
	FORCEINLINE constexpr TTupleBasicElement& operator=(const TTupleBasicElement&) = default;
	FORCEINLINE constexpr TTupleBasicElement& operator=(TTupleBasicElement&&)      = default;
	FORCEINLINE constexpr ~TTupleBasicElement()                                    = default;
	
	FORCEINLINE constexpr                T&  GetValue()               &  { return static_cast<               T& >(Value); }
	FORCEINLINE constexpr const          T&  GetValue() const         &  { return static_cast<const          T& >(Value); }
	FORCEINLINE constexpr       volatile T&  GetValue()       volatile&  { return static_cast<      volatile T& >(Value); }
	FORCEINLINE constexpr const volatile T&  GetValue() const volatile&  { return static_cast<const volatile T& >(Value); }
	FORCEINLINE constexpr                T&& GetValue()               && { return static_cast<               T&&>(Value); }
	FORCEINLINE constexpr const          T&& GetValue() const         && { return static_cast<const          T&&>(Value); }
	FORCEINLINE constexpr       volatile T&& GetValue()       volatile&& { return static_cast<      volatile T&&>(Value); }
	FORCEINLINE constexpr const volatile T&& GetValue() const volatile&& { return static_cast<const volatile T&&>(Value); }
};

#if RS_TUPLE_ELEMENT_STATIC_ALIAS

#define DEFINE_TTupleBasicElement(Index, Name)                                                                                 \
	template <typename T>                                                                                                      \
	struct TTupleBasicElement<T, Index>                                                                                        \
	{                                                                                                                          \
		using Name##Type = T;                                                                                                  \
		Name##Type Name;                                                                                                       \
		                                                                                                                       \
		template <typename Type>                                                                                               \
		FORCEINLINE constexpr TTupleBasicElement(Type&& Arg)                                                                   \
			: Name(Forward<Type>(Arg))                                                                                         \
		{ }                                                                                                                    \
		                                                                                                                       \
		FORCEINLINE constexpr TTupleBasicElement()                                     = default;                              \
		FORCEINLINE constexpr TTupleBasicElement(const TTupleBasicElement&)            = default;                              \
		FORCEINLINE constexpr TTupleBasicElement(TTupleBasicElement&&)                 = default;                              \
		FORCEINLINE constexpr TTupleBasicElement& operator=(const TTupleBasicElement&) = default;                              \
		FORCEINLINE constexpr TTupleBasicElement& operator=(TTupleBasicElement&&)      = default;                              \
		FORCEINLINE constexpr ~TTupleBasicElement()                                    = default;                              \
		                                                                                                                       \
		FORCEINLINE constexpr                T&  GetValue()               &  { return static_cast<               T& >(Name); } \
		FORCEINLINE constexpr const          T&  GetValue() const         &  { return static_cast<const          T& >(Name); } \
		FORCEINLINE constexpr       volatile T&  GetValue()       volatile&  { return static_cast<      volatile T& >(Name); } \
		FORCEINLINE constexpr const volatile T&  GetValue() const volatile&  { return static_cast<const volatile T& >(Name); } \
		FORCEINLINE constexpr                T&& GetValue()               && { return static_cast<               T&&>(Name); } \
		FORCEINLINE constexpr const          T&& GetValue() const         && { return static_cast<const          T&&>(Name); } \
		FORCEINLINE constexpr       volatile T&& GetValue()       volatile&& { return static_cast<      volatile T&&>(Name); } \
		FORCEINLINE constexpr const volatile T&& GetValue() const volatile&& { return static_cast<const volatile T&&>(Name); } \
	}

DEFINE_TTupleBasicElement(0x0, First);
DEFINE_TTupleBasicElement(0x1, Second);
DEFINE_TTupleBasicElement(0x2, Third);
DEFINE_TTupleBasicElement(0x3, Fourth);
DEFINE_TTupleBasicElement(0x4, Fifth);
DEFINE_TTupleBasicElement(0x5, Sixth);
DEFINE_TTupleBasicElement(0x6, Seventh);
DEFINE_TTupleBasicElement(0x7, Eighth);
DEFINE_TTupleBasicElement(0x8, Ninth);
DEFINE_TTupleBasicElement(0x9, Tenth);
DEFINE_TTupleBasicElement(0xA, Eleventh);
DEFINE_TTupleBasicElement(0xB, Twelfth);
DEFINE_TTupleBasicElement(0xC, Thirteenth);
DEFINE_TTupleBasicElement(0xD, Fourteenth);
DEFINE_TTupleBasicElement(0xE, Fifteenth);
DEFINE_TTupleBasicElement(0xF, Sixteenth);

#undef DEFINE_TTupleBasicElement

#endif

template <typename... Ts>
FORCEINLINE constexpr TTuple<TUnwrapRefDecay<Ts>...> MakeTupleImpl(Ts&&... Args)
{
	return TTuple<TUnwrapRefDecay<Ts>...>(Forward<Ts>(Args)...);
}

template <typename Indices, typename... Ts>
class TTupleImpl;

template <size_t... Indices, typename... Ts>
class TTupleImpl<TIndexSequence<Indices...>, Ts...> : public TTupleBasicElement<Ts, Indices>...
{
protected:

	FORCEINLINE constexpr TTupleImpl()                             = default;
	FORCEINLINE constexpr TTupleImpl(const TTupleImpl&)            = default;
	FORCEINLINE constexpr TTupleImpl(TTupleImpl&&)                 = default;
	FORCEINLINE constexpr TTupleImpl& operator=(const TTupleImpl&) = default;
	FORCEINLINE constexpr TTupleImpl& operator=(TTupleImpl&&)      = default;
	FORCEINLINE constexpr ~TTupleImpl()                            = default;

	template <typename... ArgTypes>
	FORCEINLINE constexpr explicit TTupleImpl(FForwardingConstructor, ArgTypes&&... Args)
		: TTupleBasicElement<Ts, Indices>(Forward<ArgTypes>(Args))...
	{ }

	template <typename TupleType>
	FORCEINLINE constexpr explicit TTupleImpl(FOtherTupleConstructor, TupleType&& InValue)
		: TTupleBasicElement<Ts, Indices>(Forward<TupleType>(InValue).template GetValue<Indices>())...
	{ }

};

template <typename Indices, typename... Ts>
class TTupleHelper;

template <size_t... Indices>
class TTupleHelper<TIndexSequence<Indices...>>
{
public:

	template <typename LHSTupleType, typename RHSTupleType>
	FORCEINLINE static constexpr void Assign(LHSTupleType& LHS, RHSTupleType&& RHS)
	{
		static_assert(sizeof...(Indices) == TTupleArityImpl<TRemoveCVRef<LHSTupleType>>::Value
			&& TTupleArityImpl<TRemoveCVRef<LHSTupleType>>::Value == TTupleArityImpl<TRemoveCVRef<RHSTupleType>>::Value,
			"Cannot assign tuple from different size");
		
		((LHS.template GetValue<Indices>() = Forward<RHSTupleType>(RHS).template GetValue<Indices>()), ...);
	}

	template <typename F, typename TTupleType>
	FORCEINLINE static constexpr auto Apply(F&& Func, TTupleType&& Arg)
	{
		return Invoke(Forward<F>(Func), Forward<TTupleType>(Arg).template GetValue<Indices>()...);
	}

	template <typename F, typename TTupleType>
	FORCEINLINE static constexpr auto Transform(F&& Func, TTupleType&& Arg)
	{
		return MakeTupleImpl(Invoke(Forward<F>(Func), Forward<TTupleType>(Arg).template GetValue<Indices>())...);
	}

	template <typename T, typename TTupleType>
	FORCEINLINE static constexpr T Construct(TTupleType&& Arg)
	{
		return T(Forward<TTupleType>(Arg).template GetValue<Indices>()...);
	}

};

template <typename R, typename Indices>
struct TTupleThreeWay;

template <typename R, size_t I, size_t... Indices>
struct TTupleThreeWay<R, TIndexSequence<I, Indices...>>
{
	template <typename LHSTupleType, typename RHSTupleType>
	FORCEINLINE static constexpr R Do(const LHSTupleType& LHS, const RHSTupleType& RHS)
	{
		auto Result = SynthThreeWayCompare(LHS.template GetValue<I>(), RHS.template GetValue<I>());
		if (Result != 0) return Result;
		return TTupleThreeWay<R, TIndexSequence<Indices...>>::Do(LHS, RHS);
	}
};

template <typename R>
struct TTupleThreeWay<R, TIndexSequence<>>
{
	template <typename LHSTupleType, typename RHSTupleType>
	FORCEINLINE static constexpr R Do(const LHSTupleType& LHS, const RHSTupleType& RHS)
	{
		return R::equivalent;
	}
};

template <typename, typename> struct TTTupleWeaklyEqualityComparable;

template <typename T, typename U, typename... Ts, typename... Us>
struct TTTupleWeaklyEqualityComparable<TTypeSequence<T, Ts...>, TTypeSequence<U, Us...>>
	: TBoolConstant<CWeaklyEqualityComparable<T, U> && TTTupleWeaklyEqualityComparable<TTypeSequence<Ts...>, TTypeSequence<Us...>>::Value>
{ };

template <>
struct TTTupleWeaklyEqualityComparable<TTypeSequence<>, TTypeSequence<>> : FTrue { };

template <typename TSequence, typename USequence>
concept CTTupleWeaklyEqualityComparable = TTTupleWeaklyEqualityComparable<TSequence, USequence>::Value;

template <typename, typename> struct TTTupleSynthThreeWayComparable;

template <typename T, typename U, typename... Ts, typename... Us>
struct TTTupleSynthThreeWayComparable<TTypeSequence<T, Ts...>, TTypeSequence<U, Us...>>
	: TBoolConstant<CSynthThreeWayComparable<T, U> && TTTupleSynthThreeWayComparable<TTypeSequence<Ts...>, TTypeSequence<Us...>>::Value>
{ };

template <>
struct TTTupleSynthThreeWayComparable<TTypeSequence<>, TTypeSequence<>> : FTrue { };

template <typename TSequence, typename USequence>
concept CTTupleSynthThreeWayComparable = TTTupleSynthThreeWayComparable<TSequence, USequence>::Value;

NAMESPACE_PRIVATE_END

template <typename T>
concept CTTuple = NAMESPACE_PRIVATE::TIsTTuple<TRemoveCV<T>>::Value;

template <CTTuple T>
inline constexpr size_t TTupleArity = NAMESPACE_PRIVATE::TTupleArityImpl<TRemoveCV<T>>::Value;

template <typename T, CTTuple U>
inline constexpr size_t TTupleIndex = NAMESPACE_PRIVATE::TTupleIndexImpl<T, TRemoveCV<U>>::Value;

template <size_t I, CTTuple U>
using TTupleElement = TCopyCV<U, typename NAMESPACE_PRIVATE::TTupleElementImpl<I, TRemoveCV<U>>::Type>;

template <typename... Ts>
class TTuple : public NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Ts...>, Ts...>
{
private:

	using Super = NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Ts...>, Ts...>;
	using Helper = NAMESPACE_PRIVATE::TTupleHelper<TIndexSequenceFor<Ts...>>;

public:

	FORCEINLINE constexpr TTuple() = default;
	
	template <typename... ArgTypes> requires (sizeof...(Ts) >= 1 && sizeof...(ArgTypes) == sizeof...(Ts))
		&& (true && ... && CConstructibleFrom<Ts, ArgTypes&&>)
	FORCEINLINE constexpr explicit (!(true && ... && CConvertibleTo<ArgTypes&&, Ts>)) TTuple(ArgTypes&&... Args)
		: Super(NAMESPACE_PRIVATE::ForwardingConstructor, Forward<ArgTypes>(Args)...)
	{ }
	
	template <typename... Us> requires (sizeof...(Us) == sizeof...(Ts)
		&& (true && ... && CConstructibleFrom<Ts, const Us&>)
		&& NAMESPACE_PRIVATE::TTupleConvertCopy<sizeof...(Ts) != 1, Ts..., Us...>::Value)
	FORCEINLINE constexpr explicit (!(true && ... && CConvertibleTo<Us&&, Ts>)) TTuple(const TTuple<Us...>& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, InValue)
	{ }

	template <typename... Us> requires (sizeof...(Us) == sizeof...(Ts)
		&& (true && ... && CConstructibleFrom<Ts, Us&&>)
		&& NAMESPACE_PRIVATE::TTupleConvertMove<sizeof...(Ts) != 1, Ts..., Us...>::Value)
	FORCEINLINE constexpr explicit (!(true && ... && CConvertibleTo<Us&&, Ts>)) TTuple(TTuple<Us...>&& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, MoveTemp(InValue))
	{ }

	FORCEINLINE constexpr TTuple(const TTuple&) = default;
	FORCEINLINE constexpr TTuple(TTuple&&)      = default;

	template <typename... Us> requires (sizeof...(Us) == sizeof...(Ts)
		&& (true && ... && CAssignableFrom<Ts&, const Us&>))
	FORCEINLINE constexpr TTuple& operator=(const TTuple<Us...>& InValue)
	{
		Helper::Assign(*this, InValue);
		return *this;
	}

	template <typename... Us> requires (sizeof...(Us) == sizeof...(Ts)
		&& (true && ... && CAssignableFrom<Ts&, Us&&>))
	FORCEINLINE constexpr TTuple& operator=(TTuple<Us...>&& InValue)
	{
		Helper::Assign(*this, MoveTemp(InValue));
		return *this;
	}

	FORCEINLINE constexpr TTuple& operator=(const TTuple&) = default;
	FORCEINLINE constexpr TTuple& operator=(TTuple&&)      = default;
	
	template <typename... Us> requires (sizeof...(Ts) == sizeof...(Us) && NAMESPACE_PRIVATE::CTTupleWeaklyEqualityComparable<TTypeSequence<Ts...>, TTypeSequence<Us...>>)
	friend FORCEINLINE constexpr bool operator==(const TTuple& LHS, const TTuple<Us...>& RHS)
	{
		if constexpr (sizeof...(Ts) != sizeof...(Us)) return false;

		return [&LHS, &RHS]<size_t... Indices>(TIndexSequence<Indices...>) -> bool
		{
			return (true && ... && (LHS.template GetValue<Indices>() == RHS.template GetValue<Indices>()));
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}
	
	template <typename... Us> requires (sizeof...(Ts) == sizeof...(Us) && NAMESPACE_PRIVATE::CTTupleSynthThreeWayComparable<TTypeSequence<Ts...>, TTypeSequence<Us...>>)
	friend FORCEINLINE constexpr TCommonComparisonCategory<TSynthThreeWayResult<Ts, Us>...> operator<=>(const TTuple& LHS, const TTuple<Us...>& RHS)
	{
		using R = TCommonComparisonCategory<TSynthThreeWayResult<Ts, Us>...>;
		return NAMESPACE_PRIVATE::TTupleThreeWay<R, TMakeIndexSequence<sizeof...(Ts)>>::Do(LHS, RHS);
	}

	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue()               &  { return static_cast<               NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const         &  { return static_cast<const          NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue()       volatile&  { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const volatile&  { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue()               && { return static_cast<               NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const         && { return static_cast<const          NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue()       volatile&& { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const volatile&& { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Ts...>>, I>&&>(*this).GetValue(); }

	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue()               &  { return static_cast<               TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const         &  { return static_cast<const          TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue()       volatile&  { return static_cast<      volatile TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const volatile&  { return static_cast<const volatile TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue()               && { return static_cast<               TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const         && { return static_cast<const          TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue()       volatile&& { return static_cast<      volatile TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const volatile&& { return static_cast<const volatile TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Ts...>>>(); }

	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func)               &  { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func) const         &  { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func)       volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func) const volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func)               && { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func) const         && { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func)       volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires (CInvocable<F, Ts...>) FORCEINLINE constexpr decltype(auto) Apply(F&& Func) const volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func)               &  { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func) const         &  { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func)       volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func) const volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func)               && { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func) const         && { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func)       volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Ts> && !CSameAs<void, TInvokeResult<F, Ts>>)) FORCEINLINE constexpr decltype(auto) Transform(F&& Func) const volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct()               &  { return Helper::template Construct<T>(static_cast<               TTuple& >(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct() const         &  { return Helper::template Construct<T>(static_cast<const          TTuple& >(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct()       volatile&  { return Helper::template Construct<T>(static_cast<      volatile TTuple& >(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct() const volatile&  { return Helper::template Construct<T>(static_cast<const volatile TTuple& >(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct()               && { return Helper::template Construct<T>(static_cast<               TTuple&&>(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct() const         && { return Helper::template Construct<T>(static_cast<const          TTuple&&>(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct()       volatile&& { return Helper::template Construct<T>(static_cast<      volatile TTuple&&>(*this)); }
	template <typename T> requires (CConstructibleFrom<T, Ts...>) FORCEINLINE constexpr T Construct() const volatile&& { return Helper::template Construct<T>(static_cast<const volatile TTuple&&>(*this)); }
	
	friend FORCEINLINE constexpr size_t GetTypeHash(const TTuple& A) requires (true && ... && CHashable<Ts>)
	{
		return [&A]<size_t... Indices>(TIndexSequence<Indices...>) -> size_t
		{
			return HashCombine(GetTypeHash(A.template GetValue<Indices>())...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

	friend FORCEINLINE constexpr void Swap(TTuple& A, TTuple& B) requires (true && ... && (CMoveConstructible<Ts> && CSwappable<Ts>))
	{
		[&A, &B]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			((Swap(A.template GetValue<Indices>(), B.template GetValue<Indices>())), ...);
		}
		(TMakeIndexSequence<sizeof...(Ts)>());
	}

};

template <typename... Ts>
TTuple(Ts...) -> TTuple<Ts...>;

template <typename T, typename U>
using TPair = TTuple<T, U>;

template <typename... Ts>
FORCEINLINE constexpr TTuple<TUnwrapRefDecay<Ts>...> MakeTuple(Ts&&... Args)
{
	return TTuple<TUnwrapRefDecay<Ts>...>(Forward<Ts>(Args)...);
}

template <typename... Ts>
FORCEINLINE constexpr TTuple<Ts&...> Tie(Ts&... Args)
{
	return TTuple<Ts&...>(Args...);
}

template <typename... Ts>
FORCEINLINE constexpr TTuple<Ts&&...> ForwardAsTuple(Ts&&... Args)
{
	return TTuple<Ts&&...>(Forward<Ts>(Args)...);
}

NAMESPACE_PRIVATE_BEGIN

struct FTupleEndFlag { };

template <typename... TTupleTypes>
struct TTupleCatResultImpl;

template <typename... Ts, typename... TTupleTypes>
struct TTupleCatResultImpl<TTuple<Ts...>, TTupleTypes...>
{
	using Type = typename TTupleCatResultImpl<TTupleTypes..., Ts...>::Type;
};

template <typename... Ts>
struct TTupleCatResultImpl<FTupleEndFlag, Ts...>
{
	using Type = TTuple<Ts...>;
};

template <typename R, typename Indices>
struct TTupleCatMake;

template <typename... RTypes, size_t... Indices>
struct TTupleCatMake<TTuple<RTypes...>, TIndexSequence<Indices...>>
{
	template <typename T, typename U>
	struct ForwardType { using Type = TConditional<CRValueReference<T>, TRemoveReference<U>&&, U>; };

	template <typename TTupleType>
	FORCEINLINE static constexpr TTuple<RTypes...> Do(TTupleType&& InValue)
	{
		return TTuple<RTypes...>
			(
				static_cast<typename ForwardType<RTypes, decltype(Forward<TTupleType>(InValue).template GetValue<Indices>())>::Type>
					(
						Forward<TTupleType>(InValue).template GetValue<Indices>()
					)...
			);
	}
};

template <typename ForwardIndices, typename TTupleIndices>
struct TTupleCatForward;

template <size_t... ForwardIndices, size_t... TTupleIndices>
struct TTupleCatForward<TIndexSequence<ForwardIndices...>, TIndexSequence<TTupleIndices...>>
{
	template <typename ForwardType, typename TTupleType>
	FORCEINLINE static constexpr decltype(auto) Do(ForwardType&& ForwardTuple, TTupleType&& InValue)
	{
		return ForwardAsTuple(Forward<ForwardType>(ForwardTuple).template GetValue<ForwardIndices>()..., Forward<TTupleType>(InValue).template GetValue<TTupleIndices>()...);
	}
};

template <typename R>
struct TTupleCatImpl
{
	template <typename ForwardType, typename TTupleType, typename... OtherTTupleTypes>
	FORCEINLINE static constexpr decltype(auto) Do(ForwardType&& ForwardTuple, TTupleType&& InValue, OtherTTupleTypes&&... OtherValue)
	{
		return Do(TTupleCatForward<
			TMakeIndexSequence<TTupleArity<TRemoveReference<ForwardType>>>,
			TMakeIndexSequence<TTupleArity<TRemoveReference<TTupleType>>>>
			::Do(Forward<ForwardType>(ForwardTuple), Forward<TTupleType>(InValue)), Forward<OtherTTupleTypes>(OtherValue)...);
	}

	template <typename ForwardType>
	FORCEINLINE static constexpr decltype(auto) Do(ForwardType&& ForwardTuple)
	{
		return TTupleCatMake<R, TMakeIndexSequence<TTupleArity<ForwardType>>>::Do(Forward<ForwardType>(ForwardTuple));
	}
};

template <typename Indices>
struct TTupleVisitImpl;

template <size_t I, size_t... Indices>
struct TTupleVisitImpl<TIndexSequence<I, Indices...>>
{
	template <typename F, typename... TupleTypes>
	FORCEINLINE static constexpr void Do(F&& Func, TupleTypes&&... Tuples)
	{
		Invoke(Forward<F>(Func), Forward<TupleTypes>(Tuples).template GetValue<I>()...);
		TTupleVisitImpl<TIndexSequence<Indices...>>::Do(Forward<F>(Func), Forward<TupleTypes>(Tuples)...);
	}
};

template <>
struct TTupleVisitImpl<TIndexSequence<>>
{
	template <typename... TupleTypes>
	FORCEINLINE static constexpr void Do(TupleTypes&&... Tuples) { }
};

NAMESPACE_PRIVATE_END

template <typename... TTupleTypes> requires (true && ... && CTTuple<TRemoveCVRef<TTupleTypes>>)
using TTupleCatResult = typename NAMESPACE_PRIVATE::TTupleCatResultImpl<TRemoveReference<TTupleTypes>..., NAMESPACE_PRIVATE::FTupleEndFlag>::Type;;

template <typename... TTupleTypes> requires (true && ... && CTTuple<TRemoveCVRef<TTupleTypes>>)
FORCEINLINE constexpr decltype(auto) TupleCat(TTupleTypes&&... Args)
{
	using R = TTupleCatResult<TTupleTypes...>;
	if constexpr (sizeof...(Args) == 0) return R();
	else return NAMESPACE_PRIVATE::TTupleCatImpl<R>::Do(Forward<TTupleTypes>(Args)...);
}

template <typename F, typename FirstTupleType, typename... TupleTypes>
	requires (CTTuple<TRemoveReference<FirstTupleType>> && (true && ... && CTTuple<TRemoveReference<TupleTypes>>))
FORCEINLINE constexpr void VisitTuple(F&& Func, FirstTupleType&& FirstTuple, TupleTypes&&... Tuples)
{
	NAMESPACE_PRIVATE::TTupleVisitImpl<TMakeIndexSequence<TTupleArity<TRemoveReference<FirstTupleType>>>>
		::Do(Forward<F>(Func), Forward<FirstTupleType>(FirstTuple), Forward<TupleTypes>(Tuples)...);
}

template <typename... Ts, typename... Us> requires (requires { typename TTuple<TCommonType<Ts, Us>...>; })
struct TBasicCommonType<TTuple<Ts...>, TTuple<Us...>>
{
	using Type = TTuple<TCommonType<Ts, Us>...>;
};

template <typename... Ts, typename... Us, template<typename> typename TQualifiers, template<typename> typename UQualifiers>
	requires (requires { typename TTuple<TCommonReference<TQualifiers<Ts>, UQualifiers<Us>>...>; })
struct TBasicCommonReference<TTuple<Ts...>, TTuple<Us...>, TQualifiers, UQualifiers>
{
	using Type = TTuple<TCommonReference<TQualifiers<Ts>, UQualifiers<Us>>...>;
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

NAMESPACE_STD_BEGIN

// Support structure binding, should not be directly used
template <typename... Ts> struct tuple_size<NAMESPACE_REDCRAFT::TTuple<Ts...>> : integral_constant<size_t, NAMESPACE_REDCRAFT::TTupleArity<NAMESPACE_REDCRAFT::TTuple<Ts...>>> { };
template <size_t I, typename... Ts> struct tuple_element<I, NAMESPACE_REDCRAFT::TTuple<Ts...>> { using type = NAMESPACE_REDCRAFT::TTupleElement<I, NAMESPACE_REDCRAFT::TTuple<Ts...>>; };

NAMESPACE_STD_END

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Support structure binding, should not be directly used
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(               TTuple<Ts...>&  InValue) { return static_cast<               TTuple<Ts...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(const          TTuple<Ts...>&  InValue) { return static_cast<const          TTuple<Ts...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(      volatile TTuple<Ts...>&  InValue) { return static_cast<      volatile TTuple<Ts...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(const volatile TTuple<Ts...>&  InValue) { return static_cast<const volatile TTuple<Ts...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(               TTuple<Ts...>&& InValue) { return static_cast<               TTuple<Ts...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(const          TTuple<Ts...>&& InValue) { return static_cast<const          TTuple<Ts...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(      volatile TTuple<Ts...>&& InValue) { return static_cast<      volatile TTuple<Ts...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Ts> FORCEINLINE constexpr decltype(auto) get(const volatile TTuple<Ts...>&& InValue) { return static_cast<const volatile TTuple<Ts...>&&>(InValue).template GetValue<Index>(); }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
