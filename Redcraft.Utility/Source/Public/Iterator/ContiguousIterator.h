#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
#include "Iterator/RandomAccessIterator.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

/**
 * A concept specifies a type is a contiguous iterator.
 * Add the 'operator->' to the random access iterator and requires the 'operator*' returns a true reference type.
 */
template <typename I>
concept CContiguousIterator = CRandomAccessIterator<I> && CLValueReference<TIteratorReference<I>>
	&& CSameAs<TIteratorElement<I>, TRemoveCVRef<TIteratorReference<I>>>
	&& CSameAs<TIteratorPointer<I>,  TAddPointer<TIteratorReference<I>>>
	&& requires(I& Iter)
	{
		{ ToAddress(Iter) } -> CSameAs<TAddPointer<TIteratorReference<I>>>;
	};

/** This is an example of a contiguous iterator, indicate the traits that define a contiguous iterator. */
template <CLValueReference T>
struct IContiguousIterator /* : IBidirectionalIterator<T> */
{
	/** The element type of the indirectly readable type. See 'IIndirectlyReadable'. */
	using ElementType = TRemoveCVRef<T>;

	/** Default constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IContiguousIterator();

	/** Copy constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IContiguousIterator(const IContiguousIterator&);

	/** Copy assignment operator. See 'IIncrementable' and 'ISentinelFor'. */
	IContiguousIterator* operator=(const IContiguousIterator&);

	/** Equality operator. See 'IIncrementable' and 'ISentinelFor'. */
	friend bool operator==(const IContiguousIterator&, const IContiguousIterator&);

	/** Three-way comparison operator. See 'IRandomAccessIterator'. */
	friend strong_ordering operator<=>(const IContiguousIterator& LHS, const IContiguousIterator& RHS);

	/**
	 * Dereference operator. See 'IForwardIterator'.
	 * Specify, the return type must be a true reference type and refer to an element of a contiguous sequence, not a proxy class.
	 */
	T operator*() const;

	/** Indirection operator. Return the address of the element that the iterator is pointing to. */
	TAddPointer<T> operator->() const;

	/** Pre-increment operator. See 'IWeaklyIncrementable'. */
	IContiguousIterator& operator++();

	/** Pre-decrement operator. See 'IBidirectionalIterator'. */
	IContiguousIterator& operator--();

	/** Post-increment operator. See 'IIncrementable'. */
	IContiguousIterator operator++(int);

	/** Post-decrement operator. See 'IBidirectionalIterator'. */
	IContiguousIterator operator--(int);

	/** Addition assignment operator. See 'IRandomAccessIterator'. */
	IContiguousIterator& operator+=(ptrdiff);

	/** Subtraction assignment operator. See 'IRandomAccessIterator'. */
	IContiguousIterator& operator-=(ptrdiff);

	/** Addition operator. See 'IRandomAccessIterator'. */
	IContiguousIterator operator+(ptrdiff) const;

	/** Subtraction operator. See 'IRandomAccessIterator'. */
	IContiguousIterator operator-(ptrdiff) const;

	/** Addition operator. See 'IRandomAccessIterator'. */
	friend IContiguousIterator operator+(ptrdiff, const IContiguousIterator&);

	/** Subtraction operator. See 'IRandomAccessIterator'. */
	friend ptrdiff operator-(const IContiguousIterator&, const IContiguousIterator&);

	/** Subscript operator. See 'IRandomAccessIterator'. */
	T operator[](ptrdiff) const;
};

// Use 'IContiguousIterator<int>' represents a contiguous iterator
static_assert(CContiguousIterator<IContiguousIterator<int&>>);
static_assert(    COutputIterator<IContiguousIterator<int&>, int>);

// The 'int*' is the most typical example of a contiguous iterator
static_assert(CContiguousIterator<int*>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
