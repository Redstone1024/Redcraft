#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/Miscellaneous.h"
#include "TypeTraits/CopyQualifiers.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// The class template is a customization point that allows users to influence the result of TCommonType for user
template <typename T, typename U> struct TBasicCommonType { };

// The class template is a customization point that allows users to influence the result of TCommonReference for user
template <typename T, typename U, template <typename> typename TQualifiers, template <typename> typename UQualifiers> struct TBasicCommonReference { };

NAMESPACE_PRIVATE_BEGIN

// Template class declaration for the common type implementation
template <typename...>
struct TCommonTypeImpl;

// If sizeof...(Ts) is zero, there is no member FType
template <>
struct TCommonTypeImpl<> { };

// If sizeof...(Ts) is one, the member FType names the same type as TCommonType<T, T> if it exists; otherwise there is no member FType
template <typename T>
struct TCommonTypeImpl<T> : TCommonTypeImpl<T, T> { };

// If sizeof...(Ts) is two

// If applying TDecay to at least one of T and U produces a different type, the member FType names the same type as TCommonType<TDecay<T>, TDecay<U>>, if it exists; if not, there is no member FType
template <typename T, typename U> concept CDecayed = CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>;
template <typename T, typename U> requires (!CDecayed<T, U>)
struct TCommonTypeImpl<T, U> : TCommonTypeImpl<TDecay<T>, TDecay<U>> { };

// Otherwise, if there is a user specialization for TBasicCommonType<T, U>::FType, that specialization is used
template <typename T, typename U> concept CBasicCommonType = requires { typename TBasicCommonType<T, U>::FType; };
template <typename T, typename U> requires (CDecayed<T, U> && CBasicCommonType<T, U>)
struct TCommonTypeImpl<T, U> : TBasicCommonType<T, U>
{
	// If such a specialization has a member named type,
	// it must be a public and unambiguous member that names a cv-unqualified non-reference type to which both T and U are explicitly convertible
	// Additionally, TBasicCommonType<T, U>::FType and TBasicCommonType<U, T>::FType must denote the same type
	static_assert(CSameAs<typename TBasicCommonType<T, U>::FType, TDecay<typename TBasicCommonType<T, U>::FType>>,                                    "The basic common type must be a cv-unqualified non-reference type");
	static_assert(CConstructibleFrom<typename TBasicCommonType<T, U>::FType, T&&> && CConstructibleFrom<typename TBasicCommonType<T, U>::FType, U&&>, "The basic common type must be a type which both T and U are explicitly convertible");
	static_assert(CSameAs<typename TBasicCommonType<T, U>::FType, typename TBasicCommonType<U, T>::FType>,                                            "TBasicCommonType<T, U>::FType and TBasicCommonType<U, T>::FType must denote the same type");
};

// Otherwise, if TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())> is a valid type, the member FType denotes that type
template <typename T, typename U> concept CConditionalType = requires { typename TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())>; };
template <typename T, typename U> requires (CDecayed<T, U> && !CBasicCommonType<T, U> && CConditionalType<T, U>)
struct TCommonTypeImpl<T, U> { using FType = TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())>; };

// Otherwise, if TDecay<decltype(false ? DeclVal<CRT>() : DeclVal<CRU>())> is a valid type,
// where CRT and CRU are const TRemoveReference<T>& and const TRemoveReference<U>& respectively, the member FType denotes that type
template <typename T, typename U> concept CConditionalCRefType = requires { typename TDecay<decltype(false ? DeclVal<const TRemoveReference<T>&>() : DeclVal<const TRemoveReference<U>&>())>; };
template <typename T, typename U> requires (CDecayed<T, U> && !CBasicCommonType<T, U> && !CConditionalType<T, U> && CConditionalCRefType<T, U>)
struct TCommonTypeImpl<T, U> { using FType = TDecay<decltype(false ? DeclVal<const TRemoveReference<T>&>() : DeclVal<const TRemoveReference<U>&>())>; };

// Otherwise, there is no member FType

// If sizeof...(Ts) is greater than two

// If TCommonType<T, U> exists, the member FType denotes TCommonType<TCommonType<T, U>, R...> if such a type exists
template <typename T, typename U, typename W, typename... Ts> requires (requires { typename TCommonTypeImpl<T, U>::FType; })
struct TCommonTypeImpl<T, U, W, Ts...> : TCommonTypeImpl<typename TCommonTypeImpl<T, U>::FType, W, Ts...> { };

// In all other cases, there is no member FType
template <typename...>
struct TCommonTypeImpl { };

NAMESPACE_PRIVATE_END

NAMESPACE_PRIVATE_BEGIN

// Template class declaration for the common reference implementation
template <typename...>
struct TCommonReferenceImpl;

// Template class declaration for the simple common reference type implementation
template <typename, typename>
struct TSimpleCommonReferenceImpl;

// Template class declaration for the adding qualifiers implementation
template <typename>
struct TQualifiersImpl;

// If sizeof...(T) is zero, there is no member FType
template <>
struct TCommonReferenceImpl<> { };

// If sizeof...(T) is one, the member FType names the same type as T
template <typename T>
struct TCommonReferenceImpl<T> { using FType = T; };

// If sizeof...(Ts) is two

// If T and U are both reference types, and the simple common reference type S of T and U exists, then the member FType names S
template <typename T, typename U> concept CSimpleCommonReference = requires { typename TSimpleCommonReferenceImpl<T, U>::FType; };
template <typename T, typename U> requires (CSimpleCommonReference<T, U>)
struct TCommonReferenceImpl<T, U> : TSimpleCommonReferenceImpl<T, U> { };

// Otherwise, if TBasicCommonReference<TRemoveCVRef<T>, TRemoveCVRef<U>, TQ, UQ>::FType exists
template <typename T, typename U> struct TBasicCommonReferenceImpl : TBasicCommonReference<TRemoveCVRef<T>, TRemoveCVRef<U>, TQualifiersImpl<T>::template FApply, TQualifiersImpl<U>::template FApply> { };
template <typename T, typename U> concept CBasicCommonReference = requires { typename TBasicCommonReferenceImpl<T, U>::FType; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && CBasicCommonReference<T, U>)
struct TCommonReferenceImpl<T, U> : TBasicCommonReferenceImpl<T, U>
{
	// If such a specialization has a member named type,
	// it must be a public and unambiguous member that names a type to which both TQualifiers<T> and UQualifiers<U> are convertible
	// Additionally, TBasicCommonReference<T, U, TQualifiers, UQualifiers>::FType and TBasicCommonReference<U, T, UQualifiers, TQualifiers>::FType must denote the same type
	static_assert(CConvertibleTo<T, typename TBasicCommonReferenceImpl<T, U>::FType> && CConvertibleTo<U, typename TBasicCommonReferenceImpl<T, U>::FType>, "The basic common reference must be a type to which both TQualifiers<T> and UQualifiers<U> are convertible");
	static_assert(CSameAs<typename TBasicCommonReferenceImpl<T, U>::FType, typename TBasicCommonReferenceImpl<U, T>::FType>,                                "TBasicCommonReference<T, U, TQualifiers, UQualifiers>::FType and TBasicCommonReference<U, T, UQualifiers, TQualifiers>::FType must denote the same type");
};

// Where TQ and UQ is a unary alias template such that TQ<U> is U with the addition of T's cv-ref qualifiers, then the member FType names that type
template <typename T> struct TQualifiersImpl      { template <typename U> using FApply =                     TCopyCV<T, U>;  };
template <typename T> struct TQualifiersImpl<T&>  { template <typename U> using FApply = TAddLValueReference<TCopyCV<T, U>>; };
template <typename T> struct TQualifiersImpl<T&&> { template <typename U> using FApply = TAddRValueReference<TCopyCV<T, U>>; };

// Otherwise, if decltype(false ? Val<T>() : Val<U>()), where val is a function template <typename T> T Val();, is a valid type, then the member FType names that type
template <typename T> T Val();
template <typename T, typename U> concept CConditionalValType = requires { typename TVoid<decltype(false ? Val<T>() : Val<U>())>; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && !CBasicCommonReference<T, U> && CConditionalValType<T, U>)
struct TCommonReferenceImpl<T, U> { using FType = decltype(false ? Val<T>() : Val<U>()); };

// Otherwise, if TCommonType<T, U> is a valid type, then the member FType names that type
template <typename T, typename U> concept CCommonTypeImpl = requires { typename TCommonTypeImpl<T, U>; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && !CBasicCommonReference<T, U> && !CConditionalValType<T, U> && CCommonTypeImpl<T, U>)
struct TCommonReferenceImpl<T, U> : TCommonTypeImpl<T, U> { };

// Otherwise, there is no member FType

// If sizeof...(Ts) is greater than two

// If TCommonReference<T, U> exists, the member FType denotes TCommonReference<TCommonReference<T, U>, R...> if such a type exists
template <typename T, typename U, typename W, typename... Ts> requires (requires { typename TCommonReferenceImpl<T, U>::FType; })
struct TCommonReferenceImpl<T, U, W, Ts...> : TCommonReferenceImpl<typename TCommonReferenceImpl<T, U>::FType, W, Ts...> { };

// In all other cases, there is no member FType
template <typename...>
struct TCommonReferenceImpl { };

// If T is CV1 X & and U is CV2 Y & (i.e., both are lvalue reference types):
// their simple common reference type is decltype(false ? DeclVal<CV12 X &>() : DeclVal<CV12 Y &>()),
// where CV12 is the union of CV1 and CV2, if that type exists and is a reference type
template <typename T, typename U> requires (CLValueReference<decltype(false ? DeclVal<TCopyCV<T, U>&>() : DeclVal<TCopyCV<U, T>&>())>)
struct TSimpleCommonReferenceImpl<T&, U&> { using FType = decltype(false ? DeclVal<TCopyCV<T, U>&>() : DeclVal<TCopyCV<U, T>&>()); };

// If T and U are both rvalue reference types:
// if the simple common reference type of T & and U & exists, then let C denote that type's corresponding rvalue reference type
// If CConvertibleTo<T, C> and CConvertibleTo<U, C> are both true, then the simple common reference type of T and U is C
template <typename T, typename U> requires (CConvertibleTo<T&&, TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::FType> &&> && CConvertibleTo<U&&, TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::FType> &&>)
struct TSimpleCommonReferenceImpl<T&&, U&&> { using FType = TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::FType> &&; };

// Otherwise, one of the two types must be a lvalue reference type A & and the other must be a rvalue reference type B &&
// Let D denote the simple common reference type of A & and B const &, if any
// If D exists and CConvertibleTo<B&&, D> is true, then the simple common reference type is D
template <typename T, typename U> requires (CConvertibleTo<U&&, typename TSimpleCommonReferenceImpl<T&, const U&>::FType>)
struct TSimpleCommonReferenceImpl<T&, U&&> { using FType = typename TSimpleCommonReferenceImpl<T&, const U&>::FType; };
template <typename T, typename U> struct TSimpleCommonReferenceImpl<T&&, U&> : TSimpleCommonReferenceImpl<U&, T&&> { }; // The order is not important

// Otherwise, there's no simple common reference type
template <typename T, typename U>
struct TSimpleCommonReferenceImpl { };

NAMESPACE_PRIVATE_END

template <typename... Ts> using TCommonType      = typename NAMESPACE_PRIVATE::TCommonTypeImpl<Ts...>::FType;
template <typename... Ts> using TCommonReference = typename NAMESPACE_PRIVATE::TCommonReferenceImpl<Ts...>::FType;

template <typename... Ts>
concept CCommonReference =
	requires { typename TCommonReference<Ts...>; }
	&& (true && ... && CConvertibleTo<Ts, TCommonReference<Ts...>>);

template <typename... Ts>
concept CCommonType =
	requires { typename TCommonType<Ts...>; (static_cast<TCommonType<Ts...>>(DeclVal<Ts>()), ...); }
	&& CCommonReference<const TAddLValueReference<Ts>...> && CCommonReference<TCommonReference<TCommonType<Ts...>>, TCommonReference<const TAddLValueReference<Ts>...>>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
