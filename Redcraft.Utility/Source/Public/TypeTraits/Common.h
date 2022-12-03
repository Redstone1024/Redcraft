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

// Template class declaration for the common Type implementation
template <typename...>
struct TCommonTypeImpl;

// If sizeof...(Ts) is zero, there is no member Type
template <>
struct TCommonTypeImpl<> { };

// If sizeof...(Ts) is one, the member Type names the same Type as TCommonType<T, T> if it exists; otherwise there is no member Type
template <typename T>
struct TCommonTypeImpl<T> : TCommonTypeImpl<T, T> { };

// If sizeof...(Ts) is two

// If applying TDecay to at least one of T and U produces a different Type, the member Type names the same Type as TCommonType<TDecay<T>, TDecay<U>>, if it exists; if not, there is no member Type
template <typename T, typename U> concept CDecayed = CSameAs<T, TDecay<T>> && CSameAs<U, TDecay<U>>;
template <typename T, typename U> requires (!CDecayed<T, U>)
struct TCommonTypeImpl<T, U> : TCommonTypeImpl<TDecay<T>, TDecay<U>> { };

// Otherwise, if there is a user specialization for TBasicCommonType<T, U>::Type, that specialization is used
template <typename T, typename U> concept CBasicCommonType = requires { typename TBasicCommonType<T, U>::Type; };
template <typename T, typename U> requires (CDecayed<T, U> && CBasicCommonType<T, U>)
struct TCommonTypeImpl<T, U> : TBasicCommonType<T, U>
{
	// If such a specialization has a member named type,
	// it must be a public and unambiguous member that names a cv-unqualified non-reference type to which both T and U are explicitly convertible
	// Additionally, TBasicCommonType<T, U>::Type and TBasicCommonType<U, T>::Type must denote the same type
	static_assert(CSameAs<typename TBasicCommonType<T, U>::Type, TDecay<typename TBasicCommonType<T, U>::Type>>,                                    "The basic common type must be a cv-unqualified non-reference type");
	static_assert(CConstructibleFrom<typename TBasicCommonType<T, U>::Type, T&&> && CConstructibleFrom<typename TBasicCommonType<T, U>::Type, U&&>, "The basic common type must be a type which both T and U are explicitly convertible");
	static_assert(CSameAs<typename TBasicCommonType<T, U>::Type, typename TBasicCommonType<U, T>::Type>,                                            "TBasicCommonType<T, U>::Type and TBasicCommonType<U, T>::Type must denote the same type");
};

// Otherwise, if TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())> is a valid Type, the member Type denotes that Type
template <typename T, typename U> concept CConditionalType = requires { typename TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())>; };
template <typename T, typename U> requires (CDecayed<T, U> && !CBasicCommonType<T, U> && CConditionalType<T, U>)
struct TCommonTypeImpl<T, U> { using Type = TDecay<decltype(false ? DeclVal<T>() : DeclVal<U>())>; };

// Otherwise, if TDecay<decltype(false ? DeclVal<CRT>() : DeclVal<CRU>())> is a valid Type, 
// where CRT and CRU are const TRemoveReference<T>& and const TRemoveReference<U>& respectively, the member Type denotes that Type
template <typename T, typename U> concept CConditionalCRefType = requires { typename TDecay<decltype(false ? DeclVal<const TRemoveReference<T>&>() : DeclVal<const TRemoveReference<U>&>())>; };
template <typename T, typename U> requires (CDecayed<T, U> && !CBasicCommonType<T, U> && !CConditionalType<T, U> && CConditionalCRefType<T, U>)
struct TCommonTypeImpl<T, U> { using Type = TDecay<decltype(false ? DeclVal<const TRemoveReference<T>&>() : DeclVal<const TRemoveReference<U>&>())>; };

// Otherwise, there is no member Type

// If sizeof...(Ts) is greater than two

// If TCommonType<T, U> exists, the member Type denotes TCommonType<TCommonType<T, U>, R...> if such a Type exists
template <typename T, typename U, typename W, typename... Ts> requires (requires { typename TCommonTypeImpl<T, U>::Type; })
struct TCommonTypeImpl<T, U, W, Ts...> : TCommonTypeImpl<typename TCommonTypeImpl<T, U>::Type, W, Ts...> { };

// In all other cases, there is no member Type
template <typename...>
struct TCommonTypeImpl { };

NAMESPACE_PRIVATE_END

NAMESPACE_PRIVATE_BEGIN

// Template class declaration for the common reference implementation
template <typename...>
struct TCommonReferenceImpl;

// Template class declaration for the simple common reference Type implementation
template <typename, typename>
struct TSimpleCommonReferenceImpl;

// Template class declaration for the adding qualifiers implementation
template <typename>
struct TQualifiersImpl;

// If sizeof...(T) is zero, there is no member Type
template <>
struct TCommonReferenceImpl<> { };

// If sizeof...(T) is one, the member Type names the same Type as T
template <typename T>
struct TCommonReferenceImpl<T> { using Type = T; };

// If sizeof...(Ts) is two

// If T and U are both reference types, and the simple common reference Type S of T and U exists, then the member Type Type names S
template <typename T, typename U> concept CSimpleCommonReference = requires { typename TSimpleCommonReferenceImpl<T, U>::Type; };
template <typename T, typename U> requires (CSimpleCommonReference<T, U>)
struct TCommonReferenceImpl<T, U> : TSimpleCommonReferenceImpl<T, U> { };

// Otherwise, if TBasicCommonReference<TRemoveCVRef<T>, TRemoveCVRef<U>, TQ, UQ>::Type exists
template <typename T, typename U> struct TBasicCommonReferenceImpl : TBasicCommonReference<TRemoveCVRef<T>, TRemoveCVRef<U>, TQualifiersImpl<T>::template Apply, TQualifiersImpl<U>::template Apply> { };
template <typename T, typename U> concept CBasicCommonReference = requires { typename TBasicCommonReferenceImpl<T, U>::Type; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && CBasicCommonReference<T, U>)
struct TCommonReferenceImpl<T, U> : TBasicCommonReferenceImpl<T, U>
{
	// If such a specialization has a member named type,
	// it must be a public and unambiguous member that names a type to which both TQualifiers<T> and UQualifiers<U> are convertible
	// Additionally, TBasicCommonReference<T, U, TQualifiers, UQualifiers>::Type and TBasicCommonReference<U, T, UQualifiers, TQualifiers>::Type must denote the same type
	static_assert(CConvertibleTo<T, typename TBasicCommonReferenceImpl<T, U>::Type> && CConvertibleTo<U, typename TBasicCommonReferenceImpl<T, U>::Type>, "The basic common reference must be a type to which both TQualifiers<T> and UQualifiers<U> are convertible");
	static_assert(CSameAs<typename TBasicCommonReferenceImpl<T, U>::Type, typename TBasicCommonReferenceImpl<U, T>::Type>,                                "TBasicCommonReference<T, U, TQualifiers, UQualifiers>::Type and TBasicCommonReference<U, T, UQualifiers, TQualifiers>::Type must denote the same type");
};

// Where TQ and UQ is a unary alias template such that TQ<U> is U with the addition of T's cv-ref qualifiers, then the member Type Type names that Type
template <typename T> struct TQualifiersImpl      { template <typename U> using Apply =                     TCopyCV<T, U>;  };
template <typename T> struct TQualifiersImpl<T&>  { template <typename U> using Apply = TAddLValueReference<TCopyCV<T, U>>; };
template <typename T> struct TQualifiersImpl<T&&> { template <typename U> using Apply = TAddRValueReference<TCopyCV<T, U>>; };

// Otherwise, if decltype(false ? Val<T>() : Val<U>()), where val is a function template template <typename T> T Val();, is a valid Type, then the member Type Type names that Type
template <typename T> T Val();
template <typename T, typename U> concept CConditionalValType = requires { typename TVoid<decltype(false ? Val<T>() : Val<U>())>; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && !CBasicCommonReference<T, U> && CConditionalValType<T, U>)
struct TCommonReferenceImpl<T, U> { using Type = decltype(false ? Val<T>() : Val<U>()); };

// Otherwise, if TCommonType<T, U> is a valid Type, then the member Type Type names that Type
template <typename T, typename U> concept CCommonTypeImpl = requires { typename TCommonTypeImpl<T, U>; };
template <typename T, typename U> requires (!CSimpleCommonReference<T, U> && !CBasicCommonReference<T, U> && !CConditionalValType<T, U> && CCommonTypeImpl<T, U>)
struct TCommonReferenceImpl<T, U> : TCommonTypeImpl<T, U> { };

// Otherwise, there is no member Type

// If sizeof...(Ts) is greater than two

// If TCommonReference<T, U> exists, the member Type denotes TCommonReference<TCommonReference<T, U>, R...> if such a Type exists
template <typename T, typename U, typename W, typename... Ts> requires (requires { typename TCommonReferenceImpl<T, U>::Type; })
struct TCommonReferenceImpl<T, U, W, Ts...> : TCommonReferenceImpl<typename TCommonReferenceImpl<T, U>::Type, W, Ts...> { };

// In all other cases, there is no member Type
template <typename...>
struct TCommonReferenceImpl { };

// If T is CV1 X & and U is CV2 Y & (i.e., both are lvalue reference types):
// their simple common reference Type is decltype(false ? DeclVal<CV12 X &>() : DeclVal<CV12 Y &>()),
// where CV12 is the union of CV1 and CV2, if that Type exists and is a reference Type
template <typename T, typename U> requires (CLValueReference<decltype(false ? DeclVal<TCopyCV<T, U>&>() : DeclVal<TCopyCV<U, T>&>())>)
struct TSimpleCommonReferenceImpl<T&, U&> { using Type = decltype(false ? DeclVal<TCopyCV<T, U>&>() : DeclVal<TCopyCV<U, T>&>()); };

// If T and U are both rvalue reference types:
// if the simple common reference Type of T & and U & exists, then let C denote that Type's corresponding rvalue reference Type
// If CConvertibleTo<T, C> and CConvertibleTo<U, C> are both true, then the simple common reference Type of T and U is C
template <typename T, typename U> requires (CConvertibleTo<T&&, TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::Type> &&> && CConvertibleTo<U&&, TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::Type> &&>)
struct TSimpleCommonReferenceImpl<T&&, U&&> { using Type = TRemoveReference<typename TSimpleCommonReferenceImpl<T&, U&>::Type> &&; };

// Otherwise, one of the two types must be an lvalue reference Type A & and the other must be an rvalue reference Type B &&
// Let D denote the simple common reference Type of A & and B const &, if any
// If D exists and CConvertibleTo<B&&, D> is true, then the simple common reference Type is D
template <typename T, typename U> requires (CConvertibleTo<U&&, typename TSimpleCommonReferenceImpl<T&, const U&>::Type>)
struct TSimpleCommonReferenceImpl<T&, U&&> { using Type = typename TSimpleCommonReferenceImpl<T&, const U&>::Type; };
template <typename T, typename U> struct TSimpleCommonReferenceImpl<T&&, U&> : TSimpleCommonReferenceImpl<U&, T&&> { }; // The order is not important

// Otherwise, there's no simple common reference Type
template <typename T, typename U>
struct TSimpleCommonReferenceImpl { };

NAMESPACE_PRIVATE_END

template <typename... Ts> using TCommonType      = typename NAMESPACE_PRIVATE::TCommonTypeImpl<Ts...>::Type;
template <typename... Ts> using TCommonReference = typename NAMESPACE_PRIVATE::TCommonReferenceImpl<Ts...>::Type;

template <typename... Ts>
concept CCommonReference =
	requires { typename TCommonReference<Ts...>; }
	&& (true && ... && CConvertibleTo<Ts, TCommonReference<Ts...>>);

template <typename... Ts>
concept CCommonType =
	requires { typename TCommonType<Ts...>; }
	&& (true && ... && CConstructibleFrom<TCommonReference<Ts...>, Ts&&>)
	&& CCommonReference<const Ts&...> && CCommonReference<TCommonType<Ts...>&, TCommonReference<const Ts&...>>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
