#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
#include "Iterator/ForwardIterator.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

/** A concept specifies a type is a bidirectional iterator. Add the decrement operator to the forward iterator. */
template <typename I>
concept CBidirectionalIterator = CForwardIterator<I>
	&& requires(I Iter) {
		{ --Iter } -> CSameAs<I&>;
		{ Iter-- } -> CSameAs<I >;
	};

/**
 * This is an example of a bidirectional iterator, indicate the traits that define a bidirectional iterator.
 * Regardless of the order in which the increment and decrement operators are applied,
 * the result is always the same if both operations are performed the same number of times.
 */
template <CReference T>
struct IBidirectionalIterator /* : IForwardIterator<T> */
{
	/** The element type of the indirectly readable type. See 'IIndirectlyReadable'. */
	using ElementType = TRemoveCVRef<T>;

	/** Default constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IBidirectionalIterator();

	/** Copy constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IBidirectionalIterator(const IBidirectionalIterator&);

	/** Copy assignment operator. See 'IIncrementable' and 'ISentinelFor'. */
	IBidirectionalIterator* operator=(const IBidirectionalIterator&);

	/** Equality operator. See 'IIncrementable' and 'ISentinelFor'. */
	friend bool operator==(const IBidirectionalIterator&, const IBidirectionalIterator&);

	/** Dereference operator. See 'IForwardIterator'. */
	T operator*() const;

	/** Pre-increment operator. See 'IWeaklyIncrementable'. */
	IBidirectionalIterator& operator++();

	/** Pre-decrement operator. */
	IBidirectionalIterator& operator--();

	/** Post-increment operator. See 'IIncrementable'. */
	IBidirectionalIterator operator++(int);

	/** Post-decrement operator. */
	IBidirectionalIterator operator--(int);
};

// Use 'IBidirectionalIterator<int>' represents a bidirectional iterator.
static_assert(CBidirectionalIterator<IBidirectionalIterator<int&>>);
static_assert(       COutputIterator<IBidirectionalIterator<int&>, int>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
