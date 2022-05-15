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
	Relaxed                = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_relaxed),
	Consume                = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_consume),
	Acquire                = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_acquire),
	Release                = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_release),
	AcquireRelease         = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_acq_rel),
	SequentiallyConsistent = static_cast<typename TUnderlyingType<NAMESPACE_STD::memory_order>::Type>(NAMESPACE_STD::memory_order_seq_cst),
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

template <typename T, bool bIsRef = false> requires CTriviallyCopyable<T>
	&& TIsCopyConstructible<T>::Value && TIsMoveConstructible<T>::Value
	&& TIsCopyAssignable<T>::Value && TIsMoveAssignable<T>::Value
struct TAtomic : public FSingleton
{
protected:

	using ElementType = typename TConditional<bIsRef, NAMESPACE_STD::atomic_ref<T>, NAMESPACE_STD::atomic<T>>::Type;

public:

	using ValueType = T;

	static constexpr bool bIsAlwaysLockFree = ElementType::is_always_lock_free;

	static constexpr size_t RequiredAlignment = NAMESPACE_STD::atomic_ref<T>::required_alignment;

	constexpr TAtomic()                  requires (!bIsRef) : Element()        { };
	constexpr TAtomic(ValueType Desired) requires (!bIsRef) : Element(Desired) { };

	FORCEINLINE explicit TAtomic(ValueType& Desired) requires (bIsRef) : Element(Desired) { check(Memory::IsAligned(&Desired, RequiredAlignment)); };
	FORCEINLINE          TAtomic(TAtomic&   InValue) requires (bIsRef) : Element(InValue) { };

	FORCEINLINE ValueType operator=(ValueType Desired)                                     { return Element = Desired; }
	FORCEINLINE ValueType operator=(ValueType Desired) volatile requires bIsAlwaysLockFree { return Element = Desired; }

	FORCEINLINE void Store(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)                                     { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); Element.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Store(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires bIsAlwaysLockFree { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); Element.store(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const                                     { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return Element.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile requires bIsAlwaysLockFree { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return Element.load(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE operator ValueType() const                                     { return static_cast<ValueType>(Element); }
	FORCEINLINE operator ValueType() const volatile requires bIsAlwaysLockFree { return static_cast<ValueType>(Element); }
	
	FORCEINLINE ValueType Exchange(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)                                     { return Element.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType Exchange(ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires bIsAlwaysLockFree { return Element.exchange(Desired, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false)
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return Element.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return Element.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}
	
	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Success, EMemoryOrder Failure, bool bIsWeak = false) volatile requires bIsAlwaysLockFree
	{
		MEMORY_ORDER_CHECK(Failure, 0x01 | 0x02 | 0x04 | 0x20);
		if (bIsWeak) return Element.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
		else       return Element.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Success), static_cast<NAMESPACE_STD::memory_order>(Failure));
	}

	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false)
	{
		if (bIsWeak) return Element.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return Element.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}

	FORCEINLINE bool CompareExchange(ValueType& Expected, ValueType Desired, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent, bool bIsWeak = false) volatile requires bIsAlwaysLockFree
	{
		if (bIsWeak) return Element.compare_exchange_weak(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
		else       return Element.compare_exchange_strong(Expected, Desired, static_cast<NAMESPACE_STD::memory_order>(Order));
	}
	
	FORCEINLINE void Wait(ValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); Element.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(ValueType Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); Element.wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) Element.notify_all(); else Element.notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) Element.notify_all(); else Element.notify_one(); }
	
	template <typename F> requires TIsInvocableResult<ValueType, F, ValueType>::Value
	FORCEINLINE ValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)
	{
		ValueType Temp(Load(EMemoryOrder::Relaxed));
		while (!CompareExchange(Temp, InvokeResult<ValueType>(Forward<F>(Func), Temp), Order));
		return Temp;
	}

	template <typename F> requires TIsInvocableResult<ValueType, F, ValueType>::Value && bIsAlwaysLockFree
	FORCEINLINE ValueType FetchFn(F&& Func, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile
	{
		ValueType Temp(Load(EMemoryOrder::Relaxed));
		while (!CompareExchange(Temp, InvokeResult<ValueType>(Forward<F>(Func), Temp), Order));
		return Temp;
	}

	FORCEINLINE ValueType FetchAdd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return Element.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAdd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return Element.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CPointer<T>                      { return Element.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAdd(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CPointer<T> && bIsAlwaysLockFree { return Element.fetch_add(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchSub(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return Element.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchSub(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return Element.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CPointer<T>                      { return Element.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchSub(ptrdiff InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CPointer<T> && bIsAlwaysLockFree { return Element.fetch_sub(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchMul(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old * InValue; }); }
	FORCEINLINE ValueType FetchMul(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old * InValue; }); }
	
	FORCEINLINE ValueType FetchDiv(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old / InValue; }); }
	FORCEINLINE ValueType FetchDiv(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old / InValue; }); }
	
	FORCEINLINE ValueType FetchMod(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old % InValue; }); }
	FORCEINLINE ValueType FetchMod(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old % InValue; }); }
	
	FORCEINLINE ValueType FetchAnd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return Element.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchAnd(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element.fetch_and(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
		
	FORCEINLINE ValueType FetchOr(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return Element.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchOr(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element.fetch_or(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchXor(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return Element.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE ValueType FetchXor(ValueType InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element.fetch_xor(InValue, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE ValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old << InValue; }); }
	FORCEINLINE ValueType FetchLsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old << InValue; }); }
	
	FORCEINLINE ValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          requires CIntegral<T>                      { return FetchFn([InValue](ValueType Old) -> ValueType { return Old >> InValue; }); }
	FORCEINLINE ValueType FetchRsh(size_t InValue, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchFn([InValue](ValueType Old) -> ValueType { return Old >> InValue; }); }
	
	FORCEINLINE ValueType operator++()          requires (CIntegral<T> || CPointer<T>)                      { return ++Element; }
	FORCEINLINE ValueType operator++() volatile requires (CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree { return ++Element; }
	
	FORCEINLINE ValueType operator++(int)          requires (CIntegral<T> || CPointer<T>)                      { return Element++; }
	FORCEINLINE ValueType operator++(int) volatile requires (CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree { return Element++; }
	
	FORCEINLINE ValueType operator--()          requires (CIntegral<T> || CPointer<T>)                      { return --Element; }
	FORCEINLINE ValueType operator--() volatile requires (CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree { return --Element; }
	
	FORCEINLINE ValueType operator--(int)          requires (CIntegral<T> || CPointer<T>)                      { return Element--; }
	FORCEINLINE ValueType operator--(int) volatile requires (CIntegral<T> || CPointer<T>) && bIsAlwaysLockFree { return Element--; }
	
	FORCEINLINE ValueType operator+=(ValueType InValue)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return Element += InValue; }
	FORCEINLINE ValueType operator+=(ValueType InValue) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return Element += InValue; }
	
	FORCEINLINE ValueType operator+=(ptrdiff InValue)          requires CPointer<T>                      { return Element += InValue; }
	FORCEINLINE ValueType operator+=(ptrdiff InValue) volatile requires CPointer<T> && bIsAlwaysLockFree { return Element += InValue; }
	
	FORCEINLINE ValueType operator-=(ValueType InValue)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return Element -= InValue; }
	FORCEINLINE ValueType operator-=(ValueType InValue) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return Element -= InValue; }
	
	FORCEINLINE ValueType operator-=(ptrdiff InValue)          requires CPointer<T>                      { return Element -= InValue; }
	FORCEINLINE ValueType operator-=(ptrdiff InValue) volatile requires CPointer<T> && bIsAlwaysLockFree { return Element -= InValue; }
	
	FORCEINLINE ValueType operator*=(ValueType InValue)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchMul(InValue) * InValue; }
	FORCEINLINE ValueType operator*=(ValueType InValue) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchMul(InValue) * InValue; }
	
	FORCEINLINE ValueType operator/=(ValueType InValue)          requires (CIntegral<T> || CFloatingPoint<T>)                      { return FetchDiv(InValue) / InValue; }
	FORCEINLINE ValueType operator/=(ValueType InValue) volatile requires (CIntegral<T> || CFloatingPoint<T>) && bIsAlwaysLockFree { return FetchDiv(InValue) / InValue; }
	
	FORCEINLINE ValueType operator%=(ValueType InValue)          requires CIntegral<T>                      { return FetchMod(InValue) % InValue; }
	FORCEINLINE ValueType operator%=(ValueType InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchMod(InValue) % InValue; }
	
	FORCEINLINE ValueType operator&=(ValueType InValue)          requires CIntegral<T>                      { return Element &= InValue; }
	FORCEINLINE ValueType operator&=(ValueType InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element &= InValue; }
	
	FORCEINLINE ValueType operator|=(ValueType InValue)          requires CIntegral<T>                      { return Element |= InValue; }
	FORCEINLINE ValueType operator|=(ValueType InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element |= InValue; }
	
	FORCEINLINE ValueType operator^=(ValueType InValue)          requires CIntegral<T>                      { return Element ^= InValue; }
	FORCEINLINE ValueType operator^=(ValueType InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return Element ^= InValue; }
	
	FORCEINLINE ValueType operator<<=(size_t InValue)          requires CIntegral<T>                      { return FetchLsh(InValue) << InValue; }
	FORCEINLINE ValueType operator<<=(size_t InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchLsh(InValue) << InValue; }
	
	FORCEINLINE ValueType operator>>=(size_t InValue)          requires CIntegral<T>                      { return FetchRsh(InValue) >> InValue; }
	FORCEINLINE ValueType operator>>=(size_t InValue) volatile requires CIntegral<T> && bIsAlwaysLockFree { return FetchRsh(InValue) >> InValue; }
	
protected:

	ElementType Element;

};

template <typename T>
using TAtomicRef = TAtomic<T, true>;

template <typename T>
TAtomic(T) -> TAtomic<T>;

struct FAtomicFlag : public FSingleton
{
public:

	constexpr FAtomicFlag() : Element() { };

	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); Element.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Clear(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x08 | 0x20); Element.clear(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)          { return Element.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE bool TestAndSet(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) volatile { return Element.test_and_set(static_cast<NAMESPACE_STD::memory_order>(Order)); }

	FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return Element.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE bool Test(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); return Element.test(static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const          { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(Element).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	FORCEINLINE void Wait(bool Old, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const volatile { MEMORY_ORDER_CHECK(Order, 0x01 | 0x02 | 0x04 | 0x20); const_cast<const NAMESPACE_STD::atomic_flag&>(Element).wait(Old, static_cast<NAMESPACE_STD::memory_order>(Order)); }
	
	FORCEINLINE void Notify(bool bIsAll = false)          { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(Element).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(Element).notify_one(); }
	FORCEINLINE void Notify(bool bIsAll = false) volatile { if (bIsAll) const_cast<NAMESPACE_STD::atomic_flag&>(Element).notify_all(); else const_cast<NAMESPACE_STD::atomic_flag&>(Element).notify_one(); }
	
protected:

	NAMESPACE_STD::atomic_flag Element;

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
