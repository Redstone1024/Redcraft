#pragma once

#include "CoreTypes.h"
#include "TypeTraits/Swappable.h"
#include "TypeTraits/Comparable.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename T>
concept CMovable = CObject<T>
	&& CMoveConstructible<T>
	&& CMoveAssignable<T>
	&& CSwappable<T>;

template <typename T>
concept CCopyable = CMovable<T>
	&& CCopyConstructible<T>
	&& CCopyAssignable<T>;

template <typename T>
concept CSemiregular = CCopyable<T> && CDefaultConstructible<T>;

template <typename T>
concept CRegular = CSemiregular<T> && CEqualityComparable<T>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
