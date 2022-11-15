#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Memory/MemoryOperator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// TAny's CustomStorage concept, see FAnyDefaultStorage.
template <typename T>
concept CAnyCustomStorage = 
//	CSameAs<decltype(T::InlineSize),      const size_t> &&
//	CSameAs<decltype(T::InlineAlignment), const size_t> &&
	requires(const T& A)
	{
		{ A.InlineAllocation() } -> CSameAs<const void*>;
		{ A.HeapAllocation()   } -> CSameAs<void*>;
		{ A.TypeInfo()         } -> CSameAs<uintptr>;
	} &&
	requires(T& A)
	{
		{ A.InlineAllocation() } -> CSameAs<void*>;
		{ A.HeapAllocation()   } -> CSameAs<void*&>;
		{ A.TypeInfo()         } -> CSameAs<uintptr&>;
	} &&
	requires(T& A, const T& B, T&& C)
	{
		A.CopyCustom(B);
		A.MoveCustom(MoveTemp(C));
	};

// TAny's default storage structure.
struct alignas(16) FAnyDefaultStorage
{
	// The built-in copy/move operators are disabled and CopyCustom/MoveCustom is used instead of them.

	// You can add custom variables like this.
	//Type Variable;

	//~ Begin CAnyCustomStorage Interface
	inline static constexpr size_t InlineSize      = 64 - sizeof(uintptr);
	inline static constexpr size_t InlineAlignment = 16;
	constexpr       void* InlineAllocation()       { return &InlineAllocationImpl; }
	constexpr const void* InlineAllocation() const { return &InlineAllocationImpl; }
	constexpr void*&      HeapAllocation()         { return HeapAllocationImpl;    }
	constexpr void*       HeapAllocation()   const { return HeapAllocationImpl;    }
	constexpr uintptr&    TypeInfo()               { return TypeInfoImpl;          }
	constexpr uintptr     TypeInfo()         const { return TypeInfoImpl;          }
	constexpr void CopyCustom(const FAnyDefaultStorage&  InValue) { /* Variable =          InValue.Variable;  */ } // You just need to copy the custom variables.
	constexpr void MoveCustom(      FAnyDefaultStorage&& InValue) { /* Variable = MoveTemp(InValue.Variable); */ } // You just need to move the custom variables.
	//~ End CAnyCustomStorage Interface

	union
	{
		uint8 InlineAllocationImpl[InlineSize];
		void* HeapAllocationImpl;
	};

	uintptr TypeInfoImpl;

};

static_assert(CAnyCustomStorage<FAnyDefaultStorage>);

// You can add custom storage area through CustomStorage, such as TFunction.
// It is not recommended to use this, FAny is recommended.
template <CAnyCustomStorage CustomStorage = FAnyDefaultStorage>
class TAny
{
public:

	inline static constexpr size_t InlineSize      = CustomStorage::InlineSize;
	inline static constexpr size_t InlineAlignment = CustomStorage::InlineAlignment;

	constexpr TAny() { Storage.TypeInfo() = 0; }

	constexpr TAny(FInvalid) : TAny() { }
	
	FORCEINLINE TAny(const TAny& InValue)
	{
		Storage.CopyCustom(InValue.Storage);

		Storage.TypeInfo() = InValue.Storage.TypeInfo();

		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Trivial:
			Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
			break;
		case ERepresentation::Small:
			GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
			break;
		case ERepresentation::Big:
			Storage.HeapAllocation() = Memory::Malloc(GetTypeInfoImpl().TypeSize, GetTypeInfoImpl().TypeAlignment);
			GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
			break;
		default: check_no_entry();
		}
	}

	FORCEINLINE TAny(TAny&& InValue)
	{
		Storage.MoveCustom(MoveTemp(InValue.Storage));

		Storage.TypeInfo() = InValue.Storage.TypeInfo();

		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Trivial:
			Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
			break;
		case ERepresentation::Small:
			GetTypeInfoImpl().MoveConstructImpl(GetAllocation(), InValue.GetAllocation());
			break;
		case ERepresentation::Big:
			Storage.HeapAllocation() = InValue.Storage.HeapAllocation();
			InValue.Storage.TypeInfo() = 0;
			break;
		default: check_no_entry();
		}
	}

	template <typename T, typename... Types> requires CDestructible<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, Types&&...>
	FORCEINLINE explicit TAny(TInPlaceType<T>, Types&&... Args)
	{
		using SelectedType = TDecay<T>;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
	}

	template <typename T> requires (!CSameAs<TDecay<T>, TAny>) && (!CTInPlaceType<TDecay<T>>)
		&& CDestructible<TDecay<T>> && CConstructibleFrom<TDecay<T>, T&&>
	FORCEINLINE TAny(T&& InValue) : TAny(InPlaceType<TDecay<T>>, Forward<T>(InValue))
	{ }

	FORCEINLINE ~TAny()
	{
		ResetImpl();
	}

	FORCEINLINE TAny& operator=(const TAny& InValue)
	{
		if (&InValue == this) return *this;

		Storage.CopyCustom(InValue.Storage);

		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (GetTypeInfo() == InValue.GetTypeInfo())
		{
			switch (GetRepresentation())
			{
			case ERepresentation::Trivial:
				Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
				break;
			case ERepresentation::Small:
			case ERepresentation::Big:
				GetTypeInfoImpl().CopyAssignImpl(GetAllocation(), InValue.GetAllocation());
				break;
			default: check_no_entry();
			}
		}
		else
		{
			ResetImpl();

			Storage.TypeInfo() = InValue.Storage.TypeInfo();

			switch (GetRepresentation())
			{
			case ERepresentation::Trivial:
				Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
				break;
			case ERepresentation::Small:
				GetTypeInfoImpl().CopyConstructImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case ERepresentation::Big:
				Storage.HeapAllocation() = Memory::Malloc(GetTypeInfoImpl().TypeSize, GetTypeInfoImpl().TypeAlignment);
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

		Storage.MoveCustom(MoveTemp(InValue.Storage));

		if (!InValue.IsValid())
		{
			Reset();
		}
		else if (GetTypeInfo() == InValue.GetTypeInfo())
		{
			switch (GetRepresentation())
			{
			case ERepresentation::Trivial:
				Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
				break;
			case ERepresentation::Small:
				GetTypeInfoImpl().MoveAssignImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case ERepresentation::Big:
				ResetImpl();
				Storage.HeapAllocation() = InValue.Storage.HeapAllocation();
				InValue.Storage.TypeInfo() = 0;
				break;
			default: check_no_entry();
			}
		}
		else
		{
			ResetImpl();

			Storage.TypeInfo() = InValue.Storage.TypeInfo();

			switch (GetRepresentation())
			{
			case ERepresentation::Trivial:
				Memory::Memcpy(Storage.InlineAllocation(), InValue.Storage.InlineAllocation(), Storage.InlineSize);
				break;
			case ERepresentation::Small:
				GetTypeInfoImpl().MoveConstructImpl(GetAllocation(), InValue.GetAllocation());
				break;
			case ERepresentation::Big:
				Storage.HeapAllocation() = InValue.Storage.HeapAllocation();
				InValue.Storage.TypeInfo() = 0;
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	template <typename T> requires (!CSameAs<TDecay<T>, TAny>) && (!CTInPlaceType<TDecay<T>>)
		&& CDestructible<TDecay<T>> && CConstructibleFrom<TDecay<T>, T&&>
	FORCEINLINE TAny& operator=(T&& InValue)
	{
		using SelectedType = TDecay<T>;

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

	template <typename T, typename... Types> requires CDestructible<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, Types&&...>
	FORCEINLINE TDecay<T>& Emplace(Types&&... Args)
	{
		ResetImpl();
		
		using SelectedType = TDecay<T>;
		EmplaceImpl<SelectedType>(Forward<Types>(Args)...);
		return GetValue<SelectedType>();
	}

	constexpr const type_info& GetTypeInfo() const { return IsValid() ? *GetTypeInfoImpl().NativeTypeInfo : typeid(void); }

	constexpr bool           IsValid() const { return Storage.TypeInfo() != 0; }
	constexpr explicit operator bool() const { return Storage.TypeInfo() != 0; }

	template <typename T> constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == typeid(T) : false; }

	template <typename T> requires CDestructible<TDecay<T>>
	constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetAllocation());  }
	
	template <typename T> requires CDestructible<TDecay<T>>
	constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetAllocation())); }
	
	template <typename T> requires CDestructible<TDecay<T>>
	constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetAllocation());  }
	
	template <typename T> requires CDestructible<TDecay<T>>
	constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetAllocation())); }
	
	template <typename T> requires CSameAs<T, TDecay<T>>&& CDestructible<TDecay<T>>
	constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	
	template <typename T> requires CSameAs<T, TDecay<T>>&& CDestructible<TDecay<T>>
	constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	constexpr       CustomStorage& GetCustomStorage()       requires (!CSameAs<CustomStorage, FAnyDefaultStorage>) { return Storage; }
	constexpr const CustomStorage& GetCustomStorage() const requires (!CSameAs<CustomStorage, FAnyDefaultStorage>) { return Storage; }

	FORCEINLINE void Reset()
	{
		ResetImpl();
		Storage.TypeInfo() = 0;
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

	CustomStorage Storage;

	static constexpr uintptr_t RepresentationMask = 3;

	enum class ERepresentation : uint8
	{
		Trivial, // Trivial & Inline
		Small,   // InlineAllocation
		Big,     // HeapAllocation
	};

	struct FTypeInfoImpl
	{
		const type_info* NativeTypeInfo;

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
			
			: NativeTypeInfo (&typeid(T))
			, TypeSize       ( sizeof(T))
			, TypeAlignment  (alignof(T))

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

	constexpr ERepresentation    GetRepresentation() const { return            static_cast<ERepresentation>(Storage.TypeInfo() &  RepresentationMask); }
	constexpr const FTypeInfoImpl& GetTypeInfoImpl() const { return *reinterpret_cast<const FTypeInfoImpl*>(Storage.TypeInfo() & ~RepresentationMask); }

	constexpr       void* GetAllocation()       { return GetRepresentation() == ERepresentation::Trivial || GetRepresentation() == ERepresentation::Small ? Storage.InlineAllocation() : Storage.HeapAllocation(); }
	constexpr const void* GetAllocation() const { return GetRepresentation() == ERepresentation::Trivial || GetRepresentation() == ERepresentation::Small ? Storage.InlineAllocation() : Storage.HeapAllocation(); }

	template <typename SelectedType, typename... Types>
	FORCEINLINE void EmplaceImpl(Types&&... Args)
	{
		static constexpr const FTypeInfoImpl SelectedTypeInfo(InPlaceType<SelectedType>);
		Storage.TypeInfo() = reinterpret_cast<uintptr>(&SelectedTypeInfo);

		constexpr bool bIsInlineStorable = sizeof(SelectedType) <= Storage.InlineSize && alignof(SelectedType) <= Storage.InlineAlignment;
		constexpr bool bIsTriviallyStorable = bIsInlineStorable && CTrivial<SelectedType> && CTriviallyCopyable<SelectedType>;

		if constexpr (bIsTriviallyStorable)
		{
			new(Storage.InlineAllocation()) SelectedType(Forward<Types>(Args)...);
			Storage.TypeInfo() |= static_cast<uintptr>(ERepresentation::Trivial);
		}
		else if constexpr (bIsInlineStorable)
		{
			new(Storage.InlineAllocation()) SelectedType(Forward<Types>(Args)...);
			Storage.TypeInfo() |= static_cast<uintptr>(ERepresentation::Small);
		}
		else
		{
			Storage.HeapAllocation() = new SelectedType(Forward<Types>(Args)...);
			Storage.TypeInfo() |= static_cast<uintptr>(ERepresentation::Big);
		}
	}

	FORCEINLINE void ResetImpl()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Trivial:
			break;
		case ERepresentation::Small:
			GetTypeInfoImpl().DestroyImpl(GetAllocation());
			break;
		case ERepresentation::Big:
			GetTypeInfoImpl().DestroyImpl(GetAllocation());
			Memory::Free(Storage.HeapAllocation());
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

template <typename T, CAnyCustomStorage StorageType>
constexpr bool operator==(const TAny<StorageType>& LHS, const T& RHS)
{
	return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
}

template <CAnyCustomStorage StorageType>
constexpr bool operator==(const TAny<StorageType>& LHS, FInvalid)
{
	return !LHS.IsValid();
}

NAMESPACE_PRIVATE_BEGIN

template <typename T>                    struct TIsTAny                    : FFalse { };
template <CAnyCustomStorage StorageType> struct TIsTAny<TAny<StorageType>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T>
concept CTAny = NAMESPACE_PRIVATE::TIsTAny<T>::Value;

using FAny = TAny<>;

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
