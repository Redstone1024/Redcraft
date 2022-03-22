#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
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

template <typename T, typename... Types>
struct TVariantSelectedType;

template <typename T, typename U, typename... Types>
struct TVariantSelectedType<T, U, Types...>
{
	using TypeAlternativeA = typename TConditional<TIsConstructible<U, T&&>::Value, U, void>::Type;
	using TypeAlternativeB = typename TVariantSelectedType<T, Types...>::Type;

	using Type = typename TConditional<TIsSame<TypeAlternativeA, void>::Value, TypeAlternativeB,
				 typename TConditional<TIsSame<TypeAlternativeB, void>::Value, TypeAlternativeA, 
				 typename TConditional<TIsSame<TypeAlternativeB,    T>::Value, TypeAlternativeB, TypeAlternativeA>::Type>::Type>::Type;

	// 0 - Type not found
	// 1 - Same type found
	// 2 - Multiple types found
	// 3 - The type found
	static constexpr uint8 Flag = TIsSame<Type, void>::Value ? 0 :
								  TIsSame<Type,    T>::Value ? 1 :
								  !TIsSame<TypeAlternativeA, void>::Value && !TIsSame<TypeAlternativeB, void>::Value ? 2 : 3;

	static constexpr bool Value = Flag & 1;

};

template <typename T>
struct TVariantSelectedType<T>
{
	static constexpr uint8 Flag = 0;
	using Type = void;
};

template <typename T>
constexpr void VariantDestroy(void* InValue)
{
	if constexpr (!TIsDestructible<T>::Value) check_no_entry();
	else if constexpr (!TIsTriviallyDestructible<T>::Value)
	{
		typedef T DestructOptionalType;
		reinterpret_cast<T*>(InValue)->DestructOptionalType::~DestructOptionalType();
	}
}

using FVariantDestroyFunc = void(*)(void*);

template <typename T>
constexpr void VariantCopyConstruct(void* Target, const void* Source)
{
	if constexpr (!TIsCopyConstructible<T>::Value || TIsConst<T>::Value) check_no_entry();
	else new(reinterpret_cast<T*>(Target)) T(*reinterpret_cast<const T*>(Source));
}

using FVariantCopyConstructFunc = void(*)(void*, const void*);

template <typename T>
constexpr void VariantMoveConstruct(void* Target, void* Source)
{
	if constexpr (!TIsMoveConstructible<T>::Value || TIsConst<T>::Value) check_no_entry();
	else new(reinterpret_cast<T*>(Target)) T(MoveTemp(*reinterpret_cast<T*>(Source)));
}

using FVariantMoveConstructFunc = void(*)(void*, void*);

template <typename T>
constexpr void VariantCopyAssign(void* Target, const void* Source)
{
	if constexpr (!TIsCopyAssignable<T>::Value || TIsConst<T>::Value) check_no_entry();
	else *reinterpret_cast<T*>(Target) = *reinterpret_cast<const T*>(Source);
}

using FVariantCopyAssignFunc = void(*)(void*, const void*);

template <typename T>
constexpr void VariantMoveAssign(void* Target, void* Source)
{
	if constexpr (!TIsMoveAssignable<T>::Value || TIsConst<T>::Value) check_no_entry();
	else *reinterpret_cast<T*>(Target) = MoveTemp(*reinterpret_cast<T*>(Source));
}

using FVariantMoveAssignFunc = void(*)(void*, void*);

template <typename T>
constexpr bool VariantEqualityOperator(const void* LHS, const void* RHS)
{
	if constexpr (!CEqualityComparable<T>) check_no_entry();
	else return *reinterpret_cast<const T*>(LHS) == *reinterpret_cast<const T*>(RHS);
	return false;
}

using FVariantEqualityOperatorFunc = bool(*)(const void*, const void*);

template <typename T>
constexpr bool VariantInequalityOperator(const void* LHS, const void* RHS)
{
	if constexpr (!CEqualityComparable<T>) check_no_entry();
	else return *reinterpret_cast<const T*>(LHS) != *reinterpret_cast<const T*>(RHS);
	return false;
}

using FVariantInequalityOperatorFunc = bool(*)(const void*, const void*);

template <typename... Types>
struct TVariantHelper
{
	static constexpr FVariantDestroyFunc            DestroyFuncs[]            = { VariantDestroy<Types>...            };
	static constexpr FVariantCopyConstructFunc      CopyConstructFuncs[]      = { VariantCopyConstruct<Types>...      };
	static constexpr FVariantMoveConstructFunc      MoveConstructFuncs[]      = { VariantMoveConstruct<Types>...      };
	static constexpr FVariantCopyAssignFunc         CopyAssignFuncs[]         = { VariantCopyAssign<Types>...         };
	static constexpr FVariantMoveAssignFunc         MoveAssignFuncs[]         = { VariantMoveAssign<Types>...         };
	static constexpr FVariantEqualityOperatorFunc   EqualityOperatorFuncs[]   = { VariantEqualityOperator<Types>...   };
	static constexpr FVariantInequalityOperatorFunc InequalityOperatorFuncs[] = { VariantInequalityOperator<Types>... };
};

template <typename R, typename F, typename T>
constexpr R VariantVisitLValue(F&& Func, void* Arg)
{
	if constexpr (!TIsInvocableResult<R, F, T>::Value) check_no_entry();
	else if constexpr(TIsVoid<R>::Value) Invoke(Forward<F>(Func), *reinterpret_cast<T*>(Arg));
	else return InvokeResult<R>(Forward<F>(Func), *reinterpret_cast<T*>(Arg));
}

template <typename R, typename F>
using FVariantVisitLValueFunc = R(*)(F&&, void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitRValue(F&& Func, void* Arg)
{
	if constexpr (!TIsInvocableResult<R, F, T>::Value) check_no_entry();
	else if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), MoveTemp(*reinterpret_cast<T*>(Arg)));
	else return InvokeResult<R>(Forward<F>(Func), MoveTemp(*reinterpret_cast<T*>(Arg)));
}

template <typename R, typename F>
using FVariantVisitRValueFunc = R(*)(F&&, void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitConstLValue(F&& Func, const void* Arg)
{
	if constexpr (!TIsInvocableResult<R, F, T>::Value) check_no_entry();
	else if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), *reinterpret_cast<const T*>(Arg));
	else return InvokeResult<R>(Forward<F>(Func), *reinterpret_cast<const T*>(Arg));
}

template <typename R, typename F>
using FVariantVisitConstLValueFunc = R(*)(F&&, const void*);

template <typename R, typename F, typename T>
constexpr R VariantVisitConstRValue(F&& Func, const void* Arg)
{
	if constexpr (!TIsInvocableResult<R, F, T>::Value) check_no_entry();
	else if constexpr (TIsVoid<R>::Value) Invoke(Forward<F>(Func), MoveTemp(*reinterpret_cast<const T*>(Arg)));
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

template <typename... Types> requires (true && ... && (TIsObject<Types>::Value && !TIsArray<Types>::Value && TIsDestructible<Types>::Value))
struct TVariant
{
	struct FAlternativeSize : TConstant<size_t, sizeof...(Types)> { };

	template <size_t I>   struct TAlternativeType  : NAMESPACE_PRIVATE::TVariantAlternativeType<I, Types...>  { };
	template <typename T> struct TAlternativeIndex : NAMESPACE_PRIVATE::TVariantAlternativeIndex<T, Types...> { };

	constexpr TVariant() : TypeIndex(INDEX_NONE) { };

	constexpr TVariant(FInvalid) : TVariant() { };

	constexpr TVariant(const TVariant& InValue)
		: TypeIndex(InValue.GetIndex())
	{
		if (GetIndex() != INDEX_NONE) FHelper::CopyConstructFuncs[InValue.GetIndex()](&Value, &InValue.Value);
	}

	constexpr TVariant(TVariant&& InValue)
		: TypeIndex(InValue.GetIndex())
	{
		if (GetIndex() != INDEX_NONE) FHelper::MoveConstructFuncs[InValue.GetIndex()](&Value, &InValue.Value);
	}

	template <size_t I, typename... ArgTypes> requires (I < FAlternativeSize::Value)
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
		&& (!TIsInPlaceTypeSpecialization<typename TRemoveCVRef<T>::Type>::Value) && (!TIsInPlaceIndexSpecialization<typename TRemoveCVRef<T>::Type>::Value)
		&& (!TIsSame<typename TRemoveCVRef<T>::Type, TVariant>::Value)
	constexpr TVariant(T&& InValue) : TVariant(InPlaceType<typename NAMESPACE_PRIVATE::TVariantSelectedType<typename TRemoveReference<T>::Type, Types...>::Type>, Forward<T>(InValue))
	{ }

	constexpr ~TVariant()
	{
		Reset();
	}

	constexpr TVariant& operator=(const TVariant& InValue)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) FHelper::CopyAssignFuncs[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{	
			Reset();
			FHelper::CopyConstructFuncs[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = InValue.GetIndex();
		}

		return *this;
	}

	constexpr TVariant& operator=(TVariant&& InValue)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) FHelper::MoveAssignFuncs[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{
			Reset();
			FHelper::MoveConstructFuncs[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = InValue.GetIndex();
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

	template <size_t I, typename... ArgTypes> requires (I < FAlternativeSize::Value)
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

	constexpr size_t GetIndex()        const { return TypeIndex; }
	constexpr bool IsValid()           const { return GetIndex() != INDEX_NONE; }
	constexpr explicit operator bool() const { return GetIndex() != INDEX_NONE; }

	template <size_t   I> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == I                           : false; }
	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == TAlternativeIndex<T>::Value : false; }
	
	constexpr       void* GetData()       { return &Value; }
	constexpr const void* GetData() const { return &Value; }

	template <size_t I> constexpr       typename TAlternativeType<I>::Type&  GetValue() &       { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      TAlternativeType<I>::Type*>(&Value);  }
	template <size_t I> constexpr       typename TAlternativeType<I>::Type&& GetValue() &&      { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      TAlternativeType<I>::Type*>(&Value)); }
	template <size_t I> constexpr const typename TAlternativeType<I>::Type&  GetValue() const&  { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const TAlternativeType<I>::Type*>(&Value);  }
	template <size_t I> constexpr const typename TAlternativeType<I>::Type&& GetValue() const&& { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const TAlternativeType<I>::Type*>(&Value)); }

	template <typename T> constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);  }
	template <typename T> constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value)); }
	template <typename T> constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);  }
	template <typename T> constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value)); }

	template <size_t I> constexpr typename TAlternativeType<I>::Type Get(typename TAlternativeType<I>::Type&& DefaultValue) &&     { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }
	template <size_t I> constexpr typename TAlternativeType<I>::Type Get(typename TAlternativeType<I>::Type&& DefaultValue) const& { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }

	template <typename T> constexpr T Get(T&& DefaultValue) &&     { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> constexpr T Get(T&& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	template <typename F>
	constexpr auto Visit(F&& Func) &
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F>
	constexpr auto Visit(F&& Func) &&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F>
	constexpr auto Visit(F&& Func) const&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitConstLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename F>
	constexpr auto Visit(F&& Func) const&&
	{
		using ReturnType = typename TCommonType<typename TInvokeResult<F, Types>::Type...>::Type;
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return ReturnType(NAMESPACE_PRIVATE::TVariantVisitHelper<ReturnType, F, Types...>::VisitConstRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F>
	constexpr R Visit(F&& Func) &
	{
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F>
	constexpr R Visit(F&& Func) &&
	{
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F>
	constexpr R Visit(F&& Func) const&
	{
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitConstLValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	template <typename R, typename F>
	constexpr R Visit(F&& Func) const&&
	{
		checkf(IsValid(), "It is an error to call Visit() on an wrong TVariant. Please either check IsValid().");
		return R(NAMESPACE_PRIVATE::TVariantVisitHelper<R, F, Types...>::VisitConstRValueFuncs[GetIndex()](Forward<F>(Func), &Value));
	}

	constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		FHelper::DestroyFuncs[GetIndex()](&Value);

		TypeIndex = INDEX_NONE;
	}

private:

	using FHelper = NAMESPACE_PRIVATE::TVariantHelper<Types...>;

	TAlignedUnion<1, Types...>::Type Value;
	size_t TypeIndex;

	friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;
		return FHelper::EqualityOperatorFuncs[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	friend constexpr bool operator!=(const TVariant& LHS, const TVariant& RHS)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return true;
		if (LHS.IsValid() == false) return false;
		return FHelper::InequalityOperatorFuncs[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

};

template <typename T, typename... Types>
constexpr bool operator==(const TVariant<Types...>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <typename T, typename... Types>
constexpr bool operator!=(const TVariant<Types...>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() != RHS : true;
}

template <typename T, typename... Types>
constexpr bool operator==(const T& LHS, const TVariant<Types...>& RHS)
{
	return RHS.template HoldsAlternative<T>() ? LHS == RHS.template GetValue<T>() : false;
}

template <typename T, typename... Types>
constexpr bool operator!=(const T& LHS, const TVariant<Types...>& RHS)
{
	return RHS.template HoldsAlternative<T>() ? LHS != RHS.template GetValue<T>() : true;
}

template <typename... Types>
constexpr bool operator==(const TVariant<Types...>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <typename... Types>
constexpr bool operator!=(const TVariant<Types...>& LHS, FInvalid)
{
	return LHS.IsValid();
}

template <typename... Types>
constexpr bool operator==(FInvalid, const TVariant<Types...>& RHS)
{
	return !RHS.IsValid();
}

template <typename... Types>
constexpr bool operator!=(FInvalid, const TVariant<Types...>& RHS)
{
	return RHS.IsValid();
}

template <typename... Types> requires (true && ... && (TIsMoveConstructible<Types>::Value && TIsSwappable<Types>::Value))
constexpr void Swap(TVariant<Types...>& A, TVariant<Types...>& B)
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

	TVariant<Types...> Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

template <typename    T    > struct TIsVariantSpecialization                     : FFalse { };
template <typename... Types> struct TIsVariantSpecialization<TVariant<Types...>> : FTrue  { };

template <size_t I, typename VariantType> requires TIsVariantSpecialization<typename TRemoveCVRef<VariantType>::Type>::Value
struct TVariantAlternativeType { using Type = typename TCopyCV<typename TRemoveReference<VariantType>::Type, typename TRemoveCVRef<VariantType>::Type::template TAlternativeType<I>::Type>::Type; };

template <typename T, typename VariantType> requires TIsVariantSpecialization<typename TRemoveCVRef<VariantType>::Type>::Value
struct TVariantAlternativeIndex : VariantType::template TAlternativeIndex<T> { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
