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
template <CReferenceable T>
struct IForwardIterator /* : IInputIterator<T>, IIncrementable, ISentinelFor<IForwardIterator> */
{
	// ~Begin CInputIterator.

	using ElementType = TRemoveCVRef<T>;

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

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
