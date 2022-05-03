#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/TypeInfo.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T, typename... Types>
struct TVariantAlternativeIndex;

template <typename T, typename U, typename... Types>
struct TVariantAlternativeIndex<T, U, Types...>
	: TConstant<size_t, TIsSame<T, U>::Value ? 0 : (TVariantAlternativeIndex<T, Types...>::Value == INDEX_NONE
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
	using TypeAlternativeA = typename TConditional<TIsConstructible<U, T&&>::Value, U, void>::Type;
	using TypeAlternativeB = typename TVariantSelectedType<T, Types...>::Type;

	using Type = typename TConditional<TIsSame<typename TRemoveCVRef<TypeAlternativeA>::Type, void>::Value, TypeAlternativeB,
				 typename TConditional<TIsSame<typename TRemoveCVRef<TypeAlternativeB>::Type, void>::Value, TypeAlternativeA, 
				 typename TConditional<TIsSame<typename TRemoveCVRef<TypeAlternativeB>::Type, typename TRemoveCVRef<T>::Type>::Value, TypeAlternativeB, TypeAlternativeA>::Type>::Type>::Type;

	// 0 - Type not found
	// 1 - Same type found
	// 2 - Multiple types found
	// 3 - The type found
	static constexpr uint8 Flag = TIsSame<typename TRemoveCVRef<Type>::Type, void>::Value ? 0 :
								  TIsSame<typename TRemoveCVRef<TypeAlternativeA>::Type, typename TRemoveCVRef<TypeAlternativeB>::Type>::Value ? 2 :
								  TIsSame<typename TRemoveCVRef<            Type>::Type, typename TRemoveCVRef<               T>::Type>::Value ? 1 :
								 !TIsSame<typename TRemoveCVRef<TypeAlternativeA>::Type, void>::Value && !TIsSame<TypeAlternativeB, void>::Value ? 2 : 3;

	static constexpr bool Value = Flag & 1;

};

template <typename T>
struct TVariantSelectedType<T>
{
	static constexpr uint8 Flag = 0;
	using Type = void;
};

template <typename R, typename F, typename T>
constexpr R VariantVisitLValue(F&& Func, void* Arg)
{
	if constexpr(TIsVoid<R>::Value) Invoke(Forward<F>(Func), *reinterpret_cast<T*>(Arg));
	else return InvokeResult<R>(Forward<F>(Func), *reinterpret_cast<T*>(Arg));
}

template <typename R, typename F>
using FVariantVisitLValueFunc = R(*)(F&&, void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitRValue(F&& Func, void* Arg)
{
	if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), MoveTemp(*reinterpret_cast<T*>(Arg)));
	else return InvokeResult<R>(Forward<F>(Func), MoveTemp(*reinterpret_cast<T*>(Arg)));
}

template <typename R, typename F>
using FVariantVisitRValueFunc = R(*)(F&&, void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitConstLValue(F&& Func, const void* Arg)
{
	if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), *reinterpret_cast<const T*>(Arg));
	else return InvokeResult<R>(Forward<F>(Func), *reinterpret_cast<const T*>(Arg));
}

template <typename R, typename F>
using FVariantVisitConstLValueFunc = R(*)(F&&, const void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitConstRValue(F&& Func, const void* Arg)
{
	if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), MoveTemp(*reinterpret_cast<const T*>(Arg)));
	else return InvokeResult<R>(Forward<F>(Func), MoveTemp(*reinterpret_cast<const T*>(Arg)));
}

template <typename R, typename F>
using FVariantVisitConstRValueFunc = R(*)(F&&, const void*);

template <typename R, typename F, typename... Types>
struct TVariantVisitHelper
{
	static constexpr FVariantVisitLValueFunc<R, F>      VisitLValueFuncs[]      = { VariantVisitLValue<R, F, Types>...      };
	static constexpr FVariantVisitRValueFunc<R, F>      VisitRValueFuncs[]      = { VariantVisitRValue<R, F, Types>...      };
	static constexpr FVariantVisitConstLValueFunc<R, F> VisitConstLValueFuncs[] = { VariantVisitConstLValue<R, F, Types>... };
	static constexpr FVariantVisitConstRValueFunc<R, F> VisitConstRValueFuncs[] = { VariantVisitConstRValue<R, F, Types>... };
};

NAMESPACE_PRIVATE_END

template <typename... Types> requires (true && ... && (TIsObject<Types>::Value && !TIsArray<Types>::Value && TIsDestructible<Types>::Value)) && (sizeof...(Types) < 0xFF)
struct TVariant
{
	static constexpr size_t AlternativeSize = sizeof...(Types);

	template <size_t I>   struct TAlternativeType  : NAMESPACE_PRIVATE::TVariantAlternativeType<I, Types...>  { };
	template <typename T> struct TAlternativeIndex : NAMESPACE_PRIVATE::TVariantAlternativeIndex<T, Types...> { };

	constexpr TVariant() : TypeIndex(0xFF) { };

	constexpr TVariant(FInvalid) : TVariant() { };

	constexpr TVariant(const TVariant& InValue) requires (true && ... && TIsCopyConstructible<Types>::Value)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) TypeInfos[InValue.GetIndex()]->CopyConstruct(&Value, &InValue.Value);
	}

	constexpr TVariant(TVariant&& InValue) requires (true && ... && TIsMoveConstructible<Types>::Value)
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) TypeInfos[InValue.GetIndex()]->MoveConstruct(&Value, &InValue.Value);
	}

	template <size_t I, typename... ArgTypes> requires (I < AlternativeSize)
		&& TIsConstructible<typename TAlternativeType<I>::Type, ArgTypes...>::Value
	constexpr explicit TVariant(TInPlaceIndex<I>, ArgTypes&&... Args)
		: TypeIndex(I)
	{
		using SelectedType = typename TAlternativeType<I>::Type;
		new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
	}

	template <typename T, typename... ArgTypes> requires (TAlternativeIndex<T>::Value != INDEX_NONE)
		&& TIsConstructible<typename TAlternativeType<TAlternativeIndex<T>::Value>::Type, ArgTypes...>::Value
	constexpr explicit TVariant(TInPlaceType<T>, ArgTypes&&... Args)
		: TVariant(InPlaceIndex<TAlternativeIndex<T>::Value>, Forward<ArgTypes>(Args)...)
	{ }

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<typename TRemoveReference<T>::Type, Types...>::Value
		&& (!TIsTInPlaceType<typename TRemoveCVRef<T>::Type>::Value) && (!TIsTInPlaceIndex<typename TRemoveCVRef<T>::Type>::Value)
		&& (!TIsSame<typename TRemoveCVRef<T>::Type, TVariant>::Value)
	constexpr TVariant(T&& InValue) : TVariant(InPlaceType<typename NAMESPACE_PRIVATE::TVariantSelectedType<typename TRemoveReference<T>::Type, Types...>::Type>, Forward<T>(InValue))
	{ }

	constexpr ~TVariant()
	{
		if constexpr (!(true && ... && TIsTriviallyDestructible<Types>::Value)) Reset();
	}

	constexpr TVariant& operator=(const TVariant& InValue) requires (true && ... && (TIsCopyConstructible<Types>::Value && TIsCopyAssignable<Types>::Value))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) TypeInfos[InValue.GetIndex()]->CopyAssign(&Value, &InValue.Value);
		else
		{	
			Reset();
			TypeInfos[InValue.GetIndex()]->CopyConstruct(&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	constexpr TVariant& operator=(TVariant&& InValue) requires (true && ... && (TIsMoveConstructible<Types>::Value && TIsMoveAssignable<Types>::Value))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) TypeInfos[InValue.GetIndex()]->MoveAssign(&Value, &InValue.Value);
		else
		{
			Reset();
			TypeInfos[InValue.GetIndex()]->MoveConstruct(&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	template <typename T> requires NAMESPACE_PRIVATE::TVariantSelectedType<typename TRemoveReference<T>::Type, Types...>::Value
	constexpr TVariant& operator=(T&& InValue)
	{
		using SelectedType = typename NAMESPACE_PRIVATE::TVariantSelectedType<typename TRemoveReference<T>::Type, Types...>::Type;

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
		&& TIsConstructible<typename TAlternativeType<I>::Type, ArgTypes...>::Value
	constexpr typename TAlternativeType<I>::Type& Emplace(ArgTypes&&... Args)
	{
		Reset();

		using SelectedType = typename TAlternativeType<I>::Type;
		SelectedType* Result = new(&Value) SelectedType(Forward<ArgTypes>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	template <typename T, typename... ArgTypes> requires (TAlternativeIndex<T>::Value != INDEX_NONE)
		&& TIsConstructible<typename TAlternativeType<TAlternativeIndex<T>::Value>::Type, ArgTypes...>::Value
	constexpr T& Emplace(ArgTypes&&... Args)
	{
		return Emplace<TAlternativeIndex<T>::Value>(Forward<ArgTypes>(Args)...);
	}

	constexpr const FTypeInfo& GetTypeInfo() const { return IsValid() ? *TypeInfos[GetIndex()] : Typeid(void); }

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

	template <typename F> requires (true && ... && TIsInvocable<F, Types>::Value)
	constexpr auto Visit(F&& Func) &
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F> requires (true && ... && TIsInvocable<F, Types>::Value)
	constexpr auto Visit(F&& Func) &&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F> requires (true && ... && TIsInvocable<F, Types>::Value)
	constexpr auto Visit(F&& Func) const&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitConstLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F> requires (true && ... && TIsInvocable<F, Types>::Value)
	constexpr auto Visit(F&& Func) const&&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitConstRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F> requires (true && ... && TIsInvocableResult<R, F, Types>::Value)
	constexpr R Visit(F&& Func) &
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F> requires (true && ... && TIsInvocableResult<R, F, Types>::Value)
	constexpr R Visit(F&& Func) &&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F> requires (true && ... && TIsInvocableResult<R, F, Types>::Value)
	constexpr R Visit(F&& Func) const&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitConstLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F> requires (true && ... && TIsInvocableResult<R, F, Types>::Value)
	constexpr R Visit(F&& Func) const&&
	{
		checkf(IsValid(), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitConstRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		if constexpr (!(true && ... && TIsTriviallyDestructible<Types>::Value))
		{
			TypeInfos[GetIndex()]->Destroy(&Value);
		}

		TypeIndex = static_cast<uint8>(INDEX_NONE);
	}

	constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Types>)
	{
		if (!IsValid()) return 114514;
		return HashCombine(NAMESPACE_REDCRAFT::GetTypeHash(GetIndex()), TypeInfos[GetIndex()]->HashItem(&Value));
	}

	constexpr void Swap(TVariant& InValue) requires (true && ... && (TIsMoveConstructible<Types>::Value && TIsSwappable<Types>::Value))
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
			TypeInfos[GetIndex()]->SwapItem(&Value, &InValue.Value);
			return;
		}

		TVariant Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:
	
	static constexpr const FTypeInfo* TypeInfos[] = { &Typeid(Types)... };

	TAlignedUnion<1, Types...>::Type Value;
	uint8 TypeIndex;

	friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CEqualityComparable<Types>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;
		return TypeInfos[LHS.GetIndex()]->EqualityCompare(&LHS.Value, &RHS.Value);
	}

	friend constexpr partial_ordering operator<=>(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CSynthThreeWayComparable<Types>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return TypeInfos[LHS.GetIndex()]->SynthThreeWayCompare(&LHS.Value, &RHS.Value);
	}

};

template <typename T, typename... Types> requires (!TIsSame<T, TVariant<Types...>>::Value) && CEqualityComparable<T>
constexpr bool operator==(const TVariant<Types...>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <typename... Types>
constexpr bool operator==(const TVariant<Types...>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <typename    T    > struct TIsTVariant                     : FFalse { };
template <typename... Types> struct TIsTVariant<TVariant<Types...>> : FTrue  { };

template <typename VariantType> requires TIsTVariant<typename TRemoveCVRef<VariantType>::Type>::Value
struct TVariantAlternativeSize : TConstant<size_t, VariantType::AlternativeSize> { };

template <size_t I, typename VariantType> requires TIsTVariant<typename TRemoveCVRef<VariantType>::Type>::Value
struct TVariantAlternativeType { using Type = typename TCopyCV<typename TRemoveReference<VariantType>::Type, typename TRemoveCVRef<VariantType>::Type::template TAlternativeType<I>::Type>::Type; };

template <typename T, typename VariantType> requires TIsTVariant<typename TRemoveCVRef<VariantType>::Type>::Value
struct TVariantAlternativeIndex : VariantType::template TAlternativeIndex<T> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
