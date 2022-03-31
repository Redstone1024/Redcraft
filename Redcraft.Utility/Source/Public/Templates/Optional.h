#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Concepts/Comparable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename OptionalType> requires TIsObject<OptionalType>::Value && (!TIsArray<OptionalType>::Value) && TIsDestructible<OptionalType>::Value
struct TOptional
{
public:

	using ValueType = OptionalType;

	constexpr TOptional() : bIsValid(false) { }

	constexpr TOptional(FInvalid) : TOptional() { }

	template <typename... Types> requires TIsConstructible<OptionalType, Types...>::Value
	constexpr explicit TOptional(FInPlace, Types&&... Args)
		: bIsValid(true)
	{
		new(&Value) OptionalType(Forward<Types>(Args)...);
	}

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, T&&>::Value
		&& (!TIsSame<typename TRemoveCVRef<T>::Type, FInPlace>::Value) && (!TIsSame<typename TRemoveCVRef<T>::Type, TOptional>::Value)
	constexpr explicit (!TIsConvertible<T&&, OptionalType>::Value) TOptional(T&& InValue)
		: TOptional(InPlace, Forward<T>(InValue))
	{ }
	
	constexpr TOptional(const TOptional& InValue) requires TIsCopyConstructible<OptionalType>::Value
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(InValue.GetValue());
	}

	constexpr TOptional(TOptional&& InValue) requires TIsMoveConstructible<OptionalType>::Value
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, const T&>::Value
	constexpr explicit (!TIsConvertible<const T&, OptionalType>::Value) TOptional(const TOptional<T>& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(InValue.GetValue());
	}

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, T&&>::Value
	constexpr explicit (!TIsConvertible<T&&, OptionalType>::Value) TOptional(TOptional<T>&& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new(&Value) OptionalType(MoveTemp(InValue.GetValue()));
	}

	constexpr ~TOptional()
	{
		if constexpr (!TIsTriviallyDestructible<OptionalType>::Value) Reset();
	}

	constexpr TOptional& operator=(const TOptional& InValue) requires TIsCopyConstructible<OptionalType>::Value && TIsCopyAssignable<OptionalType>::Value
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

	constexpr TOptional& operator=(TOptional&& InValue) requires TIsMoveConstructible<OptionalType>::Value && TIsMoveAssignable<OptionalType>::Value
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

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, const T&>::Value
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

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, T&&>::Value
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

	template <typename T = OptionalType> requires TIsConstructible<OptionalType, T&&>::Value
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

	template <typename... ArgsType>
	constexpr OptionalType& Emplace(ArgsType&&... Args)
	{
		Reset();

		OptionalType* Result = new(&Value) OptionalType(Forward<ArgsType>(Args)...);
		bIsValid = true;

		return *Result;
	}

	constexpr bool           IsValid() const { return bIsValid; }
	constexpr explicit operator bool() const { return bIsValid; }
	
	constexpr       void* GetData()       { return &Value; }
	constexpr const void* GetData() const { return &Value; }

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

private:

	TAlignedStorage<sizeof(OptionalType), alignof(OptionalType)>::Type Value;
	bool bIsValid;

};

template <typename T>
TOptional(T) ->TOptional<T>;

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator==(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != RHS.IsValid()) return false;
	if (LHS.IsValid() == false) return true;
	return *LHS == *RHS;
}

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator!=(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != RHS.IsValid()) return true;
	if (LHS.IsValid() == false) return false;
	return *LHS != *RHS;
}

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator==(const TOptional<T>& LHS, const U& RHS)
{
	return LHS.IsValid() ? *LHS == RHS : false;
}

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator!=(const TOptional<T>& LHS, const U& RHS)
{
	return LHS.IsValid() ? *LHS != RHS : true;
}

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator==(const T& LHS, const TOptional<U>& RHS)
{
	return RHS.IsValid() ? LHS == *RHS : false;
}

template <typename T, typename U> requires CWeaklyEqualityComparableWith<T, U>
constexpr bool operator!=(const T& LHS, const TOptional<U>& RHS)
{
	return RHS.IsValid() ? LHS != *RHS : true;
}

template <typename T>
constexpr bool operator==(const TOptional<T>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <typename T>
constexpr bool operator!=(const TOptional<T>& LHS, FInvalid)
{
	return LHS.IsValid();
}

template <typename T>
constexpr bool operator==(FInvalid, const TOptional<T>& RHS)
{
	return !RHS.IsValid();
}

template <typename T>
constexpr bool operator!=(FInvalid, const TOptional<T>& RHS)
{
	return RHS.IsValid();
}

template <typename T> requires (TIsObject<T>::Value && !TIsArray<T>::Value && TIsDestructible<T>::Value)
constexpr TOptional<typename TDecay<T>::Type> MakeOptional(FInvalid)
{
	return TOptional<typename TDecay<T>::Type>(Invalid);
}

template <typename T> requires (TIsObject<T>::Value && !TIsArray<T>::Value && TIsDestructible<T>::Value)
constexpr TOptional<typename TDecay<T>::Type> MakeOptional(T&& InValue)
{
	return TOptional<typename TDecay<T>::Type>(Forward<T>(InValue));
}

template <typename T, typename... Types> requires (TIsObject<T>::Value && !TIsArray<T>::Value && TIsDestructible<T>::Value)
constexpr TOptional<T> MakeOptional(Types&&... Args)
{
	return TOptional<T>(InPlace, Forward<T>(Args)...);
}

template <typename T> requires TIsMoveConstructible<T>::Value&& TIsSwappable<T>::Value
constexpr void Swap(TOptional<T>& A, TOptional<T>& B)
{
	if (!A && !B) return;

	if (A && !B)
	{
		B = MoveTemp(A);
		A.Reset();
		return;
	}

	if (B && !A)
	{
		A = MoveTemp(B);
		B.Reset();
		return;
	}

	Swap(*A, *B);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
