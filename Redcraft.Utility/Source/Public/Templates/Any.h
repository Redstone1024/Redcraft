#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
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
concept CFAnyPlaceable = CDestructible<TDecay<T>> && CCopyConstructible<TDecay<T>> && CMoveConstructible<TDecay<T>>;

NAMESPACE_PRIVATE_END

/**
 * The class any describes a type-safe container for single values of any copy and move constructible type.
 * An object of class any stores an instance of any type that satisfies the constructor requirements or is empty,
 * and this is referred to as the state of the class any object. The stored instance is called the contained object.
 */
class alignas(16) FAny
{
public:

	/** Constructs an empty object. */
	FORCEINLINE constexpr FAny() { Invalidate(); }

	/** Constructs an empty object. */
	FORCEINLINE constexpr FAny(FInvalid) : FAny() { }

	/** Copies content of other into a new instance. This may use the object's copy constructor. */
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

	/** Moves content of other into a new instance. This may use the object's move constructor. */
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

	/** Constructs an object with initial content an object of type TDecay<T>, direct-initialized from Forward<T>(InValue). */
	template <typename T> requires (!CSameAs<FAny, TDecay<T>> && !CTInPlaceType<TDecay<T>>
		&& NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, T&&>)
	FORCEINLINE FAny(T&& InValue) : FAny(InPlaceType<T>, Forward<T>(InValue))
	{ }

	/** Constructs an object with initial content an object of type TDecay<T>, direct-non-list-initialized from Forward<Ts>(Args).... */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, Ts&&...>)
	FORCEINLINE explicit FAny(TInPlaceType<T>, Ts&&... Args)
	{
		EmplaceImpl<T>(Forward<Ts>(Args)...);
	}

	/** Destroys the contained object, if any, as if by a call to Reset(). */
	FORCEINLINE ~FAny()
	{
		Destroy();
	}

	/** Assigns by copying the state of 'InValue'. This may use the object's copy constructor or copy assignment operator. */
	FAny& operator=(const FAny& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

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

	/** Assigns by moving the state of 'InValue'. This may use the object's move constructor or move assignment operator. */
	FAny& operator=(FAny&& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

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

	/** Assigns the type and value of 'InValue'. This may use the object's constructor or assignment operator. */
	template <typename T> requires (!CSameAs<FAny, TDecay<T>> && !CTInPlaceType<TDecay<T>>
		&& NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, T&&>)
	FORCEINLINE FAny& operator=(T&& InValue)
	{
		using DecayedType = TDecay<T>;

		if constexpr (CAssignableFrom<DecayedType, T&&>)
		{
			if (HoldsAlternative<DecayedType>())
			{
				GetValue<DecayedType>() = Forward<T>(InValue);
				return *this;
			}
		}

		Destroy();
		EmplaceImpl<DecayedType>(Forward<T>(InValue));

		return *this;
	}
	
	/** Check if the contained value is equivalent to 'InValue'. */
	template <typename T> requires (!CSameAs<FAny, TRemoveCVRef<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CEqualityComparable<T>)
	NODISCARD FORCEINLINE constexpr bool operator==(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? GetValue<T>() == InValue : false;
	}

	/** Check that the contained value is in ordered relationship with 'InValue'. */
	template <typename T> requires (!CSameAs<FAny, TRemoveCVRef<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CSynthThreeWayComparable<T>)
	NODISCARD FORCEINLINE constexpr partial_ordering operator<=>(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? SynthThreeWayCompare(GetValue<T>(), InValue) : partial_ordering::unordered;
	}

	/** @return true if instance does not contain a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	/**
	 * Changes the contained object to one of type TDecay<T> constructed from the arguments.
	 * First destroys the current contained object (if any) by Reset(), then constructs an object of type
	 * TDecay<T>, direct-non-list-initialized from Forward<Ts>(Args)..., as the contained object.
	 *
	 * @param  Args	- The arguments to be passed to the constructor of the contained object.
	 *
	 * @return A reference to the new contained object.
	 */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::CFAnyPlaceable<T> && CConstructibleFrom<TDecay<T>, Ts&&...>)
	FORCEINLINE TDecay<T>& Emplace(Ts&&... Args)
	{
		Destroy();
		EmplaceImpl<T>(Forward<Ts>(Args)...);
		return GetValue<TDecay<T>>();
	}

	/** @return The typeid of the contained value if instance is non-empty, otherwise typeid(void). */
	NODISCARD FORCEINLINE constexpr const type_info& GetTypeInfo() const { return IsValid() ? GetTypeInfoImpl() : typeid(void); }

	/** @return true if instance contains a value, otherwise false. */
	NODISCARD FORCEINLINE constexpr           bool IsValid() const { return TypeInfo != 0; }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const { return TypeInfo != 0; }

	/** @return true if the any currently holds the alternative 'T', false otherwise. */
	template <typename T> NODISCARD FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetTypeInfo() == typeid(T) : false; }

	/** @return The contained object. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr       T&  GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(GetStorage());  }

	/** @return The contained object. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr       T&& GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(GetStorage())); }

	/** @return The contained object. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr const T&  GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(GetStorage());  }

	/** @return The contained object. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr const T&& GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TAny. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(GetStorage())); }

	/** @return The contained object when HoldsAlternative<T>() returns true, 'DefaultValue' otherwise. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr       T& Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	/** @return The contained object when HoldsAlternative<T>() returns true, 'DefaultValue' otherwise. */
	template <typename T> requires (CSameAs<T, TDecay<T>> && NAMESPACE_PRIVATE::CFAnyPlaceable<T>)
	NODISCARD FORCEINLINE constexpr const T& Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	/** If not empty, destroys the contained object. */
	FORCEINLINE void Reset()
	{
		Destroy();
		Invalidate();
	}
	
	/** Overloads the Swap algorithm for FAny. */
	friend void Swap(FAny& A, FAny& B)
	{
		if (!A.IsValid() && !B.IsValid()) return;
		
		if (A.IsValid() && !B.IsValid())
		{
			B = MoveTemp(A);
			A.Reset();
		}
		else if (!A.IsValid() && B.IsValid())
		{
			A = MoveTemp(B);
			B.Reset();
		}
		else
		{
			FAny Temp = MoveTemp(A);
			A = MoveTemp(B);
			B = MoveTemp(Temp);
		}
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
					if constexpr (CSwappable<T>)
					{
						Swap(*reinterpret_cast<T*>(A), *reinterpret_cast<T*>(B));
					}
					else
					{
						TAlignedStorage<sizeof(T), alignof(T)> TempBuffer;
						new (&TempBuffer) T(MoveTemp(*reinterpret_cast<T*>(A)));
						reinterpret_cast<T*>(A)->~T();
						new (A) T(MoveTemp(*reinterpret_cast<T*>(B)));
						reinterpret_cast<T*>(B)->~T();
						new (B) T(MoveTemp(*reinterpret_cast<T*>(&TempBuffer)));
						reinterpret_cast<T*>(&TempBuffer)->~T();
					}
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

};

static_assert(sizeof(FAny) == 64, "The byte size of FAny is unexpected");

static_assert(alignof(FAny) == 16, "The byte alignment of FAny is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
