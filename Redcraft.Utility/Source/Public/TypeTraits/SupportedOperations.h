#pragma once

#include "CoreTypes.h"
#include "TypeTraits/HelperClasses.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)
NAMESPACE_BEGIN(TypeTraits)

template <typename T> struct TIsDefaultConstructible          : TBoolConstant<NAMESPACE_STD::is_default_constructible_v<T>>           { };
template <typename T> struct TIsCopyConstructible             : TBoolConstant<NAMESPACE_STD::is_copy_constructible_v<T>>              { };
template <typename T> struct TIsMoveConstructible             : TBoolConstant<NAMESPACE_STD::is_move_constructible_v<T>>              { };
template <typename T> struct TIsCopyAssignable                : TBoolConstant<NAMESPACE_STD::is_copy_assignable_v<T>>                 { };
template <typename T> struct TIsMoveAssignable                : TBoolConstant<NAMESPACE_STD::is_move_assignable_v<T>>                 { };
template <typename T> struct TIsDestructible                  : TBoolConstant<NAMESPACE_STD::is_destructible_v<T>>                    { };
template <typename T> struct TIsTriviallyDefaultConstructible : TBoolConstant<NAMESPACE_STD::is_trivially_default_constructible_v<T>> { };
template <typename T> struct TIsTriviallyCopyConstructible    : TBoolConstant<NAMESPACE_STD::is_trivially_copy_constructible_v<T>>    { };
template <typename T> struct TIsTriviallyMoveConstructible    : TBoolConstant<NAMESPACE_STD::is_trivially_move_constructible_v<T>>    { };
template <typename T> struct TIsTriviallyCopyAssignable       : TBoolConstant<NAMESPACE_STD::is_trivially_copy_assignable_v<T>>       { };
template <typename T> struct TIsTriviallyMoveAssignable       : TBoolConstant<NAMESPACE_STD::is_trivially_move_assignable_v<T>>       { };
template <typename T> struct TIsTriviallyDestructible         : TBoolConstant<NAMESPACE_STD::is_trivially_destructible_v<T>>          { };
template <typename T> struct THasVirtualDestructor            : TBoolConstant<NAMESPACE_STD::has_virtual_destructor_v<T>>             { };

//template <typename T> struct TIsSwappable;
//template <typename T> struct TIsNothrowDefaultConstructible;
//template <typename T> struct TIsNothrowCopyConstructible;
//template <typename T> struct TIsNothrowMoveConstructible;
//template <typename T> struct TIsNothrowCopyAssignable;
//template <typename T> struct TIsNothrowMoveAssignable;
//template <typename T> struct TIsNothrowSwappable;
//template <typename T> struct TIsNothrowDestructible;

template <typename T, typename U> struct TIsAssignable          : TBoolConstant<NAMESPACE_STD::is_assignable_v<T, U>>           { };
template <typename T, typename U> struct TIsTriviallyAssignable : TBoolConstant<NAMESPACE_STD::is_trivially_assignable_v<T, U>> { };

//template <typename T, typename U> struct TIsSwappableWith;
//template <typename T, typename U> struct TIsNothrowAssignable;
//template <typename T, typename U> struct TIsNothrowSwappableWith;

template <typename T, typename... Args> struct TIsConstructible          : TBoolConstant<NAMESPACE_STD::is_constructible_v<T, Args...>>           { };
template <typename T, typename... Args> struct TIsTriviallyConstructible : TBoolConstant<NAMESPACE_STD::is_trivially_constructible_v<T, Args...>> { };

//template <typename T, typename... Args> struct TIsNothrowConstructible;

NAMESPACE_END(TypeTraits)
NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
