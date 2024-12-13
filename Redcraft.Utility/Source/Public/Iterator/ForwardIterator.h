#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "Iterator/Sentinel.h"
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
template <CReference T>
struct IForwardIterator /* : IInputIterator<T>, IIncrementable, ISentinelFor<I> */
{
	/** The element type of the indirectly readable type. See 'IIndirectlyReadable'. */
	using ElementType = TRemoveCVRef<T>;

	/** Default constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IForwardIterator();

	/** Copy constructor. See 'IIncrementable' and 'ISentinelFor'. */
	IForwardIterator(const IForwardIterator&);

	/** Copy assignment operator. See 'IIncrementable' and 'ISentinelFor'. */
	IForwardIterator* operator=(const IForwardIterator&);

	/** Equality operator. See 'IIncrementable' and 'ISentinelFor'. */
	friend bool operator==(const IForwardIterator&, const IForwardIterator&);

	/**
	 * Indirectly read the element from the indirectly readable type. See 'IIndirectlyReadable'.
	 * Indirectly write the element if the type is also an indirectly writable type. See 'IIndirectlyWritable'.
	 */
	T operator*() const;

	/** Pre-increment operator. See 'IWeaklyIncrementable'. */
	IForwardIterator& operator++();

	/** Post-increment operator. See 'IIncrementable'. */
	IForwardIterator operator++(int);
};

// Use 'IForwardIterator<int>' represents a forward iterator.
static_assert(CForwardIterator<IForwardIterator<int&>>);
static_assert( COutputIterator<IForwardIterator<int&>, int>);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
