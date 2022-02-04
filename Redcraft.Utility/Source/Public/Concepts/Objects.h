#pragma once

#include "CoreTypes.h"
#include "Concepts/Swappable.h"
#include "Concepts/Assignable.h"
#include "Concepts/Comparable.h"
#include "TypeTraits/TypeTraits.h"
#include "Concepts/Constructible.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CMovable =
	TIsObject<T>::Value &&
	CMoveConstructible<T> &&
	CAssignableFrom<T&, T> &&
	CSwappable<T>;

template <typename T>
concept CCopyable = CMovable<T> &&
	CCopyConstructible<T> &&
	CAssignableFrom<T&, T&> &&
	CAssignableFrom<T&, const T&> &&
	CAssignableFrom<T&, const T>;

template <typename T>
concept CSemiregular = CCopyable<T> && CDefaultInitializable<T>;

template <typename T>
concept CRegular = CSemiregular<T> && CEqualityComparable<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
