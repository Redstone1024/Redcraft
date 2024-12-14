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
template <CReferenceable T>
struct IRandomAccessIterator /* : IBidirectionalIterator<T>, ISizedSentinelFor<IRandomAccessIterator> */
{
	// ~Begin CBidirectionalIterator.

	using ElementType = TRemoveCVRef<T>;

	// ~End CBidirectionalIterator.

	// ~Begin CBidirectionalIterator and CSizedSentinelFor<IRandomAccessIterator>.

	IRandomAccessIterator();
	IRandomAccessIterator(const IRandomAccessIterator&);
	IRandomAccessIterator(IRandomAccessIterator&&);
	IRandomAccessIterator* operator=(const IRandomAccessIterator&);
	IRandomAccessIterator* operator=(IRandomAccessIterator&&);

	friend bool operator==(const IRandomAccessIterator&, const IRandomAccessIterator&);

	// ~End CBidirectionalIterator and CSizedSentinelFor<IRandomAccessIterator>.

	friend strong_ordering operator<=>(const IRandomAccessIterator&, const IRandomAccessIterator&);

	T operator*() const; // Also satisfies CBidirectionalIterator.

	T operator[](ptrdiff) const;

	// ~Begin CBidirectionalIterator.

	IRandomAccessIterator& operator++();
	IRandomAccessIterator& operator--();

	IRandomAccessIterator operator++(int);
	IRandomAccessIterator operator--(int);

	// ~End CBidirectionalIterator.

	IRandomAccessIterator& operator+=(ptrdiff);
	IRandomAccessIterator& operator-=(ptrdiff);

	IRandomAccessIterator operator+(ptrdiff) const;
	IRandomAccessIterator operator-(ptrdiff) const;

	friend IRandomAccessIterator operator+(ptrdiff, const IRandomAccessIterator&);

	friend ptrdiff operator-(const IRandomAccessIterator&, const IRandomAccessIterator&); // Also satisfies CSizedSentinelFor<IRandomAccessIterator>.
};

// Use IRandomAccessIterator<int> represents a random access iterator
static_assert(CRandomAccessIterator<IRandomAccessIterator<int&>>);
static_assert(      COutputIterator<IRandomAccessIterator<int&>, int>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
