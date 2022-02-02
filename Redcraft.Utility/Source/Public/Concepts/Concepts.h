#pragma once

#include "CoreTypes.h"
#include "Concepts/Same.h"
#include "Concepts/Derived.h"
#include "Concepts/BuiltinType.h"
#include "Concepts/Convertible.h"
#include "Concepts/Destructible.h"
#include "Concepts/Constructible.h"

//template <typename T> concept CBooleanTestable; // Prerequisites: Forward
//template <typename T> concept CMovable;         // Prerequisites: CAssignableFrom
//template <typename T> concept CCopyable;        // Prerequisites: CAssignableFrom
//template <typename T> concept CSemiregular;     // Prerequisites: CCopyable
//template <typename T> concept CRegular;         // Prerequisites: CEqualityComparable

//template <typename T, typename U> concept CAssignableFrom;         // Prerequisites: Forward
//template <typename T> concept CEqualityComparable;                 // Prerequisites: CBooleanTestable
//template <typename T, typename U> concept CEqualityComparableWith; // Prerequisites: CBooleanTestable
//template <typename T> concept CTotallyOrdered;                     // Prerequisites: CBooleanTestable
//template <typename T, typename U> concept CTotallyOrderedWith;     // Prerequisites: CBooleanTestable

//template <typename T, typename U> concept CCommonWith;                     // Prerequisites: Declval
//template <typename T, typename U> concept CCommonReferenceWith;            // Prerequisites: Declval
//template <typename F, typename... Args> concept CInvocable;                // Prerequisites: Invoke, Forward
//template <typename F, typename... Args> concept CRegularInvocable;         // Prerequisites: Invoke, Forward
//template <typename F, typename... Args> concept CPredicate;                // Prerequisites: CBooleanTestable, CRegularInvocable
//template <typename F, typename T, typename U> concept CRelation;           // Prerequisites: CPredicate
//template <typename F, typename T, typename U> concept CEquivalenceRelation // Prerequisites: CRelation
//template <typename F, typename T, typename U> concept CStrictWeakOrder     // Prerequisites: CRelation
