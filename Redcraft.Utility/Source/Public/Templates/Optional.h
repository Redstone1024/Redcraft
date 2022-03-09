#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
struct TOptional
{
public:

	using Type = T;

	constexpr TOptional() : bIsValid(false) { }

	template <typename... Types> requires TIsConstructible<T, Types...>::Value
	constexpr explicit TOptional(EInPlace, Types&&... Args)
		: bIsValid(true)
	{
		new(&Value) T(Forward<Types>(Args)...);
	}

	template <typename U = T> requires TIsConstructible<T, U&&>::Value && !TIsSame<typename TRemoveCVRef<U>::Type, EInPlace>::Value && !TIsSame<typename TRemoveCVRef<U>::Type, TOptional>::Value
	constexpr explicit(!TIsConvertible<U&&, T>::Value) TOptional(U&& InValue)
		: TOptional(InPlace, Forward<U>(InValue))
	{ }

	template <typename U = T> requires TIsConstructible<T, const U&>::Value
	constexpr explicit(!TIsConvertible<const U&, T>::Value) TOptional(const TOptional<U>& InValue)
		: bIsValid(InValue.bIsValid)
	{
		if (InValue.bIsValid) new(&Value) T(InValue.GetValue());
	}

	template <typename U = T> requires TIsConstructible<T, U&&>::Value
	constexpr explicit(!TIsConvertible<U&&, T>::Value) TOptional(TOptional<U>&& InValue)
		: bIsValid(InValue.bIsValid)
	{
		if (InValue.bIsValid) new(&Value) T(MoveTempIfPossible(InValue).GetValue());
	}

	constexpr ~TOptional()
	{
		Reset();
	}

	template <typename U = T> requires TIsConstructible<T, const U&>::Value
	constexpr TOptional& operator=(const TOptional<U>& InValue)
	{
		if (InValue == this) return *this;

		Reset();

		if (InValue.bIsValid)
		{
			new(&Value) T(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	template <typename U = T> requires TIsConstructible<T, U&&>::Value
	constexpr TOptional& operator=(TOptional<U>&& InValue)
	{
		if (InValue == this) return *this;

		Reset();

		if (InValue.bIsValid)
		{
			new(&Value) T(MoveTempIfPossible(InValue).GetValue());
			bIsValid = true;
		}

		return *this;
	}

	template <typename U = T> requires TIsConstructible<T, U&&>::Value
	constexpr TOptional& operator=(U&& InValue)
	{
		Reset();

		new(&Value) T(MoveTempIfPossible(InValue));
		bIsValid = true;

		return *this;
	}


	template <typename... ArgsType>
	constexpr T& Emplace(ArgsType&&... Args)
	{
		Reset();

		T* Result = new(&Value) T(Forward<ArgsType>(Args)...);
		bIsValid = true;

		return *Result;
	}

	constexpr bool           IsValid() const { return bIsValid; }
	constexpr explicit operator bool() const { return bIsValid; }

	constexpr       T&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsSet() or use Get(DefaultValue) instead.")); return *(T*)&Value; }
	constexpr       T&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsSet() or use Get(DefaultValue) instead.")); return *(T*)&Value; }
	constexpr const T&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsSet() or use Get(DefaultValue) instead.")); return *(T*)&Value; }
	constexpr const T&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsSet() or use Get(DefaultValue) instead.")); return *(T*)&Value; }

	constexpr const T* operator->() const { return &GetValue(); }
	constexpr       T* operator->()       { return &GetValue(); }

	constexpr       T&  operator*() &       { return GetValue(); }
	constexpr       T&& operator*() &&      { return GetValue(); }
	constexpr const T&  operator*() const&  { return GetValue(); }
	constexpr const T&& operator*() const&& { return GetValue(); }

	template <typename U = T>
	constexpr T Get(U&& DefaultValue) &&     { return IsValid() ? GetValue() : DefaultValue; }

	template <typename U = T>
	constexpr T Get(U&& DefaultValue) const& { return IsValid() ? GetValue() : DefaultValue; }

	constexpr void Reset()
	{
		if (bIsValid)
		{
			bIsValid = false;

			typedef T DestructOptionalType;
			((T*)&Value)->DestructOptionalType::~DestructOptionalType();
		}
	}

private:

	TAlignedStorage<sizeof(T), alignof(T)>::Type Value;
	bool bIsValid;

};

template <typename T>
TOptional(T) ->TOptional<T>;

template <typename T, typename U>
constexpr bool operator==(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != LHS.IsValid()) return false;
	if (LHS.IsValid() == false) return true;
	return *LHS == *RHS;
}

template <typename T, typename U>
constexpr bool operator!=(const TOptional<T>& LHS, const TOptional<U>& RHS)
{
	if (LHS.IsValid() != LHS.IsValid()) return true;
	if (LHS.IsValid() == false) return false;
	return *LHS != *RHS;
}

//template <typename T, typename U>
//constexpr bool operator<(const TOptional<T>&, const TOptional<U>&);
//template <typename T, typename U>
//constexpr bool operator>(const TOptional<T>&, const TOptional<U>&);
//template <typename T, typename U>
//constexpr bool operator<=(const TOptional<T>&, const TOptional<U>&);
//template <typename T, typename U>
//constexpr bool operator>=(const TOptional<T>&, const TOptional<U>&);

//template <typename T, typename U> constexpr bool operator==(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator==(const T&, const TOptional<U>&);
//template <typename T, typename U> constexpr bool operator!=(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator!=(const T&, const TOptional<U>&);
//template <typename T, typename U> constexpr bool operator<(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator<(const T&, const TOptional<U>&);
//template <typename T, typename U> constexpr bool operator>(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator>(const T&, const TOptional<U>&);
//template <typename T, typename U> constexpr bool operator<=(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator<=(const T&, const TOptional<U>&);
//template <typename T, typename U> constexpr bool operator>=(const TOptional<T>&, const U&);
//template <typename T, typename U> constexpr bool operator>=(const T&, const TOptional<U>&);

template <typename T>
constexpr TOptional<typename TDecay<T>::Type> MakeOptional(T&& InValue)
{
	return TOptional<typename TDecay<T>::Type>(Forward<T>(InValue));
}

template <typename T, typename... Types>
constexpr TOptional<T> MakeOptional(Types&&... Args)
{
	return TOptional<T>(InPlace, Forward<T>(Args)...);
}

template <typename T>
constexpr void Swap(TOptional<T>& A, TOptional<T>& B)
{
	if (!A && !B) return;

	if (A && !B)
	{
		B = A;
		A.Reset();
		return;
	}

	if (B && !A)
	{
		A = B;
		B.Reset();
		return;
	}

	Swap(*A, *B);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
