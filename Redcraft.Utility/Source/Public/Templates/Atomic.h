#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Memory/Alignment.h"
#include "Templates/Function.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Noncopyable.h"

#include <atomic>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * EMemoryOrder specifies how memory accesses, including regular, non-atomic memory accesses,
 * are to be ordered around an atomic operation. Absent any constraints on a multi-core system,
 * when multiple threads simultaneously read and write to several variables, one thread can observe
 * the values change in an order different from the order another thread wrote them. Indeed,
 * the apparent order of changes can even differ among multiple reader threads. Some similar effects
 * can occur even on uniprocessor systems due to compiler transformations allowed by the memory model.
 *
 * @see https://en.cppreference.com/w/cpp/atomic/memory_order
 */
enum class EMemoryOrder : uint8
{
	Relaxed                = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_relaxed),
	Consume                = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_consume),
	Acquire                = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_acquire),
	Release                = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_release),
	AcquireRelease         = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_acq_rel),
	SequentiallyConsistent = static_cast<TUnderlyingType<NAMESPACE_STD::memory_order>>(NAMESPACE_STD::memory_order_seq_cst),
};

#if BUILD_DEBUG

NAMESPACE_PRIVATE_BEGIN

FORCEINLINE void MemoryOrderCheck(EMemoryOrder Order, uint8 Require)
{
	switch (Order)
	{
	case EMemoryOrder::Relaxed:                checkf((Require) & 0x01, TEXT("Invalid memory order.")); break;
	case EMemoryOrder::Consume:                checkf((Require) & 0x02, TEXT("Invalid memory order.")); break;
	case EMemoryOrder::Acquire:                checkf((Require) & 0x04, TEXT("Invalid memory order.")); break;
	case EMemoryOrder::Release:                checkf((Require) & 0x08, TEXT("Invalid memory order.")); break;
	case EMemoryOrder::AcquireRelease:         checkf((Require) & 0x10, TEXT("Invalid memory order.")); break;
	case EMemoryOrder::SequentiallyConsistent: checkf((Require) & 0x20, TEXT("Invalid memory order.")); break;
	default: check_no_entry();
	}
}

NAMESPACE_PRIVATE_END

#define MEMORY_ORDER_CHECK(Order, Require) NAMESPACE_PRIVATE::MemoryOrderCheck(Order, Require)

#else

#define MEMORY_ORDER_CHECK(Order, Require)

#endif

NAMESPACE_PRIVATE_BEGIN

template <typename T, bool bIsRef>
struct TAtomicImpl : FSingleton
{
protected:

	using FNativeAtomic = TConditional<bIsRef, NAMESPACE_STD::atomic_ref<T>, NAMESPACE_STD::atomic<T>>;

public:

	using FValueType = T;

	/** Indicates that the type is always lock-free */
	static constexpr bool bIsAlwaysLockFree = FNativeAtomic::is_always_lock_free;

	/** Indicates the required alignment of an object to be referenced by TAtomicRef. */
	static constexpr size_t RequiredAlignment = NAMESPACE_STD::atomic_ref<T>::required_alignment;

	/** Constructs an atomic object. */
	FORCEINLINE constexpr TAtomicImpl()                  requires (!bIsRef) : NativeAtomic()         { }
	FORCEINLINE constexpr TAtomicImpl(FValueType Desired) requires (!bIsRef) : NativeAtomic(Desired) { }

	/** Constructs an atomic reference. */
	FORCEINLINE explicit TAtomicImpl(FValueType&  Desired) requires (bIsRef) : NativeAtomic(Desired) { check(Memory::IsAligned(&Desired, RequiredAlignment)); }
	FORCEINLINE          TAtomicImpl(TAtomicImpl& InValue) requires (bIsRef) : NativeAtomic(InValue) { }

	/** Stores a value into an atomic object. */
	FORCEINLINE FValueType operator=(FValueType Desired)                                       { return NativeAtomic = Desired; }
	FORCEINLINE FValueType operator=(FValueType Desired) volatile requires (bIsAlwaysLockFree) { return NativeAtomic = Desired; }

	/** Atomically replaces the value of the atomic object with a non-atomic argument. */
	FORCEINLINE void Store(FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)                                       { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Store(FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (bIsAlwaysLockFree) { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically obtains the value of the atomic object. */
	NODISCARD FORCEINLINE FValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const                                       { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	NODISCARD FORCEINLINE FValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile requires (bIsAlwaysLockFree) { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Loads a value from an atomic object. */
	NODISCARD FORCEINLINE operator FValueType() const                                       { return static_cast<FValueType>(NativeAtomic); }
	NODISCARD FORCEINLINE operator FValueType() const volatile requires (bIsAlwaysLockFree) { return static_cast<FValueType>(NativeAtomic); }

	/** Atomically replaces the value of the atomic object and obtains the value held previously. */
	NODISCARD FORCEINLINE FValueType Exchange(FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) { return NativeAtomic.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	NODISCARD FORCEINLINE FValueType Exchange(FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (bIsAlwaysLockFree) { return NativeAtomic.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically compares the value of the atomic object with non-atomic argument and performs atomic exchange if equal or atomic load if not. */
	NODISCARD FORCEINLINE bool CompareExchange(FValueType& Expected, FValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false)
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}

	/** Atomically compares the value of the atomic object with non-atomic argument and performs atomic exchange if equal or atomic load if not. */
	NODISCARD FORCEINLINE bool CompareExchange(FValueType& Expected, FValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false) volatile requires (bIsAlwaysLockFree)
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}

	/** Atomically compares the value of the atomic object with non-atomic argument and performs atomic exchange if equal or atomic load if not. */
	NODISCARD FORCEINLINE bool CompareExchange(FValueType& Expected, FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false)
	{
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}

	/** Atomically compares the value of the atomic object with non-atomic argument and performs atomic exchange if equal or atomic load if not. */
	NODISCARD FORCEINLINE bool CompareExchange(FValueType& Expected, FValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false) volatile requires (bIsAlwaysLockFree)
	{
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}

	/** Blocks the thread until notified and the atomic value changes. */
	FORCEINLINE void Wait(FValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); NativeAtomic.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(FValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); NativeAtomic.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Notifies at least one or all threads blocked waiting on the atomic object. */
	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) NativeAtomic.notify_all(); else NativeAtomic.notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) NativeAtomic.notify_all(); else NativeAtomic.notify_one(); }

	/** Atomically executes the 'Func' on the value stored in the atomic object and obtains the value held previously. */
	template <typename F> requires (CInvocableResult<FValueType, F, FValueType>)
	FORCEINLINE FValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)
	{
		FValueType Temp(Load(EMemoryOrder::Relaxed));
		// We do a weak read here because we require a loop.
		while (!CompareExchange(Temp, InvokeResult<FValueType>(Forward<F>(Func), Temp), Order, true));
		return Temp;
	}

	/** Atomically executes the 'Func' on the value stored in the atomic object and obtains the value held previously. */
	template <typename F> requires (CInvocableResult<FValueType, F, FValueType> && bIsAlwaysLockFree)
	FORCEINLINE FValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile
	{
		FValueType Temp(Load(EMemoryOrder::Relaxed));
		// We do a weak read here because we require a loop.
		while (!CompareExchange(Temp, InvokeResult<FValueType>(Forward<F>(Func), Temp), Order, true));
		return Temp;
	}

	/** Atomically adds the argument to the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchAdd(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchAdd(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically adds the argument to the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CPointer<T>                     ) { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically subtracts the argument from the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchSub(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchSub(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically subtracts the argument from the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CPointer<T>                     ) { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically multiples the argument from the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchMul(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](FValueType Old) -> FValueType { return Old * InValue; }); }
	FORCEINLINE FValueType FetchMul(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](FValueType Old) -> FValueType { return Old * InValue; }); }

	/** Atomically divides the argument from the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchDiv(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](FValueType Old) -> FValueType { return Old / InValue; }); }
	FORCEINLINE FValueType FetchDiv(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](FValueType Old) -> FValueType { return Old / InValue; }); }

	/** Atomically models the argument from the value stored in the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchMod(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old % InValue; }); }
	FORCEINLINE FValueType FetchMod(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old % InValue; }); }

	/** Atomically performs bitwise AND between the argument and the value of the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchAnd(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchAnd(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically performs bitwise OR between the argument and the value of the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchOr(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchOr(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically performs bitwise XOR between the argument and the value of the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchXor(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE FValueType FetchXor(FValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically performs bitwise LSH between the argument and the value of the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old << InValue; }); }
	FORCEINLINE FValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old << InValue; }); }

	/** Atomically performs bitwise RSH between the argument and the value of the atomic object and obtains the value held previously. */
	FORCEINLINE FValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old >> InValue; }); }
	FORCEINLINE FValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](FValueType Old) -> FValueType { return Old >> InValue; }); }

	/** Increments the atomic value by one. */
	FORCEINLINE FValueType operator++()          requires ((CIntegral<T> || CPointer<T>)                     ) { return ++NativeAtomic; }
	FORCEINLINE FValueType operator++() volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return ++NativeAtomic; }

	/** Increments the atomic value by one. */
	FORCEINLINE FValueType operator++(int)          requires ((CIntegral<T> || CPointer<T>)                     ) { return NativeAtomic++; }
	FORCEINLINE FValueType operator++(int) volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return NativeAtomic++; }

	/** Decrements the atomic value by one. */
	FORCEINLINE FValueType operator--()          requires ((CIntegral<T> || CPointer<T>)                     ) { return --NativeAtomic; }
	FORCEINLINE FValueType operator--() volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return --NativeAtomic; }

	/** Decrements the atomic value by one. */
	FORCEINLINE FValueType operator--(int)          requires ((CIntegral<T> || CPointer<T>)                     ) { return NativeAtomic--; }
	FORCEINLINE FValueType operator--(int) volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return NativeAtomic--; }

	/** Adds with the atomic value. */
	FORCEINLINE FValueType operator+=(FValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return NativeAtomic += InValue; }
	FORCEINLINE FValueType operator+=(FValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return NativeAtomic += InValue; }

	/** Adds with the atomic value. */
	FORCEINLINE FValueType operator+=(ptrdiff InValue)          requires (CPointer<T>                     ) { return NativeAtomic += InValue; }
	FORCEINLINE FValueType operator+=(ptrdiff InValue) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic += InValue; }

	/** Subtracts with the atomic value. */
	FORCEINLINE FValueType operator-=(FValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return NativeAtomic -= InValue; }
	FORCEINLINE FValueType operator-=(FValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return NativeAtomic -= InValue; }

	/** Subtracts with the atomic value. */
	FORCEINLINE FValueType operator-=(ptrdiff InValue)          requires (CPointer<T>                     ) { return NativeAtomic -= InValue; }
	FORCEINLINE FValueType operator-=(ptrdiff InValue) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic -= InValue; }

	/** Multiples with the atomic value. */
	FORCEINLINE FValueType operator*=(FValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return FetchMul(InValue) * InValue; }
	FORCEINLINE FValueType operator*=(FValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return FetchMul(InValue) * InValue; }

	/** Divides with the atomic value. */
	FORCEINLINE FValueType operator/=(FValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return FetchDiv(InValue) / InValue; }
	FORCEINLINE FValueType operator/=(FValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return FetchDiv(InValue) / InValue; }

	/** Models with the atomic value. */
	FORCEINLINE FValueType operator%=(FValueType InValue)          requires (CIntegral<T>                     ) { return FetchMod(InValue) % InValue; }
	FORCEINLINE FValueType operator%=(FValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchMod(InValue) % InValue; }

	/** Performs bitwise AND with the atomic value. */
	FORCEINLINE FValueType operator&=(FValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic &= InValue; }
	FORCEINLINE FValueType operator&=(FValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic &= InValue; }

	/** Performs bitwise OR with the atomic value. */
	FORCEINLINE FValueType operator|=(FValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic |= InValue; }
	FORCEINLINE FValueType operator|=(FValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic |= InValue; }

	/** Performs bitwise XOR with the atomic value. */
	FORCEINLINE FValueType operator^=(FValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic ^= InValue; }
	FORCEINLINE FValueType operator^=(FValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic ^= InValue; }

	/** Performs bitwise LSH with the atomic value. */
	FORCEINLINE FValueType operator<<=(size_t InValue)          requires (CIntegral<T>                     ) { return FetchLsh(InValue) << InValue; }
	FORCEINLINE FValueType operator<<=(size_t InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchLsh(InValue) << InValue; }

	/** Performs bitwise RSH with the atomic value. */
	FORCEINLINE FValueType operator>>=(size_t InValue)          requires (CIntegral<T>                     ) { return FetchRsh(InValue) >> InValue; }
	FORCEINLINE FValueType operator>>=(size_t InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchRsh(InValue) >> InValue; }

protected:

	FNativeAtomic NativeAtomic;

};

NAMESPACE_PRIVATE_END

template <typename T> requires (CTriviallyCopyable<T>
	&& CCopyConstructible<T> && CMoveConstructible<T>
	&& CCopyAssignable<T> && CMoveAssignable<T>)
struct TAtomic final : STRONG_INHERIT(NAMESPACE_PRIVATE::TAtomicImpl<T, false>);

template <typename T> requires (CTriviallyCopyable<T>)
struct TAtomicRef final : STRONG_INHERIT(NAMESPACE_PRIVATE::TAtomicImpl<T, true>);

template <typename T>
TAtomic(T) -> TAtomic<T>;

template <typename T>
TAtomicRef(T&) -> TAtomicRef<T>;

static_assert(sizeof(TAtomic<int32>) == sizeof(int32), "The byte size of TAtomic is unexpected");

/**
 * FAtomicFlag is an atomic boolean type. Unlike all specializations of TAtomic, it is guaranteed to be lock-free.
 * Unlike TAtomic<bool>, FAtomicFlag does not provide load or store operations.
 */
struct FAtomicFlag final : FSingleton
{
public:

	/** Constructs an atomic flag. */
	FORCEINLINE constexpr FAtomicFlag() = default;

	/** Atomically sets flag to false. */
	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically sets the flag to true and obtains its previous value. */
	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { return NativeAtomic.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { return NativeAtomic.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Atomically returns the value of the flag. */
	NODISCARD FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	NODISCARD FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Blocks the thread until notified and the atomic value changes. */
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(NativeAtomic).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(NativeAtomic).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	/** Notifies at least one or all threads blocked waiting on the atomic object. */
	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_one(); }

private:

	NAMESPACE_STD::atomic_flag NativeAtomic;

};

template <typename T>
NODISCARD inline T KillDependency(T InValue)
{
	T Temp(InValue);
	return Temp;
}

extern "C" FORCEINLINE void AtomicThreadFence(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) { NAMESPACE_STD::atomic_thread_fence(static_cast<NAMESPACE_STD::memory_order>(Order)); }
extern "C" FORCEINLINE void AtomicSignalFence(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) { NAMESPACE_STD::atomic_signal_fence(static_cast<NAMESPACE_STD::memory_order>(Order)); }

#undef MEMORY_ORDER_CHECK

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
