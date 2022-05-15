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

struct FForwardingConstructor { explicit FForwardingConstructor() = default; };
struct FOtherTupleConstructor { explicit FOtherTupleConstructor() = default; };

inline constexpr FForwardingConstructor ForwardingConstructor{ };
inline constexpr FOtherTupleConstructor OtherTupleConstructor{ };

template <typename T, typename... Types>
struct TTupleElementIndex;

template <typename T, typename U, typename... Types>
struct TTupleElementIndex<T, U, Types...>
	: TConstant<size_t, TIsSame<T, U>::Value ? 0 : (TTupleElementIndex<T, Types...>::Value == INDEX_NONE
		? INDEX_NONE : TTupleElementIndex<T, Types...>::Value + 1)>
{ };

template <typename T>
struct TTupleElementIndex<T> : TConstant<size_t, INDEX_NONE> { };

template <size_t I, typename... Types>
struct TTupleElementType;

template <size_t I, typename T, typename... Types>
struct TTupleElementType<I, T, Types...>
{
	static_assert(I < sizeof...(Types) + 1, "Tuple type index is invalid");
	using Type = TTupleElementType<I - 1, Types...>::Type;
};

template <typename T, typename... Types>
struct TTupleElementType<0, T, Types...> { using Type = T; };

template <>
struct TTupleElementType<0> { };

template <typename T, size_t Index>
struct TTupleElement
{
private:

	using ValueType = T;
	ValueType Value;

public:

	template <typename Type>
	constexpr TTupleElement(Type&& Arg)
		: Value(Forward<Type>(Arg))
	{ }

	TTupleElement() = default;
	TTupleElement(TTupleElement&&) = default;
	TTupleElement(const TTupleElement&) = default;
	TTupleElement& operator=(TTupleElement&&) = default;
	TTupleElement& operator=(const TTupleElement&) = default;
	
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

#define DEFINE_TTupleElement(Index, Name)                                                                          \
	template <typename T>                                                                                          \
	struct TTupleElement<T, Index>                                                                                 \
	{                                                                                                              \
		using Name##Type = T;                                                                                      \
		Name##Type Name;                                                                                           \
		                                                                                                           \
		template <typename Type>                                                                                   \
		constexpr TTupleElement(Type&& Arg)                                                                        \
			: Name(Forward<Type>(Arg))                                                                             \
		{ }                                                                                                        \
		                                                                                                           \
		TTupleElement() = default;                                                                                 \
		TTupleElement(TTupleElement&&) = default;                                                                  \
		TTupleElement(const TTupleElement&) = default;                                                             \
		TTupleElement& operator=(TTupleElement&&) = default;                                                       \
		TTupleElement& operator=(const TTupleElement&) = default;                                                  \
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

DEFINE_TTupleElement(0x0, First);
DEFINE_TTupleElement(0x1, Second);
DEFINE_TTupleElement(0x2, Third);
DEFINE_TTupleElement(0x3, Fourth);
DEFINE_TTupleElement(0x4, Fifth);
DEFINE_TTupleElement(0x5, Sixth);
DEFINE_TTupleElement(0x6, Seventh);
DEFINE_TTupleElement(0x7, Eighth);
DEFINE_TTupleElement(0x8, Ninth);
DEFINE_TTupleElement(0x9, Tenth);
DEFINE_TTupleElement(0xA, Eleventh);
DEFINE_TTupleElement(0xB, Twelfth);
DEFINE_TTupleElement(0xC, Thirteenth);
DEFINE_TTupleElement(0xD, Fourteenth);
DEFINE_TTupleElement(0xE, Fifteenth);
DEFINE_TTupleElement(0xF, Sixteenth);

#undef DEFINE_TTupleElement

#endif

template <typename... Types>
constexpr TTuple<typename TUnwrapRefDecay<Types>::Type...> MakeTupleImpl(Types&&... Args)
{
	return TTuple<typename TUnwrapRefDecay<Types>::Type...>(Forward<Types>(Args)...);
}

template <typename Indices, typename... Types>
struct TTupleImpl;

template <size_t... Indices, typename... Types>
struct TTupleImpl<TIndexSequence<Indices...>, Types...> : TTupleElement<Types, Indices>...
{
protected:

	TTupleImpl() = default;

	template <typename... ArgTypes>
	explicit TTupleImpl(FForwardingConstructor, ArgTypes&&... Args)
		: TTupleElement<Types, Indices>(Forward<ArgTypes>(Args))...
	{ }

	template <typename TupleType>
	explicit TTupleImpl(FOtherTupleConstructor, TupleType&& InValue)
		: TTupleElement<Types, Indices>(Forward<TupleType>(InValue).template GetValue<Indices>())...
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
		static_assert(sizeof...(Indices) == LHS.ElementSize && LHS.ElementSize == RHS.ElementSize, "Cannot assign tuple from different size");
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

template <typename... Types>
struct TTuple : NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Types...>, Types...>
{
private:

	using Super = NAMESPACE_PRIVATE::TTupleImpl<TIndexSequenceFor<Types...>, Types...>;
	using Helper = NAMESPACE_PRIVATE::TTupleHelper<TIndexSequenceFor<Types...>>;

public:

	static constexpr size_t ElementSize = sizeof...(Types);

	template <size_t I>   struct TElementType  : NAMESPACE_PRIVATE::TTupleElementType<I, Types...>  { };
	template <typename T> struct TElementIndex : NAMESPACE_PRIVATE::TTupleElementIndex<T, Types...> { };

	TTuple() = default;

	template <typename... ArgTypes> requires (ElementSize > 0) && (sizeof...(ArgTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, ArgTypes&&>::Value)
		&& (true && ... && TIsConvertible<ArgTypes&&, Types>::Value)
	constexpr TTuple(ArgTypes&&... Args)
		: Super(NAMESPACE_PRIVATE::ForwardingConstructor, Forward<ArgTypes>(Args)...)
	{ }

	template <typename... ArgTypes> requires (ElementSize > 0) && (sizeof...(ArgTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, ArgTypes&&>::Value)
		&& (!(true && ... && TIsConvertible<ArgTypes&&, Types>::Value))
	constexpr explicit TTuple(ArgTypes&&... Args)
		: Super(NAMESPACE_PRIVATE::ForwardingConstructor, Forward<ArgTypes>(Args)...)
	{ }
	
	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, const OtherTypes&>::Value)
		&& ((ElementSize != 1) || !(TIsConvertible<const TTuple<OtherTypes...>&, typename TElementType<0>::Type>::Value
			|| TIsConstructible<typename TElementType<0>::Type, const TTuple<OtherTypes...>&>::Value
			|| TIsSame<typename TElementType<0>::Type, typename TTuple<OtherTypes...>::template TElementType<0>::Type>::Value))
		&& (true && ... && TIsConvertible<OtherTypes&&, Types>::Value)
	constexpr TTuple(const TTuple<OtherTypes...>& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, InValue)
	{ }

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, const OtherTypes&>::Value)
		&& ((ElementSize != 1) || !(TIsConvertible<const TTuple<OtherTypes...>&, typename TElementType<0>::Type>::Value
			|| TIsConstructible<typename TElementType<0>::Type, const TTuple<OtherTypes...>&>::Value
			|| TIsSame<typename TElementType<0>::Type, typename TTuple<OtherTypes...>::template TElementType<0>::Type>::Value))
		&& (!(true && ... && TIsConvertible<OtherTypes&&, Types>::Value))
	constexpr explicit TTuple(const TTuple<OtherTypes...>& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, InValue)
	{ }

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, OtherTypes&&>::Value)
		&& ((ElementSize != 1) || !(TIsConvertible<TTuple<OtherTypes...>&&, typename TElementType<0>::Type>::Value
			|| TIsConstructible<typename TElementType<0>::Type, TTuple<OtherTypes...>&&>::Value
			|| TIsSame<typename TElementType<0>::Type, typename TTuple<OtherTypes...>::template TElementType<0>::Type>::Value))
		&& (true && ... && TIsConvertible<OtherTypes&&, Types>::Value)
	constexpr TTuple(TTuple<OtherTypes...>&& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, MoveTemp(InValue))
	{ }

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsConstructible<Types, OtherTypes&&>::Value)
		&& ((ElementSize != 1) || !(TIsConvertible<TTuple<OtherTypes...>&&, typename TElementType<0>::Type>::Value
			|| TIsConstructible<typename TElementType<0>::Type, TTuple<OtherTypes...>&&>::Value
			|| TIsSame<typename TElementType<0>::Type, typename TTuple<OtherTypes...>::template TElementType<0>::Type>::Value))
		&& (!(true && ... && TIsConvertible<OtherTypes&&, Types>::Value))
	constexpr explicit TTuple(TTuple<OtherTypes...>&& InValue)
		: Super(NAMESPACE_PRIVATE::OtherTupleConstructor, MoveTemp(InValue))
	{ }

	TTuple(const TTuple&) = default;
	TTuple(TTuple&&) = default;
	
	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsAssignable<Types&, const OtherTypes&>::Value)
	constexpr TTuple& operator=(const TTuple<OtherTypes...>& InValue)
	{
		Helper::Assign(*this, InValue);
		return *this;
	}

	template <typename... OtherTypes> requires (sizeof...(OtherTypes) == ElementSize)
		&& (true && ... && TIsAssignable<Types&, OtherTypes&&>::Value)
	constexpr TTuple& operator=(TTuple<OtherTypes...>&& InValue)
	{
		Helper::Assign(*this, MoveTemp(InValue));
		return *this;
	}

	TTuple& operator=(const TTuple&) = default;
	TTuple& operator=(TTuple&&) = default;

	template <size_t I> requires (I < ElementSize) constexpr                TElementType<I>::Type&  GetValue()               &  { return static_cast<               NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr const          TElementType<I>::Type&  GetValue() const         &  { return static_cast<const          NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr       volatile TElementType<I>::Type&  GetValue()       volatile&  { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr const volatile TElementType<I>::Type&  GetValue() const volatile&  { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>& >(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr                TElementType<I>::Type&& GetValue()               && { return static_cast<               NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr const          TElementType<I>::Type&& GetValue() const         && { return static_cast<const          NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr       volatile TElementType<I>::Type&& GetValue()       volatile&& { return static_cast<      volatile NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>&&>(*this).GetValue(); }
	template <size_t I> requires (I < ElementSize) constexpr const volatile TElementType<I>::Type&& GetValue() const volatile&& { return static_cast<const volatile NAMESPACE_PRIVATE::TTupleElement<typename TElementType<I>::Type, I>&&>(*this).GetValue(); }

	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr                T&  GetValue()               &  { return static_cast<               TTuple& >(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr const          T&  GetValue() const         &  { return static_cast<const          TTuple& >(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr       volatile T&  GetValue()       volatile&  { return static_cast<      volatile TTuple& >(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr const volatile T&  GetValue() const volatile&  { return static_cast<const volatile TTuple& >(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr                T&& GetValue()               && { return static_cast<               TTuple&&>(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr const          T&& GetValue() const         && { return static_cast<const          TTuple&&>(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr       volatile T&& GetValue()       volatile&& { return static_cast<      volatile TTuple&&>(*this).GetValue<TElementIndex<T>::Value>(); }
	template <typename T> requires (TElementIndex<T>::Value != INDEX_NONE) constexpr const volatile T&& GetValue() const volatile&& { return static_cast<const volatile TTuple&&>(*this).GetValue<TElementIndex<T>::Value>(); }

	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func)               &  { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func) const         &  { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func)       volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func) const volatile&  { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func)               && { return Helper::Apply(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func) const         && { return Helper::Apply(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func)       volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires TIsInvocable<F, Types...>::Value constexpr auto Apply(F&& Func) const volatile&& { return Helper::Apply(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args)               &  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<               TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args) const         &  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const          TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args)       volatile&  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<      volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args) const volatile&  { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args)               && { return Helper::ApplyAfter(Forward<F>(Func), static_cast<               TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args) const         && { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const          TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args)       volatile&& { return Helper::ApplyAfter(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, ArgTypes..., Types...>::Value constexpr auto ApplyAfter(F&& Func, ArgTypes&&... Args) const volatile&& { return Helper::ApplyAfter(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }

	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args)               &  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<               TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args) const         &  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const          TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args)       volatile&  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<      volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args) const volatile&  { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const volatile TTuple& >(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args)               && { return Helper::ApplyBefore(Forward<F>(Func), static_cast<               TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args) const         && { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const          TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args)       volatile&& { return Helper::ApplyBefore(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	template <typename F, typename... ArgTypes> requires TIsInvocable<F, Types..., ArgTypes...>::Value constexpr auto ApplyBefore(F&& Func, ArgTypes&&... Args) const volatile&& { return Helper::ApplyBefore(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this), Forward<ArgTypes>(Args)...); }
	
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func)               &  { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple& >(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func) const         &  { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple& >(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func)       volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func) const volatile&  { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple& >(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func)               && { return Helper::Transform(Forward<F>(Func), static_cast<               TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func) const         && { return Helper::Transform(Forward<F>(Func), static_cast<const          TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func)       volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<      volatile TTuple&&>(*this)); }
	template <typename F> requires (true && ... && (TIsInvocable<F, Types>::Value && !TIsSame<void, typename TInvokeResult<F, Types>::Type>::Value)) constexpr auto Transform(F&& Func) const volatile&& { return Helper::Transform(Forward<F>(Func), static_cast<const volatile TTuple&&>(*this)); }

	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct()               &  { return Helper::template Construct<T>(static_cast<               TTuple& >(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct() const         &  { return Helper::template Construct<T>(static_cast<const          TTuple& >(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct()       volatile&  { return Helper::template Construct<T>(static_cast<      volatile TTuple& >(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct() const volatile&  { return Helper::template Construct<T>(static_cast<const volatile TTuple& >(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct()               && { return Helper::template Construct<T>(static_cast<               TTuple&&>(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct() const         && { return Helper::template Construct<T>(static_cast<const          TTuple&&>(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct()       volatile&& { return Helper::template Construct<T>(static_cast<      volatile TTuple&&>(*this)); }
	template <typename T> requires TIsConstructible<T, Types...>::Value constexpr T Construct() const volatile&& { return Helper::template Construct<T>(static_cast<const volatile TTuple&&>(*this)); }
	
	constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Types>)
	{
		return [this]<size_t... Indices>(TIndexSequence<Indices...>) -> size_t { return HashCombine(NAMESPACE_REDCRAFT::GetTypeHash(GetValue<Indices>())...); } (TMakeIndexSequence<ElementSize>());
	}

	constexpr void Swap(TTuple& InValue) requires (true && ... && (TIsMoveConstructible<Types>::Value&& TIsSwappable<Types>::Value))
	{
		[&A = *this, &B = InValue]<size_t... Indices>(TIndexSequence<Indices...>) { ((NAMESPACE_REDCRAFT::Swap(A.template GetValue<Indices>(), B.template GetValue<Indices>())), ...); } (TMakeIndexSequence<ElementSize>());
	}

};

template <typename... Types>
TTuple(Types...) -> TTuple<Types...>;

template <typename T, typename U>
using TPair = TTuple<T, U>;

template <typename    T    > struct TIsTTuple                   : FFalse { };
template <typename... Types> struct TIsTTuple<TTuple<Types...>> : FTrue  { };

template <typename TupleType> requires TIsTTuple<typename TRemoveCVRef<TupleType>::Type>::Value
struct TTupleElementSize : TConstant<size_t, TRemoveCVRef<TupleType>::Type::ElementSize> { };

template <size_t I, typename TupleType> requires TIsTTuple<typename TRemoveCVRef<TupleType>::Type>::Value
struct TTupleElementType { using Type = typename TCopyCVRef<typename TRemoveReference<TupleType>::Type, typename TRemoveCVRef<TupleType>::Type::template TElementType<I>::Type>::Type; };

template <typename T, typename TupleType> requires TIsTTuple<typename TRemoveCVRef<TupleType>::Type>::Value
struct TTupleElementIndex : TupleType::template TElementIndex<T> { };

template <typename... Types>
constexpr TTuple<typename TUnwrapRefDecay<Types>::Type...> MakeTuple(Types&&... Args)
{
	return TTuple<typename TUnwrapRefDecay<Types>::Type...>(Forward<Types>(Args)...);
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
	struct ForwardType { using Type = typename TConditional<CRValueReference<T>, typename TRemoveReference<U>::Type&&, U>::Type; };

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
	static constexpr auto F(ForwardType&& ForwardTuple, TTupleType&& InValue)
	{
		return ForwardAsTuple(Forward<ForwardType>(ForwardTuple).template GetValue<ForwardIndices>()..., Forward<TTupleType>(InValue).template GetValue<TTupleIndices>()...);
	}
};

template <typename R>
struct TTupleCatImpl
{
	template <typename ForwardType, typename TTupleType, typename... OtherTTupleTypes>
	static constexpr auto F(ForwardType&& ForwardTuple, TTupleType&& InValue, OtherTTupleTypes&&... OtherValue)
	{
		return F(TTupleCatForward<TMakeIndexSequence<TTupleElementSize<ForwardType>::Value>, TMakeIndexSequence<TTupleElementSize<TTupleType>::Value>>::F(Forward<ForwardType>(ForwardTuple), Forward<TTupleType>(InValue)), Forward<OtherTTupleTypes>(OtherValue)...);
	}

	template <typename ForwardType>
	static constexpr auto F(ForwardType&& ForwardTuple)
	{
		return TTupleCatMake<R, TMakeIndexSequence<TTupleElementSize<ForwardType>::Value>>::F(Forward<ForwardType>(ForwardTuple));
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

template <typename... TTupleTypes> requires (true && ... && (TIsTTuple<typename TRemoveCVRef<TTupleTypes>::Type>::Value))
struct TTupleCatResult { using Type = typename NAMESPACE_PRIVATE::TTupleCatResultImpl<typename TRemoveReference<TTupleTypes>::Type..., NAMESPACE_PRIVATE::FTupleEndFlag>::Type; };

template <typename... TTupleTypes> requires (true && ... && (TIsTTuple<typename TRemoveCVRef<TTupleTypes>::Type>::Value))
constexpr auto TupleCat(TTupleTypes&&... Args)
{
	using R = typename TTupleCatResult<TTupleTypes...>::Type;
	if constexpr (sizeof...(Args) == 0) return R();
	else return NAMESPACE_PRIVATE::TTupleCatImpl<R>::F(Forward<TTupleTypes>(Args)...);
}

template <typename... LHSTypes, typename... RHSTypes> requires ((sizeof...(LHSTypes) != sizeof...(RHSTypes)) || (true && ... && CWeaklyEqualityComparableWith<LHSTypes, RHSTypes>))
constexpr bool operator==(const TTuple<LHSTypes...>& LHS, const TTuple<RHSTypes...>& RHS)
{
	if constexpr (sizeof...(LHSTypes) != sizeof...(RHSTypes)) return false;
	return[&LHS, &RHS]<size_t... Indices>(TIndexSequence<Indices...>) -> bool {	return (true && ... && (LHS.template GetValue<Indices>() == RHS.template GetValue<Indices>())); } (TMakeIndexSequence<sizeof...(LHSTypes)>());
}

template <typename... LHSTypes, typename... RHSTypes> requires ((sizeof...(LHSTypes) == sizeof...(RHSTypes)) && (true && ... && (CSynthThreeWayComparableWith<LHSTypes, RHSTypes>)))
constexpr typename TCommonComparisonCategory<typename TSynthThreeWayResult<LHSTypes, RHSTypes>::Type...>::Type operator<=>(const TTuple<LHSTypes...>& LHS, const TTuple<RHSTypes...>& RHS)
{
	using R = typename TCommonComparisonCategory<typename TSynthThreeWayResult<LHSTypes, RHSTypes>::Type...>::Type;
	return NAMESPACE_PRIVATE::TTupleThreeWay<R, TMakeIndexSequence<sizeof...(LHSTypes)>>::F(LHS, RHS);
}

template <typename F> requires TIsInvocable<F>::Value
constexpr void VisitTuple(F&& Func) { }

template <typename F, typename FirstTupleType, typename... TupleTypes>
constexpr void VisitTuple(F&& Func, FirstTupleType&& FirstTuple, TupleTypes&&... Tuples)
{
	NAMESPACE_PRIVATE::TTupleVisitImpl<TMakeIndexSequence<TTupleElementSize<FirstTupleType>::Value>>::F(Forward<F>(Func), Forward<FirstTupleType>(FirstTuple), Forward<TupleTypes>(Tuples)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

NAMESPACE_STD_BEGIN

// Support structure binding, should not be directly used
template <typename... Types> struct tuple_size<NAMESPACE_REDCRAFT::TTuple<Types...>> : integral_constant<size_t, NAMESPACE_REDCRAFT::TTupleElementSize<NAMESPACE_REDCRAFT::TTuple<Types...>>::Value> { };
template <size_t I, typename... Types> struct tuple_element<I, NAMESPACE_REDCRAFT::TTuple<Types...>> { using type = typename NAMESPACE_REDCRAFT::TTupleElementType<I, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type; };

NAMESPACE_STD_END

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Support structure binding, should not be directly used
template <size_t Index, typename ...Types> constexpr                typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&  get(               NAMESPACE_REDCRAFT::TTuple<Types...>&  InValue) { return static_cast<               typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type& >(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr const          typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&  get(const          NAMESPACE_REDCRAFT::TTuple<Types...>&  InValue) { return static_cast<const          typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type& >(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr       volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&  get(      volatile NAMESPACE_REDCRAFT::TTuple<Types...>&  InValue) { return static_cast<      volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type& >(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr const volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&  get(const volatile NAMESPACE_REDCRAFT::TTuple<Types...>&  InValue) { return static_cast<const volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type& >(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr                typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&& get(               NAMESPACE_REDCRAFT::TTuple<Types...>&& InValue) { return static_cast<               typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&&>(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr const          typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&& get(const          NAMESPACE_REDCRAFT::TTuple<Types...>&& InValue) { return static_cast<const          typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&&>(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr       volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&& get(      volatile NAMESPACE_REDCRAFT::TTuple<Types...>&& InValue) { return static_cast<      volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&&>(InValue.template GetValue<Index>()); }
template <size_t Index, typename ...Types> constexpr const volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&& get(const volatile NAMESPACE_REDCRAFT::TTuple<Types...>&& InValue) { return static_cast<const volatile typename TTupleElementType<Index, NAMESPACE_REDCRAFT::TTuple<Types...>>::Type&&>(InValue.template GetValue<Index>()); }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
