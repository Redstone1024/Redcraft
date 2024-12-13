#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

/**
 * A concept specifies a type is a sentinel for an iterator and expression 'Iter == Sentinel' is valid.
 * In addition, the type must be default constructible and copyable.
 */
template <typename S, typename I>
concept CSentinelFor = CSemiregular<S> && CInputOrOutputIterator<I> && CWeaklyEqualityComparable<S, I>;

/** This is an example of a sentinel for an iterator, indicate the traits that define a sentinel for an iterator. */
template <CInputOrOutputIterator I>
struct ISentinelFor
{
	/** Default constructor. */
	ISentinelFor();

	/** Copy constructor. */
	ISentinelFor(const ISentinelFor&);

	/** Copy assignment operator. */
	ISentinelFor* operator=(const ISentinelFor&);

	/** Equality operator. */
	bool operator==(const I&) const&;
};

// Use 'ISentinelFor' represents a sentinel for an iterator.
static_assert(CSentinelFor<ISentinelFor<IInputOrOutputIterator<int>>, IInputOrOutputIterator<int>>);

// The 'CSentinelFor' requires this code is valid.
static_assert(
	requires(ISentinelFor<IInputOrOutputIterator<int>> Sentinel, IInputOrOutputIterator<int> Iter)
	{
		{ Iter == Sentinel } -> CBooleanTestable;
		{ Sentinel == Iter } -> CBooleanTestable;
	}
);

/** Disable the 'CSizedSentinelFor' concept for specific types. */
template <typename S, typename I>
inline constexpr bool bDisableSizedSentinelFor = false;

/**
 * A concept specifies a type is a sized sentinel for an iterator and expressions 'Sentinel - Iter' and 'Iter - Sentinel' are valid,
 * and the 'Sentinel - Iter' is equal to negative 'Iter - Sentinel'.
 * In addition, the type must be default constructible and copyable.
 */
template <typename S, typename I>
concept CSizedSentinelFor = CSentinelFor<S, I>
	&& !bDisableSizedSentinelFor<TRemoveCVRef<S>, TRemoveCVRef<I>>
	&& requires(const I& Iter, const S& Sentinel)
	{
		{ Sentinel - Iter } -> CSameAs<ptrdiff>;
		{ Iter - Sentinel } -> CSameAs<ptrdiff>;
	};

/** This is an example of a sized sentinel for an iterator, indicate the traits that define a sized sentinel for an iterator. */
template <CInputOrOutputIterator I>
struct ISizedSentinelFor /* : ISentinelFor<I> */
{
	/** Default constructor. */
	ISizedSentinelFor();

	/** Copy constructor. */
	ISizedSentinelFor(const ISizedSentinelFor&);

	/** Copy assignment operator. */
	ISizedSentinelFor& operator=(const ISizedSentinelFor&);

	/** Equality operator. */
	bool operator==(const I&) const&;

	/** Subtraction operator. The 'Sentinel - Iter' is equal to negative 'Iter - Sentinel'. */
	friend ptrdiff operator-(const I&, const ISizedSentinelFor&);
	friend ptrdiff operator-(const ISizedSentinelFor&, const I&);
};

// Use 'ISizedSentinelFor' represents a sized sentinel for an iterator.
static_assert(CSizedSentinelFor<ISizedSentinelFor<IInputOrOutputIterator<int>>, IInputOrOutputIterator<int>>);

// The 'CSentinelFor' requires this code is valid.
static_assert(
	requires(ISizedSentinelFor<IInputOrOutputIterator<int>> Sentinel, IInputOrOutputIterator<int> Iter)
	{
		{ Iter == Sentinel } -> CBooleanTestable;
		{ Sentinel == Iter } -> CBooleanTestable;
		{ Iter - Sentinel }  -> CSameAs<ptrdiff>;
		{ Sentinel - Iter }  -> CSameAs<ptrdiff>;
	}
);

#if PLATFORM_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
