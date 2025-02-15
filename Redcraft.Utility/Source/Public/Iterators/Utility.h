#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T> using TWithReference = T&;

template <typename I> struct TIteratorElementImpl     {                             };
template <typename T> struct TIteratorElementImpl<T*> { using FType = TRemoveCV<T>; };

template <typename I> requires (requires { typename I::FElementType; })
struct TIteratorElementImpl<I> { using FType = typename I::FElementType; };

template <typename I> struct TIteratorPointerImpl     {                   };
template <typename T> struct TIteratorPointerImpl<T*> { using FType = T*; };

template <typename I> requires (requires(I& Iter) { { Iter.operator->() } -> CPointer; })
struct TIteratorPointerImpl<I> { using FType = decltype(DeclVal<I&>().operator->()); };

NAMESPACE_PRIVATE_END

template <typename T>
concept CReferenceable = requires { typename NAMESPACE_PRIVATE::TWithReference<T>; };

template <typename T>
concept CDereferenceable = requires(T& A) { { *A } -> CReferenceable; };

template <typename I>
using TIteratorElement = typename NAMESPACE_PRIVATE::TIteratorElementImpl<TRemoveCVRef<I>>::FType;

template <typename I>
using TIteratorPointer = typename NAMESPACE_PRIVATE::TIteratorPointerImpl<TRemoveCVRef<I>>::FType;

template <CReferenceable I>
using TIteratorReference = decltype(*DeclVal<I&>());

template <CReferenceable I> requires (requires(I& Iter) { { MoveTemp(*Iter) } -> CReferenceable; })
using TIteratorRValueReference = decltype(MoveTemp(*DeclVal<I&>()));

NAMESPACE_PRIVATE_BEGIN

template <typename I>
concept CIndirectlyReadable =
	requires(const I Iter)
	{
		typename TIteratorElement<I>;
		typename TIteratorReference<I>;
		typename TIteratorRValueReference<I>;
		{          *Iter  } -> CSameAs<TIteratorReference<I>>;
		{ MoveTemp(*Iter) } -> CSameAs<TIteratorRValueReference<I>>;
	}
	&& CSameAs<TIteratorElement<I>, TRemoveCVRef<TIteratorElement<I>>>
	&& CCommonReference<TIteratorReference<I>&&, TIteratorElement<I>&>
	&& CCommonReference<TIteratorReference<I>&&, TIteratorRValueReference<I>&&>
	&& CCommonReference<TIteratorRValueReference<I>&&, const TIteratorElement<I>&>;

NAMESPACE_PRIVATE_END

/** A concept specifies a type is indirectly readable by expression '*Iter'. */
template <typename I>
concept CIndirectlyReadable = NAMESPACE_PRIVATE::CIndirectlyReadable<TRemoveCVRef<I>>;

/** This is an example of an indirectly readable type, indicate the traits that define an indirectly readable type. */
template <CReferenceable T>
struct IIndirectlyReadable
{
	/**
	 * The element type of the indirectly readable type.
	 * It must be a non-const, non-volatile and non-reference type and can be referenced, i.e. not a void type.
	 */
	using FElementType = TRemoveCVRef<T>;

	/**
	 * Indirectly read the element from the indirectly readable type.
	 * The return type may not be const FElementType&, this concept only requires that the return type
	 * and FElementType has some relationship, such as copy constructible to FElementType if the type is copyable.
	 * This means that returning a proxy class castable to FElementType is also valid.
	 * If this is an iterator adaptor, use decltype(auto) to forward the return value.
	 */
	T operator*() const;
};

// Use IIndirectlyReadable<int> represents an indirectly readable type and int is the regular element type.
static_assert(CIndirectlyReadable<IIndirectlyReadable<int>> && CRegular<int>);

// The CIndirectlyReadable requires this code is valid.
static_assert(
	requires(IIndirectlyReadable<int> Iter, int& A)
	{
		A = *Iter;
	}
);

/** A concept specifies a type is indirectly writable by expression '*Iter = A'. */
template <typename I, typename T>
concept CIndirectlyWritable =
	requires(I&& Iter, T&& A)
	{
		*Iter             = Forward<T>(A);
		*Forward<I>(Iter) = Forward<T>(A);
		const_cast<const TIteratorReference<I>&&>(*Iter)             = Forward<T>(A);
		const_cast<const TIteratorReference<I>&&>(*Forward<I>(Iter)) = Forward<T>(A);
	};

/** This is an example of an indirectly writable type, indicate the traits that define an indirectly writable type. */
template <CReferenceable T>
struct IIndirectlyWritable
{
	/**
	 * Indirectly write the element from the indirectly writable type.
	 * The return type may not be T&, this concept only requires that the return type and T has some relationship,
	 * such as can be assigned from T& if the type is copyable or T&& if the type is movable.
	 * This means that returning a proxy class can be assigned from T is also valid.
	 * If this is also an indirectly readable type, the equivalent value is read after writing.
	 * If this is an iterator adaptor, use decltype(auto) to forward the return value.
	 */
	T operator*() const;
};

// Use IIndirectlyWritable<int> represents an indirectly writable type and int is the regular element type.
static_assert(CIndirectlyWritable<IIndirectlyWritable<int&>, int> && CRegular<int>);

// The CIndirectlyWritable requires this code is valid.
static_assert(
	requires(IIndirectlyWritable<int&> Iter, int& A)
	{
		*Iter = A;
	}
);

/** A concept specifies a type is incrementable by expression '++Iter' and the type must be movable. */
template <typename I>
concept CWeaklyIncrementable = CMovable<I>
	&& requires(I Iter) { { ++Iter } -> CSameAs<I&>; Iter++; };

/** This is an example of a weakly incrementable type, indicate the traits that define a weakly incrementable type. */
struct IWeaklyIncrementable
{
	IWeaklyIncrementable(IWeaklyIncrementable&&);
	IWeaklyIncrementable* operator=(IWeaklyIncrementable&&);

	IWeaklyIncrementable& operator++();

	/** Post-increment operator. Specify, the concept not requires the return type is any specific type, so the return type can be void. */
	void operator++(int);
};

// Use IWeaklyIncrementable represents a weakly incrementable type.
static_assert(CWeaklyIncrementable<IWeaklyIncrementable>);

/**
 * A concept specifies a type is incrementable by expression 'Iter++' and the expression returns the original value.
 * In addition, the type must be default constructible, copyable and weakly equality comparable.
 */
template <typename I>
concept CIncrementable = CRegular<I> && CWeaklyIncrementable<I>
	&& requires(I Iter) { { Iter++ } -> CSameAs<I>; };

/**
 * This is an example of an incrementable type, indicate the traits that define an incrementable type.
 * The copy object of this type produced by copy constructor, copy assignment or post-increment
 * should produce the same effect as the original object when incrementing.
 */
struct IIncrementable /* : IWeaklyIncrementable */
{
	IIncrementable();
	IIncrementable(const IIncrementable&);
	IIncrementable(IIncrementable&&);                 // Also satisfies IWeaklyIncrementable.
	IIncrementable* operator=(const IIncrementable&);
	IIncrementable* operator=(IIncrementable&&);      // Also satisfies IWeaklyIncrementable.

	friend bool operator==(const IIncrementable&, const IIncrementable&);

	IIncrementable& operator++(); // Also satisfies IWeaklyIncrementable.

	/** Post-increment operator. Specify, the concept requires the return value is the original value before incrementing. */
	IIncrementable operator++(int);
};

// Use IIncrementable represents an incrementable type.
static_assert(CIncrementable<IIncrementable>);

/**
 * A concept specifies a type is potentially an iterator. It only requires weakly incrementable and dereferenceable.
 * This concept should only be used in scenarios where the specific type of the iterator is not important, such as iterator adapters.
 */
template <typename I>
concept CInputOrOutputIterator = CWeaklyIncrementable<I>
	&& requires(I Iter) { { *Iter } -> CReferenceable; };

/** This is an example of an input or output iterator, indicate the traits that define an input or output iterator. */
template <CReferenceable T>
struct IInputOrOutputIterator /* : IWeaklyIncrementable */
{
	// ~Begin CWeklyIncrementable.

	IInputOrOutputIterator(IInputOrOutputIterator&&);
	IInputOrOutputIterator* operator=(IInputOrOutputIterator&&);

	IInputOrOutputIterator& operator++();

	void operator++(int);

	// ~End CWeklyIncrementable.

	/** Dereference operator. It does not matter what the return type is, as long as it is referenceable. */
	T operator*() const;
};

// Use IInputOrOutputIterator represents an input or output iterator.
static_assert(CInputOrOutputIterator<IInputOrOutputIterator<int>>);

/** A concept specifies a type is an input iterator. */
template <typename I>
concept CInputIterator = CInputOrOutputIterator<I> && CIndirectlyReadable<I>;

/** This is an example of an input iterator, indicate the traits that define an input iterator. */
template <CReferenceable T>
struct IInputIterator /* : IInputOrOutputIterator, IIndirectlyReadable */
{
	// ~Begin CIndirectlyReadable.

	using FElementType = TRemoveCVRef<T>;

	// ~End CIndirectlyReadable.

	// ~Begin CInputOrOutputIterator.

	IInputIterator(IInputIterator&&);
	IInputIterator* operator=(IInputIterator&&);

	T operator*() const; // Also satisfies CIndirectlyReadable.

	IInputIterator& operator++();

	void operator++(int);

	// ~End CInputOrOutputIterator.
};

// Use IInputIterator<int> represents an input iterator and int is the regular element type.
static_assert(CInputIterator<IInputIterator<int>> && CRegular<int>);

// The CInputIterator requires this code is valid.
static_assert(
	requires(IInputIterator<int> Iter, int& A)
	{
		++Iter;
		Iter++;
		A = *++Iter;
		A =   *Iter;
	}
);

/** A concept specifies a type is an output iterator and expression '*Iter++ = A' is valid to write the element. */
template <typename I, typename T>
concept COutputIterator = CInputOrOutputIterator<I> && CIndirectlyWritable<I, T>
	&& requires(I Iter) { { Iter++ } -> CIndirectlyWritable<T>; };

/** This is an example of an output iterator, indicate the traits that define an output iterator. */
template <CReferenceable T>
struct IOutputIterator /* : IInputOrOutputIterator, IIndirectlyWritable<T> */
{
	// ~Begin CIndirectlyWritable.

	IOutputIterator(IOutputIterator&&);
	IOutputIterator* operator=(IOutputIterator&&);

	T operator*() const; // Also satisfies CIndirectlyWritable.

	IOutputIterator& operator++();

	/**
	 * Post-increment operator.
	 * Specify, the concept not requires the return type is self type,
	 * but requires the expression '*Iter++ = A;' is equivalent to '*Iter = A; ++Iter;'.
	 * This means that returning a proxy class that satisfies CIndirectlyWritable<T> is also valid.
	 * Also satisfies CIndirectlyWritable.
	 */
	IIndirectlyWritable<T> operator++(int);

	// ~End CIndirectlyWritable.
};

// Use IOutputIterator<int> represents an output iterator and int is the regular element type.
static_assert(COutputIterator<IOutputIterator<int&>, int> && CRegular<int>);

// The CInputIterator requires this code is valid.
static_assert(
	requires(IOutputIterator<int&> Iter, int& A)
	{
		++Iter;
		Iter++;
		*++Iter = A;
		*Iter++ = A;
		  *Iter = A;
	}
);

#define ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT public:                        \
	NODISCARD FORCEINLINE constexpr auto begin()       { return Begin(); } \
	NODISCARD FORCEINLINE constexpr auto begin() const { return Begin(); } \
	NODISCARD FORCEINLINE constexpr auto end()         { return End();   } \
	NODISCARD FORCEINLINE constexpr auto end()   const { return End();   }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
