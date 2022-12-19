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

template <typename OptionalType> requires (CDestructible<OptionalType>)
class TOptional;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTOptional               : FFalse { };
template <typename T> struct TIsTOptional<TOptional<T>> : FTrue  { };

template <typename T, typename U>
concept CTOptionalAllowUnwrappable =
	 !(CConstructibleFrom<U,       TOptional<T>& >
	|| CConstructibleFrom<U, const TOptional<T>& >
	|| CConstructibleFrom<U,       TOptional<T>&&>
	|| CConstructibleFrom<U, const TOptional<T>&&>
	|| CConvertibleTo<      TOptional<T>&,  U>
	|| CConvertibleTo<const TOptional<T>&,  U>
	|| CConvertibleTo<      TOptional<T>&&, U>
	|| CConvertibleTo<const TOptional<T>&&, U>
	|| CAssignableFrom<U&,       TOptional<T>& >
	|| CAssignableFrom<U&, const TOptional<T>& >
	|| CAssignableFrom<U&,       TOptional<T>&&>
	|| CAssignableFrom<U&, const TOptional<T>&&>);

NAMESPACE_PRIVATE_END

template <typename T> concept CTOptional = NAMESPACE_PRIVATE::TIsTOptional<TRemoveCV<T>>::Value;

template <typename OptionalType> requires (CDestructible<OptionalType>)
class TOptional
{
public:

	using ValueType = OptionalType;

	FORCEINLINE constexpr TOptional() : bIsValid(false) { }

	FORCEINLINE constexpr TOptional(FInvalid) : TOptional() { }

	template <typename... Ts> requires (CConstructibleFrom<OptionalType, Ts...>)
	FORCEINLINE constexpr explicit TOptional(FInPlace, Ts&&... Args)
		: bIsValid(true)
	{
		new (&Value) OptionalType(Forward<Ts>(Args)...);
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&>)
		&& (!CSameAs<TRemoveCVRef<T>, FInPlace>) && (!CSameAs<TOptional, TRemoveCVRef<T>>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(T&& InValue)
		: TOptional(InPlace, Forward<T>(InValue))
	{ }
	
	FORCEINLINE constexpr TOptional(const TOptional& InValue) requires (CTriviallyCopyConstructible<OptionalType>) = default;

	FORCEINLINE constexpr TOptional(const TOptional& InValue) requires (CCopyConstructible<OptionalType> && !CTriviallyCopyConstructible<OptionalType>)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) OptionalType(InValue.GetValue());
	}

	FORCEINLINE constexpr TOptional(TOptional&& InValue) requires (CTriviallyMoveConstructible<OptionalType>) = default;

	FORCEINLINE constexpr TOptional(TOptional&& InValue) requires (CMoveConstructible<OptionalType> && !CTriviallyMoveConstructible<OptionalType>)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, const T&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<T, OptionalType>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const T&, OptionalType>) TOptional(const TOptional<T>& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) OptionalType(InValue.GetValue());
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<T, OptionalType>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<T&&, OptionalType>) TOptional(TOptional<T>&& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	FORCEINLINE constexpr ~TOptional() requires (CTriviallyDestructible<OptionalType>) = default;

	FORCEINLINE constexpr ~TOptional() requires (!CTriviallyDestructible<OptionalType>)
	{
		Reset();
	}

	FORCEINLINE constexpr TOptional& operator=(const TOptional& InValue) requires (CTriviallyCopyConstructible<OptionalType> && CTriviallyCopyAssignable<OptionalType>) = default;

	constexpr TOptional& operator=(const TOptional& InValue) requires (CCopyConstructible<OptionalType> && CCopyAssignable<OptionalType>
		&& !CTriviallyCopyConstructible<OptionalType> && !CTriviallyCopyAssignable<OptionalType>)
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
			new (&Value) OptionalType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	FORCEINLINE constexpr TOptional& operator=(TOptional&& InValue) requires (CTriviallyMoveConstructible<OptionalType> && CTriviallyMoveAssignable<OptionalType>) = default;

	constexpr TOptional& operator=(TOptional&& InValue) requires (CMoveConstructible<OptionalType> && CMoveAssignable<OptionalType>
		&& !CTriviallyMoveConstructible<OptionalType> && !CTriviallyMoveAssignable<OptionalType>)
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
			new (&Value) OptionalType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, const T&>
		&& CAssignableFrom<OptionalType&, const T&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<T, OptionalType>)
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
			new (&Value) OptionalType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&>
		&& CAssignableFrom<OptionalType&, T&&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<T, OptionalType>)
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
			new (&Value) OptionalType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	template <typename T = OptionalType> requires (CConstructibleFrom<OptionalType, T&&> && CAssignableFrom<OptionalType&, T&&>)
	FORCEINLINE constexpr TOptional& operator=(T&& InValue)
	{
		if (IsValid()) GetValue() = Forward<T>(InValue);
		else
		{
			new (&Value) OptionalType(Forward<T>(InValue));
			bIsValid = true;
		}

		return *this;
	}
	
	template <typename T = OptionalType> requires (CWeaklyEqualityComparable<OptionalType, T>)
	friend FORCEINLINE constexpr bool operator==(const TOptional& LHS, const TOptional<T>& RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return false;
		if (LHS.IsValid() == false) return true;
		return *LHS == *RHS;
	}

	template <typename T = OptionalType> requires (CSynthThreeWayComparable<OptionalType, T>)
	friend FORCEINLINE constexpr partial_ordering operator<=>(const TOptional& LHS, const TOptional<T>& RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return SynthThreeWayCompare(*LHS, *RHS);
	}

	template <typename T = OptionalType> requires (!CTOptional<T>&& CWeaklyEqualityComparable<OptionalType, T>)
	FORCEINLINE constexpr bool operator==(const T& InValue) const&
	{
		return IsValid() ? GetValue() == InValue : false;
	}
	
	template <typename T = OptionalType> requires (!CTOptional<T>&& CSynthThreeWayComparable<OptionalType, T>)
	FORCEINLINE constexpr partial_ordering operator<=>(const T& InValue) const&
	{
		return IsValid() ? SynthThreeWayCompare(GetValue(), InValue) : partial_ordering::unordered;
	}

	FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	template <typename... ArgTypes> requires (CConstructibleFrom<OptionalType, ArgTypes...>)
	FORCEINLINE constexpr OptionalType& Emplace(ArgTypes&&... Args)
	{
		Reset();

		OptionalType* Result = new (&Value) OptionalType(Forward<ArgTypes>(Args)...);
		bIsValid = true;

		return *Result;
	}

	FORCEINLINE constexpr bool           IsValid() const { return bIsValid; }
	FORCEINLINE constexpr explicit operator bool() const { return bIsValid; }
	
	FORCEINLINE constexpr       OptionalType&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      OptionalType*>(&Value);   }
	FORCEINLINE constexpr       OptionalType&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      OptionalType*>(&Value));  }
	FORCEINLINE constexpr const OptionalType&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const OptionalType*>(&Value);   }
	FORCEINLINE constexpr const OptionalType&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const OptionalType*>(&Value));  }

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
		if (bIsValid)
		{
			bIsValid = false;

			typedef OptionalType DestructOptionalType;
			((OptionalType*)&Value)->DestructOptionalType::~DestructOptionalType();
		}
	}

	friend FORCEINLINE constexpr size_t GetTypeHash(const TOptional& A) requires (CHashable<OptionalType>)
	{
		if (!A.IsValid()) return 2824517378;
		return GetTypeHash(A.GetValue());
	}

	template <typename T> requires (CMoveConstructible<OptionalType> && CSwappable<OptionalType>)
	friend constexpr void Swap(TOptional& A, TOptional& B)
	{
		if (!A.IsValid() && !B.IsValid()) return;

		if (A.IsValid() && !B.IsValid())
		{
			B = MoveTemp(A);
			A.Reset();
		}
		else if (!A.IsValid() && B.IsValid())
		{
			A = MoveTemp(B);
			B.Reset();
		}
		else
		{
			Swap(A.GetValue(), B.GetValue());
		}
	}

private:

	TAlignedStorage<sizeof(OptionalType), alignof(OptionalType)> Value;
	bool bIsValid;

};

template <typename T>
TOptional(T) -> TOptional<T>;

template <typename T> requires (CDestructible<T>)
FORCEINLINE constexpr TOptional<TDecay<T>> MakeOptional(FInvalid)
{
	return TOptional<TDecay<T>>(Invalid);
}

template <typename T> requires (CDestructible<T> && CConstructibleFrom<T, T&&>)
FORCEINLINE constexpr TOptional<T> MakeOptional(T&& InValue)
{
	return TOptional<T>(Forward<T>(InValue));
}

template <typename T, typename... Ts> requires (CDestructible<T> && CConstructibleFrom<T, Ts...>)
FORCEINLINE constexpr TOptional<T> MakeOptional(Ts&&... Args)
{
	return TOptional<T>(InPlace, Forward<T>(Args)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
