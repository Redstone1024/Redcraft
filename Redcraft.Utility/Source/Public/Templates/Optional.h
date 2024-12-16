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

/** The class template manages an optional contained value or reference, i.e. a value or reference that may or may not be present. */
template <typename OptionalType, bool = CReference<OptionalType>> requires (CDestructible<TRemoveReference<OptionalType>>)
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

/** The class template manages an optional contained value, i.e. a value that may or may not be present. */
template <typename T> requires (!CReference<T>)
class TOptional<T, false> final
{
public:

	using FValueType = T;

	static_assert(!CReference<FValueType>);

	/** Constructs an object that does not contain a value. */
	FORCEINLINE constexpr TOptional() : bIsValid(false) { }

	/** Constructs an object that does not contain a value. */
	FORCEINLINE constexpr TOptional(FInvalid) : TOptional() { }

	/** Constructs an object with initial content an object, direct-initialized from Forward<U>(InValue). */
	template <typename U = T> requires (CConstructibleFrom<T, U&&>)
		&& (!CSameAs<TRemoveCVRef<U>, FInPlace>) && (!CSameAs<TOptional, TRemoveCVRef<U>>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&&, T>) TOptional(U&& InValue)
		: TOptional(InPlace, Forward<U>(InValue))
	{ }

	/** Constructs an object with initial content an object, direct-non-list-initialized from Forward<Ts>(Args).... */
	template <typename... Ts> requires (CConstructibleFrom<T, Ts...>)
	FORCEINLINE constexpr explicit TOptional(FInPlace, Ts&&... Args)
		: bIsValid(true)
	{
		new (&Value) FValueType(Forward<Ts>(Args)...);
	}

	/** Constructs an object with initial content an object, direct-non-list-initialized from IL, Forward<Ts>(Args).... */
	template <typename W, typename... Ts> requires (CConstructibleFrom<T, initializer_list<W>, Ts...>)
	FORCEINLINE constexpr explicit TOptional(FInPlace, initializer_list<W> IL, Ts&&... Args)
		: bIsValid(true)
	{
		new (&Value) FValueType(IL, Forward<Ts>(Args)...);
	}

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TOptional(const TOptional& InValue) requires (CTriviallyCopyConstructible<T>) = default;

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TOptional(const TOptional& InValue) requires (CCopyConstructible<T> && !CTriviallyCopyConstructible<T>)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) FValueType(InValue.GetValue());
	}

	/** Moves content of other into a new instance. */
	FORCEINLINE constexpr TOptional(TOptional&& InValue) requires (CTriviallyMoveConstructible<T>) = default;

	/** Moves content of other into a new instance. */
	FORCEINLINE constexpr TOptional(TOptional&& InValue) requires (CMoveConstructible<T> && !CTriviallyMoveConstructible<T>)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) FValueType(MoveTemp(InValue.GetValue()));
	}

	/** Converting copy constructor. */
	template <typename U> requires (CConstructibleFrom<T, const U&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const U&, T>) TOptional(const TOptional<U>& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) FValueType(InValue.GetValue());
	}

	/** Converting move constructor. */
	template <typename U> requires (CConstructibleFrom<T, U&&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&&, T>) TOptional(TOptional<U>&& InValue)
		: bIsValid(InValue.IsValid())
	{
		if (InValue.IsValid()) new (&Value) FValueType(MoveTemp(InValue.GetValue()));
	}

	/** Destroys the contained object, if any, as if by a call to Reset(). */
	FORCEINLINE constexpr ~TOptional() requires (CTriviallyDestructible<T>) = default;

	/** Destroys the contained object, if any, as if by a call to Reset(). */
	FORCEINLINE constexpr ~TOptional() requires (!CTriviallyDestructible<T>)
	{
		Reset();
	}

	/** Assigns by copying the state of 'InValue'. */
	FORCEINLINE constexpr TOptional& operator=(const TOptional& InValue) requires (CTriviallyCopyConstructible<T> && CTriviallyCopyAssignable<T>) = default;

	/** Assigns by copying the state of 'InValue'. */
	constexpr TOptional& operator=(const TOptional& InValue) requires (CCopyConstructible<T> && CCopyAssignable<T>
		&& !CTriviallyCopyConstructible<T> && !CTriviallyCopyAssignable<T>)
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
			new (&Value) FValueType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	/** Assigns by moving the state of 'InValue'. */
	FORCEINLINE constexpr TOptional& operator=(TOptional&& InValue) requires (CTriviallyMoveConstructible<T> && CTriviallyMoveAssignable<T>) = default;

	/** Assigns by moving the state of 'InValue'. */
	constexpr TOptional& operator=(TOptional&& InValue) requires (CMoveConstructible<T> && CMoveAssignable<T>
		&& !CTriviallyMoveConstructible<T> && !CTriviallyMoveAssignable<T>)
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
			new (&Value) FValueType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	/** Assigns by copying the state of 'InValue'. */
	template <typename U> requires (CConstructibleFrom<T, const U&>
		&& CAssignableFrom<T&, const U&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	constexpr TOptional& operator=(const TOptional<U>& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = InValue.GetValue();
		else
		{
			new (&Value) FValueType(InValue.GetValue());
			bIsValid = true;
		}

		return *this;
	}

	/** Assigns by moving the state of 'InValue'. */
	template <typename U> requires (CConstructibleFrom<T, U&&>
		&& CAssignableFrom<T&, U&&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	constexpr TOptional& operator=(TOptional<U>&& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (IsValid()) GetValue() = MoveTemp(InValue.GetValue());
		else
		{
			new (&Value) FValueType(MoveTemp(InValue.GetValue()));
			bIsValid = true;
		}

		return *this;
	}

	/** Assigns the value of 'InValue'. */
	template <typename U = T> requires (CConstructibleFrom<T, U&&> && CAssignableFrom<T&, U&&>)
	FORCEINLINE constexpr TOptional& operator=(U&& InValue)
	{
		if (IsValid()) GetValue() = Forward<U>(InValue);
		else
		{
			new (&Value) FValueType(Forward<U>(InValue));
			bIsValid = true;
		}

		return *this;
	}

	/** Check if the two optional are equivalent. */
	template <typename U> requires (CWeaklyEqualityComparable<T, U>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(const TOptional& LHS, const TOptional<U>& RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return false;
		if (LHS.IsValid() == false) return true;
		return *LHS == *RHS;
	}

	/** Check the order relationship between two optional. */
	template <typename U> requires (CSynthThreeWayComparable<T, U>)
	NODISCARD friend FORCEINLINE constexpr partial_ordering operator<=>(const TOptional& LHS, const TOptional<U>& RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return SynthThreeWayCompare(*LHS, *RHS);
	}

	/** Check if the optional value is equivalent to 'InValue'. */
	template <typename U> requires (!CTOptional<U> && CWeaklyEqualityComparable<T, U>)
	NODISCARD FORCEINLINE constexpr bool operator==(const U& InValue) const&
	{
		return IsValid() ? GetValue() == InValue : false;
	}

	/** Check that the optional value is in ordered relationship with 'InValue'. */
	template <typename U> requires (!CTOptional<U> && CSynthThreeWayComparable<T, U>)
	NODISCARD FORCEINLINE constexpr partial_ordering operator<=>(const U& InValue) const&
	{
		return IsValid() ? SynthThreeWayCompare(GetValue(), InValue) : partial_ordering::unordered;
	}

	/** @return true if instance does not contain a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	/**
	 * Changes the contained object to one constructed from the arguments.
	 * First destroys the current contained object (if any) by Reset(),
	 * then constructs an object, direct-non-list-initialized from Forward<Ts>(Args)..., as the contained object.
	 *
	 * @param  Args - The arguments to be passed to the constructor of the object.
	 *
	 * @return A reference to the new object.
	 */
	template <typename... Ts> requires (CConstructibleFrom<T, Ts...>)
	FORCEINLINE constexpr T& Emplace(Ts&&... Args)
	{
		Reset();

		T* Result = new (&Value) FValueType(Forward<Ts>(Args)...);
		bIsValid = true;

		return *Result;
	}

	/**
	 * Changes the contained object to one constructed from the arguments.
	 * First destroys the current contained object (if any) by Reset(),
	 * then constructs an object, direct-non-list-initialized from IL, Forward<Ts>(Args)..., as the contained object.
	 *
	 * @param  IL, Args - The arguments to be passed to the constructor of the object.
	 *
	 * @return A reference to the new object.
	 */
	template <typename W, typename... Ts> requires (CConstructibleFrom<T, initializer_list<W>, Ts...>)
	FORCEINLINE constexpr T& Emplace(initializer_list<W> IL, Ts&&... Args)
	{
		Reset();

		T* Result = new (&Value) FValueType(IL, Forward<Ts>(Args)...);
		bIsValid = true;

		return *Result;
	}

	/** @return true if instance contains a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return bIsValid; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return bIsValid; }

	/** @return The contained object. */
	NODISCARD FORCEINLINE constexpr       T&  GetValue() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);   }
	NODISCARD FORCEINLINE constexpr       T&& GetValue() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value));  }
	NODISCARD FORCEINLINE constexpr const T&  GetValue() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);   }
	NODISCARD FORCEINLINE constexpr const T&& GetValue() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value));  }

	/** @return The contained object. */
	NODISCARD FORCEINLINE constexpr       T&  operator*() &       { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);   }
	NODISCARD FORCEINLINE constexpr       T&& operator*() &&      { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value));  }
	NODISCARD FORCEINLINE constexpr const T&  operator*() const&  { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);   }
	NODISCARD FORCEINLINE constexpr const T&& operator*() const&& { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value));  }

	/** @return The pointer to the contained object. */
	NODISCARD FORCEINLINE constexpr const T* operator->() const { return &GetValue(); }
	NODISCARD FORCEINLINE constexpr       T* operator->()       { return &GetValue(); }

	/** @return The contained object when IsValid() returns true, 'DefaultValue' otherwise. */
	NODISCARD FORCEINLINE constexpr       T& Get(      T& DefaultValue) &      { return IsValid() ? GetValue() : DefaultValue;  }
	NODISCARD FORCEINLINE constexpr const T& Get(const T& DefaultValue) const& { return IsValid() ? GetValue() : DefaultValue;  }

	/** If not empty, destroys the contained object. */
	FORCEINLINE constexpr void Reset()
	{
		if (bIsValid)
		{
			bIsValid = false;

			reinterpret_cast<T*>(&Value)->~FValueType();
		}
	}

	/** Overloads the GetTypeHash algorithm for TOptional. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TOptional& A) requires (CHashable<T>)
	{
		if (!A.IsValid()) return 2824517378;
		return GetTypeHash(A.GetValue());
	}

	/** Overloads the Swap algorithm for TOptional. */
	friend constexpr void Swap(TOptional& A, TOptional& B) requires (CMoveConstructible<T> && CSwappable<T>)
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

	TAlignedStorage<sizeof(T), alignof(T)> Value;
	bool bIsValid;

};

/** The class template manages an optional contained reference, i.e. a reference that may or may not be present. */
template <typename T> requires (CLValueReference<T>)
class TOptional<T, true> final
{
public:

	using FValueType = TRemoveReference<T>;

	static_assert(!CReference<FValueType>);

	/** Constructs an object that does not contain a reference. */
	FORCEINLINE constexpr TOptional() : Ptr(nullptr) { }

	/** Constructs an object that does not contain a reference. */
	FORCEINLINE constexpr TOptional(FInvalid) : TOptional() { }

	/** Constructs an object with initial content an object, direct-initialized from Forward<U>(InValue). */
	template <typename U = T> requires (CConstructibleFrom<T, U>)
		&& (!CSameAs<TRemoveCVRef<U>, FInPlace>) && (!CSameAs<TOptional, TRemoveCVRef<U>>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&&, T>) TOptional(U&& InValue)
		: Ptr(AddressOf(static_cast<T>(Forward<U>(InValue))))
	{ }

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TOptional(const TOptional&) = default;
	FORCEINLINE constexpr TOptional(TOptional&&)      = default;

	/** Converting constructor. */
	template <typename U> requires (CConstructibleFrom<T, U> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U, T>) TOptional(TOptional<U, true> InValue)
		: Ptr(InValue.IsValid() ? AddressOf(static_cast<T>(InValue.GetValue())) : nullptr)
	{ }

	/** Converting constructor. */
	template <typename U> requires (!CConst<FValueType> && CConstructibleFrom<T, U&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<U&, T>) TOptional(TOptional<U, false>& InValue)
		: Ptr(InValue.IsValid() ? AddressOf(static_cast<T>(InValue.GetValue())) : nullptr)
	{ }

	/** Converting constructor. */
	template <typename U> requires (CConst<FValueType> && CConstructibleFrom<T, const U&> && NAMESPACE_PRIVATE::CTOptionalAllowUnwrappable<U, T>)
	FORCEINLINE constexpr explicit (!CConvertibleTo<const U&, T>) TOptional(const TOptional<U, false>& InValue)
		: Ptr(InValue.IsValid() ? AddressOf(static_cast<T>(InValue.GetValue())) : nullptr)
	{ }

	/** Assigns by copying the state of 'InValue'. */
	FORCEINLINE constexpr TOptional& operator=(const TOptional&) = default;
	FORCEINLINE constexpr TOptional& operator=(TOptional&&)      = default;

	/** Destructor. */
	FORCEINLINE constexpr ~TOptional() = default;

	/** Check if the two optional are equivalent. */
	template <typename U> requires (CWeaklyEqualityComparable<T, U>)
	NODISCARD friend FORCEINLINE constexpr bool operator==(TOptional LHS, TOptional<U, true> RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return false;
		if (LHS.IsValid() == false) return true;
		return *LHS == *RHS;
	}

	/** Check the order relationship between two optional. */
	template <typename U> requires (CSynthThreeWayComparable<T, U>)
	NODISCARD friend FORCEINLINE constexpr partial_ordering operator<=>(TOptional LHS, TOptional<U, true> RHS)
	{
		if (LHS.IsValid() != RHS.IsValid()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return SynthThreeWayCompare(*LHS, *RHS);
	}

	/** Check if the optional reference is equivalent to 'InValue'. */
	template <typename U> requires (!CTOptional<U> && CWeaklyEqualityComparable<T, U>)
	NODISCARD FORCEINLINE constexpr bool operator==(const U& InValue) const&
	{
		return IsValid() ? GetValue() == InValue : false;
	}

	/** Check that the optional reference is in ordered relationship with 'InValue'. */
	template <typename U> requires (!CTOptional<U> && CSynthThreeWayComparable<T, U>)
	NODISCARD FORCEINLINE constexpr partial_ordering operator<=>(const U& InValue) const&
	{
		return IsValid() ? SynthThreeWayCompare(GetValue(), InValue) : partial_ordering::unordered;
	}

	/** @return true if instance does not contain a reference, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	/** @return true if instance contains a reference, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool           IsValid() const { return Ptr != nullptr; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return Ptr != nullptr; }

	/** @return The contained object. */
	NODISCARD FORCEINLINE constexpr T GetValue() const { checkf(IsValid(), TEXT("It is an error to call GetValue() on an unset TOptional. Please either check IsValid() or use Get(DefaultValue) instead.")); return *Ptr; }

	/** @return The pointer to the contained object. */
	NODISCARD FORCEINLINE constexpr auto operator->() const { return Ptr; }

	/** @return The contained object. */
	NODISCARD FORCEINLINE constexpr T operator*() const { return GetValue(); }

	/** @return The contained object when IsValid() returns true, 'DefaultValue' otherwise. */
	NODISCARD FORCEINLINE constexpr T Get(T DefaultValue) const { return IsValid() ? GetValue() : DefaultValue; }

	/** If not empty, destroys the contained object. */
	FORCEINLINE constexpr void Reset()
	{
		Ptr = nullptr;
	}

	/** Overloads the GetTypeHash algorithm for TOptional. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TOptional& A) requires (CHashable<T>)
	{
		if (!A.IsValid()) return 2824517378;
		return GetTypeHash(A.GetValue());
	}

private:

	FValueType* Ptr;

};

template <typename T>
TOptional(T) -> TOptional<T>;

/** Creates an optional object from value. */
template <typename T> requires (CDestructible<TDecay<T>> && CConstructibleFrom<TDecay<T>, T>)
NODISCARD FORCEINLINE constexpr TOptional<TDecay<T>> MakeOptional(T&& InValue)
{
	return TOptional<TDecay<T>>(Forward<T>(InValue));
}

/** Creates an optional object constructed in-place from Args.... */
template <typename T, typename... Ts> requires (CDestructible<T> && CConstructibleFrom<T, Ts...>)
NODISCARD FORCEINLINE constexpr TOptional<T> MakeOptional(Ts&&... Args)
{
	return TOptional<T>(InPlace, Forward<T>(Args)...);
}

/** Creates an optional object constructed in-place from IL, Args.... */
template <typename T, typename U, typename... Ts> requires (CDestructible<T> && CConstructibleFrom<T, initializer_list<U>, Ts...>)
NODISCARD FORCEINLINE constexpr TOptional<T> MakeOptional(initializer_list<U> IL, Ts&&... Args)
{
	return TOptional<T>(InPlace, IL, Forward<T>(Args)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
