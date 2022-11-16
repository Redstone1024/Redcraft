#pragma once

#include "CoreTypes.h"
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

template <typename... Ts> requires (true && ... && CDestructible<Ts>)
class TVariant;

NAMESPACE_PRIVATE_BEGIN

template <typename    T    > struct TIsTVariant                     : FFalse { };
template <typename... Ts> struct TIsTVariant<TVariant<Ts...>> : FTrue  { };

template <typename TupleType>
struct TVariantNumImpl;

template <typename... Ts>
struct TVariantNumImpl<TVariant<Ts...>> : TConstant<size_t, sizeof...(Ts)> { };

template <typename... Ts>
struct TVariantNumImpl<const TVariant<Ts...>> : TConstant<size_t, sizeof...(Ts)> { };

template <typename... Ts>
struct TVariantNumImpl<volatile TVariant<Ts...>> : TConstant<size_t, sizeof...(Ts)> { };

template <typename... Ts>
struct TVariantNumImpl<const volatile TVariant<Ts...>> : TConstant<size_t, sizeof...(Ts)> { };

template <typename T, typename TupleType>
struct TVariantIndexImpl;

template <typename T, typename U, typename... Ts>
struct TVariantIndexImpl<T, TVariant<U, Ts...>> : TConstant<size_t, TVariantIndexImpl<T, TVariant<Ts...>>::Value + 1>
{
	static_assert(sizeof...(Ts) != 0, "Non-existent types in variant");
};

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, TVariant<T, Ts...>> : TConstant<size_t, 0>
{
	static_assert((true && ... && !CSameAs<T, Ts>), "Duplicate type in variant");
};

template <typename T>
struct TVariantIndexImpl<T, TVariant<>> : TConstant<size_t, INDEX_NONE> { };

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, const TVariant<Ts...>> : TVariantIndexImpl<T, TVariant<Ts...>> { };

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, volatile TVariant<Ts...>> : TVariantIndexImpl<T, TVariant<Ts...>> { };

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, const volatile TVariant<Ts...>> : TVariantIndexImpl<T, TVariant<Ts...>> { };

template <size_t I, typename TupleType>
struct TVariantAlternativeImpl;

template <size_t I, typename T, typename... Ts>
struct TVariantAlternativeImpl<I, TVariant<T, Ts...>>
{
	static_assert(I < sizeof...(Ts) + 1, "Invalid index in variant");
	using Type = TVariantAlternativeImpl<I - 1, TVariant<Ts...>>::Type;
};

template <typename T, typename... Ts>
struct TVariantAlternativeImpl<0, TVariant<T, Ts...>> { using Type = T; };

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, TVariant<Ts...>> { };

template <>
struct TVariantAlternativeImpl<0, TVariant<>> { };

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, const TVariant<Ts...>> { using Type = TAddConst<typename TVariantAlternativeImpl<I, TVariant<Ts...>>::Type>; };

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, volatile TVariant<Ts...>> { using Type = TAddVolatile<typename TVariantAlternativeImpl<I, TVariant<Ts...>>::Type>; };

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, const volatile TVariant<Ts...>> { using Type = TAddCV<typename TVariantAlternativeImpl<I, TVariant<Ts...>>::Type>; };

template <typename T, typename... Ts>
struct TVariantSelectedType;

template <typename T, typename U, typename... Ts>
struct TVariantSelectedType<T, U, Ts...>
{
	using TypeAlternativeA = TConditional<CConstructibleFrom<U, T&&>, U, void>;
	using TypeAlternativeB = typename TVariantSelectedType<T, Ts...>::Type;

	using Type = TConditional<CSameAs<TRemoveCVRef<TypeAlternativeA>, void>, TypeAlternativeB,
				 TConditional<CSameAs<TRemoveCVRef<TypeAlternativeB>, void>, TypeAlternativeA, 
				 TConditional<CSameAs<TRemoveCVRef<TypeAlternativeB>, TRemoveCVRef<T>>, TypeAlternativeB, TypeAlternativeA>>>;

	// 0 - Type not found
	// 1 - Same type found
	// 2 - Multiple types found
	// 3 - The type found
	static constexpr uint8 Flag = CSameAs<TRemoveCVRef<Type>, void> ? 0 :
								  CSameAs<TRemoveCVRef<TypeAlternativeA>, TRemoveCVRef<TypeAlternativeB>> ? 2 :
								  CSameAs<TRemoveCVRef<            Type>, TRemoveCVRef<               T>> ? 1 :
								 !CSameAs<TRemoveCVRef<TypeAlternativeA>, void> && !CSameAs<TypeAlternativeB, void> ? 2 : 3;

	static constexpr bool Value = Flag & 1;

};

template <typename T>
struct TVariantSelectedType<T>
{
	static constexpr uint8 Flag = 0;
	using Type = void;
};

NAMESPACE_PRIVATE_END

template <typename T>
concept CTVariant = NAMESPACE_PRIVATE::TIsTVariant<T>::Value;

template <typename VariantType>
inline constexpr size_t TVariantNum = NAMESPACE_PRIVATE::TVariantNumImpl<VariantType>::Value;

template <typename T, typename VariantType>
inline constexpr size_t TVariantIndex = NAMESPACE_PRIVATE::TVariantIndexImpl<T, VariantType>::Value;

template <size_t I, typename VariantType>
using TVariantAlternative = typename NAMESPACE_PRIVATE::TVariantAlternativeImpl<I, VariantType>::Type;

template <typename... Ts> requires (true && ... && CDestructible<Ts>)
class TVariant
{
public:

	constexpr TVariant() : TypeIndex(0xFF) { };

	constexpr TVariant(FInvalid) : TVariant() { };

	constexpr TVariant(const TVariant& InValue) requires (true && ... && CCopyConstructible<Ts>)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	constexpr TVariant(TVariant&& InValue) requires (true && ... && CMoveConstructible<Ts>)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	template <size_t I, typename... ArgTypes> requires (I < sizeof...(Ts))
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, ArgTypes...>
	constexpr explicit TVariant(TInPlaceIndex<I>, ArgTypes&&... Args)
		: TypeIndex(I)
	{
		using SelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
	}

	template <typename T, typename... ArgTypes> requires CConstructibleFrom<T, ArgTypes...>
	constexpr explicit TVariant(TInPlaceType<T>, ArgTypes&&... Args)
		: TVariant(InPlaceIndex<TVariantIndex<T, TVariant<Ts...>>>, Forward<ArgTypes>(Args)...)
	{ }

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Ts...>::Value
		&& (!CTInPlaceType<TRemoveCVRef<T>>) && (!CTInPlaceIndex<TRemoveCVRef<T>>)
		&& (!CSameAs<TRemoveCVRef<T>, TVariant>)
	constexpr TVariant(T&& InValue) : TVariant(InPlaceType<typename NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Ts...>::Type>, Forward<T>(InValue))
	{ }

	constexpr ~TVariant()
	{
		if constexpr (!(true && ... && CTriviallyDestructible<Ts>)) Reset();
	}

	constexpr TVariant& operator=(const TVariant& InValue) requires (true && ... && (CCopyConstructible<Ts> && CCopyAssignable<Ts>))
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

	constexpr TVariant& operator=(TVariant&& InValue) requires (true && ... && (CMoveConstructible<Ts> && CMoveAssignable<Ts>))
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

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Ts...>::Value
	constexpr TVariant& operator=(T&& InValue)
	{
		using SelectedType = typename NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Ts...>::Type;

		if (GetIndex() == TVariantIndex<SelectedType, TVariant<Ts...>>) GetValue<SelectedType>() = Forward<T>(InValue);
		else
		{
			Reset();
			new(&Value) SelectedType(Forward<T>(InValue));
			TypeIndex = TVariantIndex<SelectedType, TVariant<Ts...>>;
		}

		return *this;
	}

	template <size_t I, typename... ArgTypes> requires (I < sizeof...(Ts))
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, ArgTypes...>
	constexpr TVariantAlternative<I, TVariant<Ts...>>& Emplace(ArgTypes&&... Args)
	{
		Reset();

		using SelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		SelectedType* Result = new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	template <typename T, typename... ArgTypes> requires CConstructibleFrom<T, ArgTypes...>
	constexpr T& Emplace(ArgTypes&&... Args)
	{
		return Emplace<TVariantIndex<T, TVariant<Ts...>>>(Forward<ArgTypes>(Args)...);
	}

	constexpr const type_info& GetTypeInfo() const { return IsValid() ? *TypeInfos[GetIndex()] : typeid(void); }

	constexpr size_t GetIndex()        const { return TypeIndex != 0xFF ? TypeIndex : INDEX_NONE; }
	constexpr bool IsValid()           const { return TypeIndex != 0xFF; }
	constexpr explicit operator bool() const { return TypeIndex != 0xFF; }

	template <size_t   I> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == I                                    : false; }
	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == TVariantIndex<T, TVariant<Ts...>> : false; }

	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }
	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }

	template <typename T> constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);  }
	template <typename T> constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value)); }
	template <typename T> constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);  }
	template <typename T> constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value)); }

	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) Get(      TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) &      { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }
	template <size_t I> requires (I < sizeof...(Ts)) constexpr decltype(auto) Get(const TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) const& { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }

	template <typename T> constexpr decltype(auto) Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> constexpr decltype(auto) Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	template <typename F> requires (true && ... && CInvocable<F, Ts>)
	FORCEINLINE decltype(auto) Visit(F&& Func) &
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = TCommonType<TInvokeResult<F, Ts>...>;

		using FInvokeImpl = ReturnType(*)(F&&, void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), *reinterpret_cast<Ts*>(This)); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Ts>)
	FORCEINLINE decltype(auto) Visit(F&& Func) &&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = TCommonType<TInvokeResult<F, Ts>...>;

		using FInvokeImpl = ReturnType(*)(F&&, void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), MoveTemp(*reinterpret_cast<Ts*>(This))); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Ts>)
	FORCEINLINE decltype(auto) Visit(F&& Func) const&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = TCommonType<TInvokeResult<F, Ts>...>;

		using FInvokeImpl = ReturnType(*)(F&&, const void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, const void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), *reinterpret_cast<const Ts*>(This)); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Ts>)
	FORCEINLINE decltype(auto) Visit(F&& Func) const&&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = TCommonType<TInvokeResult<F, Ts>...>;

		using FInvokeImpl = ReturnType(*)(F&&, const void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, const void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), MoveTemp(*reinterpret_cast<const Ts*>(This))); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Ts>)
	FORCEINLINE R Visit(F&& Func) &       { return Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Ts>)
	FORCEINLINE R Visit(F&& Func) &&      { return MoveTemp(*this).Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Ts>)
	FORCEINLINE R Visit(F&& Func) const&  { return Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Ts>)
	FORCEINLINE R Visit(F&& Func) const&& { return MoveTemp(*this).Visit(Forward<F>(Func)); }

	constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		if constexpr (!(true && ... && CTriviallyDestructible<Ts>))
		{
			DestroyImpl[GetIndex()](&Value);
		}

		TypeIndex = static_cast<uint8>(INDEX_NONE);
	}

	constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Ts>)
	{
		if (!IsValid()) return 114514;

		using NAMESPACE_REDCRAFT::GetTypeHash;

		using FHashImpl = size_t(*)(const void*);
		constexpr FHashImpl HashImpl[] = { [](const void* This) -> size_t { return GetTypeHash(*reinterpret_cast<const Ts*>(This)); }... };

		return HashCombine(GetTypeHash(GetIndex()), HashImpl[GetIndex()](&Value));
	}

	constexpr void Swap(TVariant& InValue) requires (true && ... && (CMoveConstructible<Ts> && CSwappable<Ts>))
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

		if (GetIndex() == InValue.GetIndex())
		{
			using NAMESPACE_REDCRAFT::Swap;

			using FSwapImpl = void(*)(void*, void*);
			constexpr FSwapImpl SwapImpl[] = { [](void* A, void* B) { Swap(*reinterpret_cast<Ts*>(A), *reinterpret_cast<Ts*>(B)); }... };

			SwapImpl[GetIndex()](&Value, &InValue.Value);

			return;
		}

		TVariant Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
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
	static constexpr FDestroyImpl       DestroyImpl[]       = { [](void* A               ) { if constexpr (requires(Ts* A                ) { Memory::Destruct      (A   ); }) Memory::Destruct      (reinterpret_cast<Ts*>(A)                                   ); else checkf(false, TEXT("The type '%s' is not destructible."),       typeid(Ts).name()); }... };

	TAlignedUnion<1, Ts...> Value;
	uint8 TypeIndex;

	friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CEqualityComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;

		using FCompareImpl = bool(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> bool { return *reinterpret_cast<const Ts*>(LHS) == *reinterpret_cast<const Ts*>(RHS); }... };

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	friend constexpr partial_ordering operator<=>(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CSynthThreeWayComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;

		using FCompareImpl = partial_ordering(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> partial_ordering { return SynthThreeWayCompare(*reinterpret_cast<const Ts*>(LHS), *reinterpret_cast<const Ts*>(RHS)); }...};

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

};

template <typename T, typename... Ts> requires (!CSameAs<T, TVariant<Ts...>>) && CEqualityComparable<T>
constexpr bool operator==(const TVariant<Ts...>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <typename... Ts>
constexpr bool operator==(const TVariant<Ts...>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
