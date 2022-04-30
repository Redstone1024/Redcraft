#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/TypeInfo.h"
#include "Miscellaneous/AssertionMacros.h"

// NOTE: Disable alignment limit warning
#pragma warning(disable : 4359)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

enum class EAnyRepresentation : uint8
{
	Trivial, // Trivial
//	Inline,  // InlineAllocation
	Small,   // Trivial & Inline
	Big,     // HeapAllocation
};

NAMESPACE_PRIVATE_END

inline constexpr size_t ANY_DEFAULT_INLINE_SIZE      = 64 - sizeof(uintptr);
inline constexpr size_t ANY_DEFAULT_INLINE_ALIGNMENT = 16;

template <size_t InlineSize, size_t InlineAlignment = ANY_DEFAULT_INLINE_ALIGNMENT> requires (Memory::IsValidAlignment(InlineAlignment))
struct alignas(InlineAlignment) TAny
{
	constexpr TAny() : TypeInfo(0) { }

	constexpr TAny(FInvalid) : TAny() { }

	FORCEINLINE TAny(const TAny& InValue)
		: TypeInfo(InValue.TypeInfo)
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
			Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
			GetTypeInfo().CopyConstruct(GetAllocation(), InValue.GetAllocation());
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
			HeapAllocation = Memory::Malloc(GetTypeInfo().GetTypeSize(), GetTypeInfo().GetTypeAlignment());
			GetTypeInfo().CopyConstruct(GetAllocation(), InValue.GetAllocation());
			break;
		default: check_no_entry();
		}
	}

	FORCEINLINE TAny(TAny&& InValue)
		: TypeInfo(InValue.TypeInfo)
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
			Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
			GetTypeInfo().MoveConstruct(GetAllocation(), InValue.GetAllocation());
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
			HeapAllocation = InValue.HeapAllocation;
			InValue.TypeInfo = 0;
			break;
		default: check_no_entry();
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
			switch (GetRepresentation())
			{
			case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
			case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
				GetTypeInfo().CopyAssign(GetAllocation(), InValue.GetAllocation());
				break;
			default: check_no_entry();
			}
		}
		else
		{
			ResetImpl();

			TypeInfo = InValue.TypeInfo;

			switch (GetRepresentation())
			{
			case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
				GetTypeInfo().CopyConstruct(GetAllocation(), InValue.GetAllocation());
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
				HeapAllocation = Memory::Malloc(GetTypeInfo().GetTypeSize(), GetTypeInfo().GetTypeAlignment());
				GetTypeInfo().CopyConstruct(GetAllocation(), InValue.GetAllocation());
				break;
			default: check_no_entry();
			}
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
			switch (GetRepresentation())
			{
			case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
				GetTypeInfo().MoveAssign(GetAllocation(), InValue.GetAllocation());
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
				ResetImpl();
				HeapAllocation = InValue.HeapAllocation;
				InValue.TypeInfo = 0;
				break;
			default: check_no_entry();
			}
		}
		else
		{
			ResetImpl();

			TypeInfo = InValue.TypeInfo;

			switch (GetRepresentation())
			{
			case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
				GetTypeInfo().MoveConstruct(GetAllocation(), InValue.GetAllocation());
				break;
			case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
				HeapAllocation = InValue.HeapAllocation;
				InValue.TypeInfo = 0;
				break;
			default: check_no_entry();
			}
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
			GetValue<SelectedType>() = Forward<T>(InValue);
		}
		else
		{
			ResetImpl();
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

	constexpr const FTypeInfo& GetTypeInfo() const { return IsValid() ? *reinterpret_cast<FTypeInfo*>(TypeInfo & ~RepresentationMask) : Typeid(void); }

	constexpr bool           IsValid() const { return TypeInfo != 0; }
	constexpr explicit operator bool() const { return TypeInfo != 0; }

	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == Typeid(T) : false; }

	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetAllocation());  }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetAllocation())); }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetAllocation());  }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value && TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetAllocation())); }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	
	template <typename T> requires TIsSame<T, typename TDecay<T>::Type>::Value&& TIsObject<typename TDecay<T>::Type>::Value && (!TIsArray<typename TDecay<T>::Type>::Value) && TIsDestructible<typename TDecay<T>::Type>::Value
	constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	FORCEINLINE void Reset()
	{
		ResetImpl();
		TypeInfo = 0;
	}

	FORCEINLINE size_t GetTypeHash() const
	{
		if (!IsValid()) return 20090007;
		return HashCombine(GetTypeInfo().GetTypeHash(), GetTypeInfo().HashItem(GetAllocation()));
	}

	FORCEINLINE void Swap(TAny& InValue)
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

		if (GetTypeInfo() == InValue.GetTypeInfo())
		{
			GetTypeInfo().SwapItem(GetAllocation(), InValue.GetAllocation());
			return;
		}
		
		TAny Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:

	static constexpr uintptr_t RepresentationMask = 3;

	union
	{
		TAlignedStorage<InlineSize, 1>::Type InlineAllocation;
		void* HeapAllocation;
	};

	uintptr TypeInfo;

	constexpr NAMESPACE_PRIVATE::EAnyRepresentation GetRepresentation() const { return static_cast<NAMESPACE_PRIVATE::EAnyRepresentation>(TypeInfo & RepresentationMask); }

	constexpr       void* GetAllocation()       { return GetRepresentation() == NAMESPACE_PRIVATE::EAnyRepresentation::Trivial || GetRepresentation() == NAMESPACE_PRIVATE::EAnyRepresentation::Small ? &InlineAllocation : HeapAllocation; }
	constexpr const void* GetAllocation() const { return GetRepresentation() == NAMESPACE_PRIVATE::EAnyRepresentation::Trivial || GetRepresentation() == NAMESPACE_PRIVATE::EAnyRepresentation::Small ? &InlineAllocation : HeapAllocation; }

	template <typename SelectedType, typename... Types>
	FORCEINLINE void EmplaceImpl(Types&&... Args)
	{
		TypeInfo = reinterpret_cast<uintptr>(&Typeid(SelectedType));

		constexpr bool bIsInlineStorable = sizeof(SelectedType) <= InlineSize && alignof(SelectedType) <= InlineAlignment;
		constexpr bool bIsTriviallyStorable = bIsInlineStorable && TIsTrivial<SelectedType>::Value && TIsTriviallyCopyable<SelectedType>::Value;

		if constexpr (bIsTriviallyStorable)
		{
			new(&InlineAllocation) SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(NAMESPACE_PRIVATE::EAnyRepresentation::Trivial);
		}
		else if constexpr (bIsInlineStorable)
		{
			new(&InlineAllocation) SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(NAMESPACE_PRIVATE::EAnyRepresentation::Small);
		}
		else
		{
			HeapAllocation = new SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(NAMESPACE_PRIVATE::EAnyRepresentation::Big);
		}
	}

	FORCEINLINE void ResetImpl()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case NAMESPACE_PRIVATE::EAnyRepresentation::Trivial:
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Small:
			GetTypeInfo().Destroy(GetAllocation());
			break;
		case NAMESPACE_PRIVATE::EAnyRepresentation::Big:
			GetTypeInfo().Destroy(GetAllocation());
			Memory::Free(HeapAllocation);
			break;
		default: check_no_entry();
		}
	}

	friend FORCEINLINE bool operator==(const TAny& LHS, const TAny& RHS)
	{
		if (LHS.GetTypeInfo() != RHS.GetTypeInfo()) return false;
		if (LHS.IsValid() == false) return true;
		return LHS.GetTypeInfo().EqualityCompare(LHS.GetAllocation(), RHS.GetAllocation());
	}

	friend FORCEINLINE partial_ordering operator<=>(const TAny& LHS, const TAny& RHS)
	{
		if (LHS.GetTypeInfo() != RHS.GetTypeInfo()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return LHS.GetTypeInfo().SynthThreeWayCompare(LHS.GetAllocation(), RHS.GetAllocation());;
	}

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
