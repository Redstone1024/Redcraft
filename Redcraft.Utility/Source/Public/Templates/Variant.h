#pragma once

#include "CoreTypes.h"
#include "Templates/Meta.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename... Ts> requires (sizeof...(Ts) > 0 && (true && ... && CDestructible<Ts>))
class TVariant;

NAMESPACE_PRIVATE_BEGIN

template <typename    T > struct TIsTVariant                  : FFalse { };
template <typename... Ts> struct TIsTVariant<TVariant<Ts...>> : FTrue  { };

template <typename VariantType>
struct TVariantNumImpl;

template <typename... Ts>
struct TVariantNumImpl<TVariant<Ts...>> : TConstant<size_t, Meta::TSize<TTypeSequence<Ts...>>> { };

template <typename T, typename VariantType>
struct TVariantIndexImpl;

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, TVariant<Ts...>> : TConstant<size_t, Meta::TIndex<T, TTypeSequence<Ts...>>> { };

template <size_t I, typename VariantType>
struct TVariantAlternativeImpl;

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, TVariant<Ts...>>
{
	using FType = Meta::TType<I, TTypeSequence<Ts...>>;
};

template <typename T, typename TSequence>
struct TVariantOverloadType
{
	using FFrontType = Meta::TFront<TSequence>;
	using FNextSequence = Meta::TPop<TSequence>;
	using FNextUniqueSequence = typename TVariantOverloadType<T, FNextSequence>::FType;

	// T_i x[] = { Forward<T>(t) };
	static constexpr bool bConditional = requires { DeclVal<void(FFrontType(&&)[1])>()({ DeclVal<T>() }); };

	using FType = TConditional<bConditional, Meta::TPush<FFrontType, FNextUniqueSequence>, FNextUniqueSequence>;
};

template <typename T>
struct TVariantOverloadType<T, TTypeSequence<>>
{
	using FType = TTypeSequence<>;
};

template <typename T, typename... Ts>
using TVariantSelectedType = Meta::TOverloadResolution<T, typename NAMESPACE_PRIVATE::TVariantOverloadType<T, TTypeSequence<Ts...>>::FType>;

NAMESPACE_PRIVATE_END

template <typename T>
concept CTVariant = NAMESPACE_PRIVATE::TIsTVariant<TRemoveCV<T>>::Value;

template <CTVariant T>
inline constexpr size_t TVariantNum = NAMESPACE_PRIVATE::TVariantNumImpl<TRemoveCV<T>>::Value;

template <typename T, CTVariant U>
inline constexpr size_t TVariantIndex = NAMESPACE_PRIVATE::TVariantIndexImpl<T, TRemoveCV<U>>::Value;

template <size_t I, CTVariant U>
using TVariantAlternative = TCopyCV<U, typename NAMESPACE_PRIVATE::TVariantAlternativeImpl<I, TRemoveCV<U>>::FType>;

/**
 * The class template TVariant represents a type-safe union. An instance of TVariant
 * holds a value of one of its alternative types, or in the case of invalid - no value.
 */
template <typename... Ts> requires (sizeof...(Ts) > 0 && (true && ... && CDestructible<Ts>))
class TVariant final
{
public:

	/** Constructs an invalid object. */
	FORCEINLINE constexpr TVariant() : TypeIndex(0xFF) { };

	/** Constructs an invalid object. */
	FORCEINLINE constexpr TVariant(FInvalid) : TVariant() { };

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TVariant(const TVariant& InValue) requires (true && ... && CTriviallyCopyConstructible<Ts>) = default;

	/** Copies content of other into a new instance. */
	FORCEINLINE constexpr TVariant(const TVariant& InValue) requires ((true && ... && CCopyConstructible<Ts>) && !(true && ... && CTriviallyCopyConstructible<Ts>))
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	/** Moves content of other into a new instance. */
	FORCEINLINE constexpr TVariant(TVariant&& InValue) requires (true && ... && CTriviallyMoveConstructible<Ts>) = default;

	/** Moves content of other into a new instance. */
	FORCEINLINE constexpr TVariant(TVariant&& InValue) requires ((true && ... && CMoveConstructible<Ts>) && !(true && ... && CTriviallyMoveConstructible<Ts>))
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	/**
	 * Converting constructor. Constructs a variant holding the alternative type that would be selected
	 * by overload resolution for the expression F(Forward<T>(InValue)) if there was an overload of
	 * imaginary function F(T) for every T from Ts... in scope at the same time, except that an overload F(T)
	 * is only considered if the declaration T X[] = { Forward<T>(InValue) }; is valid for some invented variable x.
	 * Direct-initializes the contained value as if by direct non-list-initialization from Forward<T>(InValue).
	 */
	template <typename T> requires (requires { typename NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>; }
		&& !CTInPlaceType<TRemoveCVRef<T>> && !CTInPlaceIndex<TRemoveCVRef<T>>
		&& !CSameAs<TVariant, TRemoveCVRef<T>>)
	FORCEINLINE constexpr TVariant(T&& InValue) : TVariant(InPlaceType<NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>>, Forward<T>(InValue))
	{ }

	/** Constructs a variant with the specified alternative T and initializes the contained value with the arguments Forward<Us>(Args).... */
	template <typename T, typename... Us> requires (CConstructibleFrom<T, Us...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceType<T>, Us&&... Args)
		: TVariant(InPlaceIndex<TVariantIndex<T, TVariant<Ts...>>>, Forward<Us>(Args)...)
	{ }

	/** Constructs a variant with the alternative T specified by the index I and initializes the contained value with the arguments Forward<Us>(Args).... */
	template <size_t I, typename... Us> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, Us...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceIndex<I>, Us&&... Args)
		: TypeIndex(I)
	{
		using FSelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		new (&Value) FSelectedType(Forward<Us>(Args)...);
	}

	/** Constructs a variant with the specified alternative T and initializes the contained value with the arguments IL, Forward<Us>(Args).... */
	template <typename T, typename U, typename... Us> requires (CConstructibleFrom<T, initializer_list<U>, Us...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceType<T>, initializer_list<U> IL, Us&&... Args)
		: TVariant(InPlaceIndex<TVariantIndex<T, TVariant<Ts...>>>, IL, Forward<Us>(Args)...)
	{ }

	/** Constructs a variant with the alternative T specified by the index I and initializes the contained value with the arguments IL, Forward<Us>(Args).... */
	template <size_t I, typename T, typename... Us> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, initializer_list<T>, Us...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceIndex<I>, initializer_list<T> IL, Us&&... Args)
		: TypeIndex(I)
	{
		using FSelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		new (&Value) FSelectedType(IL, Forward<Us>(Args)...);
	}

	/** Destroys the contained object, if any, as if by a call to Reset(). */
	FORCEINLINE constexpr ~TVariant() requires (true && ... && CTriviallyDestructible<Ts>) = default;

	/** Destroys the contained object, if any, as if by a call to Reset(). */
	FORCEINLINE constexpr ~TVariant() requires (!(true && ... && CTriviallyDestructible<Ts>))
	{
		Reset();
	}

	/** Assigns by copying the state of 'InValue'. */
	FORCEINLINE constexpr TVariant& operator=(const TVariant& InValue) requires (true && ... && (CTriviallyCopyConstructible<Ts> && CTriviallyCopyAssignable<Ts>)) = default;

	/** Assigns by copying the state of 'InValue'. */
	constexpr TVariant& operator=(const TVariant& InValue) requires ((true && ... && (CCopyConstructible<Ts> && CCopyAssignable<Ts>))
		&& !(true && ... && (CTriviallyCopyConstructible<Ts> && CTriviallyCopyAssignable<Ts>)))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) CopyAssignImpl[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{
			Reset();
			CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	/** Assigns by moving the state of 'InValue'. */
	FORCEINLINE constexpr TVariant& operator=(TVariant&& InValue) requires (true && ... && (CTriviallyMoveConstructible<Ts> && CTriviallyMoveAssignable<Ts>)) = default;

	/** Assigns by moving the state of 'InValue'. */
	constexpr TVariant& operator=(TVariant&& InValue) requires ((true && ... && (CMoveConstructible<Ts> && CMoveAssignable<Ts>))
		&& !(true && ... && (CTriviallyMoveConstructible<Ts> && CTriviallyMoveAssignable<Ts>)))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) MoveAssignImpl[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{
			Reset();
			MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	/** Converting assignment. Constructs a variant holding the alternative type that would be selected by overload resolution. */
	template <typename T> requires (requires { typename NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>; })
	FORCEINLINE constexpr TVariant& operator=(T&& InValue)
	{
		using FSelectedType = NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>;

		if (GetIndex() == TVariantIndex<FSelectedType, TVariant<Ts...>>) GetValue<FSelectedType>() = Forward<T>(InValue);
		else
		{
			Reset();
			new (&Value) FSelectedType(Forward<T>(InValue));
			TypeIndex = TVariantIndex<FSelectedType, TVariant<Ts...>>;
		}

		return *this;
	}

	/** Check if the two variants are equivalent. */
	NODISCARD friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CEqualityComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;

		using FCompareImpl = bool(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> bool { return *reinterpret_cast<const Ts*>(LHS) == *reinterpret_cast<const Ts*>(RHS); }... };

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	/** Check the order relationship between two variants. */
	NODISCARD friend constexpr partial_ordering operator<=>(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CSynthThreeWayComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;

		using FCompareImpl = partial_ordering(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> partial_ordering { return SynthThreeWayCompare(*reinterpret_cast<const Ts*>(LHS), *reinterpret_cast<const Ts*>(RHS)); }...};

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	/** Check if the variant value is equivalent to 'InValue'. */
	template <typename T> requires (!CSameAs<TVariant, T> && CEqualityComparable<T>)
	NODISCARD FORCEINLINE constexpr bool operator==(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? GetValue<T>() == InValue : false;
	}

	/** Check that the variant value is in ordered relationship with 'InValue'. */
	template <typename T> requires (!CSameAs<TVariant, T> && CEqualityComparable<T>)
	NODISCARD FORCEINLINE constexpr partial_ordering operator<=>(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? SynthThreeWayCompare(GetValue<T>(), InValue) : partial_ordering::unordered;
	}

	/** @return true if instance does not contain a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	/** Equivalent to Emplace<I>(Forward<Us>(Args)...), where I is the zero-based index of T in Types.... */
	template <typename T, typename... Us> requires (CConstructibleFrom<T, Us...>)
	FORCEINLINE constexpr T& Emplace(Us&&... Args)
	{
		return Emplace<TVariantIndex<T, TVariant<Ts...>>>(Forward<Us>(Args)...);
	}

	/**
	 * First, destroys the currently contained value if any.
	 * Then direct-initializes the contained value as if constructing a value of type T with the arguments Forward<Us>(Args)....
	 *
	 * @param  Args - The arguments to be passed to the constructor of the contained object.
	 *
	 * @return A reference to the new contained object.
	 */
	template <size_t I, typename... Us> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, Us...>)
	FORCEINLINE constexpr TVariantAlternative<I, TVariant<Ts...>>& Emplace(Us&&... Args)
	{
		Reset();

		using FSelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		FSelectedType* Result = new (&Value) FSelectedType(Forward<Us>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	/** Equivalent to Emplace<I>(IL, Forward<Us>(Args)...), where I is the zero-based index of T in Types.... */
	template <typename T, typename U, typename... Us> requires (CConstructibleFrom<T, initializer_list<U>, Us...>)
	FORCEINLINE constexpr T& Emplace(initializer_list<U> IL, Us&&... Args)
	{
		return Emplace<TVariantIndex<T, TVariant<Ts...>>>(IL, Forward<Us>(Args)...);
	}

	/**
	 * First, destroys the currently contained value if any.
	 * Then direct-initializes the contained value as if constructing a value of type T with the arguments IL, Forward<Us>(Args)....
	 *
	 * @param  IL, Args - The arguments to be passed to the constructor of the contained object.
	 *
	 * @return A reference to the new contained object.
	 */
	template <size_t I, typename T, typename... Us> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, initializer_list<T>, Us...>)
	FORCEINLINE constexpr TVariantAlternative<I, TVariant<Ts...>>& Emplace(initializer_list<T> IL, Us&&... Args)
	{
		Reset();

		using FSelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		FSelectedType* Result = new (&Value) FSelectedType(IL, Forward<Us>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	/** @return The typeid of the contained value if instance is non-empty, otherwise typeid(void). */
	NODISCARD FORCEINLINE constexpr const type_info& GetTypeInfo() const { return IsValid() ? *TypeInfos[GetIndex()] : typeid(void); }

	/** @return The zero-based index of the alternative held by the variant. */
	NODISCARD FORCEINLINE constexpr size_t GetIndex() const { return IsValid() ? TypeIndex : INDEX_NONE; }

	/** @return true if instance contains a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool IsValid()           const { return TypeIndex != 0xFF; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return TypeIndex != 0xFF; }

	/** @return true if the variant currently holds the alternative, false otherwise. */
	template <size_t   I> NODISCARD FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == I                                 : false; }
	template <typename T> NODISCARD FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == TVariantIndex<T, TVariant<Ts...>> : false; }

	/** @return The contained object. */
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }

	/** @return The contained object. */
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);  }
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value)); }
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);  }
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value)); }

	/** @return The contained object when HoldsAlternative<I>() returns true, 'DefaultValue' otherwise. */
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) Get(      TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) &      { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }
	template <size_t I> requires (I < sizeof...(Ts)) NODISCARD FORCEINLINE constexpr decltype(auto) Get(const TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) const& { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }

	/** @return The contained object when HoldsAlternative<T>() returns true, 'DefaultValue' otherwise. */
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> NODISCARD FORCEINLINE constexpr decltype(auto) Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	/** If not empty, destroys the contained object. */
	FORCEINLINE constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		if constexpr (!(true && ... && CTriviallyDestructible<Ts>))
		{
			DestroyImpl[GetIndex()](&Value);
		}

		TypeIndex = static_cast<uint8>(INDEX_NONE);
	}

	/** Overloads the GetTypeHash algorithm for TVariant. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TVariant& A) requires (true && ... && CHashable<Ts>)
	{
		if (!A.IsValid()) return 114514;

		using FHashImpl = size_t(*)(const void*);
		constexpr FHashImpl HashImpl[] = { [](const void* This) -> size_t { return GetTypeHash(*reinterpret_cast<const Ts*>(This)); }... };

		return HashCombine(GetTypeHash(A.GetIndex()), HashImpl[A.GetIndex()](&A.Value));
	}

	/** Overloads the Swap algorithm for TVariant. */
	friend constexpr void Swap(TVariant& A, TVariant& B) requires (true && ... && (CMoveConstructible<Ts> && CSwappable<Ts>))
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
		else if (A.GetIndex() == B.GetIndex())
		{
			using FSwapImpl = void(*)(void*, void*);
			constexpr FSwapImpl SwapImpl[] = { [](void* A, void* B) { Swap(*reinterpret_cast<Ts*>(A), *reinterpret_cast<Ts*>(B)); }... };

			SwapImpl[A.GetIndex()](&A.Value, &B.Value);
		}
		else
		{
			TVariant Temp = MoveTemp(A);
			A = MoveTemp(B);
			B = MoveTemp(Temp);
		}
	}

private:

	static constexpr const type_info* TypeInfos[] = { &typeid(Ts)... };

	using FCopyConstructImpl = void(*)(void*, const void*);
	using FMoveConstructImpl = void(*)(void*,       void*);
	using FCopyAssignImpl    = void(*)(void*, const void*);
	using FMoveAssignImpl    = void(*)(void*,       void*);
	using FDestroyImpl       = void(*)(void*             );

	static constexpr FCopyConstructImpl CopyConstructImpl[] = { [](void* A, const void* B) { if constexpr (requires(Ts* A, const Ts* B) { Memory::CopyConstruct (A, B); }) Memory::CopyConstruct (reinterpret_cast<Ts*>(A), reinterpret_cast<const Ts*>(B)); else checkf(false, TEXT("The type '%s' is not copy constructible."), typeid(Ts).name()); }... };
	static constexpr FMoveConstructImpl MoveConstructImpl[] = { [](void* A,       void* B) { if constexpr (requires(Ts* A,       Ts* B) { Memory::MoveConstruct (A, B); }) Memory::MoveConstruct (reinterpret_cast<Ts*>(A), reinterpret_cast<      Ts*>(B)); else checkf(false, TEXT("The type '%s' is not move constructible."), typeid(Ts).name()); }... };
	static constexpr FCopyAssignImpl    CopyAssignImpl[]    = { [](void* A, const void* B) { if constexpr (requires(Ts* A, const Ts* B) { Memory::CopyAssign    (A, B); }) Memory::CopyAssign    (reinterpret_cast<Ts*>(A), reinterpret_cast<const Ts*>(B)); else checkf(false, TEXT("The type '%s' is not copy assignable."),    typeid(Ts).name()); }... };
	static constexpr FMoveAssignImpl    MoveAssignImpl[]    = { [](void* A,       void* B) { if constexpr (requires(Ts* A,       Ts* B) { Memory::MoveAssign    (A, B); }) Memory::MoveAssign    (reinterpret_cast<Ts*>(A), reinterpret_cast<      Ts*>(B)); else checkf(false, TEXT("The type '%s' is not move assignable."),    typeid(Ts).name()); }... };
	static constexpr FDestroyImpl       DestroyImpl[]       = { [](void* A               ) { if constexpr (requires(Ts* A             ) { Memory::Destruct      (A   ); }) Memory::Destruct      (reinterpret_cast<Ts*>(A)                                   ); else checkf(false, TEXT("The type '%s' is not destructible."),       typeid(Ts).name()); }... };

	TAlignedUnion<1, Ts...> Value;
	uint8 TypeIndex;

};

NAMESPACE_PRIVATE_BEGIN

template <typename F, typename... VariantTypes>
struct TVariantVisitImpl
{
	struct FGetTotalNum
	{
		FORCEINLINE static constexpr size_t Do()
		{
			if (sizeof...(VariantTypes) == 0) return 0;

			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			size_t Result = 1;

			for (size_t Index = 0; Index < sizeof...(VariantTypes); ++Index)
			{
				Result *= VariantNums[Index];
			}

			return Result;
		}
	};

	struct FEncodeIndices
	{
		FORCEINLINE static constexpr size_t Do(initializer_list<size_t> Indices)
		{
			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			size_t Result = 0;

			for (size_t Index = 0; Index < sizeof...(VariantTypes); ++Index)
			{
				Result *= VariantNums[Index];
				Result += GetData(Indices)[Index];
			}

			return Result;
		}
	};

	struct FDecodeExtent
	{
		FORCEINLINE static constexpr size_t Do(size_t EncodedIndex, size_t Extent)
		{
			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			for (size_t Index = Extent + 1; Index < sizeof...(VariantTypes); ++Index)
			{
				EncodedIndex /= VariantNums[Index];
			}

			return EncodedIndex % VariantNums[Extent];
		}
	};

	template <size_t EncodedIndex, typename>
	struct FInvokeEncoded;

	template <size_t EncodedIndex, size_t... ExtentIndices>
	struct FInvokeEncoded<EncodedIndex, TIndexSequence<ExtentIndices...>>
	{
		FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
		{
			return Invoke(Forward<F>(Func), Forward<VariantTypes>(Variants).template GetValue<FDecodeExtent::Do(EncodedIndex, ExtentIndices)>()...);
		}

		template <typename Ret>
		struct FResult
		{
			FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
			{
				return InvokeResult<Ret>(Forward<F>(Func), Forward<VariantTypes>(Variants).template GetValue<FDecodeExtent::Do(EncodedIndex, ExtentIndices)>()...);
			}
		};
	};

	template <typename>
	struct FInvokeVariant;

	template <size_t... EncodedIndices>
	struct FInvokeVariant<TIndexSequence<EncodedIndices...>>
	{
		FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
		{
			using FExtentIndices = TIndexSequenceFor<VariantTypes...>;

			using FResultType = TCommonType<decltype(FInvokeEncoded<EncodedIndices, FExtentIndices>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...))...>;

			using FInvokeImplType = FResultType(*)(F&&, VariantTypes&&...);

			constexpr FInvokeImplType InvokeImpl[] = { FInvokeEncoded<EncodedIndices, FExtentIndices>::template FResult<FResultType>::Do... };

			return InvokeImpl[FEncodeIndices::Do({ Variants.GetIndex()... })](Forward<F>(Func), Forward<VariantTypes>(Variants)...);
		}

		template <typename Ret>
		struct FResult
		{
			FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
			{
				using FExtentIndices = TIndexSequenceFor<VariantTypes...>;

				using FInvokeImplType = Ret(*)(F&&, VariantTypes&&...);

				constexpr FInvokeImplType InvokeImpl[] = { FInvokeEncoded<EncodedIndices, FExtentIndices>::template FResult<Ret>::Do... };

				return InvokeImpl[FEncodeIndices::Do({ Variants.GetIndex()... })](Forward<F>(Func), Forward<VariantTypes>(Variants)...);
			}
		};
	};

	FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
	{
		return FInvokeVariant<TMakeIndexSequence<FGetTotalNum::Do()>>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...);
	}

	template <typename Ret>
	struct FResult
	{
		FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
		{
			return FInvokeVariant<TMakeIndexSequence<FGetTotalNum::Do()>>::template FResult<Ret>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...);
		}
	};
};

NAMESPACE_PRIVATE_END

/** Applies the visitor 'Func' (Callable that can be called with any combination of types from variants) to the variants 'Variants'. */
template <typename F, typename FirstVariantType, typename... VariantTypes>
	requires (CTVariant<TRemoveReference<FirstVariantType>> && (true && ... && CTVariant<TRemoveReference<VariantTypes>>))
constexpr decltype(auto) Visit(F&& Func, FirstVariantType&& FirstVariant, VariantTypes&&... Variants)
{
	checkf((true && ... && Variants.IsValid()), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
	return NAMESPACE_PRIVATE::TVariantVisitImpl<F, FirstVariantType, VariantTypes...>::Do(Forward<F>(Func), Forward<FirstVariantType>(FirstVariant), Forward<VariantTypes>(Variants)...);
}

/** Applies the visitor 'Func' (Callable that can be called with any combination of types from variants) to the variants 'Variants'. */
template <typename Ret, typename F, typename FirstVariantType, typename... VariantTypes>
	requires (CTVariant<TRemoveReference<FirstVariantType>> && (true && ... && CTVariant<TRemoveReference<VariantTypes>>))
constexpr Ret Visit(F&& Func, FirstVariantType&& FirstVariant, VariantTypes&&... Variants)
{
	checkf((true && ... && Variants.IsValid()), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
	return NAMESPACE_PRIVATE::TVariantVisitImpl<F, FirstVariantType, VariantTypes...>::template FResult<Ret>::Do(Forward<F>(Func), Forward<FirstVariantType>(FirstVariant), Forward<VariantTypes>(Variants)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
