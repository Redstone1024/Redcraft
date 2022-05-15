#pragma once

#include "CoreTypes.h"
#include "Templates/Any.h"
#include "Templates/Tuple.h"
#include "Templates/Invoke.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

// NOTE: Disable alignment limit warning
#pragma warning(disable : 4359)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

inline constexpr size_t FUNCTION_DEFAULT_INLINE_SIZE      = ANY_DEFAULT_INLINE_SIZE - sizeof(uintptr);
inline constexpr size_t FUNCTION_DEFAULT_INLINE_ALIGNMENT = ANY_DEFAULT_INLINE_ALIGNMENT;

template <typename F> requires TIsFunction<F>::Value
struct TFunctionRef;

template <typename F, size_t InlineSize, size_t InlineAlignment> requires TIsFunction<F>::Value && (Memory::IsValidAlignment(InlineAlignment))
struct TFunction;

template <typename F, size_t InlineSize, size_t InlineAlignment> requires TIsFunction<F>::Value && (Memory::IsValidAlignment(InlineAlignment))
struct TUniqueFunction;

template <typename T> struct TIsTFunctionRef                  : FFalse { };
template <typename F> struct TIsTFunctionRef<TFunctionRef<F>> : FTrue  { };

template <typename T>                     struct TIsTFunction                     : FFalse { };
template <typename F, size_t I, size_t J> struct TIsTFunction<TFunction<F, I, J>> : FTrue  { };

template <typename T>                     struct TIsTUniqueFunction                           : FFalse { };
template <typename F, size_t I, size_t J> struct TIsTUniqueFunction<TUniqueFunction<F, I, J>> : FTrue  { };

NAMESPACE_PRIVATE_BEGIN

template <typename T>
constexpr bool FunctionIsBound(const T& Func)
{
	if constexpr (TIsPointer<T>::Value || TIsMemberPointer<T>::Value || TIsTFunctionRef<T>::Value || TIsTFunction<T>::Value || TIsTUniqueFunction<T>::Value)
	{
		return !!Func;
	}
	else
	{
		return true;
	}
}

template <typename Signature, typename F> struct TIsInvocableSignature : FFalse { };

template <typename Ret, typename... Types, typename F>
struct TIsInvocableSignature<Ret(Types...), F>
	: TBoolConstant<TIsInvocableResult<Ret, F, Types...>::Value && TIsInvocableResult<Ret, F&, Types...>::Value>
{ };

template <typename Ret, typename... Types, typename F> struct TIsInvocableSignature<Ret(Types...) & , F> : TIsInvocableResult<Ret, F&, Types...> { };
template <typename Ret, typename... Types, typename F> struct TIsInvocableSignature<Ret(Types...) &&, F> : TIsInvocableResult<Ret, F , Types...> { };

template <typename Ret, typename... Types, typename F>
struct TIsInvocableSignature<Ret(Types...) const, F>
	: TBoolConstant<TIsInvocableResult<Ret, const F, Types...>::Value && TIsInvocableResult<Ret, const F&, Types...>::Value>
{ };

template <typename Ret, typename... Types, typename F> struct TIsInvocableSignature<Ret(Types...) const& , F> : TIsInvocableResult<Ret, const F&, Types...> { };
template <typename Ret, typename... Types, typename F> struct TIsInvocableSignature<Ret(Types...) const&&, F> : TIsInvocableResult<Ret, const F , Types...> { };

template <typename F>                      struct TFunctionInfo;
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...)        > { using Fn = Ret(Types...); using CVRef =       int;   };
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...) &      > { using Fn = Ret(Types...); using CVRef =       int&;  };
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...) &&     > { using Fn = Ret(Types...); using CVRef =       int&&; };
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...) const  > { using Fn = Ret(Types...); using CVRef = const int;   };
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...) const& > { using Fn = Ret(Types...); using CVRef = const int&;  };
template <typename Ret, typename... Types> struct TFunctionInfo<Ret(Types...) const&&> { using Fn = Ret(Types...); using CVRef = const int&&; };

template <typename F, typename CVRef, size_t InlineSize, size_t InlineAlignment, bool bIsRef> struct TFunctionImpl;

template <typename Ret, typename... Types, typename CVRef, size_t InlineSize, size_t InlineAlignment, bool bIsRef>
struct alignas(InlineAlignment) TFunctionImpl<Ret(Types...), CVRef, InlineSize, InlineAlignment, bIsRef>
{
public:

	using ResultType = Ret;
	using ArgumentType = TTuple<Types...>;

	TFunctionImpl() = default;
	TFunctionImpl(const TFunctionImpl&) = default;
	TFunctionImpl(TFunctionImpl&& InValue) = default;
	TFunctionImpl& operator=(const TFunctionImpl&) = default;
	TFunctionImpl& operator=(TFunctionImpl&&) = default;
	~TFunctionImpl() = default;

	FORCEINLINE ResultType operator()(Types... Args)         requires (TIsSame<CVRef,       int  >::Value) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) &       requires (TIsSame<CVRef,       int& >::Value) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) &&      requires (TIsSame<CVRef,       int&&>::Value) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const   requires (TIsSame<CVRef, const int  >::Value) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const&  requires (TIsSame<CVRef, const int& >::Value) { return CallImpl(Forward<Types>(Args)...); }
	FORCEINLINE ResultType operator()(Types... Args) const&& requires (TIsSame<CVRef, const int&&>::Value) { return CallImpl(Forward<Types>(Args)...); }

	constexpr bool           IsValid() const { return Callable != nullptr; }
	constexpr explicit operator bool() const { return Callable != nullptr; }

	FORCEINLINE const type_info& TargetType() const requires (!bIsRef) { return IsValid() ? Storage.GetTypeInfo() : typeid(void); };

	template <typename T> FORCEINLINE       T&  Target() &       requires (!bIsRef) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<      StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE       T&& Target() &&      requires (!bIsRef) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<      StorageType&&>(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&  Target() const&  requires (!bIsRef) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<const StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&& Target() const&& requires (!bIsRef) && TIsDestructible<typename TDecay<T>::Type>::Value { return static_cast<const StorageType&&>(Storage).template GetValue<T>(); }

	constexpr void Swap(TFunctionImpl& InValue) requires (!bIsRef)
	{
		using NAMESPACE_REDCRAFT::Swap;

		if (!IsValid() && !InValue.IsValid()) return;

		if (IsValid() && !InValue.IsValid())
		{
			InValue = MoveTemp(*this);
			ResetImpl();
			return;
		}

		if (InValue.IsValid() && !IsValid())
		{
			*this = MoveTemp(InValue);
			InValue.ResetImpl();
			return;
		}
		
		Swap(Callable, InValue.Callable);
		Swap(Storage, InValue.Storage);
	}

private:

	using StorageType = typename TConditional<bIsRef, typename TCopyConst<CVRef, void>::Type*, TAny<InlineSize, 1>>::Type;
	using StorageRef  = typename TConditional<bIsRef, typename TCopyConst<CVRef, void>::Type*, typename TCopyCVRef<CVRef, StorageType>::Type&>::Type;

	using CallFunc = ResultType(*)(StorageRef, Types&&...);

	StorageType Storage;
	CallFunc Callable;

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

protected:

	template <typename DecayedType, typename... ArgTypes>
	FORCEINLINE void EmplaceImpl(ArgTypes&&... Args)
	{
		using CallableType = typename TCopyConst<typename TRemoveReference<CVRef>::Type, DecayedType>::Type;

		if constexpr (bIsRef) Storage = ((reinterpret_cast<StorageType>(AddressOf(Args))), ...);
		else Storage.template Emplace<DecayedType>(Forward<ArgTypes>(Args)...);

		Callable = [](StorageRef Storage, Types&&... Args) -> ResultType
		{
			using InvokeType = typename TConditional<
				TIsReference<CVRef>::Value,
				typename TCopyCVRef<CVRef, CallableType>::Type,
				typename TCopyCVRef<CVRef, CallableType>::Type&
			>::Type;

			const auto GetFunc = [&Storage]() -> InvokeType
			{
				if constexpr (!bIsRef) return Storage.template GetValue<DecayedType>();
				else return static_cast<InvokeType>(*reinterpret_cast<CallableType*>(Storage));
			};

			return InvokeResult<ResultType>(GetFunc(), Forward<Types>(Args)...);
		};
	}

	FORCEINLINE void AssignImpl(const TFunctionImpl& InValue)
	{
		if (InValue.IsValid())
		{
			Callable = InValue.Callable;
			Storage = InValue.Storage;
		}
		else ResetImpl();
	}

	FORCEINLINE void AssignImpl(TFunctionImpl&& InValue)
	{
		if (InValue.IsValid())
		{
			Callable = InValue.Callable;
			Storage = MoveTemp(InValue.Storage);
			InValue.ResetImpl();
		}
		else ResetImpl();
	}

	constexpr void ResetImpl() { Callable = nullptr; }

};

NAMESPACE_PRIVATE_END

template <typename F> requires TIsFunction<F>::Value
struct TFunctionRef 
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		FUNCTION_DEFAULT_INLINE_SIZE,
		FUNCTION_DEFAULT_INLINE_ALIGNMENT,
		true>
{
private:

	using Super = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		FUNCTION_DEFAULT_INLINE_SIZE,
		FUNCTION_DEFAULT_INLINE_ALIGNMENT,
		true>;

public:

	TFunctionRef() = delete;

	TFunctionRef(const TFunctionRef& InValue) = default;
	TFunctionRef(TFunctionRef&& InValue) = default;

	TFunctionRef& operator=(const TFunctionRef& InValue) = delete;
	TFunctionRef& operator=(TFunctionRef&& InValue) = delete;

	template <typename T> requires (!TIsTFunctionRef<typename TDecay<T>::Type>::Value) && (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
	FORCEINLINE TFunctionRef(T&& InValue)
	{
		using DecayedType = typename TDecay<T>::Type;
		checkf(NAMESPACE_PRIVATE::FunctionIsBound(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		Super::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}
	
	template <typename T>
	TFunctionRef(const T&&) = delete;

};

template <typename F, size_t InlineSize = FUNCTION_DEFAULT_INLINE_SIZE, size_t InlineAlignment = FUNCTION_DEFAULT_INLINE_ALIGNMENT>
	requires TIsFunction<F>::Value && (Memory::IsValidAlignment(InlineAlignment))
struct TFunction 
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		InlineSize,
		InlineAlignment,
		false>
{
private:

	using Super = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		InlineSize,
		InlineAlignment,
		false>;

public:

	constexpr TFunction(nullptr_t = nullptr) { Super::ResetImpl(); }

	FORCEINLINE TFunction(const TFunction& InValue) = default;
	FORCEINLINE TFunction(TFunction&& InValue) : Super(MoveTemp(InValue)) { InValue.ResetImpl(); }

	FORCEINLINE TFunction& operator=(const TFunction& InValue)
	{
		Super::AssignImpl(InValue);
		return *this;
	}

	FORCEINLINE TFunction& operator=(TFunction&& InValue)
	{
		if (&InValue == this) return *this;
		Super::AssignImpl(MoveTemp(InValue));
		return *this;
	}

	template <typename T> requires (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& (!TIsTFunctionRef<typename TDecay<T>::Type>::Value) && (!TIsTFunction<typename TDecay<T>::Type>::Value) && (!TIsTUniqueFunction<typename TDecay<T>::Type>::Value)
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value && TIsCopyConstructible<typename TDecay<T>::Type>::Value
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
	FORCEINLINE TFunction(T&& InValue)
	{
		using DecayedType = typename TDecay<T>::Type;
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Super::ResetImpl();
		else Super::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}
	
	template <typename T, typename... ArgTypes> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value && TIsCopyConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE TFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		using DecayedType = typename TDecay<T>::Type;
		Super::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
	}

	constexpr TFunction& operator=(nullptr_t) { Super::ResetImpl(); return *this; }

	template <typename T> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& (!TIsTFunctionRef<typename TDecay<T>::Type>::Value) && (!TIsTFunction<typename TDecay<T>::Type>::Value) && (!TIsTUniqueFunction<typename TDecay<T>::Type>::Value)
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value && TIsCopyConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE TFunction& operator=(T&& InValue)
	{
		using DecayedType = typename TDecay<T>::Type;

		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Super::ResetImpl();
		else Super::template EmplaceImpl<DecayedType>(Forward<T>(InValue));

		return *this;
	}

	template <typename T, typename... ArgTypes> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value&& TIsCopyConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE typename TDecay<T>::Type& Emplace(ArgTypes&&... Args)
	{
		using DecayedType = typename TDecay<T>::Type;
		Super::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
		return Super::template Target<DecayedType>();
	}

	constexpr void Reset() { Super::ResetImpl(); }

};

template <typename F, size_t InlineSize = FUNCTION_DEFAULT_INLINE_SIZE, size_t InlineAlignment = FUNCTION_DEFAULT_INLINE_ALIGNMENT>
	requires TIsFunction<F>::Value && (Memory::IsValidAlignment(InlineAlignment))
struct TUniqueFunction
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		InlineSize,
		InlineAlignment,
	false>
{
private:

	using Super = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		InlineSize,
		InlineAlignment,
		false>;

public:

	constexpr TUniqueFunction(nullptr_t = nullptr) { Super::ResetImpl(); }

	FORCEINLINE TUniqueFunction(const TUniqueFunction& InValue) = delete;
	TUniqueFunction(TUniqueFunction&& InValue) : Super(MoveTemp(InValue)) { InValue.ResetImpl(); }

	FORCEINLINE TUniqueFunction& operator=(const TUniqueFunction& InValue) = delete;
	FORCEINLINE TUniqueFunction& operator=(TUniqueFunction&& InValue)
	{
		if (&InValue == this) return *this;
		Super::AssignImpl(MoveTemp(InValue));
		return *this;
	}

	FORCEINLINE TUniqueFunction(const TFunction<F, InlineSize, InlineAlignment>& InValue)
		: Super(*reinterpret_cast<const TUniqueFunction*>(&InValue))
	{ }

	FORCEINLINE TUniqueFunction(TFunction<F, InlineSize, InlineAlignment>&& InValue)
		: Super(MoveTemp(*reinterpret_cast<const TUniqueFunction*>(&InValue)))
	{
		InValue.Reset();
	}

	FORCEINLINE TUniqueFunction& operator=(const TFunction<F, InlineSize, InlineAlignment>& InValue)
	{
		Super::AssignImpl(*reinterpret_cast<const TUniqueFunction*>(&InValue));
		return *this;
	}

	FORCEINLINE TUniqueFunction& operator=(TFunction<F, InlineSize, InlineAlignment>&& InValue)
	{
		Super::AssignImpl(MoveTemp(*reinterpret_cast<TUniqueFunction*>(&InValue)));
		return *this;
	}

	template <typename T> requires (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& (!TIsTFunctionRef<typename TDecay<T>::Type>::Value) && (!TIsTFunction<typename TDecay<T>::Type>::Value) && (!TIsTUniqueFunction<typename TDecay<T>::Type>::Value)
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value && TIsMoveConstructible<typename TDecay<T>::Type>::Value
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
	FORCEINLINE TUniqueFunction(T&& InValue)
	{
		using DecayedType = typename TDecay<T>::Type;
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Super::ResetImpl();
		else Super::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}

	template <typename T, typename... ArgTypes> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value && TIsMoveConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE TUniqueFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		using DecayedType = typename TDecay<T>::Type;
		Super::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
	}

	constexpr TUniqueFunction& operator=(nullptr_t) { Super::ResetImpl(); return *this; }

	template <typename T> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& (!TIsTFunctionRef<typename TDecay<T>::Type>::Value) && (!TIsTFunction<typename TDecay<T>::Type>::Value) && (!TIsTUniqueFunction<typename TDecay<T>::Type>::Value)
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value&& TIsMoveConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE TUniqueFunction& operator=(T&& InValue)
	{
		using DecayedType = typename TDecay<T>::Type;

		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Super::ResetImpl();
		else Super::template EmplaceImpl<DecayedType>(Forward<T>(InValue));

		return *this;
	}
	
	template <typename T, typename... ArgTypes> requires NAMESPACE_PRIVATE::TIsInvocableSignature<F, typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, ArgTypes...>::Value&& TIsMoveConstructible<typename TDecay<T>::Type>::Value
	FORCEINLINE typename TDecay<T>::Type& Emplace(ArgTypes&&... Args)
	{
		using DecayedType = typename TDecay<T>::Type;
		Super::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
		return Super::template Target<DecayedType>();
	}

	constexpr void Reset() { Super::ResetImpl(); }

};

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

NAMESPACE_PRIVATE_BEGIN

template <typename F>
struct TNotFunction
{
	F Storage;

	TNotFunction(const TNotFunction&) = default;
	TNotFunction(TNotFunction&&) = default;

	template <typename InF>
	constexpr TNotFunction(InF&& InFunc) : Storage(Forward<InF>(InFunc)) { }

	template <typename... Types> requires TIsInvocable<F&, Types&&...>::Value
	constexpr auto operator()(Types&&... Args) &
		-> decltype(!Invoke(Storage, Forward<Types>(Args)...))
	{
		return !Invoke(Storage, Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<F&&, Types&&...>::Value
	constexpr auto operator()(Types&&... Args) &&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Types>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<const F&, Types&&...>::Value
	constexpr auto operator()(Types&&... Args) const&
		-> decltype(!Invoke(Storage, Forward<Types>(Args)...))
	{
		return !Invoke(Storage, Forward<Types>(Args)...);
	}

	template <typename... Types> requires TIsInvocable<const F&&, Types&&...>::Value
	constexpr auto operator()(Types&&... Args) const&&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Types>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Types>(Args)...);
	}
};

NAMESPACE_PRIVATE_END

template <typename F>
constexpr NAMESPACE_PRIVATE::TNotFunction<typename TDecay<F>::Type> NotFn(F&& Func)
{
	return NAMESPACE_PRIVATE::TNotFunction<typename TDecay<F>::Type>(Forward<F>(Func));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
