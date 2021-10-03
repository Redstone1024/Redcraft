#pragma once

#include "CoreTypes.h"

#include <type_traits>

NS_REDCRAFT_BEGIN
NS_BEGIN(TypeTraits)

// Primary type categories

template <typename T> struct TIsVoid				  { static constexpr bool Value = std::is_void_v<T>;					 };
template <typename T> struct TIsNullPointer			  { static constexpr bool Value = std::is_null_pointer_v<T>;             };
template <typename T> struct TIsIntegral			  { static constexpr bool Value = std::is_integral_v<T>;                 };
template <typename T> struct TIsFloatingPoint		  { static constexpr bool Value = std::is_floating_point_v<T>;           };
template <typename T> struct TIsArray				  { static constexpr bool Value = std::is_array_v<T>;                    };
template <typename T> struct TIsEnum				  { static constexpr bool Value = std::is_enum_v<T>;                     };
template <typename T> struct TIsUnion				  { static constexpr bool Value = std::is_union_v<T>;                    };
template <typename T> struct TIsClass				  { static constexpr bool Value = std::is_class_v<T>;                    };
template <typename T> struct TIsFunction			  { static constexpr bool Value = std::is_function_v<T>;				 };
template <typename T> struct TIsPointer				  { static constexpr bool Value = std::is_pointer_v<T>;				     };
template <typename T> struct TIsLValueReference		  { static constexpr bool Value = std::is_lvalue_reference_v<T>;		 };
template <typename T> struct TIsRValueReference		  { static constexpr bool Value = std::is_rvalue_reference_v<T>;		 };
template <typename T> struct TIsMemberObjectPointer	  { static constexpr bool Value = std::is_member_object_pointer_v<T>;	 };
template <typename T> struct TIsMemberFunctionPointer { static constexpr bool Value = std::is_member_function_pointer_v<T>;  };

// Composite type categories

template <typename T> struct TIsFundamental   { static constexpr bool Value = std::is_fundamental_v<T>;    };
template <typename T> struct TIsArithmetic    { static constexpr bool Value = std::is_arithmetic_v<T>;     };
template <typename T> struct TIsScalar        { static constexpr bool Value = std::is_scalar_v<T>;         };
template <typename T> struct TIsObject        { static constexpr bool Value = std::is_object_v<T>;         };
template <typename T> struct TIsCompound      { static constexpr bool Value = std::is_compound_v<T>;       };
template <typename T> struct TIsReference     { static constexpr bool Value = std::is_reference_v<T>;      };
template <typename T> struct TIsMemberPointer { static constexpr bool Value = std::is_member_pointer_v<T>; };

// Type properties

template <typename T> struct TIsConst                        { static constexpr bool Value = std::is_const_v<T>;                          };
template <typename T> struct TIsVolatile                     { static constexpr bool Value = std::is_volatile_v<T>;                       };
template <typename T> struct TIsTrivial                      { static constexpr bool Value = std::is_trivial_v<T>;                        };
template <typename T> struct TIsTriviallyCopyable            { static constexpr bool Value = std::is_trivially_copyable_v<T>;             };
template <typename T> struct TIsStandardLayout               { static constexpr bool Value = std::is_standard_layout_v<T>;                };
template <typename T> struct THasUniqueObjectRepresentations { static constexpr bool Value = std::has_unique_object_representations_v<T>; };
template <typename T> struct TIsEmpty                        { static constexpr bool Value = std::is_empty_v<T>;                          };
template <typename T> struct TIsPolymorphic                  { static constexpr bool Value = std::is_polymorphic_v<T>;                    };
template <typename T> struct TIsAbstract                     { static constexpr bool Value = std::is_abstract_v<T>;                       };
template <typename T> struct TIsFinal                        { static constexpr bool Value = std::is_final_v<T>;                          };
template <typename T> struct TIsAggregate                    { static constexpr bool Value = std::is_aggregate_v<T>;                      };
template <typename T> struct TIsSigned                       { static constexpr bool Value = std::is_signed_v<T>;                         };
template <typename T> struct TIsUnsigned                     { static constexpr bool Value = std::is_unsigned_v<T>;                       };
template <typename T> struct TIsBoundedArray                 { static constexpr bool Value = std::is_bounded_array_v<T>;                  };
template <typename T> struct TIsUnboundedArray               { static constexpr bool Value = std::is_unbounded_array_v<T>;                };

// Supported operations

template <typename T> struct TIsConstructible				  { static constexpr bool Value = std::is_constructible_v<T>;                   };
template <typename T> struct TIsTriviallyConstructible		  { static constexpr bool Value = std::is_trivially_constructible_v<T>;         };
template <typename T> struct TIsDefaultConstructible		  { static constexpr bool Value = std::is_default_constructible_v<T>;           };
template <typename T> struct TIsTriviallyDefaultConstructible { static constexpr bool Value = std::is_trivially_default_constructible_v<T>; };
template <typename T> struct TIsCopyConstructible			  { static constexpr bool Value = std::is_copy_constructible_v<T>;              };
template <typename T> struct TIsTriviallyCopyConstructible	  { static constexpr bool Value = std::is_trivially_copy_constructible_v<T>;    };
template <typename T> struct TIsMoveConstructible			  { static constexpr bool Value = std::is_move_constructible_v<T>;              };
template <typename T> struct TIsTriviallyMoveConstructible	  { static constexpr bool Value = std::is_trivially_move_constructible_v<T>;    };
template <typename T> struct TIsAssignable					  { static constexpr bool Value = std::is_assignable_v<T>;                      };
template <typename T> struct TIsTriviallyAssignable			  { static constexpr bool Value = std::is_trivially_assignable_v<T>;            };
template <typename T> struct TIsCopyAssignable				  { static constexpr bool Value = std::is_copy_assignable_v<T>;                 };
template <typename T> struct TIsTriviallyCopyAssignable		  { static constexpr bool Value = std::is_trivially_copy_assignable_v<T>;       };
template <typename T> struct TIsMoveAssignable				  { static constexpr bool Value = std::is_move_assignable_v<T>;                 };
template <typename T> struct TIsTriviallyMoveAssignable		  { static constexpr bool Value = std::is_trivially_move_assignable_v<T>;       };
template <typename T> struct TIsDestructible				  { static constexpr bool Value = std::is_destructible_v<T>;                    };
template <typename T> struct TIsTriviallyDestructible		  { static constexpr bool Value = std::is_trivially_destructible_v<T>;          };
template <typename T> struct THasVirtualDestructor			  { static constexpr bool Value = std::has_virtual_destructor_v<T>;             };
template <typename T> struct TIsSwappableWith				  { static constexpr bool Value = std::is_swappable_with_v<T>;                  };
template <typename T> struct TIsSwappable					  { static constexpr bool Value = std::is_swappable_v<T>;                       };

// Property queries

template <typename T>				struct TRank   { static constexpr size_t Value = std::rank_v<T>;      };
template <typename T, size_t N = 0> struct TExtent { static constexpr size_t Value = std::extent_v<T, N>; };

// Type relationships

template <typename T, typename U>                               struct TIsSame             { static constexpr bool Value = std::is_same_v<T, U>;                             };
template <typename T, typename U>                               struct TIsBaseOf           { static constexpr bool Value = std::is_base_of_v<T, U>;                          };
template <typename T, typename U>                               struct TIsConvertible      { static constexpr bool Value = std::is_convertible_v<T, U>;                      };
template <typename T, typename U>                               struct TIsLayoutCompatible { static constexpr bool Value = std::is_layout_compatible_v<T, U>;                };
template <typename Func, typename... ArgTypes>                  struct TIsInvocable        { static constexpr bool Value = std::is_invocable_v<Func, ArgTypes...>;           };
template <typename Result, typename Func, typename... ArgTypes> struct TIsInvocableResult  { static constexpr bool Value = std::is_invocable_r_v<Result, Func, ArgTypes...>; };

// Const-volatility specifiers

template <typename T> struct TRemoveCV		 { typedef typename std::remove_cv_t<T>     Type; };
template <typename T> struct TRemoveConst	 { typedef typename std::remove_const<T>    Type; };
template <typename T> struct TRemoveVolatile { typedef typename std::remove_volatile<T> Type; };
template <typename T> struct TAddCV			 { typedef typename std::add_cv<T>          Type; };
template <typename T> struct TAddConst		 { typedef typename std::add_const<T>       Type; };
template <typename T> struct TAddVolatile	 { typedef typename std::add_volatile<T>    Type; };

// References

template <typename T> struct TRemoveReference    { typedef typename std::remove_reference_t<T>     Type; };
template <typename T> struct TAddLValueReference { typedef typename std::add_lvalue_reference_t<T> Type; };
template <typename T> struct TAddRValueReference { typedef typename std::add_rvalue_reference_t<T> Type; };

// Pointers

template <typename T> struct TRemovePointer { typedef typename std::remove_pointer_t<T> Type; };
template <typename T> struct TAddPointer    { typedef typename std::add_pointer_t<T>    Type; };

// Sign modifiers

template <typename T> struct TMakeSigned   { typedef typename std::make_signed_t<T>   Type; };
template <typename T> struct TMakeUnsigned { typedef typename std::make_unsigned_t<T> Type; };

// Arrays

template <typename T> struct TRemoveExtent     { typedef typename std::remove_extent_t<T>      Type; };
template <typename T> struct TRemoveAllExtents { typedef typename std::remove_all_extents_t<T> Type; };

// Miscellaneous transformations

template <size_t Len, size_t Align = alignof(max_align_t)> struct TAlignedStorage { typedef typename std::aligned_storage_t<Len, Align>          Type; };
template <size_t Len, typename... Types>                   struct TAlignedUnion   { typedef typename std::aligned_union_t<Len, Types...>         Type; };
template <typename T>                                      struct TDecay          { typedef typename std::decay_t<T>                             Type; };
template <typename T>                                      struct TRemoveCVRef    { typedef typename std::remove_cvref_t<T>                      Type; };
template <bool B, typename T, typename F>                  struct TConditional    { typedef typename std::conditional_t<B, T, F>                 Type; };
template <typename... T>                                   struct TCommonType     { typedef typename std::common_type_t<T...>                    Type; };
template <typename T>                                      struct TUnderlyingType { typedef typename std::underlying_type_t<T>                   Type; };
template <typename Func, typename... ArgTypes>             struct TInvokeResult   { typedef typename std::invoke_result_t<Func, ArgTypes...>     Type; };

// Operations on traits

template <typename T, T InValue> struct TConstant                                { static constexpr T    Value = InValue;             };
template <bool InValue>          struct TBoolConstant : TConstant<bool, InValue> {                                                    };

template <typename... Types>     struct TAnd;
template <typename... RHS>       struct TAnd<TBoolConstant<true>, RHS...>        { static constexpr bool Value = TAnd<RHS...>::Value; };
template <typename... RHS>       struct TAnd<TBoolConstant<false>, RHS...>       { static constexpr bool Value = false;               };
template <>                      struct TAnd<>                                   { static constexpr bool Value = true;                };

template <typename... Types>     struct TOr;
template <typename... RHS>       struct TOr<TBoolConstant<true>, RHS...>         { static constexpr bool Value = true;                };
template <typename... RHS>       struct TOr<TBoolConstant<false>, RHS...>        { static constexpr bool Value = TOr<RHS...>::Value;  };
template <>                      struct TOr<>                                    { static constexpr bool Value = false;               };

template <typename Type>         struct TNot                                     { static constexpr bool Value = !Type::Value;        };

NS_END(TypeTraits)
NS_REDCRAFT_END
