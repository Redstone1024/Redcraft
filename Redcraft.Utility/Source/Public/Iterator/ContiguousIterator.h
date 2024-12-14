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
 * Add the operator-> to the random access iterator and requires the operator* returns a true reference type.
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
struct IContiguousIterator /* : IRandomAccessIterator<T> */
{
	// ~Begin CRandomAccessIterator.

	using ElementType = TRemoveCVRef<T>;

	IContiguousIterator();
	IContiguousIterator(const IContiguousIterator&);
	IContiguousIterator(IContiguousIterator&&);
	IContiguousIterator* operator=(const IContiguousIterator&);
	IContiguousIterator* operator=(IContiguousIterator&&);

	friend bool operator==(const IContiguousIterator&, const IContiguousIterator&);

	friend strong_ordering operator<=>(const IContiguousIterator&, const IContiguousIterator&);

	// ~End CRandomAccessIterator.

	/**
	 * Dereference operator. See IForwardIterator.
	 * Specify, the return type must be a true reference type and refer to an element of a contiguous sequence, not a proxy class.
	 * Also satisfies CRandomAccessIterator.
	 */
	T operator*() const;

	/** Indirection operator. Return the address of the element that the iterator is pointing to. */
	TAddPointer<T> operator->() const;

	// ~Begin CRandomAccessIterator.

	T operator[](ptrdiff) const;

	IContiguousIterator& operator++();
	IContiguousIterator& operator--();

	IContiguousIterator operator++(int);
	IContiguousIterator operator--(int);

	IContiguousIterator& operator+=(ptrdiff);
	IContiguousIterator& operator-=(ptrdiff);

	IContiguousIterator operator+(ptrdiff) const;
	IContiguousIterator operator-(ptrdiff) const;

	friend IContiguousIterator operator+(ptrdiff, const IContiguousIterator&);

	friend ptrdiff operator-(const IContiguousIterator&, const IContiguousIterator&);

	// ~End CRandomAccessIterator.
};

// Use IContiguousIterator<int> represents a contiguous iterator
static_assert(CContiguousIterator<IContiguousIterator<int&>>);
static_assert(    COutputIterator<IContiguousIterator<int&>, int>);

// The int* is the most typical example of a contiguous iterator
static_assert(CContiguousIterator<int*>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
