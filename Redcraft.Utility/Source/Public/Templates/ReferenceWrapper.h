#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/Optional.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename ReferencedType> requires (CObject<ReferencedType> || CFunction<ReferencedType>)
class TReferenceWrapper
{
public:

	using Type = ReferencedType;

	template <typename T = ReferencedType> requires (CConvertibleTo<T, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(T&& Object)
	{
		ReferencedType& Reference = Forward<T>(Object);
		Pointer = AddressOf(Reference);
	}

	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper&) = default;
	FORCEINLINE constexpr TReferenceWrapper(TReferenceWrapper&&)      = default;
	
	template <typename T = ReferencedType> requires (CConvertibleTo<T&, ReferencedType&>)
	FORCEINLINE constexpr TReferenceWrapper(const TReferenceWrapper<T>& InValue)
		: Pointer(InValue.Pointer)
	{ }

	template <typename T = ReferencedType> requires (CAssignableFrom<ReferencedType&, T&&>)
	FORCEINLINE constexpr TReferenceWrapper& operator=(T&& Object) { Get() = Forward<T>(Object); return *this; }

	FORCEINLINE constexpr TReferenceWrapper& operator=(const TReferenceWrapper&) = delete;
	FORCEINLINE constexpr TReferenceWrapper& operator=(TReferenceWrapper&&)      = delete;
	
	FORCEINLINE constexpr operator ReferencedType&() const { return *Pointer; }
	FORCEINLINE constexpr ReferencedType& Get()      const { return *Pointer; }

	template <typename... Ts>
	FORCEINLINE constexpr TInvokeResult<ReferencedType&, Ts...> operator()(Ts&&... Args) const
	{
		return Invoke(Get(), Forward<Ts>(Args)...);
	}

	FORCEINLINE constexpr size_t GetTypeHash() const requires (CHashable<ReferencedType>)
	{
		return NAMESPACE_REDCRAFT::GetTypeHash(Get());
	}
	
	FORCEINLINE constexpr void Swap(TReferenceWrapper& InValue)
	{
		ReferencedType* Temp = Pointer;
		Pointer = InValue.Pointer;
		InValue.Pointer = Temp;
	}

private:

	ReferencedType* Pointer;

	template <typename T> requires (CObject<T> || CFunction<T>) friend class TReferenceWrapper;

	// Optimize TOptional with these hacking
	FORCEINLINE constexpr TReferenceWrapper(FInvalid) : Pointer(nullptr) { };
	template <typename T> requires (CDestructible<T>) friend class TOptional;

};

template <typename T>
TReferenceWrapper(T&) -> TReferenceWrapper<T>;

template <typename T>
void Ref(const T&&) = delete;

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<T> Ref(T& InValue)
{
	return TReferenceWrapper<T>(InValue);
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<const T> Ref(const T& InValue)
{
	return TReferenceWrapper<const T>(InValue);
}

template <typename T>
FORCEINLINE constexpr TReferenceWrapper<const T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTReferenceWrapperImpl                       : FFalse { };
template <typename T> struct TIsTReferenceWrapperImpl<TReferenceWrapper<T>> : FTrue  { };

template <typename T> struct TUnwrapReferenceImpl                       { using Type = T;  };
template <typename T> struct TUnwrapReferenceImpl<TReferenceWrapper<T>> { using Type = T&; };

template <typename T> struct TUnwrapRefDecayImpl { using Type = typename TUnwrapReferenceImpl<TDecay<T>>::Type; };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTReferenceWrapper = NAMESPACE_PRIVATE::TIsTReferenceWrapperImpl<TRemoveCV<T>>::Value;

template <typename T>
using TUnwrapReference = typename NAMESPACE_PRIVATE::TUnwrapReferenceImpl<T>::Type;

template <typename T>
using TUnwrapRefDecay = typename NAMESPACE_PRIVATE::TUnwrapRefDecayImpl<T>::Type;

template <typename ReferencedType>
class TOptional<TReferenceWrapper<ReferencedType>>
{
private:
	
	using OptionalType = TReferenceWrapper<ReferencedType>;

	template <typename T>
	struct TAllowUnwrapping : TBoolConstant < !(
		   CConstructibleFrom<OptionalType,       TOptional<T>& >
		|| CConstructibleFrom<OptionalType, const TOptional<T>& >
		|| CConstructibleFrom<OptionalType,       TOptional<T>&&>
		|| CConstructibleFrom<OptionalType, const TOptional<T>&&>
		|| CConvertibleTo<      TOptional<T>&,  OptionalType>
		|| CConvertibleTo<const TOptional<T>&,  OptionalType>
		|| CConvertibleTo<      TOptional<T>&&, OptionalType>
		|| CConvertibleTo<const TOptional<T>&&, OptionalType>
		|| CAssignableFrom<OptionalType&,       TOptional<T>& >
		|| CAssignableFrom<OptionalType&, const TOptional<T>& >
		|| CAssignableFrom<OptionalType&,       TOptional<T>&&>
		|| CAssignableFrom<OptionalType&, const TOptional<T>&&>
	)> { };

public:

	using ValueType = OptionalType;

	FORCEINLINE constexpr TOptional() : Reference(Invalid) { }

	FORCEINLINE constexpr TOptional(FInvalid) : TOptional() { }

	template <typename... Ts> requires (CConstructibleFrom<OptionalType, Ts...>)
	FORCEINLINE constexpr explicit TOptional(FInPlace, Ts&&... Args)
		: Reference(Forward<Ts>(Args)...)
	{ }

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&>
		&& !CSameAs<TRemoveCVRef<T>, FInPlace> && !CBaseOf<TOptional, TRemoveCVRef<T>>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(T&& InValue)
		: TOptional(InPlace, Forward<T>(InValue))
	{ }
	
	FORCEINLINE TOptional(const TOptional& InValue) = default;
	FORCEINLINE TOptional(TOptional&& InValue) = default;

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, const T&> && TAllowUnwrapping<T>::Value)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const T&, OptionalType>) TOptional(const TOptional<T>& InValue)
		: Reference(InValue.Reference)
	{ }

	FORCEINLINE ~TOptional() = default;

	FORCEINLINE TOptional& operator=(const TOptional& InValue) = default;
	FORCEINLINE TOptional& operator=(TOptional&& InValue) = default;

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, const T&>
		&& CAssignableFrom<OptionalType&, const T&> && TAllowUnwrapping<T>::Value)
	FORCEINLINE constexpr TOptional& operator=(const TOptional<T>& InValue)
	{
		Reference = InValue.Reference;
		return *this;
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&> && CAssignableFrom<OptionalType&, T&&>)
	FORCEINLINE constexpr TOptional& operator=(T&& InValue)
	{
		Reference = InValue;
		return *this;
	}

	template <typename... ArgTypes> requires (CConstructibleFrom<OptionalType, ArgTypes...>)
	FORCEINLINE constexpr OptionalType& Emplace(ArgTypes&&... Args)
	{
		Reference = TReferenceWrapper<ReferencedType>(Forward<ArgTypes>(Args)...);
		return Reference;
	}

	FORCEINLINE constexpr bool           IsValid() const { return Reference.Pointer != nullptr; }
	FORCEINLINE constexpr explicit operator bool() const { return Reference.Pointer != nullptr; }
	
	FORCEINLINE constexpr       OptionalType&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	FORCEINLINE constexpr       OptionalType&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	FORCEINLINE constexpr const OptionalType&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	FORCEINLINE constexpr const OptionalType&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }

	FORCEINLINE constexpr const OptionalType* operator->() const { return &GetValue(); }
	FORCEINLINE constexpr       OptionalType* operator->()       { return &GetValue(); }

	FORCEINLINE constexpr       OptionalType&  operator*() &       { return GetValue(); }
	FORCEINLINE constexpr       OptionalType&& operator*() &&      { return GetValue(); }
	FORCEINLINE constexpr const OptionalType&  operator*() const&  { return GetValue(); }
	FORCEINLINE constexpr const OptionalType&& operator*() const&& { return GetValue(); }

	FORCEINLINE constexpr       OptionalType& Get(      OptionalType& DefaultValue) &      { return IsValid() ? GetValue() : DefaultValue;  }
	FORCEINLINE constexpr const OptionalType& Get(const OptionalType& DefaultValue) const& { return IsValid() ? GetValue() : DefaultValue;  }

	FORCEINLINE constexpr void Reset()
	{
		Reference = Invalid;
	}

	FORCEINLINE constexpr size_t GetTypeHash() const requires (CHashable<ReferencedType>)
	{
		if (!IsValid()) return 2824517378;
		return Reference.GetTypeHash();
	}

	FORCEINLINE constexpr void Swap(TOptional& InValue)
	{
		Reference.Swap(InValue.Reference);
	}

private:

	TReferenceWrapper<ReferencedType> Reference;
	template <typename T> requires (CDestructible<T>) friend class TOptional;

};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
