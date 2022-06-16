#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename OptionalType> requires CDestructible<OptionalType>
struct TOptional
{
private:
	
	template <typename T>
	struct TAllowUnwrapping : TBoolConstant<!(
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

	constexpr TOptional() : bIsValid(false) { }

	constexpr TOptional(FInvalid) : TOptional() { }

	template <typename... Types> requires CConstructibleFrom<OptionalType, Types...>
	constexpr explicit TOptional(FInPlace, Types&&... Args)
		: bIsValid(true)
	{
		new(&Value) OptionalType(Forward<Types>(Args)...);
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&>
		&& (!CSameAs<TRemoveCVRef<T>, FInPlace>) && (!CSameAs<TRemoveCVRef<T>, TOptional>)
	constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(T&& InValue)
		: TOptional(InPlace, Forward<T>(InValue))
	{ }
	
	constexpr TOptional(const TOptional& InValue) requires CCopyConstructible<OptionalType>
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(InValue.GetValue());
	}

	constexpr TOptional(TOptional&& InValue) requires CMoveConstructible<OptionalType>
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, const T&> && TAllowUnwrapping<T>::Value
	constexpr explicit (!CConvertibleTo<const T&, OptionalType>) TOptional(const TOptional<T>& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(InValue.GetValue());
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&> && TAllowUnwrapping<T>::Value
	constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(TOptional<T>&& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	constexpr ~TOptional()
	{
		if constexpr (!CTriviallyDestructible<OptionalType>) Reset();
	}

	constexpr TOptional& operator=(const TOptional& InValue) requires CCopyConstructible<OptionalType> && CCopyAssignable<OptionalType>
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = InValue.GetValue();
		else 
		{
			new(&Value) OptionalType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	constexpr TOptional& operator=(TOptional&& InValue) requires CMoveConstructible<OptionalType> && CMoveAssignable<OptionalType>
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = MoveTemp(InValue.GetValue());
		else
		{
			new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, const T&>
		&& CAssignableFrom<OptionalType&, const T&> && TAllowUnwrapping<T>::Value
	constexpr TOptional& operator=(const TOptional<T>& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = InValue.GetValue();
		else
		{
			new(&Value) OptionalType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&>
		&& CAssignableFrom<OptionalType&, T&&> && TAllowUnwrapping<T>::Value
	constexpr TOptional& operator=(TOptional<T>&& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = MoveTemp(InValue.GetValue());
		else
		{
			new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires CConstructibleFrom<OptionalType, T&&> && CAssignableFrom<OptionalType&, T&&>
	constexpr TOptional& operator=(T&& InValue)
	{
		if (IsValid()) GetValue() = Forward<T>(InValue);
		else
		{
			new(&Value) OptionalType(Forward<T>(InValue));
			bIsValid = true;
		}

		return *this;
	}

	template <typename... ArgTypes> requires CConstructibleFrom<OptionalType, ArgTypes...>
	constexpr OptionalType& Emplace(ArgTypes&&... Args)
	{
		Reset();

		OptionalType* Result = new(&Value) OptionalType(Forward<ArgTypes>(Args)...);
		bIsValid = true;

		return *Result;
	}

	constexpr bool           IsValid() const { return bIsValid; }
	constexpr explicit operator bool() const { return bIsValid; }
	
	constexpr       OptionalType&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      OptionalType*>(&Value);   }
	constexpr       OptionalType&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      OptionalType*>(&Value));  }
	constexpr const OptionalType&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const OptionalType*>(&Value);   }
	constexpr const OptionalType&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const OptionalType*>(&Value));  }

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
		if (bIsValid)
		{
			bIsValid = false;

			typedef OptionalType DestructOptionalType;
			((OptionalType*)&Value)->DestructOptionalType::~DestructOptionalType();
		}
	}

	constexpr size_t GetTypeHash() const requires CHashable<OptionalType>
	{
		if (!IsValid()) return 2824517378;
		return NAMESPACE_REDCRAFT::GetTypeHash(GetValue());
	}

	template <typename T> requires CMoveConstructible<OptionalType> && CSwappable<OptionalType>
	constexpr void Swap(TOptional& InValue)
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

		NAMESPACE_REDCRAFT::Swap(GetValue(), InValue.GetValue());
	}

private:

	TAlignedStorage<sizeof(OptionalType), alignof(OptionalType)> Value;
	bool bIsValid;

};

template <typename T>
TOptional(T) -> TOptional<T>;

template <typename T, typename U> requires CWeaklyEqualityComparable<T, U>
constexpr bool operator==(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != RHS.IsValid()) return false;
	if (LHS.IsValid() == false) return true;
	return *LHS == *RHS;
}

template <typename T, typename U> requires CSynthThreeWayComparable<T, U>
constexpr partial_ordering operator<=>(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != RHS.IsValid()) return partial_ordering::unordered;
	if (LHS.IsValid() == false) return partial_ordering::equivalent;
	return SynthThreeWayCompare(*LHS, *RHS);
}

template <typename T, typename U> requires CWeaklyEqualityComparable<T, U>
constexpr bool operator==(const TOptional<T>& LHS, const U& RHS)
{
	return LHS.IsValid() ? *LHS == RHS : false;
}

template <typename T>
constexpr bool operator==(const TOptional<T>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <typename T> requires CDestructible<T>
constexpr TOptional<TDecay<T>> MakeOptional(FInvalid)
{
	return TOptional<TDecay<T>>(Invalid);
}

template <typename T> requires CDestructible<T> && CConstructibleFrom<T, T&&>
constexpr TOptional<T> MakeOptional(T&& InValue)
{
	return TOptional<T>(Forward<T>(InValue));
}

template <typename T, typename... Types> requires CDestructible<T> && CConstructibleFrom<T, Types...>
constexpr TOptional<T> MakeOptional(Types&&... Args)
{
	return TOptional<T>(InPlace, Forward<T>(Args)...);
}

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTOptional               : FFalse { };
template <typename T> struct TIsTOptional<TOptional<T>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTOptional = NAMESPACE_PRIVATE::TIsTOptional<T>::Value;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
