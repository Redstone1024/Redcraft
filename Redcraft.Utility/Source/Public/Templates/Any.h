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

// NOTE: In the STL, the assignment operation of the std::any type uses the copy-and-swap idiom
// instead of directly calling the assignment operation of the contained value.
// The purpose of this is as follows: 
//	1) the copy assignment might not exist. 
//	2) the typical case is that the objects are different. 
//	3) it is less exception-safe
// But we don't follow the the copy-and-swap idiom, because we assume that no function throws an exception.

NAMESPACE_PRIVATE_BEGIN

template <typename T>
concept CFAnyPlaceable = CDestructible<TDecay<T>> && CCopyConstructible<TDecay<T>> && CMoveConstructible<TDecay<T>> && CSwappable<TDecay<T>>;

NAMESPACE_PRIVATE_END

class alignas(16) FAny
{
public:

	FORCEINLINE constexpr FAny() { Invalidate(); }

	FORCEINLINE constexpr FAny(FInvalid) : FAny() { }

	FAny(const FAny& InValue)
		: TypeInfo(InValue.TypeInfo)
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
			break;
		case ERepresentation::Trivial:
			Memory::Memcpy(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
			break;
		case ERepresentation::Small:
			SmallStorage.RTTI = InValue.SmallStorage.RTTI;
			SmallStorage.RTTI->CopyConstruct(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
			break;
		case ERepresentation::Big:
			BigStorage.RTTI = InValue.BigStorage.RTTI;
			BigStorage.External = Memory::Malloc(BigStorage.RTTI->TypeSize, BigStorage.RTTI->TypeAlignment);
			BigStorage.RTTI->CopyConstruct(BigStorage.External, InValue.BigStorage.External);
			break;
		default: check_no_entry();
		}
	}

	FAny(FAny&& InValue)
		: TypeInfo(InValue.TypeInfo)
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
			break;
		case ERepresentation::Trivial:
			Memory::Memmove(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
			break;
		case ERepresentation::Small:
			SmallStorage.RTTI = InValue.SmallStorage.RTTI;
			SmallStorage.RTTI->MoveConstruct(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
			break;
		case ERepresentation::Big:
			BigStorage.RTTI = InValue.BigStorage.RTTI;
			BigStorage.External = InValue.BigStorage.External;
			InValue.Invalidate();
			break;
		default: check_no_entry();
		}
	}

	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, Ts&&...>)
	FORCEINLINE explicit FAny(TInPlaceType<T>, Ts&&... Args)
	{
		EmplaceImpl<T>(Forward<Ts>(Args)...);
	}

	template <typename T> requires (!CBaseOf<FAny, TDecay<T>> && !CTInPlaceType<TDecay<T>>
		&& NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, T&&>)
	FORCEINLINE FAny(T&& InValue) : FAny(InPlaceType<TDecay<T>>, Forward<T>(InValue))
	{ }

	FORCEINLINE ~FAny()
	{
		Destroy();
	}

	FAny& operator=(const FAny& InValue)
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
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
				break;
			case ERepresentation::Small:
				SmallStorage.RTTI = InValue.SmallStorage.RTTI;
				SmallStorage.RTTI->CopyAssign(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
				break;
			case ERepresentation::Big:
				BigStorage.RTTI = InValue.BigStorage.RTTI;
				BigStorage.RTTI->CopyAssign(BigStorage.External, InValue.BigStorage.External);
				break;
			default: check_no_entry();
			}
		}
		else
		{
			Destroy();

			TypeInfo = InValue.TypeInfo;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
				break;
			case ERepresentation::Small:
				SmallStorage.RTTI = InValue.SmallStorage.RTTI;
				SmallStorage.RTTI->CopyConstruct(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
				break;
			case ERepresentation::Big:
				BigStorage.RTTI = InValue.BigStorage.RTTI;
				BigStorage.External = Memory::Malloc(BigStorage.RTTI->TypeSize, BigStorage.RTTI->TypeAlignment);
				BigStorage.RTTI->CopyConstruct(BigStorage.External, InValue.BigStorage.External);
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	FAny& operator=(FAny&& InValue)
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
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memmove(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
				break;
			case ERepresentation::Small:
				SmallStorage.RTTI = InValue.SmallStorage.RTTI;
				SmallStorage.RTTI->MoveAssign(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
				break;
			case ERepresentation::Big:
				Destroy();
				BigStorage.RTTI = InValue.BigStorage.RTTI;
				BigStorage.External = InValue.BigStorage.External;
				InValue.Invalidate();
				break;
			default: check_no_entry();
			}
		}
		else
		{
			Destroy();

			TypeInfo = InValue.TypeInfo;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memmove(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
				break;
			case ERepresentation::Small:
				SmallStorage.RTTI = InValue.SmallStorage.RTTI;
				SmallStorage.RTTI->MoveConstruct(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
				break;
			case ERepresentation::Big:
				BigStorage.RTTI = InValue.BigStorage.RTTI;
				BigStorage.External = InValue.BigStorage.External;
				InValue.Invalidate();
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	template <typename T> requires (!CBaseOf<FAny, TDecay<T>> && !CTInPlaceType<TDecay<T>>
		&& NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, T&&>)
	FORCEINLINE FAny& operator=(T&& InValue)
	{
		using DecayedType = TDecay<T>;

		if (HoldsAlternative<DecayedType>())
		{
			GetValue<DecayedType>() = Forward<T>(InValue);
		}
		else
		{
			Destroy();
			EmplaceImpl<DecayedType>(Forward<T>(InValue));
		}

		return *this;
	}

	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, Ts&&...>)
	FORCEINLINE TDecay<T>& Emplace(Ts&&... Args)
	{
		Destroy();
		EmplaceImpl<T>(Forward<Ts>(Args)...);
		return GetValue<TDecay<T>>();
	}

	FORCEINLINE constexpr const type_info& GetTypeInfo() const { return IsValid() ? GetTypeInfoImpl() : typeid(void); }

	FORCEINLINE constexpr bool           IsValid() const { return TypeInfo != 0; }
	FORCEINLINE constexpr explicit operator bool() const { return TypeInfo != 0; }

	template <typename T> FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == typeid(T) : false; }

	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetStorage());  }
	
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetStorage())); }
	
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetStorage());  }
	
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetStorage())); }
	
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	FORCEINLINE constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	FORCEINLINE void Reset()
	{
		Destroy();
		Invalidate();
	}
	
	void Swap(FAny& InValue)
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
			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				uint8 Buffer[sizeof(TrivialStorage.Internal)];
				Memory::Memmove(Buffer, TrivialStorage.Internal);
				Memory::Memmove(TrivialStorage.Internal, InValue.TrivialStorage.Internal);
				Memory::Memmove(InValue.TrivialStorage.Internal, Buffer);
				break;
			case ERepresentation::Small:
				SmallStorage.RTTI->SwapObject(&SmallStorage.Internal, &InValue.SmallStorage.Internal);
				break;
			case ERepresentation::Big:
				NAMESPACE_REDCRAFT::Swap(BigStorage.External, InValue.BigStorage.External);
				break;
			default: check_no_entry();
			}

			return;
		}
		
		FAny Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:

	struct FRTTI
	{
		const size_t TypeSize;
		const size_t TypeAlignment;

		using FCopyConstruct = void(*)(void*, const void*);
		using FMoveConstruct = void(*)(void*,       void*);
		using FCopyAssign    = void(*)(void*, const void*);
		using FMoveAssign    = void(*)(void*,       void*);
		using FDestruct      = void(*)(void*             );
		using FSwapObject    = void(*)(void*,       void*);

		const FCopyConstruct CopyConstruct;
		const FMoveConstruct MoveConstruct;
		const FCopyAssign    CopyAssign;
		const FMoveAssign    MoveAssign;
		const FDestruct      Destruct;
		const FSwapObject    SwapObject;

		template <typename T>
		FORCEINLINE constexpr FRTTI(TInPlaceType<T>)
			: TypeSize( sizeof(T)), TypeAlignment(alignof(T))
			, CopyConstruct(
				[](void* A, const void* B)
				{
					new (A) T(*reinterpret_cast<const T*>(B));
				}
			)
			, MoveConstruct(
				[](void* A, void* B)
				{
					new (A) T(MoveTemp(*reinterpret_cast<T*>(B)));
				}
			)
			, CopyAssign(
				[](void* A, const void* B)
				{
					if constexpr (CCopyAssignable<T>)
					{
						*reinterpret_cast<T*>(A) = *reinterpret_cast<const T*>(B);
					}
					else
					{
						reinterpret_cast<T*>(A)->~T();
						new (A) T(*reinterpret_cast<const T*>(B));
					}
				}
			)
			, MoveAssign(
				[](void* A, void* B)
				{
					if constexpr (CMoveAssignable<T>)
					{
						*reinterpret_cast<T*>(A) = MoveTemp(*reinterpret_cast<T*>(B));
					}
					else
					{
						reinterpret_cast<T*>(A)->~T();
						new (A) T(MoveTemp(*reinterpret_cast<T*>(B)));
					}
				}
			)
			, Destruct(
				[](void* A)
				{
					reinterpret_cast<T*>(A)->~T();
				}
			)
			, SwapObject{
				[](void* A, void* B)
				{
					NAMESPACE_REDCRAFT::Swap(*reinterpret_cast<T*>(A), *reinterpret_cast<T*>(B));
				}
			}
		{ }
	};

	struct FTrivialStorage
	{
		uint8 Internal[64 - sizeof(uintptr)];
	};

	struct FSmallStorage
	{
		uint8 Internal[sizeof(FTrivialStorage) - sizeof(const FRTTI*)];
		const FRTTI* RTTI;
	};

	struct FBigStorage
	{
		uint8 Padding[sizeof(FTrivialStorage) - sizeof(void*) - sizeof(const FRTTI*)];
		void* External;
		const FRTTI* RTTI;
	};

	static_assert(sizeof(FTrivialStorage) == sizeof(FSmallStorage));
	static_assert(sizeof(FTrivialStorage) == sizeof(  FBigStorage));

	static_assert(alignof(type_info) >= 4);

	static constexpr uintptr_t RepresentationMask = 3;

	enum class ERepresentation : uintptr
	{
		Empty   = 0, // EmptyType
		Trivial = 1, // TrivialStorage
		Small   = 2, // SmallStorage
		Big     = 3, // BigStorage
	};

	union
	{
		FTrivialStorage TrivialStorage;
		FSmallStorage   SmallStorage;
		FBigStorage     BigStorage;
	};

	uintptr TypeInfo;

	FORCEINLINE ERepresentation  GetRepresentation() const { return        static_cast<ERepresentation>(TypeInfo &  RepresentationMask); }
	FORCEINLINE const type_info& GetTypeInfoImpl()   const { return *reinterpret_cast<const type_info*>(TypeInfo & ~RepresentationMask); }

	FORCEINLINE void* GetStorage()
	{
		switch (GetRepresentation())
		{
		case ERepresentation::Empty:   return nullptr;
		case ERepresentation::Trivial: return &TrivialStorage.Internal;
		case ERepresentation::Small:   return &SmallStorage.Internal;
		case ERepresentation::Big:     return BigStorage.External;
		default: check_no_entry();     return nullptr;
		}
	}
	
	FORCEINLINE const void* GetStorage() const
	{
		switch (GetRepresentation())
		{
		case ERepresentation::Empty:   return nullptr;
		case ERepresentation::Trivial: return &TrivialStorage.Internal;
		case ERepresentation::Small:   return &SmallStorage.Internal;
		case ERepresentation::Big:     return BigStorage.External;
		default: check_no_entry();     return nullptr;
		}
	}
	
	template <typename T, typename... Ts>
	void EmplaceImpl(Ts&&... Args)
	{
		using DecayedType = TDecay<T>;

		TypeInfo = reinterpret_cast<uintptr>(&typeid(DecayedType));

		if constexpr (CEmpty<DecayedType> && CTrivial<DecayedType>) return; // ERepresentation::Empty

		constexpr bool bIsTriviallyStorable = sizeof(DecayedType) <= sizeof(TrivialStorage.Internal) && alignof(DecayedType) <= alignof(FAny) && CTriviallyCopyable<DecayedType>;
		constexpr bool bIsSmallStorable     = sizeof(DecayedType) <= sizeof(  SmallStorage.Internal) && alignof(DecayedType) <= alignof(FAny);

		static constexpr const FRTTI SelectedRTTI(InPlaceType<DecayedType>);

		if constexpr (bIsTriviallyStorable)
		{
			new (&TrivialStorage.Internal) DecayedType(Forward<Ts>(Args)...);
			TypeInfo |= static_cast<uintptr>(ERepresentation::Trivial);
		}
		else if constexpr (bIsSmallStorable)
		{
			new (&SmallStorage.Internal) DecayedType(Forward<Ts>(Args)...);
			SmallStorage.RTTI = &SelectedRTTI;
			TypeInfo |= static_cast<uintptr>(ERepresentation::Small);
		}
		else
		{
			BigStorage.External = Memory::Malloc(sizeof(DecayedType), alignof(DecayedType));
			new (BigStorage.External) DecayedType(Forward<Ts>(Args)...);
			BigStorage.RTTI = &SelectedRTTI;
			TypeInfo |= static_cast<uintptr>(ERepresentation::Big);
		}
	}

	void Destroy()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
		case ERepresentation::Trivial:
			break;
		case ERepresentation::Small:
			SmallStorage.RTTI->Destruct(&SmallStorage.Internal);
			break;
		case ERepresentation::Big:
			BigStorage.RTTI->Destruct(BigStorage.External);
			Memory::Free(BigStorage.External);
			break;
		default: check_no_entry();
		}
	}

	FORCEINLINE constexpr void Invalidate() { TypeInfo = 0; }

	template <typename T> requires (!CBaseOf<FAny, TRemoveCVRef<T>>)
	friend FORCEINLINE constexpr bool operator==(const FAny& LHS, const T& RHS)
	{
		return LHS.template HoldsAlternative<T>() ? LHS.template GetValue<T>() == RHS : false;
	}

	friend FORCEINLINE constexpr bool operator==(const FAny& LHS, FInvalid)
	{
		return !LHS.IsValid();
	}

};

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

static_assert(alignof(FAny) == 16, "The byte alignment of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
