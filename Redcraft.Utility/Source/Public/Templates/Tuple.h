#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/IntegerSequence.h"
#include "Templates/ReferenceWrapper.h"

#include <tuple>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#define RS_TUPLE_ELEMENT_STATIC_ALIAS 1

template <typename... Types>
struct TTuple;

NAMESPACE_PRIVATE_BEGIN

template <typename    T    > struct TIsTTuple                   : FFalse { };
template <typename... Types> struct TIsTTuple<TTuple<Types...>> : FTrue  { };

struct FForwardingConstructor { explicit FForwardingConstructor() = default; };
struct FOtherTupleConstructor { explicit FOtherTupleConstructor() = default; };

inline constexpr FForwardingConstructor ForwardingConstructor{ };
inline constexpr FOtherTupleConstructor OtherTupleConstructor{ };

template <typename TupleType>
struct TTupleArityImpl;

template <typename... Types>
struct TTupleArityImpl<TTuple<Types...>> : TConstant<size_t, sizeof...(Types)> { };

template <typename... Types>
struct TTupleArityImpl<const TTuple<Types...>> : TConstant<size_t, sizeof...(Types)> { };

template <typename... Types>
struct TTupleArityImpl<volatile TTuple<Types...>> : TConstant<size_t, sizeof...(Types)> { };

template <typename... Types>
struct TTupleArityImpl<const volatile TTuple<Types...>> : TConstant<size_t, sizeof...(Types)> { };

template <typename T, typename TupleType>
struct TTupleIndexImpl;

template <typename T, typename U, typename... Types>
struct TTupleIndexImpl<T, TTuple<U, Types...>> : TConstant<size_t, TTupleIndexImpl<T, TTuple<Types...>>::Value + 1>
{
	static_assert(sizeof...(Types) != 0, "Non-existent types in tuple");
};

template <typename T, typename... Types>
struct TTupleIndexImpl<T, TTuple<T, Types...>> : TConstant<size_t, 0>
{
	static_assert((true && ... && !CSameAs<T, Types>), "Duplicate type in tuple");
};

template <typename T>
struct TTupleIndexImpl<T, TTuple<>> : TConstant<size_t, INDEX_NONE> { };

template <typename T, typename... Types>
struct TTupleIndexImpl<T, const TTuple<Types...>> : TTupleIndexImpl<T, TTuple<Types...>> { };

template <typename T, typename... Types>
struct TTupleIndexImpl<T, volatile TTuple<Types...>> : TTupleIndexImpl<T, TTuple<Types...>> { };

template <typename T, typename... Types>
struct TTupleIndexImpl<T, const volatile TTuple<Types...>> : TTupleIndexImpl<T, TTuple<Types...>> { };

template <size_t I, typename TupleType>
struct TTupleElementImpl;

template <size_t I, typename T, typename... Types>
struct TTupleElementImpl<I, TTuple<T, Types...>>
{
	static_assert(I < sizeof...(Types) + 1, "Invalid index in tuple");
	using Type = TTupleElementImpl<I - 1, TTuple<Types...>>::Type;
};

template <typename T, typename... Types>
struct TTupleElementImpl<0, TTuple<T, Types...>> { using Type = T; };

template <size_t I, typename... Types>
struct TTupleElementImpl<I, TTuple<Types...>> { };

template <>
struct TTupleElementImpl<0, TTuple<>> { };

template <size_t I, typename... Types>
struct TTupleElementImpl<I, const TTuple<Types...>> { using Type = TAddConst<typename TTupleElementImpl<I, TTuple<Types...>>::Type>; };

template <size_t I, typename... Types>
struct TTupleElementImpl<I, volatile TTuple<Types...>> { using Type = TAddVolatile<typename TTupleElementImpl<I, TTuple<Types...>>::Type>; };

template <size_t I, typename... Types>
struct TTupleElementImpl<I, const volatile TTuple<Types...>> { using Type = TAddCV<typename TTupleElementImpl<I, TTuple<Types...>>::Type>; };

template <bool bTrue, typename... Types>
struct TTupleConvertCopy : FTrue { };

template <typename T, typename U>
struct TTupleConvertCopy<false, T, U> 
	: TBoolConstant<!(CConvertibleTo<const TTuple<U>&, T>
		|| CConstructibleFrom<T, const TTuple<U>&>
		|| CSameAs<T, U>)>
{ };

template <bool bTrue, typename... Types>
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
	constexpr TTupleBasicElement(Type&& Arg)
		: Value(Forward<Type>(Arg))
	{ }

	TTupleBasicElement() = default;
	TTupleBasicElement(TTupleBasicElement&&) = default;
	TTupleBasicElement(const TTupleBasicElement&) = default;
	TTupleBasicElement& operator=(TTupleBasicElement&&) = default;
	TTupleBasicElement& operator=(const TTupleBasicElement&) = default;
	
	constexpr                T&  GetValue()               &  { return static_cast<               T& >(Value); }
	constexpr const          T&  GetValue() const         &  { return static_cast<const          T& >(Value); }
	constexpr       volatile T&  GetValue()       volatile&  { return static_cast<      volatile T& >(Value); }
	constexpr const volatile T&  GetValue() const volatile&  { return static_cast<const volatile T& >(Value); }
	constexpr                T&& GetValue()               && { return static_cast<               T&&>(Value); }
	constexpr const          T&& GetValue() const         && { return static_cast<const          T&&>(Value); }
	constexpr       volatile T&& GetValue()       volatile&& { return static_cast<      volatile T&&>(Value); }
	constexpr const volatile T&& GetValue() const volatile&& { return static_cast<const volatile T&&>(Value); }
};

#if RS_TUPLE_ELEMENT_STATIC_ALIAS

#define DEFINE_TTupleBasicElement(Index, Name)                                                                          \
	template <typename T>                                                                                          \
	struct TTupleBasicElement<T, Index>                                                                            \
	{                                                                                                              \
		using Name##Type = T;                                                                                      \
		Name##Type Name;                                                                                           \
		                                                                                                           \
		template <typename Type>                                                                                   \
		constexpr TTupleBasicElement(Type&& Arg)                                                                   \
			: Name(Forward<Type>(Arg))                                                                             \
		{ }                                                                                                        \
		                                                                                                           \
		TTupleBasicElement() = default;                                                                            \
		TTupleBasicElement(TTupleBasicElement&&) = default;                                                        \
		TTupleBasicElement(const TTupleBasicElement&) = default;                                                   \
		TTupleBasicElement& operator=(TTupleBasicElement&&) = default;                                             \
		TTupleBasicElement& operator=(const TTupleBasicElement&) = default;                                        \
		                                                                                                           \
		constexpr                T&  GetValue()               &  { return static_cast<               T& >(Name); } \
		constexpr const          T&  GetValue() const         &  { return static_cast<const          T& >(Name); } \
		constexpr       volatile T&  GetValue()       volatile&  { return static_cast<      volatile T& >(Name); } \
		constexpr const volatile T&  GetValue() const volatile&  { return static_cast<const volatile T& >(Name); } \
		constexpr                T&& GetValue()               && { return static_cast<               T&&>(Name); } \
		constexpr const          T&& GetValue() const         && { return static_cast<const          T&&>(Name); } \
		constexpr       volatile T&& GetValue()       volatile&& { return static_cast<      volatile T&&>(Name); } \
		constexpr const volatile T&& GetValue() const volatile&& { return static_cast<const volatile T&&>(Name); } \
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

template <typename... Types>
constexpr TTuple<TUnwrapRefDecay<Types>...> MakeTupleImpl(Types&&... Args)
{
	return TTuple<TUnwrapRefDecay<Types>...>(Forward<Types>(Args)...);
}

template <typename Indices, typename... Types>
struct TTupleImpl;

template <size_t... Indices, typename... Types>
struct TTupleImpl<TIndexSequence<Indices...>, Types...> : TTupleBasicElement<Types, Indices>...
{
protected:

	TTupleImpl() = default;

	template <typename... ArgTypes>
	explicit TTupleImpl(FForwardingConstructor, ArgTypes&&... Args)
		: TTupleBasicElement<Types, Indices>(Forward<ArgTypes>(Args))...
	{ }

	template <typename TupleType>
	explicit TTupleImpl(FOtherTupleConstructor, TupleType&& InValue)
		: TTupleBasicElement<Types, Indices>(Forward<TupleType>(InValue).template GetValue<Indices>())...
	{ }

	TTupleImpl(const TTupleImpl&) = default;
	TTupleImpl(TTupleImpl&&) = default;

	TTupleImpl& operator=(const TTupleImpl&) = default;
	TTupleImpl& operator=(TTupleImpl&&) = default;

};

template <typename Indices, typename... Types>
struct TTupleHelper;

template <size_t... Indices>
struct TTupleHelper<TIndexSequence<Indices...>>
{
	template <typename LHSTupleType, typename RHSTupleType>
	static constexpr void Assign(LHSTupleType& LHS, RHSTupleType&& RHS)
	{
		static_assert(sizeof...(Indices) == TTupleArityImpl<TRemoveReference<LHSTupleType>>::Value
			&& TTupleArityImpl<TRemoveReference<LHSTupleType>>::Value == TTupleArityImpl<TRemoveReference<RHSTupleType>>::Value,
			"Cannot assign tuple from different size");
		
		((LHS.template GetValue<Indices>() = Forward<RHSTupleType>(RHS).template GetValue<Indices>()), ...);
	}

	template <typename F, typename TTupleType>
	static constexpr auto Apply(F&& Func, TTupleType&& Arg)
	{
		return Invoke(Forward<F>(Func), Forward<TTupleType>(Arg).template GetValue<Indices>()...);
	}

	template <typename F, typename TTupleType, typename... ArgTypes>
	static constexpr auto ApplyAfter(F&& Func, TTupleType&& Arg, ArgTypes&&... OtherArgs)
	{
		return Invoke(Forward<F>(Func), Forward<ArgTypes>(OtherArgs)..., Forward<TTupleType>(Arg).template GetValue<Indices>()...);
	}

	template <typename F, typename TTupleType, typename... ArgTypes>
	static constexpr auto ApplyBefore(F&& Func, TTupleType&& Arg, ArgTypes&&... OtherArgs)
	{
		return Invoke(Forward<F>(Func), Forward<TTupleType>(Arg).template GetValue<Indices>()..., Forward<ArgTypes>(OtherArgs)...);
	}

	template <typename F, typename TTupleType>
	static constexpr auto Transform(F&& Func, TTupleType&& Arg)
	{
		return MakeTupleImpl(Invoke(Forward<F>(Func), Forward<TTupleType>(Arg).template GetValue<Indices>())...);
	}

	template <typename T, typename TTupleType>
	static constexpr T Construct(TTupleType&& Arg)
	{
		return T(Forward<TTupleType>(Arg).template GetValue<Indices>()...);
	}

};

NAMESPACE_PRIVATE_END

template <typename T>
concept CTTuple = NAMESPACE_PRIVATE::TIsTTuple<T>::Value;

template <typename TupleType>
inline constexpr size_t TTupleArity = NAMESPACE_PRIVATE::TTupleArityImpl<TupleType>::Value;

template <typename T, typename TupleType>
inline constexpr size_t TTupleIndex = NAMESPACE_PRIVATE::TTupleIndexImpl<T, TupleType>::Value;

template <size_t I, typename TupleType>
using TTupleElement = typename NAMESPACE_PRIVATE::TTupleElementImpl<I, TupleType>::Type;

template <typename... Types>
struct TTuple : NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Types...>, Types...>
{
private:

	using Super = NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Types...>, Types...>;
	using Helper = NAMESPACE_PRIVATE::TTupleHelper<TIndexSequenceFor<Types...>>;

public:

	TTuple() = default;
	
	template <typename... ArgTypes> requires (sizeof...(Types) >= 1) && (sizeof...(ArgTypes) == sizeof...(Types))
		&& (true && ... && CConstructibleFrom<Types, ArgTypes&&>)
	constexpr explicit (!(true && ... && CConvertibleTo<ArgTypes&&, Types>)) TTuple(ArgTypes&&... Args)
		: Super(NAMESPACE_PRIVATE::ForwardingConstructor, Forward<ArgTypes>(Args)...)
	{ }
	
	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == sizeof...(Types))
		&& (true && ... && CConstructibleFrom<Types, const OtherTypes&>)
		&& NAMESPACE_PRIVATE::TTupleConvertCopy<sizeof...(Types) != 1, Types..., OtherTypes...>::Value
	constexpr explicit (!(true && ... && CConvertibleTo<OtherTypes&&, Types>)) TTuple(const TTuple<OtherTypes...>& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, InValue)
	{ }

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == sizeof...(Types))
		&& (true && ... && CConstructibleFrom<Types, OtherTypes&&>)
		&& NAMESPACE_PRIVATE::TTupleConvertMove<sizeof...(Types) != 1, Types..., OtherTypes...>::Value
	constexpr explicit (!(true && ... && CConvertibleTo<OtherTypes&&, Types>)) TTuple(TTuple<OtherTypes...>&& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, MoveTemp(InValue))
	{ }

	TTuple(const TTuple&) = default;
	TTuple(TTuple&&) = default;
	
	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == sizeof...(Types))
		&& (true && ... && CAssignableFrom<Types&, const OtherTypes&>)
	constexpr TTuple& operator=(const TTuple<OtherTypes...>& InValue)
	{
		Helper::Assign(*this, InValue);
		return *this;
	}

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == sizeof...(Types))
		&& (true && ... && CAssignableFrom<Types&, OtherTypes&&>)
	constexpr TTuple& operator=(TTuple<OtherTypes...>&& InValue)
	{
		Helper::Assign(*this, MoveTemp(InValue));
		return *this;
	}

	TTuple& operator=(const TTuple&) = default;
	TTuple& operator=(TTuple&&) = default;

	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue()               &  { return static_cast<               NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue() const         &  { return static_cast<const          NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue()       volatile&  { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue() const volatile&  { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue()               && { return static_cast<               NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue() const         && { return static_cast<const          NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue()       volatile&& { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < sizeof...(Types)) constexpr decltype(auto) GetValue() const volatile&& { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleBasicElement<TTupleElement<I, TTuple<Types...>>, I>&&>(*this).GetValue(); }

	template <typename T> constexpr decltype(auto) GetValue()               &  { return static_cast<               TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue() const         &  { return static_cast<const          TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue()       volatile&  { return static_cast<      volatile TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue() const volatile&  { return static_cast<const volatile TTuple& >(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue()               && { return static_cast<               TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue() const         && { return static_cast<const          TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue()       volatile&& { return static_cast<      volatile TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }
	template <typename T> constexpr decltype(auto) GetValue() const volatile&& { return static_cast<const volatile TTuple&&>(*this).GetValue<TTupleIndex<T, TTuple<Types...>>>(); }

	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func)               &  { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func) const         &  { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func)       volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func) const volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func)               && { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func) const         && { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func)       volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires CInvocable<F, Types...> constexpr decltype(auto) Apply(F&& Func) const volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args)               &  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<               TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args) const         &  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const          TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args)       volatile&  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<      volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args) const volatile&  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args)               && { return Helper::ApplyAfter(Forward<F>(Func), static_cast<               TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args) const         && { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const          TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args)       volatile&& { return Helper::ApplyAfter(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, ArgTypes..., Types...> constexpr decltype(auto) ApplyAfter(F&& Func, ArgTypes&&... Args) const volatile&& { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }

	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args)               &  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<               TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args) const         &  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const          TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args)       volatile&  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<      volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args) const volatile&  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args)               && { return Helper::ApplyBefore(Forward<F>(Func), static_cast<               TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args) const         && { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const          TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args)       volatile&& { return Helper::ApplyBefore(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires CInvocable<F, Types..., ArgTypes...> constexpr decltype(auto) ApplyBefore(F&& Func, ArgTypes&&... Args) const volatile&& { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func)               &  { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func) const         &  { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func)       volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func) const volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func)               && { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func) const         && { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func)       volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (CInvocable<F, Types> && !CSameAs<void, TInvokeResult<F, Types>>)) constexpr decltype(auto) Transform(F&& Func) const volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct()               &  { return Helper::template Construct<T>(static_cast<               TTuple& >(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct() const         &  { return Helper::template Construct<T>(static_cast<const          TTuple& >(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct()       volatile&  { return Helper::template Construct<T>(static_cast<      volatile TTuple& >(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct() const volatile&  { return Helper::template Construct<T>(static_cast<const volatile TTuple& >(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct()               && { return Helper::template Construct<T>(static_cast<               TTuple&&>(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct() const         && { return Helper::template Construct<T>(static_cast<const          TTuple&&>(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct()       volatile&& { return Helper::template Construct<T>(static_cast<      volatile TTuple&&>(*this)); }
	template <typename T> requires CConstructibleFrom<T, Types...> constexpr T Construct() const volatile&& { return Helper::template Construct<T>(static_cast<const volatile TTuple&&>(*this)); }
	
	constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Types>)
	{
		return [this]<size_t... Indices>(TIndexSequence<Indices...>) -> size_t
		{
			return HashCombine(NAMESPACE_REDCRAFT::GetTypeHash(GetValue<Indices>())...);
		}
		(TMakeIndexSequence<sizeof...(Types)>());
	}

	constexpr void Swap(TTuple& InValue) requires (true && ... && (CMoveConstructible<Types>&& CSwappable<Types>))
	{
		[&A = *this, &B = InValue]<size_t... Indices>(TIndexSequence<Indices...>)
		{
			((NAMESPACE_REDCRAFT::Swap(A.template GetValue<Indices>(), B.template GetValue<Indices>())), ...);
		}
		(TMakeIndexSequence<sizeof...(Types)>());
	}

};

template <typename... Types>
TTuple(Types...) -> TTuple<Types...>;

template <typename T, typename U>
using TPair = TTuple<T, U>;

template <typename... Types>
constexpr TTuple<TUnwrapRefDecay<Types>...> MakeTuple(Types&&... Args)
{
	return TTuple<TUnwrapRefDecay<Types>...>(Forward<Types>(Args)...);
}

template <typename... Types>
constexpr TTuple<Types&...> Tie(Types&... Args)
{
	return TTuple<Types&...>(Args...);
}

template <typename... Types>
constexpr TTuple<Types&&...> ForwardAsTuple(Types&&... Args)
{
	return TTuple<Types&&...>(Forward<Types>(Args)...);
}

NAMESPACE_PRIVATE_BEGIN

struct FTupleEndFlag { };

template <typename... TTupleTypes>
struct TTupleCatResultImpl;

template <typename... Types, typename... TTupleTypes>
struct TTupleCatResultImpl<TTuple<Types...>, TTupleTypes...>
{
	using Type = typename TTupleCatResultImpl<TTupleTypes..., Types...>::Type;
};

template <typename... Types>
struct TTupleCatResultImpl<FTupleEndFlag, Types...>
{
	using Type = TTuple<Types...>;
};

template <typename R, typename Indices>
struct TTupleCatMake;

template <typename... RTypes, size_t... Indices>
struct TTupleCatMake<TTuple<RTypes...>, TIndexSequence<Indices...>>
{
	template <typename T, typename U>
	struct ForwardType { using Type = TConditional<CRValueReference<T>, TRemoveReference<U>&&, U>; };

	template <typename TTupleType>
	static constexpr TTuple<RTypes...> F(TTupleType&& InValue)
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
	static constexpr decltype(auto) F(ForwardType&& ForwardTuple, TTupleType&& InValue)
	{
		return ForwardAsTuple(Forward<ForwardType>(ForwardTuple).template GetValue<ForwardIndices>()..., Forward<TTupleType>(InValue).template GetValue<TTupleIndices>()...);
	}
};

template <typename R>
struct TTupleCatImpl
{
	template <typename ForwardType, typename TTupleType, typename... OtherTTupleTypes>
	static constexpr decltype(auto) F(ForwardType&& ForwardTuple, TTupleType&& InValue, OtherTTupleTypes&&... OtherValue)
	{
		return F(TTupleCatForward<
			TMakeIndexSequence<TTupleArity<TRemoveReference<ForwardType>>>,
			TMakeIndexSequence<TTupleArity<TRemoveReference<TTupleType>>>>
			::F(Forward<ForwardType>(ForwardTuple), Forward<TTupleType>(InValue)), Forward<OtherTTupleTypes>(OtherValue)...);
	}

	template <typename ForwardType>
	static constexpr decltype(auto) F(ForwardType&& ForwardTuple)
	{
		return TTupleCatMake<R, TMakeIndexSequence<TTupleArity<ForwardType>>>::F(Forward<ForwardType>(ForwardTuple));
	}
};

template <typename R, typename Indices>
struct TTupleThreeWay;

template <typename R, size_t I, size_t... Indices>
struct TTupleThreeWay<R, TIndexSequence<I, Indices...>>
{
	template <typename LHSTupleType, typename RHSTupleType>
	static constexpr R F(const LHSTupleType& LHS, const RHSTupleType& RHS)
	{
		auto Result = SynthThreeWayCompare(LHS.template GetValue<I>(), RHS.template GetValue<I>());
		if (Result != 0) return Result;
		return TTupleThreeWay<R, TIndexSequence<Indices...>>::F(LHS, RHS);
	}
};

template <typename R>
struct TTupleThreeWay<R, TIndexSequence<>>
{
	template <typename LHSTupleType, typename RHSTupleType>
	static constexpr R F(const LHSTupleType& LHS, const RHSTupleType& RHS)
	{
		return R::equivalent;
	}
};

template <typename Indices>
struct TTupleVisitImpl;

template <size_t I, size_t... Indices>
struct TTupleVisitImpl<TIndexSequence<I, Indices...>>
{
	template <typename G, typename... TupleTypes>
	static constexpr void F(G&& Func, TupleTypes&&... Tuples)
	{
		Invoke(Forward<G>(Func), Forward<TupleTypes>(Tuples).template GetValue<I>()...);
		TTupleVisitImpl<TIndexSequence<Indices...>>::F(Forward<G>(Func), Forward<TupleTypes>(Tuples)...);
	}
};

template <>
struct TTupleVisitImpl<TIndexSequence<>>
{
	template <typename... TupleTypes>
	static constexpr void F(TupleTypes&&... Tuples) { }
};

NAMESPACE_PRIVATE_END

template <typename... TTupleTypes> requires (true && ... && CTTuple<TRemoveCVRef<TTupleTypes>>)
using TTupleCatResult = typename NAMESPACE_PRIVATE::TTupleCatResultImpl<TRemoveReference<TTupleTypes>..., NAMESPACE_PRIVATE::FTupleEndFlag>::Type;;

template <typename... TTupleTypes> requires (true && ... && CTTuple<TRemoveCVRef<TTupleTypes>>)
constexpr decltype(auto) TupleCat(TTupleTypes&&... Args)
{
	using R = TTupleCatResult<TTupleTypes...>;
	if constexpr (sizeof...(Args) == 0) return R();
	else return NAMESPACE_PRIVATE::TTupleCatImpl<R>::F(Forward<TTupleTypes>(Args)...);
}

template <typename... LHSTypes, typename... RHSTypes> requires ((sizeof...(LHSTypes) != sizeof...(RHSTypes)) || (true && ... && CWeaklyEqualityComparable<LHSTypes, RHSTypes>))
constexpr bool operator==(const TTuple<LHSTypes...>& LHS, const TTuple<RHSTypes...>& RHS)
{
	if constexpr (sizeof...(LHSTypes) != sizeof...(RHSTypes)) return false;
	return[&LHS, &RHS]<size_t... Indices>(TIndexSequence<Indices...>) -> bool {	return (true && ... && (LHS.template GetValue<Indices>() == RHS.template GetValue<Indices>())); } (TMakeIndexSequence<sizeof...(LHSTypes)>());
}

template <typename... LHSTypes, typename... RHSTypes> requires ((sizeof...(LHSTypes) == sizeof...(RHSTypes)) && (true && ... && (CSynthThreeWayComparable<LHSTypes, RHSTypes>)))
constexpr TCommonComparisonCategory<TSynthThreeWayResult<LHSTypes, RHSTypes>...> operator<=>(const TTuple<LHSTypes...>& LHS, const TTuple<RHSTypes...>& RHS)
{
	using R = TCommonComparisonCategory<TSynthThreeWayResult<LHSTypes, RHSTypes>...>;
	return NAMESPACE_PRIVATE::TTupleThreeWay<R, TMakeIndexSequence<sizeof...(LHSTypes)>>::F(LHS, RHS);
}

template <typename F> requires CInvocable<F>
constexpr void VisitTuple(F&& Func) { }

template <typename F, typename FirstTupleType, typename... TupleTypes>
constexpr void VisitTuple(F&& Func, FirstTupleType&& FirstTuple, TupleTypes&&... Tuples)
{
	NAMESPACE_PRIVATE::TTupleVisitImpl<TMakeIndexSequence<TTupleArity<TRemoveReference<FirstTupleType>>>>
		::F(Forward<F>(Func), Forward<FirstTupleType>(FirstTuple), Forward<TupleTypes>(Tuples)...);
}

template <typename... Ts, typename... Us> requires requires { typename TTuple<TCommonType<Ts, Us>...>; }
struct TBasicCommonType<TTuple<Ts...>, TTuple<Us...>>
{
	using Type = TTuple<TCommonType<Ts, Us>...>;
};

template <typename... Ts, typename... Us, template<typename> typename TQualifiers, template<typename> typename UQualifiers>
	requires requires { typename TTuple<TCommonReference<TQualifiers<Ts>, UQualifiers<Us>>...>; }
struct TBasicCommonReference<TTuple<Ts...>, TTuple<Us...>, TQualifiers, UQualifiers>
{
	using Type = TTuple<TCommonReference<TQualifiers<Ts>, UQualifiers<Us>>...>;
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

NAMESPACE_STD_BEGIN

// Support structure binding, should not be directly used
template <typename... Types> struct tuple_size<NAMESPACE_REDCRAFT::TTuple<Types...>> : integral_constant<size_t, NAMESPACE_REDCRAFT::TTupleArity<NAMESPACE_REDCRAFT::TTuple<Types...>>> { };
template <size_t I, typename... Types> struct tuple_element<I, NAMESPACE_REDCRAFT::TTuple<Types...>> { using type = NAMESPACE_REDCRAFT::TTupleElement<I, NAMESPACE_REDCRAFT::TTuple<Types...>>; };

NAMESPACE_STD_END

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Support structure binding, should not be directly used
template <size_t Index, typename ...Types> constexpr decltype(auto) get(               TTuple<Types...>&  InValue) { return static_cast<               TTuple<Types...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(const          TTuple<Types...>&  InValue) { return static_cast<const          TTuple<Types...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(      volatile TTuple<Types...>&  InValue) { return static_cast<      volatile TTuple<Types...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(const volatile TTuple<Types...>&  InValue) { return static_cast<const volatile TTuple<Types...>& >(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(               TTuple<Types...>&& InValue) { return static_cast<               TTuple<Types...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(const          TTuple<Types...>&& InValue) { return static_cast<const          TTuple<Types...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(      volatile TTuple<Types...>&& InValue) { return static_cast<      volatile TTuple<Types...>&&>(InValue).template GetValue<Index>(); }
template <size_t Index, typename ...Types> constexpr decltype(auto) get(const volatile TTuple<Types...>&& InValue) { return static_cast<const volatile TTuple<Types...>&&>(InValue).template GetValue<Index>(); }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
