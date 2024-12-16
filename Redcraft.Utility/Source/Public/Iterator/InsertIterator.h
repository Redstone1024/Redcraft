#pragma once

#include "CoreTypes.h"
#include "Iterator/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Noncopyable.h"
#include "Templates/Utility.h"

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

NAMESPACE_PRIVATE_END

/** Creates an iterator adapter inserted in the front of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeFrontInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TInsertIterator([&Container]<typename T>(T&& A) { Container.PushFront(Forward<T>(A)); });
}

/** Creates an iterator adapter inserted in the back of the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeBackInserter(C& Container)
{
	return NAMESPACE_PRIVATE::TInsertIterator([&Container]<typename T>(T&& A) { Container.PushBack(Forward<T>(A)); });
}

/** Creates an iterator adapter inserted in the container. */
template <typename C>
NODISCARD FORCEINLINE constexpr auto MakeInserter(C& Container, const typename C::FConstIterator& InIter)
{
	return NAMESPACE_PRIVATE::TInsertIterator([&Container, Iter = InIter]<typename T>(T&& A) mutable { Iter = Container.Insert(Iter, Forward<T>(A)); });
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
