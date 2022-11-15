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

	template <typename T = ReferencedType> requires CConvertibleTo<T, ReferencedType&>
	constexpr TReferenceWrapper(T&& Object) : Pointer(AddressOf(Forward<T>(Object))) { }

	TReferenceWrapper(const TReferenceWrapper&) = default;
	
	template <typename T = ReferencedType> requires CConvertibleTo<T&, ReferencedType&>
	constexpr TReferenceWrapper(const TReferenceWrapper<T>& InValue)
		: Pointer(InValue.Pointer)
	{ }

	TReferenceWrapper& operator=(const TReferenceWrapper&) = default;
	
	template <typename T = ReferencedType> requires CConvertibleTo<T&, ReferencedType&>
	constexpr TReferenceWrapper& operator=(const TReferenceWrapper<T>& InValue)
	{
		Pointer = InValue.Pointer;
		return *this;
	}
	
	constexpr operator ReferencedType&() const { return *Pointer; }
	constexpr ReferencedType& Get()      const { return *Pointer; }

	template <typename... Types>
	constexpr TInvokeResult<ReferencedType&, Types...> operator()(Types&&... Args) const
	{
		return Invoke(Get(), Forward<Types>(Args)...);
	}

	constexpr size_t GetTypeHash() const requires CHashable<ReferencedType>
	{
		return NAMESPACE_REDCRAFT::GetTypeHash(Get());
	}
	
	constexpr void Swap(TReferenceWrapper& InValue)
	{
		ReferencedType* Temp = Pointer;
		Pointer = InValue.Pointer;
		InValue.Pointer = Temp;
	}

private:

	ReferencedType* Pointer;

	template <typename T> requires (CObject<T> || CFunction<T>) friend class TReferenceWrapper;

	// Optimize TOptional with these hacking
	constexpr TReferenceWrapper(FInvalid) : Pointer(nullptr) { };
	template <typename T> requires CDestructible<T> friend class TOptional;

};

template <typename T>
TReferenceWrapper(T&) -> TReferenceWrapper<T>;

template <typename T>
void Ref(const T&&) = delete;

template <typename T>
constexpr TReferenceWrapper<T> Ref(T& InValue)
{
	return TReferenceWrapper<T>(InValue);
}

template <typename T>
constexpr TReferenceWrapper<T> Ref(TReferenceWrapper<T> InValue)
{
	return Ref(InValue.Get());
}

template <typename T>
constexpr TReferenceWrapper<const T> Ref(const T& InValue)
{
	return TReferenceWrapper<const T>(InValue);
}

template <typename T>
constexpr TReferenceWrapper<const T> Ref(TReferenceWrapper<T> InValue)
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
concept CTReferenceWrapper = NAMESPACE_PRIVATE::TIsTReferenceWrapperImpl<T>::Value;

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

	constexpr TOptional() : Reference(Invalid) { }

	constexpr TOptional(FInvalid) : TOptional() { }

	template <typename... Types> requires CConstructibleFrom<OptionalType, Types...>
	constexpr explicit TOptional(FInPlace, Types&&... Args)
		: Reference(Forward<Types>(Args)...)
	{ }

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&>
		&& (!CSameAs<TRemoveCVRef<T>, FInPlace>) && (!CSameAs<TRemoveCVRef<T>, TOptional>)
	constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(T&& InValue)
		: TOptional(InPlace, Forward<T>(InValue))
	{ }
	
	TOptional(const TOptional& InValue) = default;
	TOptional(TOptional&& InValue) = default;

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, const T&> && TAllowUnwrapping<T>::Value
	constexpr explicit (!CConvertibleTo<const T&, OptionalType>) TOptional(const TOptional<T>& InValue)
		: Reference(InValue.Reference)
	{ }

	~TOptional() = default;

	TOptional& operator=(const TOptional& InValue) = default;
	TOptional& operator=(TOptional&& InValue) = default;

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, const T&>
		&& CAssignableFrom<OptionalType&, const T&> && TAllowUnwrapping<T>::Value
	constexpr TOptional& operator=(const TOptional<T>& InValue)
	{
		Reference = InValue.Reference;
		return *this;
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&> && CAssignableFrom<OptionalType&, T&&>
	constexpr TOptional& operator=(T&& InValue)
	{
		Reference = InValue;
		return *this;
	}

	template <typename... ArgTypes> requires CConstructibleFrom<OptionalType, ArgTypes...>
	constexpr OptionalType& Emplace(ArgTypes&&... Args)
	{
		Reference = TReferenceWrapper<ReferencedType>(Forward<ArgTypes>(Args)...);
		return Reference;
	}

	constexpr bool           IsValid() const { return Reference.Pointer != nullptr; }
	constexpr explicit operator bool() const { return Reference.Pointer != nullptr; }
	
	constexpr       OptionalType&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	constexpr       OptionalType&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	constexpr const OptionalType&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }
	constexpr const OptionalType&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return Reference; }

	constexpr const OptionalType* operator->() const { return &GetValue(); }
	constexpr       OptionalType* operator->()       { return &GetValue(); }

	constexpr       OptionalType&  operator*() &       { return GetValue(); }
	constexpr       OptionalType&& operator*() &&      { return GetValue(); }
	constexpr const OptionalType&  operator*() const&  { return GetValue(); }
	constexpr const OptionalType&& operator*() const&& { return GetValue(); }

	constexpr       OptionalType& Get(      OptionalType& DefaultValue) &      { return IsValid() ? GetValue() : DefaultValue;  }
	constexpr const OptionalType& Get(const OptionalType& DefaultValue) const& { return IsValid() ? GetValue() : DefaultValue;  }

	constexpr void Reset()
	{
		Reference = Invalid;
	}

	constexpr size_t GetTypeHash() const requires CHashable<ReferencedType>
	{
		if (!IsValid()) return 2824517378;
		return Reference.GetTypeHash();
	}

	constexpr void Swap(TOptional& InValue)
	{
		Reference.Swap(InValue.Reference);
	}

private:

	TReferenceWrapper<ReferencedType> Reference;
	template <typename T> requires CDestructible<T> friend class TOptional;

};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
