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

NAMESPACE_PRIVATE_BEGIN

template <typename T, typename... Types>
struct TVariantAlternativeIndex;

template <typename T, typename U, typename... Types>
struct TVariantAlternativeIndex<T, U, Types...>
	: TConstant<size_t, CSameAs<T, U> ? 0 : (TVariantAlternativeIndex<T, Types...>::Value == INDEX_NONE
	? INDEX_NONE : TVariantAlternativeIndex<T, Types...>::Value + 1)>
{ };

template <typename T>
struct TVariantAlternativeIndex<T> : TConstant<size_t, INDEX_NONE> { };

template <size_t I, typename... Types>
struct TVariantAlternativeType;

template <size_t I, typename T, typename... Types>
struct TVariantAlternativeType<I, T, Types...>
{
	static_assert(I < sizeof...(Types) + 1, "Variant type index is invalid");
	using Type = TVariantAlternativeType<I - 1, Types...>::Type;
};

template <typename T, typename... Types>
struct TVariantAlternativeType<0, T, Types...> { using Type = T; };

template <>
struct TVariantAlternativeType<0> { };

template <typename T, typename... Types>
struct TVariantSelectedType;

template <typename T, typename U, typename... Types>
struct TVariantSelectedType<T, U, Types...>
{
	using TypeAlternativeA = TConditional<CConstructibleFrom<U, T&&>, U, void>;
	using TypeAlternativeB = typename TVariantSelectedType<T, Types...>::Type;

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

template <typename... Types> requires (true && ... && CDestructible<Types>) && (sizeof...(Types) < 0xFF)
struct TVariant
{
	static constexpr size_t AlternativeSize = sizeof...(Types);

	template <size_t I>   struct TAlternativeType  : NAMESPACE_PRIVATE::TVariantAlternativeType<I, Types...>  { };
	template <typename T> struct TAlternativeIndex : NAMESPACE_PRIVATE::TVariantAlternativeIndex<T, Types...> { };

	constexpr TVariant() : TypeIndex(0xFF) { };

	constexpr TVariant(FInvalid) : TVariant() { };

	constexpr TVariant(const TVariant& InValue) requires (true && ... && CCopyConstructible<Types>)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	constexpr TVariant(TVariant&& InValue) requires (true && ... && CMoveConstructible<Types>)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	template <size_t I, typename... ArgTypes> requires (I < AlternativeSize)
		&& CConstructibleFrom<typename TAlternativeType<I>::Type, ArgTypes...>
	constexpr explicit TVariant(TInPlaceIndex<I>, ArgTypes&&... Args)
		: TypeIndex(I)
	{
		using SelectedType = typename TAlternativeType<I>::Type;
		new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
	}

	template <typename T, typename... ArgTypes> requires (TAlternativeIndex<T>::Value != INDEX_NONE)
		&& CConstructibleFrom<typename TAlternativeType<TAlternativeIndex<T>::Value>::Type, ArgTypes...>
	constexpr explicit TVariant(TInPlaceType<T>, ArgTypes&&... Args)
		: TVariant(InPlaceIndex<TAlternativeIndex<T>::Value>, Forward<ArgTypes>(Args)...)
	{ }

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Types...>::Value
		&& (!CTInPlaceType<TRemoveCVRef<T>>) && (!CTInPlaceIndex<TRemoveCVRef<T>>)
		&& (!CSameAs<TRemoveCVRef<T>, TVariant>)
	constexpr TVariant(T&& InValue) : TVariant(InPlaceType<typename NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Types...>::Type>, Forward<T>(InValue))
	{ }

	constexpr ~TVariant()
	{
		if constexpr (!(true && ... && CTriviallyDestructible<Types>)) Reset();
	}

	constexpr TVariant& operator=(const TVariant& InValue) requires (true && ... && (CCopyConstructible<Types> && CCopyAssignable<Types>))
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

	constexpr TVariant& operator=(TVariant&& InValue) requires (true && ... && (CMoveConstructible<Types> && CMoveAssignable<Types>))
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

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Types...>::Value
	constexpr TVariant& operator=(T&& InValue)
	{
		using SelectedType = typename NAMESPACE_PRIVATE::TVariantSelectedType<TRemoveReference<T>, Types...>::Type;

		if (GetIndex() == TAlternativeIndex<SelectedType>::Value) GetValue<SelectedType>() = Forward<T>(InValue);
		else
		{
			Reset();
			new(&Value) SelectedType(Forward<T>(InValue));
			TypeIndex = TAlternativeIndex<SelectedType>::Value;
		}

		return *this;
	}

	template <size_t I, typename... ArgTypes> requires (I < AlternativeSize)
		&& CConstructibleFrom<typename TAlternativeType<I>::Type, ArgTypes...>
	constexpr typename TAlternativeType<I>::Type& Emplace(ArgTypes&&... Args)
	{
		Reset();

		using SelectedType = typename TAlternativeType<I>::Type;
		SelectedType* Result = new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	template <typename T, typename... ArgTypes> requires (TAlternativeIndex<T>::Value != INDEX_NONE)
		&& CConstructibleFrom<typename TAlternativeType<TAlternativeIndex<T>::Value>::Type, ArgTypes...>
	constexpr T& Emplace(ArgTypes&&... Args)
	{
		return Emplace<TAlternativeIndex<T>::Value>(Forward<ArgTypes>(Args)...);
	}

	constexpr const type_info& GetTypeInfo() const { return IsValid() ? *TypeInfos[GetIndex()] : typeid(void); }

	constexpr size_t GetIndex()        const { return TypeIndex != 0xFF ? TypeIndex : INDEX_NONE; }
	constexpr bool IsValid()           const { return TypeIndex != 0xFF; }
	constexpr explicit operator bool() const { return TypeIndex != 0xFF; }

	template <size_t   I> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == I                           : false; }
	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == TAlternativeIndex<T>::Value : false; }

	template <size_t I> requires (I < AlternativeSize) constexpr       typename TAlternativeType<I>::Type&  GetValue() &       { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      TAlternativeType<I>::Type*>(&Value);  }
	template <size_t I> requires (I < AlternativeSize) constexpr       typename TAlternativeType<I>::Type&& GetValue() &&      { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      TAlternativeType<I>::Type*>(&Value)); }
	template <size_t I> requires (I < AlternativeSize) constexpr const typename TAlternativeType<I>::Type&  GetValue() const&  { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const TAlternativeType<I>::Type*>(&Value);  }
	template <size_t I> requires (I < AlternativeSize) constexpr const typename TAlternativeType<I>::Type&& GetValue() const&& { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const TAlternativeType<I>::Type*>(&Value)); }

	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);  }
	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value)); }
	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);  }
	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value)); }

	template <size_t I> requires (I < AlternativeSize) constexpr       typename TAlternativeType<I>::Type& Get(      typename TAlternativeType<I>::Type& DefaultValue) &      { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }
	template <size_t I> requires (I < AlternativeSize) constexpr const typename TAlternativeType<I>::Type& Get(const typename TAlternativeType<I>::Type& DefaultValue) const& { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }

	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr       T& Get(T& DefaultValue)&             { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> requires (TAlternativeIndex<T>::Value != INDEX_NONE) constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	template <typename F> requires (true && ... && CInvocable<F, Types>)
	FORCEINLINE decltype(auto) Visit(F&& Func) &
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = typename TCommonType<TInvokeResult<F, Types>...>::Type;

		using FInvokeImpl = ReturnType(*)(F&&, void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), *reinterpret_cast<Types*>(This)); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Types>)
	FORCEINLINE decltype(auto) Visit(F&& Func) &&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = typename TCommonType<TInvokeResult<F, Types>...>::Type;

		using FInvokeImpl = ReturnType(*)(F&&, void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), MoveTemp(*reinterpret_cast<Types*>(This))); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Types>)
	FORCEINLINE decltype(auto) Visit(F&& Func) const&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = typename TCommonType<TInvokeResult<F, Types>...>::Type;

		using FInvokeImpl = ReturnType(*)(F&&, const void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, const void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), *reinterpret_cast<const Types*>(This)); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename F> requires (true && ... && CInvocable<F, Types>)
	FORCEINLINE decltype(auto) Visit(F&& Func) const&&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));

		using ReturnType = typename TCommonType<TInvokeResult<F, Types>...>::Type;

		using FInvokeImpl = ReturnType(*)(F&&, const void*);
		static constexpr FInvokeImpl InvokeImpl[] = { [](F&& Func, const void* This) -> ReturnType { return InvokeResult<ReturnType>(Forward<F>(Func), MoveTemp(*reinterpret_cast<const Types*>(This))); }... };

		return InvokeImpl[GetIndex()](Forward<F>(Func), &Value);
	}

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Types>)
	FORCEINLINE R Visit(F&& Func) &       { return Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Types>)
	FORCEINLINE R Visit(F&& Func) &&      { return MoveTemp(*this).Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Types>)
	FORCEINLINE R Visit(F&& Func) const&  { return Visit(Forward<F>(Func)); }

	template <typename R, typename F> requires (true && ... && CInvocableResult<R, F, Types>)
	FORCEINLINE R Visit(F&& Func) const&& { return MoveTemp(*this).Visit(Forward<F>(Func)); }

	constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		if constexpr (!(true && ... && CTriviallyDestructible<Types>))
		{
			DestroyImpl[GetIndex()](&Value);
		}

		TypeIndex = static_cast<uint8>(INDEX_NONE);
	}

	constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Types>)
	{
		if (!IsValid()) return 114514;

		using NAMESPACE_REDCRAFT::GetTypeHash;

		using FHashImpl = size_t(*)(const void*);
		constexpr FHashImpl HashImpl[] = { [](const void* This) -> size_t { return GetTypeHash(*reinterpret_cast<const Types*>(This)); }... };

		return HashCombine(GetTypeHash(GetIndex()), HashImpl[GetIndex()](&Value));
	}

	constexpr void Swap(TVariant& InValue) requires (true && ... && (CMoveConstructible<Types> && CSwappable<Types>))
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
			constexpr FSwapImpl SwapImpl[] = { [](void* A, void* B) { Swap(*reinterpret_cast<Types*>(A), *reinterpret_cast<Types*>(B)); }... };

			SwapImpl[GetIndex()](&Value, &InValue.Value);

			return;
		}

		TVariant Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:
	
	static constexpr const type_info* TypeInfos[] = { &typeid(Types)... };

	using FCopyConstructImpl = void(*)(void*, const void*);
	using FMoveConstructImpl = void(*)(void*,       void*);
	using FCopyAssignImpl    = void(*)(void*, const void*);
	using FMoveAssignImpl    = void(*)(void*,       void*);
	using FDestroyImpl       = void(*)(void*             );

	static constexpr FCopyConstructImpl CopyConstructImpl[] = { [](void* A, const void* B) { if constexpr (requires(Types* A, const Types* B) { Memory::CopyConstruct (A, B); }) Memory::CopyConstruct (reinterpret_cast<Types*>(A), reinterpret_cast<const Types*>(B)); else checkf(false, TEXT("The type '%s' is not copy constructible."), typeid(Types).name()); }... };
	static constexpr FMoveConstructImpl MoveConstructImpl[] = { [](void* A,       void* B) { if constexpr (requires(Types* A,       Types* B) { Memory::MoveConstruct (A, B); }) Memory::MoveConstruct (reinterpret_cast<Types*>(A), reinterpret_cast<      Types*>(B)); else checkf(false, TEXT("The type '%s' is not move constructible."), typeid(Types).name()); }... };
	static constexpr FCopyAssignImpl    CopyAssignImpl[]    = { [](void* A, const void* B) { if constexpr (requires(Types* A, const Types* B) { Memory::CopyAssign    (A, B); }) Memory::CopyAssign    (reinterpret_cast<Types*>(A), reinterpret_cast<const Types*>(B)); else checkf(false, TEXT("The type '%s' is not copy assignable."),    typeid(Types).name()); }... };
	static constexpr FMoveAssignImpl    MoveAssignImpl[]    = { [](void* A,       void* B) { if constexpr (requires(Types* A,       Types* B) { Memory::MoveAssign    (A, B); }) Memory::MoveAssign    (reinterpret_cast<Types*>(A), reinterpret_cast<      Types*>(B)); else checkf(false, TEXT("The type '%s' is not move assignable."),    typeid(Types).name()); }... };
	static constexpr FDestroyImpl       DestroyImpl[]       = { [](void* A               ) { if constexpr (requires(Types* A                ) { Memory::Destruct      (A   ); }) Memory::Destruct      (reinterpret_cast<Types*>(A)                                   ); else checkf(false, TEXT("The type '%s' is not destructible."),       typeid(Types).name()); }... };

	TAlignedUnion<1, Types...> Value;
	uint8 TypeIndex;

	friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CEqualityComparable<Types>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;

		using FCompareImpl = bool(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> bool { return *reinterpret_cast<const Types*>(LHS) == *reinterpret_cast<const Types*>(RHS); }... };

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	friend constexpr partial_ordering operator<=>(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CSynthThreeWayComparable<Types>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;

		using FCompareImpl = partial_ordering(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> partial_ordering { return SynthThreeWayCompare(*reinterpret_cast<const Types*>(LHS), *reinterpret_cast<const Types*>(RHS)); }...};

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

};

template <typename T, typename... Types> requires (!CSameAs<T, TVariant<Types...>>) && CEqualityComparable<T>
constexpr bool operator==(const TVariant<Types...>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <typename... Types>
constexpr bool operator==(const TVariant<Types...>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

NAMESPACE_PRIVATE_BEGIN

template <typename    T    > struct TIsTVariant                     : FFalse { };
template <typename... Types> struct TIsTVariant<TVariant<Types...>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTVariant = NAMESPACE_PRIVATE::TIsTVariant<T>::Value;

template <typename VariantType> requires CTVariant<TRemoveCVRef<VariantType>>
struct TVariantAlternativeSize : TConstant<size_t, VariantType::AlternativeSize> { };

template <size_t I, typename VariantType> requires CTVariant<TRemoveCVRef<VariantType>>
struct TVariantAlternativeType { using Type = TCopyCV<TRemoveReference<VariantType>, typename TRemoveCVRef<VariantType>::template TAlternativeType<I>::Type>; };

template <typename T, typename VariantType> requires CTVariant<TRemoveCVRef<VariantType>>
struct TVariantAlternativeIndex : VariantType::template TAlternativeIndex<T> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
