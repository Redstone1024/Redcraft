#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/MemoryOperator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

// NOTE: Disable alignment limit warning
#pragma warning(disable : 4359)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

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
		case EAnyRepresentation::Trivial:
			Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
			break;
		case EAnyRepresentation::Small:
			GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
			break;
		case EAnyRepresentation::Big:
			HeapAllocation = Memory::Malloc(GetTypeInfoImpl().TypeSize, GetTypeInfoImpl().TypeAlignment);
			GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
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
		case EAnyRepresentation::Trivial:
			Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
			break;
		case EAnyRepresentation::Small:
			GetTypeInfoImpl().MoveConstructImpl(GetAllocation(), InValue.GetAllocation());
			break;
		case EAnyRepresentation::Big:
			HeapAllocation = InValue.HeapAllocation;
			InValue.TypeInfo = 0;
			break;
		default: check_no_entry();
		}
	}

	template <typename T, typename... Types> requires CDestructible<typename TDecay<T>::Type>
		&& CConstructibleFrom<typename TDecay<T>::Type, Types...>
	FORCEINLINE explicit TAny(TInPlaceType<T>, Types&&... Args)
	{
		using SelectedType = typename TDecay<T>::Type;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
	}

	template <typename T> requires (!CSameAs<typename TDecay<T>::Type, TAny>) && (!CTInPlaceType<typename TDecay<T>::Type>)
		&& CDestructible<typename TDecay<T>::Type> && CConstructibleFrom<typename TDecay<T>::Type, T&&>
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
			case EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case EAnyRepresentation::Small:
			case EAnyRepresentation::Big:
				GetTypeInfoImpl().CopyAssignImpl(GetAllocation(), InValue.GetAllocation());
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
			case EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case EAnyRepresentation::Small:
				GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case EAnyRepresentation::Big:
				HeapAllocation = Memory::Malloc(GetTypeInfoImpl().TypeSize, GetTypeInfoImpl().TypeAlignment);
				GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
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
			case EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case EAnyRepresentation::Small:
				GetTypeInfoImpl().MoveAssignImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case EAnyRepresentation::Big:
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
			case EAnyRepresentation::Trivial:
				Memory::Memcpy(InlineAllocation, InValue.InlineAllocation);
				break;
			case EAnyRepresentation::Small:
				GetTypeInfoImpl().MoveConstructImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case EAnyRepresentation::Big:
				HeapAllocation = InValue.HeapAllocation;
				InValue.TypeInfo = 0;
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	template <typename T> requires (!CSameAs<typename TDecay<T>::Type, TAny>) && (!CTInPlaceType<typename TDecay<T>::Type>)
		&& CDestructible<typename TDecay<T>::Type> && CConstructibleFrom<typename TDecay<T>::Type, T&&>
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

	template <typename T, typename... Types> requires CDestructible<typename TDecay<T>::Type>
		&& CConstructibleFrom<typename TDecay<T>::Type, T&&>
	FORCEINLINE typename TDecay<T>::Type& Emplace(Types&&... Args)
	{
		ResetImpl();
		
		using SelectedType = typename TDecay<T>::Type;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
		return GetValue<SelectedType>();
	}

	constexpr const type_info& GetTypeInfo() const { return IsValid() ? *GetTypeInfoImpl().TypeInfo : typeid(void); }

	constexpr bool           IsValid() const { return TypeInfo != 0; }
	constexpr explicit operator bool() const { return TypeInfo != 0; }

	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == typeid(T) : false; }

	template <typename T> requires CDestructible<typename TDecay<T>::Type>
	constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetAllocation());  }
	
	template <typename T> requires CDestructible<typename TDecay<T>::Type>
	constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetAllocation())); }
	
	template <typename T> requires CDestructible<typename TDecay<T>::Type>
	constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetAllocation());  }
	
	template <typename T> requires CDestructible<typename TDecay<T>::Type>
	constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetAllocation())); }
	
	template <typename T> requires CSameAs<T, typename TDecay<T>::Type>&& CDestructible<typename TDecay<T>::Type>
	constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	
	template <typename T> requires CSameAs<T, typename TDecay<T>::Type>&& CDestructible<typename TDecay<T>::Type>
	constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	FORCEINLINE void Reset()
	{
		ResetImpl();
		TypeInfo = 0;
	}

	FORCEINLINE size_t GetTypeHash() const
	{
		using NAMESPACE_REDCRAFT::GetTypeHash;
		if (!IsValid()) return 20090007;
		return HashCombine(GetTypeHash(GetTypeInfo()), GetTypeInfoImpl().HashImpl(GetAllocation()));
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
			GetTypeInfoImpl().SwapImpl(GetAllocation(), InValue.GetAllocation());
			return;
		}
		
		TAny Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:

	static constexpr uintptr_t RepresentationMask = 3;

	enum class EAnyRepresentation : uint8
	{
		Trivial, // Trivial
	//	Inline,  // InlineAllocation
		Small,   // Trivial & Inline
		Big,     // HeapAllocation
	};

	struct FTypeInfoImpl
	{
		const type_info* TypeInfo;

		const size_t TypeSize;
		const size_t TypeAlignment;

		using FCopyConstructImpl = void(*)(void*, const void*);
		using FMoveConstructImpl = void(*)(void*,       void*);
		using FCopyAssignImpl    = void(*)(void*, const void*);
		using FMoveAssignImpl    = void(*)(void*,       void*);
		using FDestroyImpl       = void(*)(void*             );
		
		using FEqualityCompareImpl      = bool             (*)(const void*, const void*);
		using FSynthThreeWayCompareImpl = partial_ordering (*)(const void*, const void*);
		using FHashImpl                 = size_t           (*)(const void*             );
		using FSwapImpl                 = void             (*)(      void*,       void*);
		
		const FCopyConstructImpl CopyConstructImpl;
		const FMoveConstructImpl MoveConstructImpl;
		const FCopyAssignImpl    CopyAssignImpl;
		const FMoveAssignImpl    MoveAssignImpl;
		const FDestroyImpl       DestroyImpl;

		const FEqualityCompareImpl      EqualityCompareImpl;
		const FSynthThreeWayCompareImpl SynthThreeWayCompareImpl;
		const FHashImpl                 HashImpl;
		const FSwapImpl                 SwapImpl;

		template <typename T>
		constexpr FTypeInfoImpl(TInPlaceType<T>)
			
			: TypeInfo      (&typeid(T))
			, TypeSize      ( sizeof(T))
			, TypeAlignment (alignof(T))

			, CopyConstructImpl ([](void* A, const void* B) { if constexpr (requires(T* A, const T* B) { Memory::CopyConstruct (A, B); }) Memory::CopyConstruct (reinterpret_cast<T*>(A), reinterpret_cast<const T*>(B)); else checkf(false, TEXT("The type '%s' is not copy constructible."), typeid(Types).name()); })
			, MoveConstructImpl ([](void* A,       void* B) { if constexpr (requires(T* A,       T* B) { Memory::MoveConstruct (A, B); }) Memory::MoveConstruct (reinterpret_cast<T*>(A), reinterpret_cast<      T*>(B)); else checkf(false, TEXT("The type '%s' is not move constructible."), typeid(Types).name()); })
			, CopyAssignImpl    ([](void* A, const void* B) { if constexpr (requires(T* A, const T* B) { Memory::CopyAssign    (A, B); }) Memory::CopyAssign    (reinterpret_cast<T*>(A), reinterpret_cast<const T*>(B)); else checkf(false, TEXT("The type '%s' is not copy assignable."),    typeid(Types).name()); })
			, MoveAssignImpl    ([](void* A,       void* B) { if constexpr (requires(T* A,       T* B) { Memory::MoveAssign    (A, B); }) Memory::MoveAssign    (reinterpret_cast<T*>(A), reinterpret_cast<      T*>(B)); else checkf(false, TEXT("The type '%s' is not move assignable."),    typeid(Types).name()); })
			, DestroyImpl       ([](void* A               ) { if constexpr (requires(T* A            ) { Memory::Destruct      (A   ); }) Memory::Destruct      (reinterpret_cast<T*>(A)                               ); else checkf(false, TEXT("The type '%s' is not destructible."),       typeid(Types).name()); })
			
			, EqualityCompareImpl      ([](const void* A, const void* B) -> bool             { if constexpr (CEqualityComparable<T>     ) return                                          (*reinterpret_cast<const T*>(A) ==  *reinterpret_cast<const T*>(B)); else checkf(false, TEXT("The type '%s' is not equality comparable."),        typeid(T).name()); return false;                       })
			, SynthThreeWayCompareImpl ([](const void* A, const void* B) -> partial_ordering { if constexpr (CSynthThreeWayComparable<T>) return NAMESPACE_REDCRAFT::SynthThreeWayCompare (*reinterpret_cast<const T*>(A),    *reinterpret_cast<const T*>(B)); else checkf(false, TEXT("The type '%s' is not synth three-way comparable."), typeid(T).name()); return partial_ordering::unordered; })
			, HashImpl                 ([](const void* A               ) -> size_t           { if constexpr (CHashable<T>               ) return NAMESPACE_REDCRAFT::GetTypeHash          (*reinterpret_cast<const T*>(A)                                   ); else checkf(false, TEXT("The type '%s' is not hashable."),                   typeid(T).name()); return 1080551797;                  })
			, SwapImpl                 ([](      void* A,       void* B) -> void             { if constexpr (CSwappable<T>              )        NAMESPACE_REDCRAFT::Swap                 (*reinterpret_cast<      T*>(A),    *reinterpret_cast<      T*>(B)); else checkf(false, TEXT("The type '%s' is not swappable."),                  typeid(T).name());                                     })
	
		{ }
	};

	union
	{
		TAlignedStorage<InlineSize, 1>::Type InlineAllocation;
		void* HeapAllocation;
	};

	uintptr TypeInfo;

	constexpr EAnyRepresentation   GetRepresentation() const { return         static_cast<EAnyRepresentation>(TypeInfo &  RepresentationMask); }
	constexpr const FTypeInfoImpl& GetTypeInfoImpl()   const { return *reinterpret_cast<const FTypeInfoImpl*>(TypeInfo & ~RepresentationMask); }

	constexpr       void* GetAllocation()       { return GetRepresentation() == EAnyRepresentation::Trivial || GetRepresentation() == EAnyRepresentation::Small ? &InlineAllocation : HeapAllocation; }
	constexpr const void* GetAllocation() const { return GetRepresentation() == EAnyRepresentation::Trivial || GetRepresentation() == EAnyRepresentation::Small ? &InlineAllocation : HeapAllocation; }

	template <typename SelectedType, typename... Types>
	FORCEINLINE void EmplaceImpl(Types&&... Args)
	{
		static constexpr const FTypeInfoImpl SelectedTypeInfo(InPlaceType<SelectedType>);
		TypeInfo = reinterpret_cast<uintptr>(&SelectedTypeInfo);

		constexpr bool bIsInlineStorable = sizeof(SelectedType) <= InlineSize && alignof(SelectedType) <= InlineAlignment;
		constexpr bool bIsTriviallyStorable = bIsInlineStorable && CTrivial<SelectedType> && CTriviallyCopyable<SelectedType>;

		if constexpr (bIsTriviallyStorable)
		{
			new(&InlineAllocation) SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(EAnyRepresentation::Trivial);
		}
		else if constexpr (bIsInlineStorable)
		{
			new(&InlineAllocation) SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(EAnyRepresentation::Small);
		}
		else
		{
			HeapAllocation = new SelectedType(Forward<Types>(Args)...);
			TypeInfo |= static_cast<uintptr>(EAnyRepresentation::Big);
		}
	}

	FORCEINLINE void ResetImpl()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case EAnyRepresentation::Trivial:
			break;
		case EAnyRepresentation::Small:
			GetTypeInfoImpl().DestroyImpl(GetAllocation());
			break;
		case EAnyRepresentation::Big:
			GetTypeInfoImpl().DestroyImpl(GetAllocation());
			Memory::Free(HeapAllocation);
			break;
		default: check_no_entry();
		}
	}

	friend FORCEINLINE bool operator==(const TAny& LHS, const TAny& RHS)
	{
		if (LHS.GetTypeInfo() != RHS.GetTypeInfo()) return false;
		if (LHS.IsValid() == false) return true;
		return LHS.GetTypeInfoImpl().EqualityCompareImpl(LHS.GetAllocation(), RHS.GetAllocation());
	}

	friend FORCEINLINE partial_ordering operator<=>(const TAny& LHS, const TAny& RHS)
	{
		if (LHS.GetTypeInfo() != RHS.GetTypeInfo()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;
		return LHS.GetTypeInfoImpl().SynthThreeWayCompareImpl(LHS.GetAllocation(), RHS.GetAllocation());;
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

NAMESPACE_PRIVATE_BEGIN

template <typename T>                                struct TIsTAny                                    : FFalse { };
template <size_t InlineSize, size_t InlineAlignment> struct TIsTAny<TAny<InlineSize, InlineAlignment>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTAny = NAMESPACE_PRIVATE::TIsTAny<T>::Value;

using FAny = TAny<ANY_DEFAULT_INLINE_SIZE>;

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
