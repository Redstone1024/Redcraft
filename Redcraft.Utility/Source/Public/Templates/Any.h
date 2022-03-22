#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Templates/Utility.h"
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
	if constexpr (!TIsCopyAssignable<T>::Value) check_no_entry();
	else *reinterpret_cast<T*>(Target) = *reinterpret_cast<const T*>(Source);
}

using FAnyCopyAssignFunc = void(*)(void*, const void*);

template <typename T>
void AnyMoveAssign(void* Target, void* Source)
{
	if constexpr (!TIsMoveAssignable<T>::Value) check_no_entry();
	else *reinterpret_cast<T*>(Target) = MoveTemp(*reinterpret_cast<T*>(Source));
}

using FAnyMoveAssignFunc = void(*)(void*, void*);

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
	};
};

inline constexpr size_t ANY_DEFAULT_INLINE_SIZE = 64 - sizeof(FTypeInfo) - sizeof(const FAnyRTTI*);
inline constexpr size_t ANY_DEFAULT_INLINE_ALIGNMENT = Memory::MINIMUM_ALIGNMENT;

NAMESPACE_PRIVATE_END

template <size_t InlineSize, size_t InlineAlignment = NAMESPACE_PRIVATE::ANY_DEFAULT_INLINE_ALIGNMENT>
struct TAny
{
	template <typename T>
	struct TIsInlineStorable : TBoolConstant<sizeof(T) <= InlineSize && alignof(T) <= InlineAlignment> { };

	template <typename T>
	struct TIsTriviallyStorable : TBoolConstant<TIsInlineStorable<T>::Value && TIsTrivial<T>::Value && TIsTriviallyCopyable<T>::Value> { };

	constexpr TAny() : TypeInfo(Typeid(void)), RTTI(nullptr) { }

	TAny(FInvalid) : TAny() { }

	TAny(const TAny& InValue)
		: TypeInfo(InValue.TypeInfo), RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
		else if (IsInline()) RTTI->CopyConstruct(GetData(), InValue.GetData());
		else DynamicValue = RTTI->CopyNew(InValue.GetData());
	}

	TAny(TAny&& InValue)
		: TypeInfo(InValue.TypeInfo), RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
		else if (IsInline()) RTTI->MoveConstruct(GetData(), InValue.GetData());
		else
		{
			DynamicValue = InValue.DynamicValue;
			InValue.TypeInfo = Typeid(void);
		}
	}

	template <typename T, typename... Types> requires TIsObject<typename TDecay<T>::Type>::Value
		&& (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, Types...>::Value
	explicit TAny(TInPlaceType<T>, Types&&... Args)
		: TypeInfo(Typeid(typename TDecay<T>::Type))
	{
		using SelectedType = typename TDecay<T>::Type;

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

	template <typename T> requires (!TIsSame<typename TDecay<T>::Type, TAny>::Value) && (!TIsInPlaceTypeSpecialization<typename TDecay<T>::Type>::Value)
		&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	TAny(T&& InValue) : TAny(InPlaceType<typename TDecay<T>::Type>, Forward<T>(InValue))
	{ }

	~TAny()
	{
		Reset();
	}

	TAny& operator=(const TAny& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (TypeInfo == InValue.TypeInfo)
		{
			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else RTTI->CopyAssign(GetData(), InValue.GetData());
		}
		else
		{
			Reset();

			TypeInfo = InValue.TypeInfo;
			RTTI = InValue.RTTI;

			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->CopyConstruct(GetData(), InValue.GetData());
			else DynamicValue = RTTI->CopyNew(InValue.GetData());
		}

		return *this;
	}

	TAny& operator=(TAny&& InValue)
	{
		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (TypeInfo == InValue.TypeInfo)
		{
			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->MoveAssign(GetData(), InValue.GetData());
			else
			{
				RTTI->Delete(DynamicValue);
				DynamicValue = InValue.DynamicValue;
				InValue.TypeInfo = Typeid(void);
			}
		}
		else
		{
			Reset();

			TypeInfo = InValue.TypeInfo;
			RTTI = InValue.RTTI;

			if (IsTrivial()) Memory::Memcpy(InlineValue, InValue.InlineValue);
			else if (IsInline()) RTTI->MoveConstruct(GetData(), InValue.GetData());
			else DynamicValue = RTTI->MoveNew(InValue.GetData());
		}

		return *this;
	}

	template <typename T> requires (!TIsSame<typename TDecay<T>::Type, TAny>::Value) && (!TIsInPlaceTypeSpecialization<typename TDecay<T>::Type>::Value)
		&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	TAny& operator=(T&& InValue)
	{
		using SelectedType = typename TDecay<T>::Type;

		if (TypeInfo == Typeid(SelectedType))
		{
			if constexpr (TIsTriviallyStorable<SelectedType>::Value)
				Memory::Memcpy(&InlineValue, &InValue, sizeof(SelectedType));
			else GetValue<SelectedType>() = Forward<T>(InValue);
		}
		else
		{
			Reset();

			TypeInfo = Typeid(SelectedType);

			if constexpr (TIsTriviallyStorable<SelectedType>::Value)
			{
				new(&InlineValue) SelectedType(Forward<T>(InValue));
				RTTI = nullptr;
			}
			else if constexpr (TIsInlineStorable<SelectedType>::Value)
			{
				new(&InlineValue) SelectedType(Forward<T>(InValue));
				RTTI = &NAMESPACE_PRIVATE::TAnyRTTIHelper<SelectedType, true>::RTTI;
			}
			else
			{
				DynamicValue = new SelectedType(Forward<T>(InValue));
				RTTI = &NAMESPACE_PRIVATE::TAnyRTTIHelper<SelectedType, false>::RTTI;
			}
		}

		return *this;
	}

	template <typename T, typename... Types> requires TIsObject<typename TDecay<T>::Type>::Value
		&& (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
		&& TIsConstructible<typename TDecay<T>::Type, T&&>::Value
	typename TDecay<T>::Type& Emplace(Types&&... Args)
	{
		Reset();
		
		using SelectedType = typename TDecay<T>::Type;

		TypeInfo = Typeid(SelectedType);

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

		return GetValue<SelectedType>();
	}

	constexpr FTypeInfo  GetTypeInfo() const { return TypeInfo;                                  }
	constexpr bool           IsValid() const { return TypeInfo != Typeid(void);                  }
	constexpr explicit operator bool() const { return TypeInfo != Typeid(void);                  }
	constexpr bool          IsInline() const { return RTTI != nullptr ? RTTI->bIsInline : true;  }
	constexpr bool         IsTrivial() const { return RTTI == nullptr;                           }

	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? TypeInfo == Typeid(T) : false; }

	constexpr       void* GetData()       { return IsInline() ? &InlineValue : DynamicValue; }
	constexpr const void* GetData() const { return IsInline() ? &InlineValue : DynamicValue; }
	
	template <typename T> constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetData());  }
	template <typename T> constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetData())); }
	template <typename T> constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetData());  }
	template <typename T> constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetData())); }
	
	template <typename T> constexpr T Get(T&& DefaultValue) &&     { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> constexpr T Get(T&& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	void Reset()
	{
		if (!IsValid()) return;

		TypeInfo = Typeid(void);

		if (IsTrivial());
		else if (IsInline()) RTTI->Destroy(&InlineValue);
		else RTTI->Delete(DynamicValue);

		RTTI = nullptr;
	}

private:

	union
	{
		TAlignedStorage<InlineSize, InlineAlignment>::Type InlineValue;
		void* DynamicValue;
	};

	FTypeInfo TypeInfo;
	const NAMESPACE_PRIVATE::FAnyRTTI* RTTI;

};

template <typename T, size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(const TAny<InlineSize, InlineAlignment>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <typename T, size_t InlineSize, size_t InlineAlignment>
constexpr bool operator!=(const TAny<InlineSize, InlineAlignment>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() != RHS : true;
}

template <typename T, size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(const T& LHS, const TAny<InlineSize, InlineAlignment>& RHS)
{
	return RHS.template HoldsAlternative<T>() ? LHS == RHS.template GetValue<T>() : false;
}

template <typename T, size_t InlineSize, size_t InlineAlignment>
constexpr bool operator!=(const T& LHS, const TAny<InlineSize, InlineAlignment>& RHS)
{
	return RHS.template HoldsAlternative<T>() ? LHS != RHS.template GetValue<T>() : true;
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(const TAny<InlineSize, InlineAlignment>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr bool operator!=(const TAny<InlineSize, InlineAlignment>& LHS, FInvalid)
{
	return LHS.IsValid();
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr bool operator==(FInvalid, const TAny<InlineSize, InlineAlignment>& RHS)
{
	return !RHS.IsValid();
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr bool operator!=(FInvalid, const TAny<InlineSize, InlineAlignment>& RHS)
{
	return RHS.IsValid();
}

template <size_t InlineSize, size_t InlineAlignment>
constexpr void Swap(TAny<InlineSize, InlineAlignment>& A, TAny<InlineSize, InlineAlignment>& B)
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

	TAny<InlineSize, InlineAlignment> Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

using FAny = TAny<NAMESPACE_PRIVATE::ANY_DEFAULT_INLINE_SIZE>;

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
