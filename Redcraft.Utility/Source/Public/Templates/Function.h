#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Address.h"
#include "Templates/Meta.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

// NOTE: In the STL, the assignment operation of the std::any type uses the copy-and-swap idiom
// instead of directly calling the assignment operation of the contained value.
// But we don't follow the the copy-and-swap idiom, see "Templates/Any.h".
// This class implements assignment operations in a way that assumes no assignment operations of the type,
// because the assignment operations of TFunction are in most cases different between LHS and RHS.

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CFunction F>
class TFunctionRef;

template <CFunction F>
class TFunction;

template <CFunction F>
class TUniqueFunction;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTFunctionRef                  : FFalse { };
template <typename F> struct TIsTFunctionRef<TFunctionRef<F>> : FTrue  { };

template <typename T> struct TIsTFunction               : FFalse { };
template <typename F> struct TIsTFunction<TFunction<F>> : FTrue  { };

template <typename T> struct TIsTUniqueFunction                     : FFalse { };
template <typename F> struct TIsTUniqueFunction<TUniqueFunction<F>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTFunctionRef    = NAMESPACE_PRIVATE::TIsTFunctionRef<TRemoveCV<T>>::Value;
template <typename T> concept CTFunction       = NAMESPACE_PRIVATE::TIsTFunction<TRemoveCV<T>>::Value;
template <typename T> concept CTUniqueFunction = NAMESPACE_PRIVATE::TIsTUniqueFunction<TRemoveCV<T>>::Value;

NAMESPACE_PRIVATE_BEGIN

template <bool bIsRef, bool bIsUnique>
class TFunctionStorage;

template <bool bIsUnique>
class TFunctionStorage<true, bIsUnique>
{
public:

	FORCEINLINE constexpr TFunctionStorage()                                   = default;
	FORCEINLINE constexpr TFunctionStorage(const TFunctionStorage&)            = default;
	FORCEINLINE constexpr TFunctionStorage(TFunctionStorage&&)                 = default;
	FORCEINLINE constexpr TFunctionStorage& operator=(const TFunctionStorage&) = delete;
	FORCEINLINE constexpr TFunctionStorage& operator=(TFunctionStorage&&)      = delete;
	FORCEINLINE constexpr ~TFunctionStorage()                                  = default;

	FORCEINLINE constexpr uintptr GetValuePtr() const { return ValuePtr; }
	FORCEINLINE constexpr uintptr GetCallable() const { return Callable; }

	FORCEINLINE constexpr bool IsValid() const { return ValuePtr != 0; }

	// Use Invalidate() to invalidate the storage or use Emplace<T>() to emplace a new object after destruction.
	FORCEINLINE constexpr void Destroy() { }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	FORCEINLINE constexpr void Invalidate() { ValuePtr = 0; }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	template <typename T, typename U>
	FORCEINLINE constexpr void Emplace(uintptr InCallable, U&& Args)
	{
		static_assert(CSameAs<TDecay<T>, TDecay<U>>);
		ValuePtr = reinterpret_cast<uintptr>(AddressOf(Args));
		Callable = InCallable;
	}

private:

	uintptr ValuePtr;
	uintptr Callable;

};

// For non-unique storage, the memory layout should be compatible with unique storage,
// i.e. it can be directly reinterpreted_cast.
template <bool bIsUnique>
class alignas(16) TFunctionStorage<false, bIsUnique>
{
public:

	FORCEINLINE constexpr TFunctionStorage() = default;

	TFunctionStorage(const TFunctionStorage& InValue) requires (!bIsUnique)
		: RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		Callable = InValue.Callable;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
			break;
		case ERepresentation::Trivial:
			Memory::Memcpy(InternalStorage, InValue.InternalStorage);
			break;
		case ERepresentation::Small:
			GetRTTI().CopyConstruct(GetStorage(), InValue.GetStorage());
			break;
		case ERepresentation::Big:
			ExternalStorage = Memory::Malloc(GetRTTI().TypeSize, GetRTTI().TypeAlignment);
			GetRTTI().CopyConstruct(GetStorage(), InValue.GetStorage());
			break;
		default: check_no_entry();
		}
	}

	TFunctionStorage(TFunctionStorage&& InValue)
		: RTTI(InValue.RTTI)
	{
		if (!IsValid()) return;

		Callable = InValue.Callable;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
			break;
		case ERepresentation::Trivial:
			Memory::Memcpy(InternalStorage, InValue.InternalStorage);
			break;
		case ERepresentation::Small:
			GetRTTI().MoveConstruct(GetStorage(), InValue.GetStorage());
			break;
		case ERepresentation::Big:
			ExternalStorage = InValue.ExternalStorage;
			InValue.Invalidate();
			break;
		default: check_no_entry();
		}
	}

	FORCEINLINE ~TFunctionStorage()
	{
		Destroy();
	}

	TFunctionStorage& operator=(const TFunctionStorage& InValue) requires (!bIsUnique)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (!InValue.IsValid())
		{
			Destroy();
			Invalidate();
		}
		else
		{
			Destroy();

			RTTI = InValue.RTTI;
			Callable = InValue.Callable;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(InternalStorage, InValue.InternalStorage);
				break;
			case ERepresentation::Small:
				GetRTTI().CopyConstruct(GetStorage(), InValue.GetStorage());
				break;
			case ERepresentation::Big:
				ExternalStorage = Memory::Malloc(GetRTTI().TypeSize, GetRTTI().TypeAlignment);
				GetRTTI().CopyConstruct(GetStorage(), InValue.GetStorage());
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	TFunctionStorage& operator=(TFunctionStorage&& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (!InValue.IsValid())
		{
			Destroy();
			Invalidate();
		}
		else
		{
			Destroy();

			RTTI = InValue.RTTI;
			Callable = InValue.Callable;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(InternalStorage, InValue.InternalStorage);
				break;
			case ERepresentation::Small:
				GetRTTI().MoveConstruct(GetStorage(), InValue.GetStorage());
				break;
			case ERepresentation::Big:
				ExternalStorage = InValue.ExternalStorage;
				InValue.Invalidate();
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	FORCEINLINE constexpr uintptr GetValuePtr() const { return reinterpret_cast<uintptr>(GetStorage()); }
	FORCEINLINE constexpr uintptr GetCallable() const { return Callable;                                }

	FORCEINLINE constexpr bool IsValid() const { return RTTI != 0; }

	// Use Invalidate() to invalidate the storage or use Emplace<T>() to emplace a new object after destruction.
	void Destroy()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
		case ERepresentation::Trivial:
			break;
		case ERepresentation::Small:
			GetRTTI().Destruct(GetStorage());
			break;
		case ERepresentation::Big:
			GetRTTI().Destruct(GetStorage());
			Memory::Free(ExternalStorage);
			break;
		default: check_no_entry();
		}
	}

	// Make sure you call this function after you have destroyed the held object using Destroy().
	FORCEINLINE constexpr void Invalidate() { RTTI = 0; }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	template <typename T, typename... Ts>
	void Emplace(uintptr InCallable, Ts&&... Args)
	{
		Callable = InCallable;

		using DecayedType = TDecay<T>;

		static constexpr const FRTTI SelectedRTTI(InPlaceType<DecayedType>);
		RTTI = reinterpret_cast<uintptr>(&SelectedRTTI);

		if constexpr (CEmpty<DecayedType> && CTrivial<DecayedType>) return; // ERepresentation::Empty

		constexpr bool bIsTriviallyStorable = sizeof(DecayedType) <= sizeof(InternalStorage) && alignof(DecayedType) <= alignof(TFunctionStorage) && CTriviallyCopyable<DecayedType>;
		constexpr bool bIsSmallStorable     = sizeof(DecayedType) <= sizeof(InternalStorage) && alignof(DecayedType) <= alignof(TFunctionStorage);

		if constexpr (bIsTriviallyStorable)
		{
			new (&InternalStorage) DecayedType(Forward<Ts>(Args)...);
			RTTI |= static_cast<uintptr>(ERepresentation::Trivial);
		}
		else if constexpr (bIsSmallStorable)
		{
			new (&InternalStorage) DecayedType(Forward<Ts>(Args)...);
			RTTI |= static_cast<uintptr>(ERepresentation::Small);
		}
		else
		{
			ExternalStorage = new DecayedType(Forward<Ts>(Args)...);
			RTTI |= static_cast<uintptr>(ERepresentation::Big);
		}

	}

	friend void Swap(TFunctionStorage& A, TFunctionStorage& B)
	{
		if (!A.IsValid() && !B.IsValid()) return;

		if (A.IsValid() && !B.IsValid())
		{
			B = MoveTemp(A);
			A.Destroy();
			A.Invalidate();
		}
		else if (!A.IsValid() && B.IsValid())
		{
			A = MoveTemp(B);
			B.Destroy();
			B.Invalidate();
		}
		else
		{
			TFunctionStorage Temp = MoveTemp(A);
			A = MoveTemp(B);
			B = MoveTemp(Temp);
		}
	}

private:

	struct FMovableRTTI
	{
		const size_t TypeSize;
		const size_t TypeAlignment;

		using FMoveConstruct = void(*)(void*, void*);
		using FDestruct      = void(*)(void*       );

		const FMoveConstruct MoveConstruct;
		const FDestruct      Destruct;

		template <typename T>
		FORCEINLINE constexpr FMovableRTTI(TInPlaceType<T>)
			: TypeSize(sizeof(T)), TypeAlignment(alignof(T))
			, MoveConstruct(
				[](void* A, void* B)
				{
					new (A) T(MoveTemp(*reinterpret_cast<T*>(B)));
				}
			)
			, Destruct(
				[](void* A)
				{
					reinterpret_cast<T*>(A)->~T();
				}
			)
		{ }
	};

	struct FCopyableRTTI : public FMovableRTTI
	{
		using FCopyConstruct = void(*)(void*, const void*);

		const FCopyConstruct CopyConstruct;

		template <typename T>
		FORCEINLINE constexpr FCopyableRTTI(TInPlaceType<T>)
			: FMovableRTTI(InPlaceType<T>)
			, CopyConstruct(
				[](void* A, const void* B)
				{
					new (A) T(*reinterpret_cast<const T*>(B));
				}
			)
		{ }
	};

	using FRTTI = TConditional<bIsUnique, FMovableRTTI, FCopyableRTTI>;

	static_assert(alignof(FRTTI) >= 4);

	static constexpr uintptr_t RepresentationMask = 3;

	enum class ERepresentation : uintptr
	{
		Empty   = 0, // EmptyType
		Trivial = 1, // Trivial & Internal
		Small   = 2, // InternalStorage
		Big     = 3, // ExternalStorage
	};

	union
	{
		uint8 InternalStorage[64 - sizeof(uintptr) - sizeof(uintptr)];
		void* ExternalStorage;
	};

	uintptr RTTI;
	uintptr Callable;

	FORCEINLINE constexpr ERepresentation GetRepresentation() const { return    static_cast<ERepresentation>(RTTI &  RepresentationMask); }
	FORCEINLINE constexpr    const FRTTI& GetRTTI()           const { return *reinterpret_cast<const FRTTI*>(RTTI & ~RepresentationMask); }

	FORCEINLINE constexpr void* GetStorage()
	{
		switch (GetRepresentation())
		{
		case ERepresentation::Empty:   return nullptr;
		case ERepresentation::Trivial: return &InternalStorage;
		case ERepresentation::Small:   return &InternalStorage;
		case ERepresentation::Big:     return ExternalStorage;
		default: check_no_entry();     return nullptr;
		}
	}

	FORCEINLINE constexpr const void* GetStorage() const
	{
		switch (GetRepresentation())
		{
		case ERepresentation::Empty:   return nullptr;
		case ERepresentation::Trivial: return &InternalStorage;
		case ERepresentation::Small:   return &InternalStorage;
		case ERepresentation::Big:     return ExternalStorage;
		default: check_no_entry();     return nullptr;
		}
	}
};

template <typename T>
FORCEINLINE constexpr bool FunctionIsBound(const T& Func)
{
	if constexpr (CPointer<T> || CMemberPointer<T> || CTFunctionRef<T> || CTFunction<T> || CTUniqueFunction<T>)
	{
		return !!Func;
	}
	else
	{
		return true;
	}
}

template <typename Signature, typename F> struct TIsInvocableSignature : FFalse { };

template <typename Ret, typename... Ts, typename F>
struct TIsInvocableSignature<Ret(Ts...), F>
	: TBoolConstant<CInvocableResult<Ret, F, Ts...> && CInvocableResult<Ret, F&, Ts...>>
{ };

template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) & , F> : TBoolConstant<CInvocableResult<Ret, F&, Ts...>> { };
template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) &&, F> : TBoolConstant<CInvocableResult<Ret, F , Ts...>> { };

template <typename Ret, typename... Ts, typename F>
struct TIsInvocableSignature<Ret(Ts...) const, F>
	: TBoolConstant<CInvocableResult<Ret, const F, Ts...> && CInvocableResult<Ret, const F&, Ts...>>
{ };

template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) const& , F> : TBoolConstant<CInvocableResult<Ret, const F&, Ts...>> { };
template <typename Ret, typename... Ts, typename F> struct TIsInvocableSignature<Ret(Ts...) const&&, F> : TBoolConstant<CInvocableResult<Ret, const F , Ts...>> { };

template <typename F>                   struct TFunctionInfo;
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...)        > { using Fn = Ret(Ts...); using CVRef =       int;   };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) &      > { using Fn = Ret(Ts...); using CVRef =       int&;  };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) &&     > { using Fn = Ret(Ts...); using CVRef =       int&&; };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const  > { using Fn = Ret(Ts...); using CVRef = const int;   };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const& > { using Fn = Ret(Ts...); using CVRef = const int&;  };
template <typename Ret, typename... Ts> struct TFunctionInfo<Ret(Ts...) const&&> { using Fn = Ret(Ts...); using CVRef = const int&&; };

template <typename F, typename CVRef, bool bIsRef, bool bIsUnique = false> class TFunctionImpl;

template <typename Ret, typename... Ts, typename CVRef, bool bIsRef, bool bIsUnique>
class TFunctionImpl<Ret(Ts...), CVRef, bIsRef, bIsUnique>
{
public:

	using ResultType = Ret;
	using ArgumentType = TTypeSequence<Ts...>;

	FORCEINLINE constexpr TFunctionImpl()                                = default;
	FORCEINLINE constexpr TFunctionImpl(const TFunctionImpl&)            = default;
	FORCEINLINE constexpr TFunctionImpl(TFunctionImpl&&)                 = default;
	FORCEINLINE constexpr TFunctionImpl& operator=(const TFunctionImpl&) = default;
	FORCEINLINE constexpr TFunctionImpl& operator=(TFunctionImpl&&)      = default;
	FORCEINLINE constexpr ~TFunctionImpl()                               = default;

	/** Invokes the stored callable function target with the parameters args. */
	FORCEINLINE ResultType operator()(Ts... Args)         requires (CSameAs<CVRef,       int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &       requires (CSameAs<CVRef,       int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &&      requires (CSameAs<CVRef,       int&&>) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const   requires (CSameAs<CVRef, const int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&  requires (CSameAs<CVRef, const int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&& requires (CSameAs<CVRef, const int&&>) { return CallImpl(Forward<Ts>(Args)...); }

	/** @return false if instance stores a callable function target, true otherwise. */
	NODISCARD FORCEINLINE constexpr bool operator==(nullptr_t) const& requires (!bIsRef) { return !IsValid(); }

	/** @return true if instance stores a callable function target, false otherwise. */
	NODISCARD FORCEINLINE constexpr           bool IsValid() const requires (!bIsRef) { return Storage.IsValid(); }
	NODISCARD FORCEINLINE constexpr explicit operator bool() const requires (!bIsRef) { return Storage.IsValid(); }

private:

	using CallableType = ResultType(*)(uintptr, Ts&&...);

	TFunctionStorage<bIsRef, bIsUnique> Storage;

	FORCEINLINE ResultType CallImpl(Ts&&... Args) const
	{
		checkf(Storage.IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		CallableType Callable = reinterpret_cast<CallableType>(Storage.GetCallable());
		return Callable(Storage.GetValuePtr(), Forward<Ts>(Args)...);
	}

protected:

	// Use Invalidate() to invalidate the storage or use Emplace<T>() to emplace a new object after destruction.
	FORCEINLINE constexpr void Destroy() { Storage.Destroy(); }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	FORCEINLINE constexpr void Invalidate() { Storage.Invalidate(); }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	template <typename T, typename... Us>
	FORCEINLINE constexpr TDecay<T>& Emplace(Us&&... Args)
	{
		using DecayedType = TDecay<T>;

		// This add a l-value reference to a non-reference type, while preserving the r-value reference.
		using ObjectType = TCopyCVRef<CVRef, DecayedType>;
		using InvokeType = TConditional<CReference<ObjectType>, ObjectType, ObjectType&>;

		CallableType Callable = [](uintptr ObjectPtr, Ts&&... Args) -> ResultType
		{
			return InvokeResult<ResultType>(
				static_cast<InvokeType>(*reinterpret_cast<DecayedType*>(ObjectPtr)),
				Forward<Ts>(Args)...
			);
		};

		Storage.template Emplace<DecayedType>(
			reinterpret_cast<uintptr>(Callable),
			Forward<Us>(Args)...
		);

		return *reinterpret_cast<DecayedType*>(Storage.GetValuePtr());
	}

	friend FORCEINLINE constexpr void Swap(TFunctionImpl& A, TFunctionImpl& B) requires (!bIsRef) { Swap(A.Storage, B.Storage); }

};

NAMESPACE_PRIVATE_END

/**
 * A class which represents a reference to something callable. The important part here is *reference* - if
 * you bind it to a lambda and the lambda goes out of scope, you will be left with an invalid reference.
 *
 * If you also want to take ownership of the callable thing, e.g. you want to return a lambda from a
 * function, you should use TFunction. TFunctionRef does not concern itself with ownership because it's
 * intended to be FAST.
 *
 * TFunctionRef is most useful when you want to parameterize a function with some caller-defined code
 * without making it a template.
 */
template <CFunction F>
class TFunctionRef final
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		true>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		true>;

public:

	/** Remove the default initialization and disallow the construction of a TFunctionRef that does not store the function target. */
	FORCEINLINE constexpr TFunctionRef() = delete;

	FORCEINLINE constexpr TFunctionRef(const TFunctionRef&) = default;
	FORCEINLINE constexpr TFunctionRef(TFunctionRef&&)      = default;

	/**
	 * We delete the assignment operators because we don't want it to be confused with being related to
	 * regular C++ reference assignment - i.e. calling the assignment operator of whatever the reference
	 * is bound to - because that's not what TFunctionRef does, nor is it even capable of doing that.
	 */
	FORCEINLINE constexpr TFunctionRef& operator=(const TFunctionRef&) = delete;
	FORCEINLINE constexpr TFunctionRef& operator=(TFunctionRef&&)      = delete;

	/** Constructor which binds a TFunctionRef to a callable object. */
	template <typename T> requires (!CTFunctionRef<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE constexpr TFunctionRef(T&& InValue)
	{
		checkf(NAMESPACE_PRIVATE::FunctionIsBound(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		Impl::template Emplace<T>(Forward<T>(InValue));
	}

	template <typename T>
	TFunctionRef(const T&& InValue) = delete;

};

/**
 * A class which represents a copy of something callable.
 *
 * It takes a copy of whatever is bound to it, meaning you can return it from functions and store them in
 * objects without caring about the lifetime of the original object being bound.
 */
template <CFunction F>
class TFunction final
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false, false>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false, false>;

public:

	/**  Default constructor. */
	FORCEINLINE constexpr TFunction(nullptr_t = nullptr) { Impl::Invalidate(); }

	FORCEINLINE TFunction(const TFunction&)            = default;
	FORCEINLINE TFunction(TFunction&&)                 = default;
	FORCEINLINE TFunction& operator=(const TFunction&) = default;
	FORCEINLINE TFunction& operator=(TFunction&&)      = default;

	/**
	 * Constructs an TFunction with initial content an function object of type TDecay<T>,
	 * direct-initialized from Forward<T>(InValue).
	 */
	template <typename T> requires (!CTInPlaceType<TDecay<T>>
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TFunction(T&& InValue)
	{
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::Invalidate();
		else Impl::template Emplace<T>(Forward<T>(InValue));
	}

	/**
	 * Constructs an TFunction with initial content an function object of type TDecay<T>,
	 * direct-non-list-initialized from Forward<Ts>(Args)....
	 */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, Ts...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TFunction(TInPlaceType<T>, Ts&&... Args)
	{
		Impl::template Emplace<T>(Forward<Ts>(Args)...);
	}

	/**
	 * Constructs an TFunction with initial content an function object of type TDecay<T>,
	 * direct-non-list-initialized from IL, Forward<Ts>(Args)....
	 */
	template <typename T, typename U, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, initializer_list<U>, Ts...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TFunction(TInPlaceType<T>, initializer_list<U> IL, Ts&&... Args)
	{
		Impl::template Emplace<T>(IL, Forward<Ts>(Args)...);
	}

	/** Removes any bound callable from the TFunction, restoring it to the default empty state. */
	FORCEINLINE constexpr TFunction& operator=(nullptr_t) { Reset(); return *this; }

	/** Assigns the type and value of 'InValue'. */
	template <typename T> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TFunction& operator=(T&& InValue)
	{
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Reset();
		else Emplace<T>(Forward<T>(InValue));

		return *this;
	}

	/**
	 * Changes the function object to one of type TDecay<T> constructed from the arguments.
	 * First destroys the current function object (if any) by Reset(), then constructs an object of type
	 * TDecay<T>, direct-non-list-initialized from Forward<Ts>(Args)..., as the function object.
	 *
	 * @param  Args - The arguments to be passed to the constructor of the function object.
	 *
	 * @return A reference to the new function object.
	 */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, Ts...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(Ts&&... Args)
	{
		Impl::Destroy();
		return Impl::template Emplace<T>(Forward<Ts>(Args)...);
	}

	/**
	 * Changes the function object to one of type TDecay<T> constructed from the arguments.
	 * First destroys the current function object (if any) by Reset(), then constructs an object of type
	 * TDecay<T>, direct-non-list-initialized from IL, Forward<Ts>(Args)..., as the function object.
	 *
	 * @param  IL, Args - The arguments to be passed to the constructor of the function object.
	 *
	 * @return A reference to the new function object.
	 */
	template <typename T, typename U, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, initializer_list<U>, Ts...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(initializer_list<U> IL, Ts&&... Args)
	{
		Impl::Destroy();
		return Impl::template Emplace<T>(IL, Forward<Ts>(Args)...);
	}

	/** Removes any bound callable from the TFunction, restoring it to the default empty state. */
	FORCEINLINE constexpr void Reset() { Impl::Destroy(); Impl::Invalidate(); }

	/** Overloads the Swap algorithm for TFunction. */
	friend FORCEINLINE constexpr void Swap(TFunction& A, TFunction& B) { Swap(static_cast<Impl&>(A), static_cast<Impl&>(B)); }

};

/**
 * A class which represents a copy of something callable, but is move-only.
 *
 * It takes a copy of whatever is bound to it, meaning you can return it from functions and store them in
 * objects without caring about the lifetime of the original object being bound.
 */
template <CFunction F>
class TUniqueFunction final
	: public NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false, true>
{
private:

	using Impl = NAMESPACE_PRIVATE::TFunctionImpl<
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::Fn,
		typename NAMESPACE_PRIVATE::TFunctionInfo<F>::CVRef,
		false, true>;

public:

	/**  Default constructor. */
	FORCEINLINE constexpr TUniqueFunction(nullptr_t = nullptr) { Impl::Invalidate(); }

	FORCEINLINE TUniqueFunction(const TUniqueFunction&)            = delete;
	FORCEINLINE TUniqueFunction(TUniqueFunction&&)                 = default;
	FORCEINLINE TUniqueFunction& operator=(const TUniqueFunction&) = delete;
	FORCEINLINE TUniqueFunction& operator=(TUniqueFunction&&)      = default;

	/** Constructor from TFunction to TUniqueFunction. */
	FORCEINLINE TUniqueFunction(const TFunction<F>& InValue)
	{
		new (this) TFunction<F>(InValue);
	}

	/** Constructor from TFunction to TUniqueFunction. */
	FORCEINLINE TUniqueFunction(TFunction<F>&& InValue)
	{
		new (this) TFunction<F>(MoveTemp(InValue));
	}

	/** Assignment operator from TFunction to TUniqueFunction. */
	FORCEINLINE TUniqueFunction& operator=(const TFunction<F>& InValue)
	{
		*reinterpret_cast<TFunction<F>*>(this) = InValue;
		return *this;
	}

	/** Assignment operator from TFunction to TUniqueFunction. */
	FORCEINLINE TUniqueFunction& operator=(TFunction<F>&& InValue)
	{
		*reinterpret_cast<TFunction<F>*>(this) = MoveTemp(InValue);
		return *this;
	}

	/**
	 * Constructs an TUniqueFunction with initial content an function object of type TDecay<T>,
	 * direct-initialized from Forward<T>(InValue).
	 */
	template <typename T> requires (!CTInPlaceType<TDecay<T>>
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TUniqueFunction(T&& InValue)
	{
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::Invalidate();
		else Impl::template Emplace<T>(Forward<T>(InValue));
	}

	/**
	 * Constructs an TUniqueFunction with initial content an function object of type TDecay<T>,
	 * direct-non-list-initialized from Forward<Ts>(Args)....
	 */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, Ts...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TUniqueFunction(TInPlaceType<T>, Ts&&... Args)
	{
		Impl::template Emplace<T>(Forward<Ts>(Args)...);
	}

	/**
	 * Constructs an TUniqueFunction with initial content an function object of type TDecay<T>,
	 * direct-non-list-initialized from IL, Forward<Ts>(Args)....
	 */
	template <typename T, typename U, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, initializer_list<U>, Ts...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TUniqueFunction(TInPlaceType<T>, initializer_list<U> IL, Ts&&... Args)
	{
		Impl::template Emplace<T>(IL, Forward<Ts>(Args)...);
	}

	/** Removes any bound callable from the TUniqueFunction, restoring it to the default empty state. */
	FORCEINLINE constexpr TUniqueFunction& operator=(nullptr_t) { Impl::Destroy(); Impl::Invalidate(); return *this; }

	template <typename T> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TUniqueFunction& operator=(T&& InValue)
	{
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Reset();
		else Emplace<T>(Forward<T>(InValue));

		return *this;
	}

	/**
	 * Changes the function object to one of type TDecay<T> constructed from the arguments.
	 * First destroys the current function object (if any) by Reset(), then constructs an object of type
	 * TDecay<T>, direct-non-list-initialized from Forward<Ts>(Args)..., as the function object.
	 *
	 * @param  Args	- The arguments to be passed to the constructor of the function object.
	 *
	 * @return A reference to the new function object.
	 */
	template <typename T, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, Ts...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(Ts&&... Args)
	{
		Impl::Destroy();
		using DecayedType = TDecay<T>;
		return Impl::template Emplace<T>(Forward<Ts>(Args)...);
	}

	/**
	 * Changes the function object to one of type TDecay<T> constructed from the arguments.
	 * First destroys the current function object (if any) by Reset(), then constructs an object of type
	 * TDecay<T>, direct-non-list-initialized from IL, Forward<Ts>(Args)..., as the function object.
	 *
	 * @param  IL, Args - The arguments to be passed to the constructor of the function object.
	 *
	 * @return A reference to the new function object.
	 */
	template <typename T, typename U, typename... Ts> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, initializer_list<U>, Ts...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(initializer_list<U> IL, Ts&&... Args)
	{
		Impl::Destroy();
		using DecayedType = TDecay<T>;
		return Impl::template Emplace<T>(IL, Forward<Ts>(Args)...);
	}

	/** Removes any bound callable from the TUniqueFunction, restoring it to the default empty state. */
	FORCEINLINE constexpr void Reset() { Impl::Destroy(); Impl::Invalidate(); }

	/** Overloads the Swap algorithm for TUniqueFunction. */
	friend FORCEINLINE constexpr void Swap(TUniqueFunction& A, TUniqueFunction& B) { Swap(static_cast<Impl&>(A), static_cast<Impl&>(B)); }

};

static_assert(sizeof(TFunction<void()>)       == 64, "The byte size of TFunction is unexpected");
static_assert(sizeof(TUniqueFunction<void()>) == 64, "The byte size of TUniqueFunction is unexpected");

static_assert(alignof(TFunction<void()>)       == 16, "The byte alignment of TFunction is unexpected");
static_assert(alignof(TUniqueFunction<void()>) == 16, "The byte alignment of TUniqueFunction is unexpected");

NAMESPACE_PRIVATE_BEGIN

template <typename F>
struct TNotFunction
{
	F Storage;

	template <typename... Ts> requires (CInvocable<F&, Ts&&...>)
	FORCEINLINE constexpr auto operator()(Ts&&... Args) &
		-> decltype(!Invoke(Storage, Forward<Ts>(Args)...))
	{
		return !Invoke(Storage, Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<F&&, Ts&&...>)
	FORCEINLINE constexpr auto operator()(Ts&&... Args) &&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Ts>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<const F&, Ts&&...>)
	FORCEINLINE constexpr auto operator()(Ts&&... Args) const&
		-> decltype(!Invoke(Storage, Forward<Ts>(Args)...))
	{
		return !Invoke(Storage, Forward<Ts>(Args)...);
	}

	template <typename... Ts> requires (CInvocable<const F&&, Ts&&...>)
	FORCEINLINE constexpr auto operator()(Ts&&... Args) const&&
		-> decltype(!Invoke(MoveTemp(Storage), Forward<Ts>(Args)...))
	{
		return !Invoke(MoveTemp(Storage), Forward<Ts>(Args)...);
	}
};

NAMESPACE_PRIVATE_END

/** Creates a forwarding call wrapper that returns the negation of the callable object it holds. */
template <typename F> requires (CConstructibleFrom<F, F&&> && CMoveConstructible<F>)
NODISCARD FORCEINLINE constexpr NAMESPACE_PRIVATE::TNotFunction<TDecay<F>> NotFn(F&& Func)
{
	return { Forward<F>(Func) };
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
