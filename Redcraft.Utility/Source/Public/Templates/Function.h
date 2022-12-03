#pragma once

#include "CoreTypes.h"
#include "Templates/Any.h"
#include "Templates/Meta.h"
#include "Templates/Invoke.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CFunction F>
class TFunctionRef;

template <CFunction F>
class TFunction;

template <CFunction F>
class TUniqueFunction;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTFunctionRef                  : FFalse { };
template <typename F> struct TIsTFunctionRef<TFunctionRef<F>> : FTrue  { };

template <typename T> struct TIsTFunction               : FFalse { };
template <typename F> struct TIsTFunction<TFunction<F>> : FTrue  { };

template <typename T> struct TIsTUniqueFunction                     : FFalse { };
template <typename F> struct TIsTUniqueFunction<TUniqueFunction<F>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTFunctionRef    = NAMESPACE_PRIVATE::TIsTFunctionRef<TRemoveCV<T>>::Value;
template <typename T> concept CTFunction       = NAMESPACE_PRIVATE::TIsTFunction<TRemoveCV<T>>::Value;
template <typename T> concept CTUniqueFunction = NAMESPACE_PRIVATE::TIsTUniqueFunction<TRemoveCV<T>>::Value;

NAMESPACE_PRIVATE_BEGIN

template <typename T>
constexpr bool FunctionIsBound(const T& Func)
{
	if constexpr (CPointer<T> || CMemberPointer<T> || CTFunctionRef<T> || CTFunction<T> || CTUniqueFunction<T>)
	{
		return !!Func;
	}
	else
	{
		return true;
	}
}

template <typename Signature, typename F> struct TIsInvocableSignature : FFalse { };

template <typename Ret, typename... Ts, typename F>
struct TIsInvocableSignature<Ret(Ts...), F>
	: TBoolConstant<CInvocableResult<Ret, F, Ts...> && CInvocableResult<Ret, F&, Ts...>>
{ };

template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) & , F> : TBoolConstant<CInvocableResult<Ret, F&, Ts...>> { };
template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) &&, F> : TBoolConstant<CInvocableResult<Ret, F , Ts...>> { };

template <typename Ret, typename... Ts, typename F>
struct TIsInvocableSignature<Ret(Ts...) const, F>
	: TBoolConstant<CInvocableResult<Ret, const F, Ts...> && CInvocableResult<Ret, const F&, Ts...>>
{ };

template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) const& , F> : TBoolConstant<CInvocableResult<Ret, const F&, Ts...>> { };
template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) const&&, F> : TBoolConstant<CInvocableResult<Ret, const F , Ts...>> { };

template <typename F>                      struct TFunctionInfo;
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...)        > { using Fn = Ret(Ts...); using CVRef =       int;   };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) &      > { using Fn = Ret(Ts...); using CVRef =       int&;  };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) &&     > { using Fn = Ret(Ts...); using CVRef =       int&&; };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const  > { using Fn = Ret(Ts...); using CVRef = const int;   };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const& > { using Fn = Ret(Ts...); using CVRef = const int&;  };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const&&> { using Fn = Ret(Ts...); using CVRef = const int&&; };

template <typename F, typename CVRef, bool bIsRef> class TFunctionImpl;

template <typename Ret, typename... Ts, typename CVRef, bool bIsRef>
class TFunctionImpl<Ret(Ts...), CVRef, bIsRef>
{
public:

	using ResultType = Ret;
	using ArgumentType = TTypeSequence<Ts...>;
	
	TFunctionImpl() = default;
	TFunctionImpl(const TFunctionImpl&) = default;
	TFunctionImpl(TFunctionImpl&& InValue) = default;
	TFunctionImpl& operator=(const TFunctionImpl&) = default;
	TFunctionImpl& operator=(TFunctionImpl&&) = default;
	~TFunctionImpl() = default;

	FORCEINLINE ResultType operator()(Ts... Args)         requires (CSameAs<CVRef,       int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &       requires (CSameAs<CVRef,       int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &&      requires (CSameAs<CVRef,       int&&>) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const   requires (CSameAs<CVRef, const int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&  requires (CSameAs<CVRef, const int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&& requires (CSameAs<CVRef, const int&&>) { return CallImpl(Forward<Ts>(Args)...); }

	constexpr bool           IsValid() const { return GetCallableImpl() != nullptr; }
	constexpr explicit operator bool() const { return GetCallableImpl() != nullptr; }

	FORCEINLINE const type_info& TargetType() const requires (!bIsRef) { return IsValid() ? Storage.GetTypeInfo() : typeid(void); };

	template <typename T> FORCEINLINE       T&  Target() &       requires (!bIsRef && CDestructible<TDecay<T>>) { return static_cast<      StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE       T&& Target() &&      requires (!bIsRef && CDestructible<TDecay<T>>) { return static_cast<      StorageType&&>(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&  Target() const&  requires (!bIsRef && CDestructible<TDecay<T>>) { return static_cast<const StorageType& >(Storage).template GetValue<T>(); }
	template <typename T> FORCEINLINE const T&& Target() const&& requires (!bIsRef && CDestructible<TDecay<T>>) { return static_cast<const StorageType&&>(Storage).template GetValue<T>(); }

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
		
		Swap(Storage, InValue.Storage);
	}

private:

	using StoragePtrType = TCopyConst<CVRef, void>*;
	using CallableType = ResultType(*)(StoragePtrType, Ts&&...);

	struct FunctionRefStorage
	{
		StoragePtrType Ptr;
		CallableType Callable;
	};
	
	template <typename CallableType>
	struct alignas(16) FFunctionStorage : FSingleton
	{
		//~ Begin CAnyCustomStorage Interface
		inline static constexpr size_t InlineSize      = 64 - sizeof(uintptr) - sizeof(CallableType);
		inline static constexpr size_t InlineAlignment = 16;
		constexpr       void* InlineAllocation()       { return &InlineAllocationImpl; }
		constexpr const void* InlineAllocation() const { return &InlineAllocationImpl; }
		constexpr void*&      HeapAllocation()         { return HeapAllocationImpl;    }
		constexpr void*       HeapAllocation()   const { return HeapAllocationImpl;    }
		constexpr uintptr&    TypeInfo()               { return TypeInfoImpl;          }
		constexpr uintptr     TypeInfo()         const { return TypeInfoImpl;          }
		constexpr void CopyCustom(const FFunctionStorage&  InValue) { Callable = InValue.Callable; }
		constexpr void MoveCustom(      FFunctionStorage&& InValue) { Callable = InValue.Callable; }
		//~ End CAnyCustomStorage Interface
	
		union
		{
			uint8 InlineAllocationImpl[InlineSize];
			void* HeapAllocationImpl;
		};
	
		uintptr TypeInfoImpl;
	
		CallableType Callable;
	
	};

	using FunctionStorage = TAny<FFunctionStorage<CallableType>>;
	using StorageType = TConditional<bIsRef, FunctionRefStorage, FunctionStorage>;

	StorageType Storage;

	FORCEINLINE CallableType& GetCallableImpl()
	{
		if constexpr (bIsRef) return Storage.Callable;
		else return Storage.GetCustomStorage().Callable;
	}

	FORCEINLINE CallableType  GetCallableImpl() const
	{
		if constexpr (bIsRef) return Storage.Callable;
		else return Storage.GetCustomStorage().Callable;
	}

	FORCEINLINE ResultType CallImpl(Ts&&... Args)
	{
		checkf(IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		if constexpr (bIsRef) return GetCallableImpl()(Storage.Ptr, Forward<Ts>(Args)...);
		else return GetCallableImpl()(&Storage, Forward<Ts>(Args)...);
	}

	FORCEINLINE ResultType CallImpl(Ts&&... Args) const
	{
		checkf(IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		if constexpr (bIsRef) return GetCallableImpl()(Storage.Ptr, Forward<Ts>(Args)...);
		else return GetCallableImpl()(&Storage, Forward<Ts>(Args)...);
	}

protected: // These functions should not be used by user-defined class

	template <typename DecayedType, typename... ArgTypes>
	FORCEINLINE void EmplaceImpl(ArgTypes&&... Args)
	{
		using FuncType = TCopyConst<TRemoveReference<CVRef>, DecayedType>;

		if constexpr (bIsRef) Storage.Ptr = (AddressOf(Args), ...);
		else Storage.template Emplace<DecayedType>(Forward<ArgTypes>(Args)...);

		GetCallableImpl() = [](StoragePtrType Storage, Ts&&... Args) -> ResultType
		{
			using InvokeType = TConditional<
				CReference<CVRef>,
				TCopyCVRef<CVRef, FuncType>,
				TCopyCVRef<CVRef, FuncType>&
			>;

			const auto GetFunc = [Storage]() -> InvokeType
			{
				if constexpr (!bIsRef) return static_cast<TCopyConst<CVRef, FunctionStorage>*>(Storage)->template GetValue<DecayedType>();
				else return static_cast<InvokeType>(*reinterpret_cast<FuncType*>(Storage));
			};

			return InvokeResult<ResultType>(GetFunc(), Forward<Ts>(Args)...);
		};
	}

	FORCEINLINE void AssignImpl(const TFunctionImpl& InValue)
	{
		if (InValue.IsValid()) Storage = InValue.Storage;
		else ResetImpl();
	}

	FORCEINLINE void AssignImpl(TFunctionImpl&& InValue)
	{
		if (InValue.IsValid())
		{
			Storage = MoveTemp(InValue.Storage);
			InValue.ResetImpl();
		}
		else ResetImpl();
	}

	constexpr void ResetImpl() { GetCallableImpl() = nullptr; }

};

NAMESPACE_PRIVATE_END

template <CFunction F>
class TFunctionRef
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		true>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		true>;

public:

	TFunctionRef() = delete;

	TFunctionRef(const TFunctionRef& InValue) = default;
	TFunctionRef(TFunctionRef&& InValue) = default;

	TFunctionRef& operator=(const TFunctionRef& InValue) = delete;
	TFunctionRef& operator=(TFunctionRef&& InValue) = delete;

	template <typename T> requires (!CTFunctionRef<TDecay<T>> && !CTInPlaceType<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TFunctionRef(T&& InValue)
	{
		using DecayedType = TDecay<T>;
		checkf(NAMESPACE_PRIVATE::FunctionIsBound(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		Impl::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}
	
	template <typename T>
	TFunctionRef(const T&&) = delete;

};

template <CFunction F>
class TFunction 
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false>;

public:

	constexpr TFunction(nullptr_t = nullptr) { Impl::ResetImpl(); }

	FORCEINLINE TFunction(const TFunction& InValue) = default;
	FORCEINLINE TFunction(TFunction&& InValue) : Impl(MoveTemp(InValue)) { InValue.ResetImpl(); }

	FORCEINLINE TFunction& operator=(const TFunction& InValue)
	{
		Impl::AssignImpl(InValue);
		return *this;
	}

	FORCEINLINE TFunction& operator=(TFunction&& InValue)
	{
		if (&InValue == this) return *this;
		Impl::AssignImpl(MoveTemp(InValue));
		return *this;
	}

	template <typename T> requires (!CTInPlaceType<TDecay<T>>
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CCopyConstructible<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TFunction(T&& InValue)
	{
		using DecayedType = TDecay<T>;
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::ResetImpl();
		else Impl::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}
	
	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CCopyConstructible<TDecay<T>>)
	FORCEINLINE TFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		using DecayedType = TDecay<T>;
		Impl::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
	}

	constexpr TFunction& operator=(nullptr_t) { Impl::ResetImpl(); return *this; }

	template <typename T> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CCopyConstructible<TDecay<T>>)
	FORCEINLINE TFunction& operator=(T&& InValue)
	{
		using DecayedType = TDecay<T>;

		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::ResetImpl();
		else Impl::template EmplaceImpl<DecayedType>(Forward<T>(InValue));

		return *this;
	}

	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CCopyConstructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(ArgTypes&&... Args)
	{
		using DecayedType = TDecay<T>;
		Impl::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
		return Impl::template Target<DecayedType>();
	}

	constexpr void Reset() { Impl::ResetImpl(); }

};

template <CFunction F>
class TUniqueFunction
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false>;

public:

	constexpr TUniqueFunction(nullptr_t = nullptr) { Impl::ResetImpl(); }

	FORCEINLINE TUniqueFunction(const TUniqueFunction& InValue) = delete;
	TUniqueFunction(TUniqueFunction&& InValue) : Impl(MoveTemp(InValue)) { InValue.ResetImpl(); }

	FORCEINLINE TUniqueFunction& operator=(const TUniqueFunction& InValue) = delete;
	FORCEINLINE TUniqueFunction& operator=(TUniqueFunction&& InValue)
	{
		if (&InValue == this) return *this;
		Impl::AssignImpl(MoveTemp(InValue));
		return *this;
	}

	FORCEINLINE TUniqueFunction(const TFunction<F>& InValue)
		: Impl(*reinterpret_cast<const TUniqueFunction*>(&InValue))
	{ }

	FORCEINLINE TUniqueFunction(TFunction<F>&& InValue)
		: Impl(MoveTemp(*reinterpret_cast<const TUniqueFunction*>(&InValue)))
	{
		InValue.Reset();
	}

	FORCEINLINE TUniqueFunction& operator=(const TFunction<F>& InValue)
	{
		Impl::AssignImpl(*reinterpret_cast<const TUniqueFunction*>(&InValue));
		return *this;
	}

	FORCEINLINE TUniqueFunction& operator=(TFunction<F>&& InValue)
	{
		Impl::AssignImpl(MoveTemp(*reinterpret_cast<TUniqueFunction*>(&InValue)));
		return *this;
	}

	template <typename T> requires (!CTInPlaceType<TDecay<T>>
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CMoveConstructible<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TUniqueFunction(T&& InValue)
	{
		using DecayedType = TDecay<T>;
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::ResetImpl();
		else Impl::template EmplaceImpl<DecayedType>(Forward<T>(InValue));
	}

	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CMoveConstructible<TDecay<T>>)
	FORCEINLINE TUniqueFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		using DecayedType = TDecay<T>;
		Impl::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
	}

	constexpr TUniqueFunction& operator=(nullptr_t) { Impl::ResetImpl(); return *this; }

	template <typename T> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CMoveConstructible<TDecay<T>>)
	FORCEINLINE TUniqueFunction& operator=(T&& InValue)
	{
		using DecayedType = TDecay<T>;

		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::ResetImpl();
		else Impl::template EmplaceImpl<DecayedType>(Forward<T>(InValue));

		return *this;
	}
	
	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CMoveConstructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(ArgTypes&&... Args)
	{
		using DecayedType = TDecay<T>;
		Impl::template EmplaceImpl<DecayedType>(Forward<ArgTypes>(Args)...);
		return Impl::template Target<DecayedType>();
	}

	constexpr void Reset() { Impl::ResetImpl(); }

};

template <CFunction F>
constexpr bool operator==(const TFunctionRef<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <CFunction F>
constexpr bool operator==(const TFunction<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <CFunction F>
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

	template <typename... Ts> requires (CInvocable<F&, Ts&&...>)
	constexpr auto operator()(Ts&&... Args) &
		-> decltype(!Invoke(Storage, Forward<Ts>(Args)...))
	{
		return !Invoke(Storage, Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<F&&, Ts&&...>)
	constexpr auto operator()(Ts&&... Args) &&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Ts>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<const F&, Ts&&...>)
	constexpr auto operator()(Ts&&... Args) const&
		-> decltype(!Invoke(Storage, Forward<Ts>(Args)...))
	{
		return !Invoke(Storage, Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<const F&&, Ts&&...>)
	constexpr auto operator()(Ts&&... Args) const&&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Ts>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Ts>(Args)...);
	}
};

NAMESPACE_PRIVATE_END

template <typename F> requires (CConstructibleFrom<F, F&&>)
constexpr NAMESPACE_PRIVATE::TNotFunction<TDecay<F>> NotFn(F&& Func)
{
	return NAMESPACE_PRIVATE::TNotFunction<TDecay<F>>(Forward<F>(Func));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
