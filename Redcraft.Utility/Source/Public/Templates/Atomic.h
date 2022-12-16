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

	using NativeAtomicType = TConditional<bIsRef, NAMESPACE_STD::atomic_ref<T>, NAMESPACE_STD::atomic<T>>;

public:

	using ValueType = T;

	static constexpr bool bIsAlwaysLockFree = NativeAtomicType::is_always_lock_free;

	static constexpr size_t RequiredAlignment = NAMESPACE_STD::atomic_ref<T>::required_alignment;

	FORCEINLINE constexpr TAtomicImpl()                  requires (!bIsRef) : NativeAtomic()        { };
	FORCEINLINE constexpr TAtomicImpl(ValueType Desired) requires (!bIsRef) : NativeAtomic(Desired) { };

	FORCEINLINE explicit TAtomicImpl(ValueType&   Desired) requires (bIsRef) : NativeAtomic(Desired) { check(Memory::IsAligned(&Desired, RequiredAlignment)); };
	FORCEINLINE          TAtomicImpl(TAtomicImpl& InValue) requires (bIsRef) : NativeAtomic(InValue) { };

	FORCEINLINE ValueType operator=(ValueType Desired)                                       { return NativeAtomic = Desired; }
	FORCEINLINE ValueType operator=(ValueType Desired) volatile requires (bIsAlwaysLockFree) { return NativeAtomic = Desired; }

	FORCEINLINE void Store(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)                                       { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Store(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (bIsAlwaysLockFree) { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const                                       { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile requires (bIsAlwaysLockFree) { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE operator ValueType() const                                       { return static_cast<ValueType>(NativeAtomic); }
	FORCEINLINE operator ValueType() const volatile requires (bIsAlwaysLockFree) { return static_cast<ValueType>(NativeAtomic); }
	
	FORCEINLINE ValueType Exchange(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)                                       { return NativeAtomic.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType Exchange(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (bIsAlwaysLockFree) { return NativeAtomic.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false)
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}
	
	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false) volatile requires (bIsAlwaysLockFree)
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}

	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false)
	{
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}

	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false) volatile requires (bIsAlwaysLockFree)
	{
		if (bIsWeak) return NativeAtomic.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return NativeAtomic.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}
	
	FORCEINLINE void Wait(ValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); NativeAtomic.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(ValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); NativeAtomic.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) NativeAtomic.notify_all(); else NativeAtomic.notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) NativeAtomic.notify_all(); else NativeAtomic.notify_one(); }
	
	template <typename F> requires (CInvocableResult<ValueType, F, ValueType>)
	FORCEINLINE ValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)
	{
		ValueType Temp(Load(EMemoryOrder::Relaxed));
		while (!CompareExchange(Temp, InvokeResult<ValueType>(Forward<F>(Func), Temp), Order));
		return Temp;
	}

	template <typename F> requires (CInvocableResult<ValueType, F, ValueType> && bIsAlwaysLockFree)
	FORCEINLINE ValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile
	{
		ValueType Temp(Load(EMemoryOrder::Relaxed));
		while (!CompareExchange(Temp, InvokeResult<ValueType>(Forward<F>(Func), Temp), Order));
		return Temp;
	}

	FORCEINLINE ValueType FetchAdd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAdd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CPointer<T>                     ) { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchSub(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchSub(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CPointer<T>                     ) { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchMul(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old * InValue; }); }
	FORCEINLINE ValueType FetchMul(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old * InValue; }); }
	
	FORCEINLINE ValueType FetchDiv(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old / InValue; }); }
	FORCEINLINE ValueType FetchDiv(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old / InValue; }); }
	
	FORCEINLINE ValueType FetchMod(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old % InValue; }); }
	FORCEINLINE ValueType FetchMod(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old % InValue; }); }
	
	FORCEINLINE ValueType FetchAnd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAnd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
		
	FORCEINLINE ValueType FetchOr(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchOr(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchXor(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return NativeAtomic.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchXor(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old << InValue; }); }
	FORCEINLINE ValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old << InValue; }); }
	
	FORCEINLINE ValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T>                     ) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old >> InValue; }); }
	FORCEINLINE ValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchFn([InValue](ValueType Old) -> ValueType { return Old >> InValue; }); }
	
	FORCEINLINE ValueType operator++()          requires ((CIntegral<T> || CPointer<T>)                     ) { return ++NativeAtomic; }
	FORCEINLINE ValueType operator++() volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return ++NativeAtomic; }
	
	FORCEINLINE ValueType operator++(int)          requires ((CIntegral<T> || CPointer<T>)                     ) { return NativeAtomic++; }
	FORCEINLINE ValueType operator++(int) volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return NativeAtomic++; }
	
	FORCEINLINE ValueType operator--()          requires ((CIntegral<T> || CPointer<T>)                     ) { return --NativeAtomic; }
	FORCEINLINE ValueType operator--() volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return --NativeAtomic; }
	
	FORCEINLINE ValueType operator--(int)          requires ((CIntegral<T> || CPointer<T>)                     ) { return NativeAtomic--; }
	FORCEINLINE ValueType operator--(int) volatile requires ((CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree) { return NativeAtomic--; }
	
	FORCEINLINE ValueType operator+=(ValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return NativeAtomic += InValue; }
	FORCEINLINE ValueType operator+=(ValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return NativeAtomic += InValue; }
	
	FORCEINLINE ValueType operator+=(ptrdiff InValue)          requires (CPointer<T>                     ) { return NativeAtomic += InValue; }
	FORCEINLINE ValueType operator+=(ptrdiff InValue) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic += InValue; }
	
	FORCEINLINE ValueType operator-=(ValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return NativeAtomic -= InValue; }
	FORCEINLINE ValueType operator-=(ValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return NativeAtomic -= InValue; }
	
	FORCEINLINE ValueType operator-=(ptrdiff InValue)          requires (CPointer<T>                     ) { return NativeAtomic -= InValue; }
	FORCEINLINE ValueType operator-=(ptrdiff InValue) volatile requires (CPointer<T> && bIsAlwaysLockFree) { return NativeAtomic -= InValue; }
	
	FORCEINLINE ValueType operator*=(ValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return FetchMul(InValue) * InValue; }
	FORCEINLINE ValueType operator*=(ValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return FetchMul(InValue) * InValue; }
	
	FORCEINLINE ValueType operator/=(ValueType InValue)          requires ((CIntegral<T> || CFloatingPoint<T>)                     ) { return FetchDiv(InValue) / InValue; }
	FORCEINLINE ValueType operator/=(ValueType InValue) volatile requires ((CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree) { return FetchDiv(InValue) / InValue; }
	
	FORCEINLINE ValueType operator%=(ValueType InValue)          requires (CIntegral<T>                     ) { return FetchMod(InValue) % InValue; }
	FORCEINLINE ValueType operator%=(ValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchMod(InValue) % InValue; }
	
	FORCEINLINE ValueType operator&=(ValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic &= InValue; }
	FORCEINLINE ValueType operator&=(ValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic &= InValue; }
	
	FORCEINLINE ValueType operator|=(ValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic |= InValue; }
	FORCEINLINE ValueType operator|=(ValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic |= InValue; }
	
	FORCEINLINE ValueType operator^=(ValueType InValue)          requires (CIntegral<T>                     ) { return NativeAtomic ^= InValue; }
	FORCEINLINE ValueType operator^=(ValueType InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return NativeAtomic ^= InValue; }
	
	FORCEINLINE ValueType operator<<=(size_t InValue)          requires (CIntegral<T>                     ) { return FetchLsh(InValue) << InValue; }
	FORCEINLINE ValueType operator<<=(size_t InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchLsh(InValue) << InValue; }
	
	FORCEINLINE ValueType operator>>=(size_t InValue)          requires (CIntegral<T>                     ) { return FetchRsh(InValue) >> InValue; }
	FORCEINLINE ValueType operator>>=(size_t InValue) volatile requires (CIntegral<T> && bIsAlwaysLockFree) { return FetchRsh(InValue) >> InValue; }
	
protected:

	NativeAtomicType NativeAtomic;

};

NAMESPACE_PRIVATE_END

template <typename T> requires (CTriviallyCopyable<T>
	&& CCopyConstructible<T> && CMoveConstructible<T>
	&& CCopyAssignable<T> && CMoveAssignable<T>)
struct TAtomic : STRONG_INHERIT(NAMESPACE_PRIVATE::TAtomicImpl<T, false>);

template <typename T> requires (CTriviallyCopyable<T>)
struct TAtomicRef : STRONG_INHERIT(NAMESPACE_PRIVATE::TAtomicImpl<T, true>);

template <typename T>
TAtomic(T) -> TAtomic<T>;

template <typename T>
TAtomicRef(T&) -> TAtomicRef<T>;

static_assert(sizeof(TAtomic<int32>) == sizeof(int32), "The byte size of TAtomic is unexpected");

struct FAtomicFlag : FSingleton
{
public:

	FORCEINLINE constexpr FAtomicFlag() : NativeAtomic() { };

	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); NativeAtomic.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { return NativeAtomic.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { return NativeAtomic.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return NativeAtomic.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(NativeAtomic).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(NativeAtomic).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(NativeAtomic).notify_one(); }
	
private:

	NAMESPACE_STD::atomic_flag NativeAtomic;

};

template <typename T>
inline T KillDependency(T InValue)
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
