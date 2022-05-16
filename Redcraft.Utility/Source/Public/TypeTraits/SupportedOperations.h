#pragma once

#include "CoreTypes.h"

#include <type_traits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

// Cpp17Destructible requirements
template <typename T> concept CCpp17Destructible = NAMESPACE_STD::is_object_v<T> && !NAMESPACE_STD::is_array_v<T> && NAMESPACE_STD::is_destructible_v<T>;

NAMESPACE_PRIVATE_END

template <typename T> concept CDefaultConstructible = NAMESPACE_STD::is_default_constructible_v<T>;
template <typename T> concept CCopyConstructible    = NAMESPACE_STD::is_copy_constructible_v<T>;
template <typename T> concept CMoveConstructible    = NAMESPACE_STD::is_move_constructible_v<T>;
template <typename T> concept CCopyAssignable       = NAMESPACE_STD::is_copy_assignable_v<T>;
template <typename T> concept CMoveAssignable       = NAMESPACE_STD::is_move_assignable_v<T>;
template <typename T> concept CDestructible         = NAMESPACE_PRIVATE::CCpp17Destructible<T>; // Use Cpp17Destructible requirements instead of std::is_destructible

template <typename T> concept CTriviallyDefaultConstructible = CDefaultConstructible<T> && NAMESPACE_STD::is_trivially_default_constructible_v<T>;
template <typename T> concept CTriviallyCopyConstructible    = CCopyConstructible<T>    && NAMESPACE_STD::is_trivially_copy_constructible_v<T>;
template <typename T> concept CTriviallyMoveConstructible    = CMoveConstructible<T>    && NAMESPACE_STD::is_trivially_move_constructible_v<T>;
template <typename T> concept CTriviallyCopyAssignable       = CCopyAssignable<T>       && NAMESPACE_STD::is_trivially_copy_assignable_v<T>;
template <typename T> concept CTriviallyMoveAssignable       = CMoveAssignable<T>       && NAMESPACE_STD::is_trivially_move_assignable_v<T>;
template <typename T> concept CTriviallyDestructible         = CDestructible<T>         && NAMESPACE_STD::is_trivially_destructible_v<T>;
template <typename T> concept CVirtualDestructible           = CDestructible<T>         && NAMESPACE_STD::has_virtual_destructor_v<T>;

//template <typename T> concept CNothrowDefaultConstructible;
//template <typename T> concept CNothrowCopyConstructible;
//template <typename T> concept CNothrowMoveConstructible;
//template <typename T> concept CNothrowCopyAssignable;
//template <typename T> concept CNothrowMoveAssignable;
//template <typename T> concept CNothrowDestructible;

template <typename T, typename U> concept CAssignable = NAMESPACE_STD::is_assignable_v<T, U>;

template <typename T, typename U> concept CTriviallyAssignable = CAssignable<T, U> && NAMESPACE_STD::is_trivially_assignable_v<T, U>;

//template <typename T, typename U> concept CNothrowAssignable;

template <typename T, typename... Args> concept CConstructible = NAMESPACE_STD::is_constructible_v<T, Args...>;

template <typename T, typename... Args> concept CTriviallyConstructible = CConstructible<T, Args...> && NAMESPACE_STD::is_trivially_constructible_v<T, Args...>;

//template <typename T, typename... Args> concept CNothrowConstructible;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
