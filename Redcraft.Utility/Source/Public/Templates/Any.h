#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/TypeInfo.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T>
void* AnyCopyNew(const void* Source)
{
	if constexpr (!TIsCopyConstructible<T>::Value) check_no_entry();
	else return new T(*reinterpret_cast<const T*>(Source));
	return nullptr;
}

using FAnyCopyNewFunc = void* (*)(const void*);

template <typename T>
void* AnyMoveNew(void* Source)
{
	if constexpr (!TIsMoveConstructible<T>::Value) check_no_entry();
	else return new T(MoveTemp(*reinterpret_cast<T*>(Source)));
	return nullptr;
}

using FAnyMoveNewFunc = void* (*)(void*);

template <typename T>
void AnyDelete(void* InValue)
{
	delete reinterpret_cast<T*>(InValue);
}

using FAnyDeleteFunc = void(*)(void*);

template <typename T>
void AnyDestroy(void* InValue)
{
	if constexpr (!TIsTriviallyDestructible<T>::Value)
	{
		typedef T DestructOptionalType;
		reinterpret_cast<T*>(InValue)->DestructOptionalType::~DestructOptionalType();
	}
}

using FAnyDestroyFunc = void(*)(void*);

template <typename T>
void AnyCopyConstruct(void* Target, const void* Source)
{
	if constexpr (!TIsCopyConstructible<T>::Value) check_no_entry();
	else new(reinterpret_cast<T*>(Target)) T(*reinterpret_cast<const T*>(Source));
}

using FAnyCopyConstructFunc = void(*)(void*, const void*);

template <typename T>
void AnyMoveConstruct(void* Target, void* Source)
{
	if constexpr (!TIsMoveConstructible<T>::Value) check_no_entry();
	else new(reinterpret_cast<T*>(Target)) T(MoveTemp(*reinterpret_cast<T*>(Source)));
}

using FAnyMoveConstructFunc = void(*)(void*, void*);

template <typename T>
void AnyCopyAssign(void* Target, const void* Source)
{
	if constexpr (TIsCopyAssignable<T>::Value) *reinterpret_cast<T*>(Target) = *reinterpret_cast<const T*>(Source);
	else if constexpr (TIsCopyConstructible<T>::Value) { AnyDestroy<T>(Target); AnyCopyConstruct<T>(Target, Source); }
	else check_no_entry();
}

using FAnyCopyAssignFunc = void(*)(void*, const void*);

template <typename T>
void AnyMoveAssign(void* Target, void* Source)
{
	if constexpr (TIsMoveAssignable<T>::Value) *reinterpret_cast<T*>(Target) = MoveTemp(*reinterpret_cast<T*>(Source));
	else if constexpr (TIsMoveConstructible<T>::Value) { AnyDestroy<T>(Target); AnyMoveConstruct<T>(Target, Source); }
	else check_no_entry();
}

using FAnyMoveAssignFunc = void(*)(void*, void*);

//template <typename T>
//constexpr bool AnyEqualityOperator(const void* LHS, const void* RHS)
//{
//	if constexpr (!CEqualityComparable<T>) check_no_entry();
//	else return *reinterpret_cast<const T*>(LHS) == *reinterpret_cast<const T*>(RHS);
//	return false;
//}
//
//using FAnyEqualityOperatorFunc = bool(*)(const void*, const void*);
//
//template <typename T>
//void AnySwap(void* A, void* B)
//{
//	if constexpr (TIsSwappable<T>::Value) Swap(*reinterpret_cast<T*>(A), *reinterpret_cast<T*>(B));
//	else check_no_entry();
//}
//
//using FAnySwapFunc = void(*)(void*, void*);
//
//template <typename T>
//size_t AnyTypeHash(const void* InValue)
//{
//	if constexpr (CHashable<T>) return GetTypeHash(*reinterpret_cast<const T*>(InValue));
//	else return 3516520171;
//}
//
//using FAnyTypeHashFunc = size_t(*)(const void*);

struct FAnyRTTI
{
	bool                       bIsInline;
	FAnyCopyNewFunc            CopyNew;
	FAnyMoveNewFunc            MoveNew;
	FAnyDeleteFunc             Delete;
	FAnyDestroyFunc            Destroy;
	FAnyCopyConstructFunc      CopyConstruct;
	FAnyMoveConstructFunc      MoveConstruct;
	FAnyCopyAssignFunc         CopyAssign;
	FAnyMoveAssignFunc         MoveAssign;
//	FAnyEqualityOperatorFunc   EqualityOperator;
//	FAnySwapFunc               SwapObject;
//	FAnyTypeHashFunc           TypeHash;
};

template <typename T, bool bInIsInline>
struct TAnyRTTIHelper
{
	static constexpr FAnyRTTI RTTI =
	{
		bInIsInline,
		AnyCopyNew<T>,
		AnyMoveNew<T>,
		AnyDelete<T>,
		AnyDestroy<T>,
		AnyCopyConstruct<T>,
		AnyMoveConstruct<T>,
		AnyCopyAssign<T>,
		AnyMoveAssign<T>,
//		AnyEqualityOperator<T>,
//		AnySwap<T>,
//		AnyTypeHash<T>,
	};
};

NAMESPACE_PRIVATE_END

inline constexpr size_t ANY_DEFAULT_INLINE_SIZE      = 48;
inline constexpr size_t ANY_DEFAULT_INLINE_ALIGNMENT = 16;

template <size_t InlineSize, size_t InlineAlignment = ANY_DEFAULT_INLINE_ALIGNMENT>
struct TAny
{
	template <typename T>
	struct TIsInlineStorable : TBoolConstant<sizeof(T) <= InlineSize && alignof(T) <= InlineAlignment> { };

	template <typename T>
	struct TIsTriviallyStorable : TBoolConstant<TIsInlineStorable<T>::Value && TIsTrivial<T>::Value && TIsTriviallyCopyable<T>::Value> { };

	constexpr TAny() : TypeInfo(nullptr), RTTI(nullptr) { }

	constexpr TAny(FInvalid) : TAny() { }

	FORCEINLINE TAny(const TAny& InValue)
		: TypeInfo(InValue.TypeInfo), RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
		else if (IsInline()) RTTI->CopyConstruct(GetData(), InValue.GetData());
		else DynamicValue = RTTI->CopyNew(InValue.GetData());
	}

	FORCEINLINE TAny(TAny&& InValue)
		: TypeInfo(InValue.TypeInfo), RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
		else if (IsInline()) RTTI->MoveConstruct(GetData(), InValue.GetData());
		else
		{
			DynamicValue = InValue.DynamicValue;
			InValue.TypeInfo = nullptr;
		}
	}

	template <typename T, typename... Types> requires TIsObject<typename TDecay<T>::Type>::Value
		&& (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, Types...>::Value
	FORCEINLINE explicit TAny(TInPlaceType<T>, Types&&... Args)
	{
		using SelectedType = typename TDecay<T>::Type;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
	}

	template <typename T> requires (!TIsSame<typename TDecay<T>::Type, TAny>::Value) && (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	FORCEINLINE TAny(T&& InValue) : TAny(InPlaceType<typename TDecay<T>::Type>, Forward<T>(InValue))
	{ }

	FORCEINLINE ~TAny()
	{
		ResetImpl();
	}

	FORCEINLINE TAny& operator=(const TAny& InValue)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (GetTypeInfo() == InValue.GetTypeInfo())
		{
			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else RTTI->CopyAssign(GetData(), InValue.GetData());
		}
		else
		{
			ResetImpl();

			TypeInfo = InValue.TypeInfo;
			RTTI = InValue.RTTI;

			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->CopyConstruct(GetData(), InValue.GetData());
			else DynamicValue = RTTI->CopyNew(InValue.GetData());
		}

		return *this;
	}

	FORCEINLINE TAny& operator=(TAny&& InValue)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (GetTypeInfo() == InValue.GetTypeInfo())
		{
			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->MoveAssign(GetData(), InValue.GetData());
			else
			{
				RTTI->Delete(DynamicValue);
				DynamicValue = InValue.DynamicValue;
				InValue.TypeInfo = nullptr;
			}
		}
		else
		{
			ResetImpl();

			TypeInfo = InValue.TypeInfo;
			RTTI = InValue.RTTI;

			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->MoveConstruct(GetData(), InValue.GetData());
			else DynamicValue = RTTI->MoveNew(InValue.GetData());
		}

		return *this;
	}

	template <typename T> requires (!TIsSame<typename TDecay<T>::Type, TAny>::Value) && (!TIsTInPlaceType<typename TDecay<T>::Type>::Value)
		&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	FORCEINLINE TAny& operator=(T&& InValue)
	{
		using SelectedType = typename TDecay<T>::Type;

		if (HoldsAlternative<SelectedType>())
		{
			if constexpr (TIsTriviallyStorable<SelectedType>::Value)
				Memory::Memcpy(&InlineValue, &InValue, sizeof(SelectedType));
			else GetValue<SelectedType>() = Forward<T>(InValue);
		}
		else
		{
			Reset();
			EmplaceImpl<SelectedType>(Forward<T>(InValue));
		}

		return *this;
	}

	template <typename T, typename... Types> requires TIsObject<typename TDecay<T>::Type>::Value
		&& (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	FORCEINLINE typename TDecay<T>::Type& Emplace(Types&&... Args)
	{
		ResetImpl();
		
		using SelectedType = typename TDecay<T>::Type;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
		return GetValue<SelectedType>();
	}

	constexpr const FTypeInfo& GetTypeInfo() const { return TypeInfo != nullptr ? *TypeInfo : Typeid(void); }

	constexpr bool           IsValid() const { return TypeInfo != nullptr;                      }
	constexpr explicit operator bool() const { return TypeInfo != nullptr;                      }
	constexpr bool          IsInline() const { return RTTI != nullptr ? RTTI->bIsInline : true; }
	constexpr bool         IsTrivial() const { return RTTI == nullptr;                          }

	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == Typeid(T) : false; }

	constexpr       void* GetData()       { return IsInline() ? &InlineValue : DynamicValue; }
	constexpr const void* GetData() const { return IsInline() ? &InlineValue : DynamicValue; }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetData());  }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetData())); }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetData());  }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetData())); }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	FORCEINLINE void Reset()
	{
		TypeInfo = nullptr;
		ResetImpl();
	}

//	constexpr size_t GetTypeHash() const
//	{
//		if (!IsValid()) return 20090007;
//		return HashCombine(NAMESPACE_REDCRAFT::GetTypeHash(GetTypeInfo()), RTTI->TypeHash(GetData()));
//	}
//
//	constexpr void Swap(TAny& InValue)
//	{
//		if (!IsValid() && !InValue.IsValid()) return;
//
//		if (IsValid() && !InValue.IsValid())
//		{
//			InValue = MoveTemp(*this);
//			Reset();
//			return;
//		}
//
//		if (InValue.IsValid() && !IsValid())
//		{
//			*this = MoveTemp(InValue);
//			InValue.Reset();
//			return;
//		}
//
//		if (GetTypeInfo() == InValue.GetTypeInfo())
//		{
//			RTTI->SwapObject(GetData(), InValue.GetData());
//			return;
//		}
//		
//		TAny Temp = MoveTemp(*this);
//		*this = MoveTemp(InValue);
//		InValue = MoveTemp(Temp);
//	}

private:

	union
	{
		TAlignedStorage<InlineSize, InlineAlignment>::Type InlineValue;
		void* DynamicValue;
	};

	const FTypeInfo* TypeInfo;
	const NAMESPACE_PRIVATE::FAnyRTTI* RTTI;

	template <typename SelectedType, typename... Types>
	FORCEINLINE void EmplaceImpl(Types&&... Args)
	{
		TypeInfo = &Typeid(SelectedType);

		if constexpr (TIsTriviallyStorable<SelectedType>::Value)
		{
			new(&InlineValue) SelectedType(Forward<Types>(Args)...);
			RTTI = nullptr;
		}
		else if constexpr (TIsInlineStorable<SelectedType>::Value)
		{
			new(&InlineValue) SelectedType(Forward<Types>(Args)...);
			RTTI = &NAMESPACE_PRIVATE::TAnyRTTIHelper<SelectedType, true>::RTTI;
		}
		else
		{
			DynamicValue = new SelectedType(Forward<Types>(Args)...);
			RTTI = &NAMESPACE_PRIVATE::TAnyRTTIHelper<SelectedType, false>::RTTI;
		}
	}

	FORCEINLINE void ResetImpl()
	{
		if (!IsValid() || IsTrivial()) return;
		else if (IsInline()) RTTI->Destroy(&InlineValue);
		else RTTI->Delete(DynamicValue);
	}

//	friend FORCEINLINE bool operator==(const TAny& LHS, const TAny& RHS)
//	{
//		if (LHS.GetTypeInfo() != RHS.GetTypeInfo()) return false;
//		if (LHS.IsValid() == false) return true;
//		return LHS.RTTI->EqualityOperator(LHS.GetData(), RHS.GetData());
//	}

};

template <typename T, size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(const TAny<InlineSize, InlineAlignment>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(const TAny<InlineSize, InlineAlignment>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <typename T>                                struct TIsTAny                                    : FFalse { };
template <size_t InlineSize, size_t InlineAlignment> struct TIsTAny<TAny<InlineSize, InlineAlignment>> : FTrue  { };

using FAny = TAny<ANY_DEFAULT_INLINE_SIZE>;

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
