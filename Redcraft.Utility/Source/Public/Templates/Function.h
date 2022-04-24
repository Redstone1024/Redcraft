#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Concepts/Same.h"
#include "Templates/Any.h"
#include "Templates/Tuple.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Concepts/Comparable.h"
#include "Concepts/Convertible.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/TypeInfo.h"
#include "Concepts/BooleanTestable.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

enum class EFunctionType
{
	Reference,
	Object,
	Unique,
};

enum class EFunctionSpecifiers
{
	None,
	LValue,
	RValue,
	Const,
	ConstLValue,
	ConstRValue,
};

template <typename F, size_t InlineSize, size_t InlineAlignment, EFunctionSpecifiers Specifiers, EFunctionType FunctionType>
struct TFunctionImpl;

template <typename T>                                                             struct TIsTFunctionImpl                               : FFalse { };
template <typename F, size_t I, size_t J, EFunctionSpecifiers S, EFunctionType E> struct TIsTFunctionImpl<TFunctionImpl<F, I, J, S, E>> : FTrue  { };

template <typename T>                                            struct TIsTFunctionRef                                                      : FFalse { };
template <typename F, size_t I, size_t J, EFunctionSpecifiers S> struct TIsTFunctionRef<TFunctionImpl<F, I, J, S, EFunctionType::Reference>> : FTrue  { };

template <typename T>                                            struct TIsTFunction                                                   : FFalse { };
template <typename F, size_t I, size_t J, EFunctionSpecifiers S> struct TIsTFunction<TFunctionImpl<F, I, J, S, EFunctionType::Object>> : FTrue  { };

template <typename T>                                            struct TIsTUniqueFunction                                                   : FFalse { };
template <typename F, size_t I, size_t J, EFunctionSpecifiers S> struct TIsTUniqueFunction<TFunctionImpl<F, I, J, S, EFunctionType::Unique>> : FTrue  { };

struct FFunctionIsBound
{
	template <typename T>
	static constexpr bool F(const T& Func)
	{
		if constexpr (TIsPointer<T>::Value || TIsMemberPointer<T>::Value || TIsTFunctionImpl<T>::Value)
		{
			return !!Func;
		}
		else
		{
			return true;
		}
	}
};

template <EFunctionSpecifiers Specifiers, typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers : FFalse { };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::None, R, F, Types...>
	: TBoolConstant<TIsInvocableResult<R, F, Types...>::Value && TIsInvocableResult<R, F&, Types...>::Value>
{ };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::LValue, R, F, Types...> : TIsInvocableResult<R, F&, Types...> { };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::RValue, R, F, Types...> : TIsInvocableResult<R, F, Types...> { };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::Const, R, F, Types...>
	: TBoolConstant<TIsInvocableResult<R, const F, Types...>::Value && TIsInvocableResult<R, const F&, Types...>::Value>
{ };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::ConstLValue, R, F, Types...> : TIsInvocableResult<R, const F&, Types...> { };

template <typename R, typename F, typename... Types>
struct TIsInvocableResultWithSpecifiers<EFunctionSpecifiers::ConstRValue, R, F, Types...> : TIsInvocableResult<R, const F, Types...> { };

template <typename T, EFunctionSpecifiers Specifiers> struct TFunctionCallSpecifiers;
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::None>        { using Type =       T& ; };
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::LValue>      { using Type =       T& ; };
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::RValue>      { using Type =       T&&; };
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::Const>       { using Type = const T& ; };
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::ConstLValue> { using Type = const T& ; };
template <typename T> struct TFunctionCallSpecifiers<T, EFunctionSpecifiers::ConstRValue> { using Type = const T&&; };

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionSpecifiers Specifiers, EFunctionType FunctionType>
struct TFunctionImpl<R(Types...), InlineSize, InlineAlignment, Specifiers, FunctionType>
{
public:

	using ResultType = R;
	using ArgumentType = TTuple<Types...>;

	constexpr TFunctionImpl(nullptr_t = nullptr) requires (FunctionType != EFunctionType::Reference) : Callable(nullptr) { }

	TFunctionImpl(const TFunctionImpl& InValue) requires (FunctionType != EFunctionType::Unique)
		: Callable(InValue.Callable), Storage(InValue.Storage)
	{ }

	TFunctionImpl(TFunctionImpl&& InValue)
		: Callable(InValue.Callable), Storage(MoveTemp(InValue.Storage))
	{ if constexpr (FunctionType != EFunctionType::Reference) InValue.Reset(); }
	
	template <typename T> requires (!TIsTFunctionImpl<typename TDecay<T>::Type>::Value) && (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& TIsInvocableResultWithSpecifiers<Specifiers, ResultType, typename TDecay<T>::Type, Types...>::Value
		&& (FunctionType == EFunctionType::Reference || TIsConstructible<typename TDecay<T>::Type, T&&>::Value)
		&& ((FunctionType == EFunctionType::Object && TIsCopyConstructible<typename TDecay<T>::Type>::Value)
			|| (FunctionType == EFunctionType::Unique && TIsMoveConstructible<typename TDecay<T>::Type>::Value)
			|| FunctionType == EFunctionType::Reference)
	FORCEINLINE TFunctionImpl(T&& InValue)
	{
		using DecayedFunctorType = typename TDecay<T>::Type;

		if constexpr (FunctionType == EFunctionType::Reference)
		{
			checkf(FFunctionIsBound::F(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		}

		if (!FFunctionIsBound::F(InValue)) Callable = nullptr;
		else EmplaceImpl<DecayedFunctorType>(Forward<T>(InValue));
	}

	template <typename T, typename... ArgTypes> requires (FunctionType != EFunctionType::Reference)
		&& TIsInvocableResultWithSpecifiers<Specifiers, ResultType, typename TDecay<T>::Type, Types...>::Value && TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value
		&& ((FunctionType == EFunctionType::Object && TIsCopyConstructible<typename TDecay<T>::Type>::Value)
			|| (FunctionType == EFunctionType::Unique && TIsMoveConstructible<typename TDecay<T>::Type>::Value))
	FORCEINLINE TFunctionImpl(TInPlaceType<T>, ArgTypes&&... Args)
	{
		using DecayedFunctorType = typename TDecay<T>::Type;
		EmplaceImpl<DecayedFunctorType>(Forward<ArgTypes>(Args)...);
	}

	template <size_t OtherInlineSize, size_t OtherInlineAlignment, EFunctionType OtherFunctionType>
		requires (FunctionType == EFunctionType::Reference) && (OtherFunctionType != EFunctionType::Reference)
	FORCEINLINE TFunctionImpl(const TFunctionImpl<R(Types...), OtherInlineSize, OtherInlineAlignment, Specifiers, OtherFunctionType>& InValue)
	{
		checkf(FFunctionIsBound::F(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		EmplaceImpl<TFunctionImpl<R(Types...), OtherInlineSize, OtherInlineAlignment, Specifiers, OtherFunctionType>>(InValue);
	}
	
	FORCEINLINE TFunctionImpl(const TFunctionImpl<R(Types...), InlineSize, InlineAlignment, Specifiers, EFunctionType::Object>& InValue) requires (FunctionType == EFunctionType::Unique)
		: Callable((*reinterpret_cast<const TFunctionImpl*>(&InValue)).Callable), Storage((*reinterpret_cast<const TFunctionImpl*>(&InValue)).Storage)
	{ }

	FORCEINLINE TFunctionImpl(TFunctionImpl<R(Types...), InlineSize, InlineAlignment, Specifiers, EFunctionType::Object>&& InValue) requires (FunctionType == EFunctionType::Unique)
		: Callable((*reinterpret_cast<TFunctionImpl*>(&InValue)).Callable), Storage(MoveTemp((*reinterpret_cast<TFunctionImpl*>(&InValue)).Storage))
	{ InValue.Reset(); }

	~TFunctionImpl() = default;

	FORCEINLINE TFunctionImpl& operator=(const TFunctionImpl& InValue) requires (FunctionType == EFunctionType::Object)
	{
		AssignImpl(InValue);
		return *this;
	}

	FORCEINLINE TFunctionImpl& operator=(TFunctionImpl&& InValue) requires (FunctionType != EFunctionType::Reference)
	{
		if (&InValue == this) return *this;
		AssignImpl(MoveTemp(InValue));
		return *this;
	}

	FORCEINLINE TFunctionImpl& operator=(const TFunctionImpl<R(Types...), InlineSize, InlineAlignment, Specifiers, EFunctionType::Object>& InValue) requires (FunctionType == EFunctionType::Unique)
	{
		AssignImpl(*reinterpret_cast<const TFunctionImpl*>(&InValue));
		return *this;
	}

	FORCEINLINE TFunctionImpl& operator=(TFunctionImpl<R(Types...), InlineSize, InlineAlignment, Specifiers, EFunctionType::Object>&& InValue) requires (FunctionType == EFunctionType::Unique)
	{
		AssignImpl(MoveTemp(*reinterpret_cast<TFunctionImpl*>(&InValue)));
		return *this;
	}

	constexpr TFunctionImpl& operator=(nullptr_t) requires (FunctionType != EFunctionType::Reference) { Reset(); return *this; }

	template <typename T> requires (FunctionType != EFunctionType::Reference) && (!TIsTFunctionImpl<typename TDecay<T>::Type>::Value)
		&& TIsInvocableResultWithSpecifiers<Specifiers, ResultType, typename TDecay<T>::Type, Types...>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
		&& ((FunctionType == EFunctionType::Object && TIsCopyConstructible<typename TDecay<T>::Type>::Value)
			|| (FunctionType == EFunctionType::Unique && TIsMoveConstructible<typename TDecay<T>::Type>::Value))
	FORCEINLINE TFunctionImpl& operator=(T&& InValue)
	{
		using DecayedFunctorType = typename TDecay<T>::Type;

		if (!FFunctionIsBound::F(InValue)) Reset();
		else EmplaceImpl<DecayedFunctorType>(Forward<T>(InValue));

		return *this;
	}

	template <typename T, typename... ArgTypes> requires (FunctionType != EFunctionType::Reference)
		&& TIsInvocableResultWithSpecifiers<Specifiers, ResultType, typename TDecay<T>::Type, Types...>::Value
		&& TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value
		&& ((FunctionType == EFunctionType::Object && TIsCopyConstructible<typename TDecay<T>::Type>::Value)
			|| (FunctionType == EFunctionType::Unique && TIsMoveConstructible<typename TDecay<T>::Type>::Value))
	FORCEINLINE typename TDecay<T>::Type& Emplace(ArgTypes&&... Args)
	{
		using DecayedFunctorType = typename TDecay<T>::Type;
		EmplaceImpl<DecayedFunctorType>(Forward<ArgTypes>(Args)...);
		return Target<DecayedFunctorType>();
	}

	FORCEINLINE ResultType operator()(Types... Args)         requires (Specifiers == EFunctionSpecifiers::None       ) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) &       requires (Specifiers == EFunctionSpecifiers::LValue     ) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) &&      requires (Specifiers == EFunctionSpecifiers::RValue     ) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const   requires (Specifiers == EFunctionSpecifiers::Const      ) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const&  requires (Specifiers == EFunctionSpecifiers::ConstLValue) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const&& requires (Specifiers == EFunctionSpecifiers::ConstRValue) { return CallImpl(Forward<Types>(Args)...); }

	constexpr bool           IsValid() const { return Callable != nullptr; }
	constexpr explicit operator bool() const { return Callable != nullptr; }

	FORCEINLINE const FTypeInfo& TargetType() const requires (FunctionType != EFunctionType::Reference) { return IsValid() ? Storage.GetTypeInfo() : Typeid(void); };

	template <typename T> FORCEINLINE       T&  Target() &       requires (FunctionType != EFunctionType::Reference) && TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<      StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE       T&& Target() &&      requires (FunctionType != EFunctionType::Reference) && TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<      StorageType&&>(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&  Target() const&  requires (FunctionType != EFunctionType::Reference) && TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<const StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&& Target() const&& requires (FunctionType != EFunctionType::Reference) && TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<const StorageType&&>(Storage).template GetValue<T>(); }

	constexpr void Reset() requires (FunctionType != EFunctionType::Reference) { Callable = nullptr; }

	constexpr void Swap(TFunctionImpl& InValue) requires (FunctionType != EFunctionType::Reference)
	{
		if (!IsValid() && !InValue.IsValid()) return;

		if (IsValid() && !InValue.IsValid())
		{
			InValue = MoveTemp(*this);
			Reset();
			return;
		}

		if (InValue.IsValid() && !IsValid())
		{
			*this = MoveTemp(InValue);
			InValue.Reset();
			return;
		}
		
		NAMESPACE_REDCRAFT::Swap(Callable, InValue.Callable);
		NAMESPACE_REDCRAFT::Swap(Storage, InValue.Storage);
	}

private:

	using StorageType = typename TConditional<FunctionType == EFunctionType::Reference, void*, TAny<InlineSize, InlineAlignment>>::Type;
	using StorageRef = typename TConditional<FunctionType == EFunctionType::Reference, void*, typename TFunctionCallSpecifiers<StorageType, Specifiers>::Type&>::Type;
	using CallFunc = ResultType(*)(StorageRef, Types&&...);

	StorageType Storage;
	CallFunc Callable;

	template <typename SelectedType, typename... ArgTypes>
	FORCEINLINE void EmplaceImpl(ArgTypes&&... Args)
	{
		if constexpr (FunctionType == EFunctionType::Reference) Storage = ((void*)&Args, ...);
		else Storage.template Emplace<SelectedType>(Forward<ArgTypes>(Args)...);

		Callable = [](StorageRef Storage, Types&&... Args) -> ResultType
		{
			const auto GetFunc = [&Storage]() -> decltype(auto)
			{
				if constexpr (FunctionType == EFunctionType::Reference) return *reinterpret_cast<SelectedType*>(Storage);
				else return Storage.template GetValue<SelectedType>();
			};

			return InvokeResult<R>(Forward<typename TFunctionCallSpecifiers<SelectedType, Specifiers>::Type>(GetFunc()), Forward<Types>(Args)...);
		};
	}

	FORCEINLINE ResultType CallImpl(Types&&... Args)
	{
		checkf(IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		return Callable(Storage, Forward<Types>(Args)...);
	}

	FORCEINLINE ResultType CallImpl(Types&&... Args) const
	{
		checkf(IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		return Callable(Storage, Forward<Types>(Args)...);
	}

	FORCEINLINE void AssignImpl(const TFunctionImpl& InValue)
	{
		if (InValue.IsValid())
		{
			Callable = InValue.Callable;
			Storage = InValue.Storage;
		}
		else Reset();
	}

	FORCEINLINE void AssignImpl(TFunctionImpl&& InValue)
	{
		if (InValue.IsValid())
		{
			Callable = InValue.Callable;
			Storage = MoveTemp(InValue.Storage);
			InValue.Reset();
		}
		else Reset();
	}

};

template <typename F, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect;

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...)        , InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::None, FunctionType>;
};

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...) &      , InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::LValue, FunctionType>;
};

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...) &&     , InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::RValue, FunctionType>;
};

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...) const  , InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::Const, FunctionType>;
};

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...) const& , InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::ConstLValue, FunctionType>;
};

template <typename R, typename... Types, size_t InlineSize, size_t InlineAlignment, EFunctionType FunctionType>
struct TFunctionSelect<R(Types...) const&&, InlineSize, InlineAlignment, FunctionType>
{
	using Type = TFunctionImpl<R(Types...), InlineSize, InlineAlignment, EFunctionSpecifiers::ConstRValue, FunctionType>;
};

NAMESPACE_PRIVATE_END

inline constexpr size_t FUNCTION_DEFAULT_INLINE_SIZE      = 32;
inline constexpr size_t FUNCTION_DEFAULT_INLINE_ALIGNMENT = 16;

template <typename F>
using TFunctionRef = typename NAMESPACE_PRIVATE::TFunctionSelect<F, INDEX_NONE, INDEX_NONE, NAMESPACE_PRIVATE::EFunctionType::Reference>::Type;

template <typename F, size_t InlineSize = FUNCTION_DEFAULT_INLINE_SIZE, size_t InlineAlignment = FUNCTION_DEFAULT_INLINE_ALIGNMENT>
using TFunction = typename NAMESPACE_PRIVATE::TFunctionSelect<F, InlineSize, InlineAlignment, NAMESPACE_PRIVATE::EFunctionType::Object>::Type;

template <typename F, size_t InlineSize = FUNCTION_DEFAULT_INLINE_SIZE, size_t InlineAlignment = FUNCTION_DEFAULT_INLINE_ALIGNMENT>
using TUniqueFunction = typename NAMESPACE_PRIVATE::TFunctionSelect<F, InlineSize, InlineAlignment, NAMESPACE_PRIVATE::EFunctionType::Unique>::Type;

template <typename T> struct TIsTFunctionRef    : NAMESPACE_PRIVATE::TIsTFunctionRef<T>    { };
template <typename T> struct TIsTFunction       : NAMESPACE_PRIVATE::TIsTFunction<T>       { };
template <typename T> struct TIsTUniqueFunction : NAMESPACE_PRIVATE::TIsTUniqueFunction<T> { };

template <typename F>
constexpr bool operator==(const TFunctionRef<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <typename F>
constexpr bool operator==(const TFunction<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <typename F>
constexpr bool operator==(const TUniqueFunction<F>& LHS, nullptr_t)
{
	return !LHS;
}

static_assert(sizeof(TFunction<void()>)       == 64, "The byte size of TFunction is unexpected");
static_assert(sizeof(TUniqueFunction<void()>) == 64, "The byte size of TUniqueFunction is unexpected");

template <typename T = void>
struct TIdentity
{
	using Type = T;

	constexpr T&& operator()(T&& InValue) const
	{
		return Forward<T>(InValue);
	}
};

template <>
struct TIdentity<void>
{
	using Type = void;

	template<typename T>
	constexpr T&& operator()(T&& InValue) const
	{
		return Forward<T>(InValue);
	}
};

NAMESPACE_PRIVATE_BEGIN

template <typename F>
struct NotFunctionType
{
	F Func;

	NotFunctionType(const NotFunctionType&) = default;
	NotFunctionType(NotFunctionType&&) = default;

	template <typename InF>
	constexpr NotFunctionType(InF&& InFunc) : Func(Forward<InF>(InFunc)) { }

	template <typename... Types> requires TIsInvocable<F&, Types&&...>::Value
		constexpr auto operator()(Types&&... Args) &
		-> decltype(!Invoke(Func, Forward<Types>(Args)...))
	{
		return !Invoke(Func, Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<F&&, Types&&...>::Value
		constexpr auto operator()(Types&&... Args) &&
		-> decltype(!Invoke(MoveTemp(Func), Forward<Types>(Args)...))
	{
		return !Invoke(MoveTemp(Func), Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<const F&, Types&&...>::Value
		constexpr auto operator()(Types&&... Args) const&
		-> decltype(!Invoke(Func, Forward<Types>(Args)...))
	{
		return !Invoke(Func, Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<const F&&, Types&&...>::Value
		constexpr auto operator()(Types&&... Args) const&&
		-> decltype(!Invoke(MoveTemp(Func), Forward<Types>(Args)...))
	{
		return !Invoke(MoveTemp(Func), Forward<Types>(Args)...);
	}
};

NAMESPACE_PRIVATE_END

template <typename F>
constexpr NAMESPACE_PRIVATE::NotFunctionType<typename TDecay<F>::Type> NotFn(F&& Func)
{
	return NAMESPACE_PRIVATE::NotFunctionType<typename TDecay<F>::Type>(Forward<F>(Func));
}

#define FUNCTOR_UNARY_OPERATOR_IMPL(Name, Operator, ConceptT, ConceptU)  \
	template <typename T = void> requires (CSameAs<T, void> || ConceptT) \
	struct Name                                                          \
	{                                                                    \
		constexpr auto operator()(const T& InValue) const                \
			-> decltype(Operator InValue)                                \
		{                                                                \
			return Operator InValue;                                     \
		}                                                                \
	};                                                                   \
	                                                                     \
	template <>                                                          \
	struct Name<void>                                                    \
	{                                                                    \
		template <typename U> requires ConceptU                          \
		constexpr auto operator()(U&& InValue) const                     \
			-> decltype(Operator Forward<U>(InValue))                    \
		{                                                                \
			return Operator Forward<U>(InValue);                         \
		}                                                                \
	}

#define FUNCTOR_BINARY_OPERATOR_IMPL(Name, Operator, ConceptT, ConceptTU) \
	template <typename T = void> requires (CSameAs<T, void> || ConceptT)  \
	struct Name                                                           \
	{                                                                     \
		constexpr auto operator()(const T& LHS, const T& RHS) const       \
			-> decltype(LHS Operator RHS)                                 \
		{                                                                 \
			return LHS Operator RHS;                                      \
		}                                                                 \
	};                                                                    \
	                                                                      \
	template <>                                                           \
	struct Name<void>                                                     \
	{                                                                     \
		template <typename T, typename U> requires ConceptTU              \
		constexpr auto operator()(T&& LHS, U&& RHS) const                 \
			-> decltype(Forward<T>(LHS) Operator Forward<U>(RHS))         \
		{                                                                 \
			return Forward<T>(LHS) Operator Forward<U>(RHS);              \
		}                                                                 \
	}

#define FUNCTOR_UNARY_OPERATOR_A_IMPL(Name, Operator)                                \
	FUNCTOR_UNARY_OPERATOR_IMPL                                                      \
	(                                                                                \
		Name, Operator,                                                              \
		(requires(const T& InValue) { { Operator InValue } -> CConvertibleTo<T>; }), \
		(requires(U&& InValue) { Operator Forward<U>(InValue); })                    \
	)

#define FUNCTOR_BINARY_OPERATOR_A_IMPL(Name, Operator)                                         \
	FUNCTOR_BINARY_OPERATOR_IMPL                                                               \
	(                                                                                          \
		Name, Operator,                                                                        \
		(requires(const T& LHS, const T& RHS) { { LHS Operator RHS } -> CConvertibleTo<T>; }), \
		(requires(T&& LHS, U&& RHS) { Forward<T>(LHS) Operator Forward<U>(RHS); })             \
	)
	
#define FUNCTOR_UNARY_OPERATOR_B_IMPL(Name, Operator)                                     \
	FUNCTOR_UNARY_OPERATOR_IMPL                                                           \
	(                                                                                     \
		Name, Operator,                                                                   \
		(requires(const T& InValue) { { Operator InValue } -> CBooleanTestable; }),       \
		(requires(U&& InValue) { { Operator Forward<U>(InValue) } -> CBooleanTestable; }) \
	)

#define FUNCTOR_BINARY_OPERATOR_B_IMPL(Name, Operator)                                                     \
	FUNCTOR_BINARY_OPERATOR_IMPL                                                                           \
	(                                                                                                      \
		Name, Operator,                                                                                    \
		(requires(const T& LHS, const T& RHS) { { LHS Operator RHS } -> CBooleanTestable; }),              \
		(requires(T&& LHS, U&& RHS) { { Forward<T>(LHS) Operator Forward<U>(RHS) } -> CBooleanTestable; }) \
	)
	
#define FUNCTOR_BINARY_OPERATOR_C_IMPL(Name, Operator) \
	FUNCTOR_BINARY_OPERATOR_IMPL                       \
	(                                                  \
		Name, Operator,                                \
		(CEqualityComparable<T>),                      \
		(CEqualityComparableWith<T, U>)                \
	)
	
#define FUNCTOR_BINARY_OPERATOR_D_IMPL(Name, Operator) \
	FUNCTOR_BINARY_OPERATOR_IMPL                       \
	(                                                  \
		Name, Operator,                                \
		(CTotallyOrdered<T>),                          \
		(CTotallyOrderedWith<T, U>)                    \
	)
	
FUNCTOR_UNARY_OPERATOR_A_IMPL (TPromote,    +);
FUNCTOR_UNARY_OPERATOR_A_IMPL (TNegate,     -);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TPlus,       +);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TMinus,      -);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TMultiplies, *);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TDivides,    /);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TModulus,    %);

FUNCTOR_UNARY_OPERATOR_A_IMPL (TBitNot, ~ );
FUNCTOR_BINARY_OPERATOR_A_IMPL(TBitAnd, & );
FUNCTOR_BINARY_OPERATOR_A_IMPL(TBitOr,  | );
FUNCTOR_BINARY_OPERATOR_A_IMPL(TBitXor, ^ );
FUNCTOR_BINARY_OPERATOR_A_IMPL(TBitLsh, <<);
FUNCTOR_BINARY_OPERATOR_A_IMPL(TBitRsh, >>);

FUNCTOR_BINARY_OPERATOR_B_IMPL(TLogicalAnd, &&);
FUNCTOR_BINARY_OPERATOR_B_IMPL(TLogicalOr,  ||);
FUNCTOR_UNARY_OPERATOR_B_IMPL (TLogicalNot, ! );

FUNCTOR_BINARY_OPERATOR_C_IMPL(TEqualTo,      ==);
FUNCTOR_BINARY_OPERATOR_C_IMPL(TNotEqualTo,   !=);
FUNCTOR_BINARY_OPERATOR_D_IMPL(TGreater,      > );
FUNCTOR_BINARY_OPERATOR_D_IMPL(TLess,         < );
FUNCTOR_BINARY_OPERATOR_D_IMPL(TGreaterEqual, >=);
FUNCTOR_BINARY_OPERATOR_D_IMPL(TLessEqual,    <=);

#undef FUNCTOR_BINARY_OPERATOR_D_IMPL
#undef FUNCTOR_BINARY_OPERATOR_C_IMPL

#undef FUNCTOR_BINARY_OPERATOR_B_IMPL
#undef FUNCTOR_UNARY_OPERATOR_B_IMPL

#undef FUNCTOR_BINARY_OPERATOR_A_IMPL
#undef FUNCTOR_UNARY_OPERATOR_A_IMPL

#undef FUNCTOR_BINARY_OPERATOR_IMPL
#undef FUNCTOR_UNARY_OPERATOR_IMPL

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
