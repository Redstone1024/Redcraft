#pragma once

#include "CoreTypes.h"
#include "Memory/Address.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Miscellaneous/Compare.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

/** A concept specifies a type is an input iterator. It is input iterator, incrementable, and sentinel for itself. */
template <typename I>
concept CForwardIterator = CInputIterator<I> && CIncrementable<I> && CSentinelFor<I, I>;

/** This is an example of a forward iterator, indicate the traits that define a forward iterator. */
template <CReferenceable T>
struct IForwardIterator /* : IInputIterator<T>, IIncrementable, ISentinelFor<IForwardIterator> */
{
	// ~Begin CInputIterator.

	using FElementType = TRemoveCVRef<T>;

	// ~End CInputIterator.

	// ~Begin CIncrementable and CSentinelFor<IForwardIterator>.

	IForwardIterator();
	IForwardIterator(const IForwardIterator&);
	IForwardIterator(IForwardIterator&&);                 // Also satisfies IInputIterator.
	IForwardIterator* operator=(const IForwardIterator&);
	IForwardIterator* operator=(IForwardIterator&&);      // Also satisfies IInputIterator.

	friend bool operator==(const IForwardIterator&, const IForwardIterator&);

	// ~End CIncrementable and CSentinelFor<IForwardIterator>.

	// ~Begin CInputIterator.

	T operator*() const; // Optional satisfies CIndirectlyWritable.

	IForwardIterator& operator++(); // Also satisfies CIncrementable.

	IForwardIterator operator++(int); // Also satisfies CIncrementable.

	// ~End CInputIterator.
};

// Use IForwardIterator<int> represents a forward iterator.
static_assert(CForwardIterator<IForwardIterator<int&>>);
static_assert( COutputIterator<IForwardIterator<int&>, int>);

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
template <CReferenceable T>
struct IBidirectionalIterator /* : IForwardIterator<T> */
{
	// ~Begin CForwardIterator.

	using FElementType = TRemoveCVRef<T>;

	IBidirectionalIterator();
	IBidirectionalIterator(const IBidirectionalIterator&);
	IBidirectionalIterator(IBidirectionalIterator&&);
	IBidirectionalIterator* operator=(const IBidirectionalIterator&);
	IBidirectionalIterator* operator=(IBidirectionalIterator&&);

	friend bool operator==(const IBidirectionalIterator&, const IBidirectionalIterator&);

	T operator*() const;

	// ~End CForwardIterator.

	IBidirectionalIterator& operator++(); // Also satisfies CForwardIterator.
	IBidirectionalIterator& operator--();

	IBidirectionalIterator operator++(int); // Also satisfies CForwardIterator.
	IBidirectionalIterator operator--(int);
};

// Use IBidirectionalIterator<int> represents a bidirectional iterator.
static_assert(CBidirectionalIterator<IBidirectionalIterator<int&>>);
static_assert(       COutputIterator<IBidirectionalIterator<int&>, int>);

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

	using FElementType = TRemoveCVRef<T>;

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

	using FElementType = TRemoveCVRef<T>;

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
