#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Iterators/Utility.h"
#include "Templates/Utility.h"
#include "Templates/Noncopyable.h"
#include "Templates/Invoke.h"
#include "Memory/Address.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename F> class TInsertProxy;
template <typename F> class TPostIncrementProxy;
template <typename F> class TInsertIterator;

template <typename F>
class TInsertProxy final : private FSingleton
{
public:

#	if	DO_CHECK
	FORCEINLINE ~TInsertProxy() { checkf(bIsProduced, TEXT("Exception insert, Ensures that the value is assigned to the inserter.")); }
#	endif

	template <typename T> requires (CInvocable<F, T>)
	FORCEINLINE constexpr void operator=(T&& InValue) const
	{
		checkf(!bIsProduced, TEXT("Exception insert, Ensure that no multiple values are assigned to the inserter."));
		Invoke(Iter.Storage, Forward<T>(InValue));
		check_code({ bIsProduced = true; });
	}

private:

	TInsertIterator<F>& Iter;

#	if	DO_CHECK
	mutable bool bIsProduced;
#	endif

	FORCEINLINE constexpr TInsertProxy(TInsertIterator<F>& InIter) : Iter(InIter) { check_code({ bIsProduced = false; }); }

	template <typename> friend class TPostIncrementProxy;
	template <typename> friend class TInsertIterator;
};

static_assert(CAssignableFrom<TInsertProxy<void(*)(int)>, int>);

template <typename F>
class TPostIncrementProxy : private FSingleton
{
public:

#	if	DO_CHECK
	FORCEINLINE ~TPostIncrementProxy() { checkf(bIsProduced, TEXT("Exception insert, Ensures that the value is assigned to the inserter.")); }
#	endif

	NODISCARD FORCEINLINE constexpr TInsertProxy<F> operator*() const
	{
		checkf(!bIsProduced, TEXT("Exception insert, Ensure that no multiple values are assigned to the inserter."));
		check_code({ bIsProduced = true; });
		return TInsertProxy(Iter);
	}

private:

	TInsertIterator<F>& Iter;

#	if	DO_CHECK
	mutable bool bIsProduced;
#	endif

	FORCEINLINE constexpr TPostIncrementProxy(TInsertIterator<F>& InIter) : Iter(InIter) { check_code({ bIsProduced = false; }); }

	template <typename> friend class TInsertProxy;
	template <typename> friend class TInsertIterator;
};

static_assert(CIndirectlyWritable<TPostIncrementProxy<void(*)(int)>, int>);

template <typename F>
class TInsertIterator final : private FNoncopyable
{
public:

	FORCEINLINE constexpr TInsertIterator() requires (CDefaultConstructible<F>) = default;

	FORCEINLINE constexpr explicit TInsertIterator(F InInserter) : Storage(MoveTemp(InInserter)) { check_code({ bIsProduced = false; }); }

	FORCEINLINE constexpr TInsertIterator(TInsertIterator&&)            = default;
	FORCEINLINE constexpr TInsertIterator& operator=(TInsertIterator&&) = default;

	NODISCARD FORCEINLINE constexpr TInsertProxy<F> operator*()
	{
		checkf(!bIsProduced, TEXT("Exception insert, Ensure that no multiple values are assigned to the inserter."));
		check_code({ bIsProduced = true; });
		return TInsertProxy<F>(*this);
	}

	FORCEINLINE constexpr TInsertIterator& operator++() { check_code({ bIsProduced = false; }); return *this; }

	FORCEINLINE constexpr TPostIncrementProxy<F> operator++(int)
	{
		checkf(!bIsProduced, TEXT("Exception insert, Ensure that no multiple values are assigned to the inserter."));
		return TPostIncrementProxy<F>(*this);
	}

private:

	F Storage;

#	if DO_CHECK
	bool bIsProduced;
#	endif

	template <typename> friend class TInsertProxy;
	template <typename> friend class TPostIncrementProxy;
};

static_assert(COutputIterator<TInsertIterator<void(*)(int)>, int>);

template <typename C>
class FFrontInserter
{
public:

	FORCEINLINE constexpr explicit FFrontInserter(C& InContainer) : Container(AddressOf(InContainer)) { }

	template <typename T>
	FORCEINLINE constexpr void operator()(T&& A) { Container->PushFront(Forward<T>(A)); }

private:

	C* Container;

};

template <typename C>
class FBackInserter
{
public:

	FORCEINLINE constexpr explicit FBackInserter(C& InContainer) : Container(AddressOf(InContainer)) { }

	template <typename T>
	FORCEINLINE constexpr void operator()(T&& A) { Container->PushBack(Forward<T>(A)); }

private:

	C* Container;

};

template <typename C>
class FInserter
{
public:

	template <typename I>
	FORCEINLINE constexpr FInserter(C& InContainer, I&& InIter) : Container(AddressOf(InContainer)), Iter(Forward<I>(InIter)) { }

	template <typename T>
	FORCEINLINE constexpr void operator()(T&& A) { Iter = Container->Insert(Iter, Forward<T>(A)); ++Iter; }

private:

	C* Container;
	typename C::FConstIterator Iter;

};

NAMESPACE_PRIVATE_END

/** Creates an iterator adapter inserted in the front of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeFrontInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TInsertIterator(NAMESPACE_PRIVATE::FFrontInserter(Container));
}

/** Creates an iterator adapter inserted in the back of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeBackInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TInsertIterator(NAMESPACE_PRIVATE::FBackInserter(Container));
}

/** Creates an iterator adapter inserted in the container. */
template <typename C, typename I>
NODISCARD FORCEINLINE constexpr auto MakeInserter(C& Container, I&& InIter)
{
	return NAMESPACE_PRIVATE::TInsertIterator(NAMESPACE_PRIVATE::FInserter(Container, Forward<I>(InIter)));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
