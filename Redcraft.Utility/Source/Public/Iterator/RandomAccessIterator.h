#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
#include "Iterator/BidirectionalIterator.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

/**
 * A concept specifies a type is a random access iterator.
 * Add the three-way comparison, addition, subtraction and subscript operators to the bidirectional iterator.
 */
template <typename I>
concept CRandomAccessIterator = CBidirectionalIterator<I> && CTotallyOrdered<I> && CSizedSentinelFor<I, I>
	&& requires(I Iter, const I Jter, const ptrdiff N) {
		{ Iter   += N } -> CSameAs<I&>;
		{ Jter   +  N } -> CSameAs<I >;
		{ N +  Jter   } -> CSameAs<I >;
		{ Iter   -= N } -> CSameAs<I&>;
		{ Jter   -  N } -> CSameAs<I >;
		{   Jter[N]   } -> CSameAs<TIteratorReference<I>>;
	};

/** This is an example of a random access iterator, indicate the traits that define a random access iterator. */
template <CReference T>
struct IRandomAccessIterator /* : IBidirectionalIterator<T> */
{
	/** The element type of the indirectly readable type. See 'IIndirectlyReadable'. */
	using ElementType = TRemoveCVRef<T>;

	/** Default constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IRandomAccessIterator();

	/** Copy constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IRandomAccessIterator(const IRandomAccessIterator&);

	/** Copy assignment operator. See 'IIncrementable' and 'ISentinelFor'. */
	IRandomAccessIterator* operator=(const IRandomAccessIterator&);

	/** Equality operator. See 'IIncrementable' and 'ISentinelFor'. */
	friend bool operator==(const IRandomAccessIterator&, const IRandomAccessIterator&);

	/** Three-way comparison operator. */
	friend strong_ordering operator<=>(const IRandomAccessIterator& LHS, const IRandomAccessIterator& RHS);

	/** Dereference operator. See 'IForwardIterator'. */
	T operator*() const;

	/** Pre-increment operator. See 'IWeaklyIncrementable'. */
	IRandomAccessIterator& operator++();

	/** Pre-decrement operator. See 'IBidirectionalIterator'. */
	IRandomAccessIterator& operator--();

	/** Post-increment operator. See 'IIncrementable'. */
	IRandomAccessIterator operator++(int);

	/** Post-decrement operator. See 'IBidirectionalIterator'. */
	IRandomAccessIterator operator--(int);

	/** Addition assignment operator. */
	IRandomAccessIterator& operator+=(ptrdiff);

	/** Subtraction assignment operator. */
	IRandomAccessIterator& operator-=(ptrdiff);

	/** Addition operator. */
	IRandomAccessIterator operator+(ptrdiff) const;

	/** Subtraction operator. */
	IRandomAccessIterator operator-(ptrdiff) const;

	/** Addition operator. */
	friend IRandomAccessIterator operator+(ptrdiff, const IRandomAccessIterator&);

	/** Subtraction operator. */
	friend ptrdiff operator-(const IRandomAccessIterator&, const IRandomAccessIterator&);

	/** Subscript operator. */
	T operator[](ptrdiff) const;
};

// Use 'IRandomAccessIterator<int>' represents a random access iterator
static_assert(CRandomAccessIterator<IRandomAccessIterator<int&>>);
static_assert(      COutputIterator<IRandomAccessIterator<int&>, int>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
