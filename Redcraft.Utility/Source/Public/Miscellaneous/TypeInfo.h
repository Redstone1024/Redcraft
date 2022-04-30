#pragma once

#include "CoreTypes.h"
#include "Concepts/Concepts.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"
#include "Memory/MemoryOperator.h"

#include <typeinfo>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FTypeInfo;

NAMESPACE_PRIVATE_BEGIN

struct FTypeInfoStatic
{
	template <typename T>
	static constexpr FTypeInfo Value = { InPlaceType<T> };
};

NAMESPACE_PRIVATE_END

struct FTypeInfo
{
	FTypeInfo()                            = delete; 
	FTypeInfo(FTypeInfo&&)                 = delete;
	FTypeInfo(const FTypeInfo&)            = delete;
	FTypeInfo& operator=(FTypeInfo&&)      = delete;
	FTypeInfo& operator=(const FTypeInfo&) = delete;

	constexpr const std::type_info& GetNative() const { return Native; };

	FORCEINLINE size_t  GetTypeHash() const { return GetNative().hash_code(); }
	FORCEINLINE const char* GetName() const { return GetNative().name();      }

	constexpr size_t GetTypeSize()      const { return TypeSize;      }
	constexpr size_t GetTypeAlignment() const { return TypeAlignment; }

	constexpr bool IsZeroConstructible             () const { return bIsZeroConstructible;             }
	constexpr bool IsBitwiseConstructible          () const { return bIsBitwiseConstructible;          }
	constexpr bool IsBitwiseRelocatable            () const { return bIsBitwiseRelocatable;            }
	constexpr bool IsBitwiseComparable             () const { return bIsBitwiseComparable;             }

	constexpr bool IsArithmetic                    () const { return bIsArithmetic;                    }
	constexpr bool IsFundamental                   () const { return bIsFundamental;                   }
	constexpr bool IsObject                        () const { return bIsObject;                        }
	constexpr bool IsScalar                        () const { return bIsScalar;                        }
	constexpr bool IsCompound                      () const { return bIsCompound;                      }
	constexpr bool IsMemberPointer                 () const { return bIsMemberPointer;                 }

	constexpr bool IsVoid                          () const { return bIsVoid;                          }
	constexpr bool IsNullPointer                   () const { return bIsNullPointer;                   }
	constexpr bool IsIntegral                      () const { return bIsIntegral;                      }
	constexpr bool IsFloatingPoint                 () const { return bIsFloatingPoint;                 }
	constexpr bool IsArray                         () const { return bIsArray;                         }
	constexpr bool IsPointer                       () const { return bIsPointer;                       }
	constexpr bool IsMemberObjectPointer           () const { return bIsMemberObjectPointer;           }
	constexpr bool IsMemberFunctionPointer         () const { return bIsMemberFunctionPointer;         }
	constexpr bool IsEnum                          () const { return bIsEnum;                          }
	constexpr bool IsUnion                         () const { return bIsUnion;                         }
	constexpr bool IsClass                         () const { return bIsClass;                         }
	constexpr bool IsFunction                      () const { return bIsFunction;                      }

	constexpr bool IsDefaultConstructible          () const { return bIsDefaultConstructible;          }
	constexpr bool IsCopyConstructible             () const { return bIsCopyConstructible;             }
	constexpr bool IsMoveConstructible             () const { return bIsMoveConstructible;             }
	constexpr bool IsCopyAssignable                () const { return bIsCopyAssignable;                }
	constexpr bool IsMoveAssignable                () const { return bIsMoveAssignable;                }
	constexpr bool IsDestructible                  () const { return bIsDestructible;                  }
	constexpr bool IsTriviallyDefaultConstructible () const { return bIsTriviallyDefaultConstructible; }
	constexpr bool IsTriviallyCopyConstructible    () const { return bIsTriviallyCopyConstructible;    }
	constexpr bool IsTriviallyMoveConstructible    () const { return bIsTriviallyMoveConstructible;    }
	constexpr bool IsTriviallyCopyAssignable       () const { return bIsTriviallyCopyAssignable;       }
	constexpr bool IsTriviallyMoveAssignable       () const { return bIsTriviallyMoveAssignable;       }
	constexpr bool IsTriviallyDestructible         () const { return bIsTriviallyDestructible;         }
	constexpr bool HasVirtualDestructor            () const { return bHasVirtualDestructor;            }

	constexpr bool IsTrivial                       () const { return bIsTrivial;                       }
	constexpr bool IsTriviallyCopyable             () const { return bIsTriviallyCopyable;             }
	constexpr bool IsStandardLayout                () const { return bIsStandardLayout;                }
	constexpr bool HasUniqueObjectRepresentations  () const { return bHasUniqueObjectRepresentations;  }
	constexpr bool IsEmpty                         () const { return bIsEmpty;                         }
	constexpr bool IsPolymorphic                   () const { return bIsPolymorphic;                   }
	constexpr bool IsAbstract                      () const { return bIsAbstract;                      }
	constexpr bool IsFinal                         () const { return bIsFinal;                         }
	constexpr bool IsAggregate                     () const { return bIsAggregate;                     }
	constexpr bool IsSigned                        () const { return bIsSigned;                        }
	constexpr bool IsUnsigned                      () const { return bIsUnsigned;                      }
	constexpr bool IsBoundedArray                  () const { return bIsBoundedArray;                  }
	constexpr bool IsUnboundedArray                () const { return bIsUnboundedArray;                }
	constexpr bool IsScopedEnum                    () const { return bIsScopedEnum;                    }

	constexpr bool IsEqualityComparable            () const { return bIsEqualityComparable;            }
	constexpr bool IsTotallyOrdered                () const { return bIsTotallyOrdered;                }
	constexpr bool IsThreeWayComparable            () const { return bIsThreeWayComparable;            }
	constexpr bool IsHashable                      () const { return bIsHashable;                      }
	constexpr bool IsSwappable                     () const { return bIsSwappable;                     }

	FORCEINLINE void DefaultConstruct  (void* Address                        ) const { return DefaultConstructImpl  (Address            ); }
	FORCEINLINE void CopyConstruct     (void* Destination, const void* Source) const { return CopyConstructImpl     (Destination, Source); }
	FORCEINLINE void MoveConstruct     (void* Destination,       void* Source) const { return MoveConstructImpl     (Destination, Source); }
	FORCEINLINE void RelocateConstruct (void* Destination,       void* Source) const { return RelocateConstructImpl (Destination, Source); }
	FORCEINLINE void CopyAssign        (void* Destination, const void* Source) const { return CopyAssignImpl        (Destination, Source); }
	FORCEINLINE void MoveAssign        (void* Destination,       void* Source) const { return MoveAssignImpl        (Destination, Source); }
	FORCEINLINE void Destroy           (void* Element                        ) const { return DestroyImpl           (Element            ); }

	FORCEINLINE bool             EqualityCompare      (const void* LHS, const void* RHS) const { return EqualityCompareImpl      (LHS, RHS); }
	FORCEINLINE partial_ordering SynthThreeWayCompare (const void* LHS, const void* RHS) const { return SynthThreeWayCompareImpl (LHS, RHS); }
	FORCEINLINE partial_ordering ThreeWayCompare      (const void* LHS, const void* RHS) const { return ThreeWayCompareImpl      (LHS, RHS); }
	FORCEINLINE size_t           HashItem             (const void* A                   ) const { return HashItemImpl             (A       ); }
	FORCEINLINE void             SwapItem             (      void* A,         void* B  ) const { return SwapItemImpl             (A,   B  ); }

private:

	const std::type_info& Native;

	const size_t TypeSize;
	const size_t TypeAlignment;

	const uint8 bIsZeroConstructible             : 1;
	const uint8 bIsBitwiseConstructible          : 1;
	const uint8 bIsBitwiseRelocatable            : 1;
	const uint8 bIsBitwiseComparable             : 1;

	const uint8 bIsArithmetic                    : 1;
	const uint8 bIsFundamental                   : 1;
	const uint8 bIsObject                        : 1;
	const uint8 bIsScalar                        : 1;
	const uint8 bIsCompound                      : 1;
	const uint8 bIsMemberPointer                 : 1;

	const uint8 bIsVoid                          : 1;
	const uint8 bIsNullPointer                   : 1;
	const uint8 bIsIntegral                      : 1;
	const uint8 bIsFloatingPoint                 : 1;
	const uint8 bIsArray                         : 1;
	const uint8 bIsPointer                       : 1;
	const uint8 bIsMemberObjectPointer           : 1;
	const uint8 bIsMemberFunctionPointer         : 1;
	const uint8 bIsEnum                          : 1;
	const uint8 bIsUnion                         : 1;
	const uint8 bIsClass                         : 1;
	const uint8 bIsFunction                      : 1;

	const uint8 bIsDefaultConstructible          : 1;
	const uint8 bIsCopyConstructible             : 1;
	const uint8 bIsMoveConstructible             : 1;
	const uint8 bIsCopyAssignable                : 1;
	const uint8 bIsMoveAssignable                : 1;
	const uint8 bIsDestructible                  : 1;
	const uint8 bIsTriviallyDefaultConstructible : 1;
	const uint8 bIsTriviallyCopyConstructible    : 1;
	const uint8 bIsTriviallyMoveConstructible    : 1;
	const uint8 bIsTriviallyCopyAssignable       : 1;
	const uint8 bIsTriviallyMoveAssignable       : 1;
	const uint8 bIsTriviallyDestructible         : 1;
	const uint8 bHasVirtualDestructor            : 1;

	const uint8 bIsTrivial                       : 1;
	const uint8 bIsTriviallyCopyable             : 1;
	const uint8 bIsStandardLayout                : 1;
	const uint8 bHasUniqueObjectRepresentations  : 1;
	const uint8 bIsEmpty                         : 1;
	const uint8 bIsPolymorphic                   : 1;
	const uint8 bIsAbstract                      : 1;
	const uint8 bIsFinal                         : 1;
	const uint8 bIsAggregate                     : 1;
	const uint8 bIsSigned                        : 1;
	const uint8 bIsUnsigned                      : 1;
	const uint8 bIsBoundedArray                  : 1;
	const uint8 bIsUnboundedArray                : 1;
	const uint8 bIsScopedEnum                    : 1;

	const uint8 bIsEqualityComparable            : 1;
	const uint8 bIsTotallyOrdered                : 1;
	const uint8 bIsThreeWayComparable            : 1;
	const uint8 bIsHashable                      : 1;
	const uint8 bIsSwappable                     : 1;

	using FDefaultConstruct  = void(*)(void*             );
	using FCopyConstruct     = void(*)(void*, const void*);
	using FMoveConstruct     = void(*)(void*,       void*);
	using FRelocateConstruct = void(*)(void*,       void*);
	using FCopyAssign        = void(*)(void*, const void*);
	using FMoveAssign        = void(*)(void*,       void*);
	using FDestroy           = void(*)(void*             );

	using FEqualityCompare      = bool             (*)(const void*, const void*);
	using FSynthThreeWayCompare = partial_ordering (*)(const void*, const void*);
	using FThreeWayCompare      = partial_ordering (*)(const void*, const void*);
	using FHashItem             = size_t           (*)(const void*             );
	using FSwapItem             = void             (*)(      void*,       void*);

	const FDefaultConstruct  DefaultConstructImpl;
	const FCopyConstruct     CopyConstructImpl;
	const FMoveConstruct     MoveConstructImpl;
	const FRelocateConstruct RelocateConstructImpl;
	const FCopyAssign        CopyAssignImpl;
	const FMoveAssign        MoveAssignImpl;
	const FDestroy           DestroyImpl;

	const FEqualityCompare      EqualityCompareImpl;
	const FSynthThreeWayCompare SynthThreeWayCompareImpl;
	const FThreeWayCompare      ThreeWayCompareImpl;
	const FHashItem             HashItemImpl;
	const FSwapItem             SwapItemImpl;

	template <typename T>
	constexpr FTypeInfo(TInPlaceType<T>) : Native(typeid(T))

		, TypeSize      (!TIsVoid<T>::Value ? sizeof (typename TConditional<TIsVoid<T>::Value, int, T>::Type) : INDEX_NONE)
		, TypeAlignment (!TIsVoid<T>::Value ? alignof(typename TConditional<TIsVoid<T>::Value, int, T>::Type) : INDEX_NONE)

		, bIsZeroConstructible             (TIsZeroConstructible<T>::Value)
		, bIsBitwiseConstructible          (TIsBitwiseConstructible<T, T>::Value)
		, bIsBitwiseRelocatable            (TIsBitwiseRelocatable<T, T>::Value)
		, bIsBitwiseComparable             (TIsBitwiseComparable<T>::Value)

		, bIsArithmetic                    (TIsArithmetic<T>::Value)
		, bIsFundamental                   (TIsFundamental<T>::Value)
		, bIsObject                        (TIsObject<T>::Value)
		, bIsScalar                        (TIsScalar<T>::Value)
		, bIsCompound                      (TIsCompound<T>::Value)
		, bIsMemberPointer                 (TIsMemberPointer<T>::Value)

		, bIsVoid                          (TIsVoid<T>::Value)
		, bIsNullPointer                   (TIsNullPointer<T>::Value)
		, bIsIntegral                      (TIsIntegral<T>::Value)
		, bIsFloatingPoint                 (TIsFloatingPoint<T>::Value)
		, bIsArray                         (TIsArray<T>::Value)
		, bIsPointer                       (TIsPointer<T>::Value)
		, bIsMemberObjectPointer           (TIsMemberObjectPointer<T>::Value)
		, bIsMemberFunctionPointer         (TIsMemberFunctionPointer<T>::Value)
		, bIsEnum                          (TIsEnum<T>::Value)
		, bIsUnion                         (TIsUnion<T>::Value)
		, bIsClass                         (TIsClass<T>::Value)
		, bIsFunction                      (TIsFunction<T>::Value)

		, bIsDefaultConstructible          (TIsDefaultConstructible<T>::Value)
		, bIsCopyConstructible             (TIsCopyConstructible<T>::Value)
		, bIsMoveConstructible             (TIsMoveConstructible<T>::Value)
		, bIsCopyAssignable                (TIsCopyAssignable<T>::Value)
		, bIsMoveAssignable                (TIsMoveAssignable<T>::Value)
		, bIsDestructible                  (TIsDestructible<T>::Value)
		, bIsTriviallyDefaultConstructible (TIsTriviallyDefaultConstructible<T>::Value)
		, bIsTriviallyCopyConstructible    (TIsTriviallyCopyConstructible<T>::Value)
		, bIsTriviallyMoveConstructible    (TIsTriviallyMoveConstructible<T>::Value)
		, bIsTriviallyCopyAssignable       (TIsTriviallyCopyAssignable<T>::Value)
		, bIsTriviallyMoveAssignable       (TIsTriviallyMoveAssignable<T>::Value)
		, bIsTriviallyDestructible         (TIsTriviallyDestructible<T>::Value)
		, bHasVirtualDestructor            (THasVirtualDestructor<T>::Value)

		, bIsTrivial                       (TIsTrivial<T>::Value)
		, bIsTriviallyCopyable             (TIsTriviallyCopyable<T>::Value)
		, bIsStandardLayout                (TIsStandardLayout<T>::Value)
		, bHasUniqueObjectRepresentations  (THasUniqueObjectRepresentations<T>::Value)
		, bIsEmpty                         (TIsEmpty<T>::Value)
		, bIsPolymorphic                   (TIsPolymorphic<T>::Value)
		, bIsAbstract                      (TIsAbstract<T>::Value)
		, bIsFinal                         (TIsFinal<T>::Value)
		, bIsAggregate                     (TIsAggregate<T>::Value)
		, bIsSigned                        (TIsSigned<T>::Value)
		, bIsUnsigned                      (TIsUnsigned<T>::Value)
		, bIsBoundedArray                  (TIsBoundedArray<T>::Value)
		, bIsUnboundedArray                (TIsUnboundedArray<T>::Value)
		, bIsScopedEnum                    (TIsScopedEnum<T>::Value)

		, bIsEqualityComparable            (CEqualityComparable<T>)
		, bIsTotallyOrdered                (CTotallyOrdered<T>)
		, bIsThreeWayComparable            (CThreeWayComparable<T>)
		, bIsHashable                      (CHashable<T>)
		, bIsSwappable                     (CSwappable<T>)

		, DefaultConstructImpl  ([](void* A               ) -> void { if constexpr (requires(T* A            ) { Memory::DefaultConstruct  (A   ); }) Memory::DefaultConstruct  (reinterpret_cast<T*>(A)                               ); else check_no_entry(); })
		, CopyConstructImpl     ([](void* A, const void* B) -> void { if constexpr (requires(T* A, const T* B) { Memory::CopyConstruct     (A, B); }) Memory::CopyConstruct     (reinterpret_cast<T*>(A), reinterpret_cast<const T*>(B)); else check_no_entry(); })
		, MoveConstructImpl     ([](void* A,       void* B) -> void { if constexpr (requires(T* A,       T* B) { Memory::MoveConstruct     (A, B); }) Memory::MoveConstruct     (reinterpret_cast<T*>(A), reinterpret_cast<      T*>(B)); else check_no_entry(); })
		, RelocateConstructImpl ([](void* A,       void* B) -> void { if constexpr (requires(T* A,       T* B) { Memory::RelocateConstruct (A, B); }) Memory::RelocateConstruct (reinterpret_cast<T*>(A), reinterpret_cast<      T*>(B)); else check_no_entry(); })
		, CopyAssignImpl        ([](void* A, const void* B) -> void { if constexpr (requires(T* A, const T* B) { Memory::CopyAssign        (A, B); }) Memory::CopyAssign        (reinterpret_cast<T*>(A), reinterpret_cast<const T*>(B)); else check_no_entry(); })
		, MoveAssignImpl        ([](void* A,       void* B) -> void { if constexpr (requires(T* A,       T* B) { Memory::MoveAssign        (A, B); }) Memory::MoveAssign        (reinterpret_cast<T*>(A), reinterpret_cast<      T*>(B)); else check_no_entry(); })
		, DestroyImpl           ([](void* A               ) -> void { if constexpr (requires(T* A            ) { Memory::Destruct          (A   ); }) Memory::Destruct          (reinterpret_cast<T*>(A)                               ); else check_no_entry(); })

		, EqualityCompareImpl      ([](const void* A, const void* B) -> bool             { if constexpr (CEqualityComparable<T>     ) return                                          (*reinterpret_cast<const T*>(A) ==  *reinterpret_cast<const T*>(B)); else return false;                        })
		, SynthThreeWayCompareImpl ([](const void* A, const void* B) -> partial_ordering { if constexpr (CSynthThreeWayComparable<T>) return NAMESPACE_REDCRAFT::SynthThreeWayCompare (*reinterpret_cast<const T*>(A),    *reinterpret_cast<const T*>(B)); else return partial_ordering::unordered;  })
		, ThreeWayCompareImpl      ([](const void* A, const void* B) -> partial_ordering { if constexpr (CThreeWayComparable<T>     ) return                                          (*reinterpret_cast<const T*>(A) <=> *reinterpret_cast<const T*>(B)); else return partial_ordering::unordered;  })
		, HashItemImpl             ([](const void* A               ) -> size_t           { if constexpr (CHashable<T>               ) return NAMESPACE_REDCRAFT::GetTypeHash          (*reinterpret_cast<const T*>(A)                                   ); else return 1080551797;                   })
		, SwapItemImpl             ([](      void* A,       void* B) -> void             { if constexpr (CSwappable<T>              )        NAMESPACE_REDCRAFT::Swap                 (*reinterpret_cast<      T*>(A),    *reinterpret_cast<      T*>(B)); else check_no_entry();                    })
	
	{ }

	friend FORCEINLINE bool operator==(const FTypeInfo& LHS, const FTypeInfo& RHS) { return &LHS != &RHS ? LHS.GetNative() == RHS.GetNative() : true; }
	friend FORCEINLINE bool operator< (const FTypeInfo& LHS, const FTypeInfo& RHS) { return LHS.GetNative().before(RHS.GetNative()); }
	friend FORCEINLINE bool operator<=(const FTypeInfo& LHS, const FTypeInfo& RHS) { return LHS == RHS || LHS < RHS; }
	friend FORCEINLINE bool operator>=(const FTypeInfo& LHS, const FTypeInfo& RHS) { return LHS == RHS || LHS > RHS; }
	friend FORCEINLINE bool operator> (const FTypeInfo& LHS, const FTypeInfo& RHS) { return !(LHS < RHS); }

	friend FORCEINLINE strong_ordering operator<=>(const FTypeInfo& LHS, const FTypeInfo& RHS)
	{
		if (LHS == RHS) return strong_ordering::equal;
		return LHS < RHS ? strong_ordering::less : strong_ordering::greater;
	}

public:

	friend NAMESPACE_PRIVATE::FTypeInfoStatic;

};

// NOTE: Unlike the standard typeid, this version only supports type and not expression
#define Typeid(...) (NAMESPACE_PRIVATE::FTypeInfoStatic::Value<typename TRemoveCVRef<__VA_ARGS__>::Type>)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
