#pragma once

#include "CoreTypes.h"
#include "Concepts/Same.h"
#include "Concepts/Derived.h"
#include "Concepts/Objects.h"
#include "Concepts/Swappable.h"
#include "Concepts/Assignable.h"
#include "Concepts/Comparable.h"
#include "Concepts/BuiltinType.h"
#include "Concepts/Convertible.h"
#include "Concepts/Destructible.h"
#include "Concepts/Constructible.h"
#include "Concepts/BooleanTestable.h"

//template <typename F, typename... Args> concept CInvocable;                // Prerequisites: Invoke, Forward
//template <typename F, typename... Args> concept CRegularInvocable;         // Prerequisites: Invoke, Forward
//template <typename F, typename... Args> concept CPredicate;                // Prerequisites: CBooleanTestable, CRegularInvocable
//template <typename F, typename T, typename U> concept CRelation;           // Prerequisites: CPredicate
//template <typename F, typename T, typename U> concept CEquivalenceRelation // Prerequisites: CRelation
//template <typename F, typename T, typename U> concept CStrictWeakOrder     // Prerequisites: CRelation
