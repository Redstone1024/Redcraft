#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
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
	FORCEINLINE constexpr void Emplace(intptr InCallable, U&& Args)
	{
		static_assert(CSameAs<TDecay<T>, TDecay<U>>);
		ValuePtr = reinterpret_cast<uintptr>(AddressOf(Args));
		Callable = InCallable;
	}
	
	FORCEINLINE constexpr void Swap(TFunctionStorage& InValue)
	{
		NAMESPACE_REDCRAFT::Swap(ValuePtr, InValue.ValuePtr);
		NAMESPACE_REDCRAFT::Swap(Callable, InValue.Callable);
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
	
	FORCEINLINE TFunctionStorage(const TFunctionStorage& InValue) requires (!bIsUnique)
		: TypeInfo(InValue.TypeInfo)
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
			GetTypeInfo().CopyConstruct(GetStorage(), InValue.GetStorage());
			break;
		case ERepresentation::Big:
			ExternalStorage = Memory::Malloc(GetTypeInfo().TypeSize, GetTypeInfo().TypeAlignment);
			GetTypeInfo().CopyConstruct(GetStorage(), InValue.GetStorage());
			break;
		default: check_no_entry();
		}
	}

	FORCEINLINE TFunctionStorage(TFunctionStorage&& InValue)
		: TypeInfo(InValue.TypeInfo)
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
			GetTypeInfo().MoveConstruct(GetStorage(), InValue.GetStorage());
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
	
	FORCEINLINE TFunctionStorage& operator=(const TFunctionStorage& InValue) requires (!bIsUnique)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Destroy();
			Invalidate();
		}
		else
		{
			Destroy();

			TypeInfo = InValue.TypeInfo;
			Callable = InValue.Callable;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(InternalStorage, InValue.InternalStorage);
				break;
			case ERepresentation::Small:
				GetTypeInfo().CopyConstruct(GetStorage(), InValue.GetStorage());
				break;
			case ERepresentation::Big:
				ExternalStorage = Memory::Malloc(GetTypeInfo().TypeSize, GetTypeInfo().TypeAlignment);
				GetTypeInfo().CopyConstruct(GetStorage(), InValue.GetStorage());
				break;
			default: check_no_entry();
			}
		}

		return *this;
	}

	FORCEINLINE TFunctionStorage& operator=(TFunctionStorage&& InValue)
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Destroy();
			Invalidate();
		}
		else
		{
			Destroy();

			TypeInfo = InValue.TypeInfo;
			Callable = InValue.Callable;

			switch (GetRepresentation())
			{
			case ERepresentation::Empty:
				break;
			case ERepresentation::Trivial:
				Memory::Memcpy(InternalStorage, InValue.InternalStorage);
				break;
			case ERepresentation::Small:
				GetTypeInfo().MoveConstruct(GetStorage(), InValue.GetStorage());
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

	FORCEINLINE constexpr bool IsValid() const { return TypeInfo != 0; }

	// Use Invalidate() to invalidate the storage or use Emplace<T>() to emplace a new object after destruction.
	FORCEINLINE void Destroy()
	{
		if (!IsValid()) return;

		switch (GetRepresentation())
		{
		case ERepresentation::Empty:
		case ERepresentation::Trivial:
			break;
		case ERepresentation::Small:
			GetTypeInfo().Destruct(GetStorage());
			break;
		case ERepresentation::Big:
			GetTypeInfo().Destruct(GetStorage());
			Memory::Free(ExternalStorage);
			break;
		default: check_no_entry();
		}
	}

	// Make sure you call this function after you have destroyed the held object using Destroy().
	FORCEINLINE constexpr void Invalidate() { TypeInfo = 0; }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	template <typename T, typename... Ts>
	FORCEINLINE void Emplace(uintptr InCallable, Ts&&... Args)
	{
		Callable = InCallable;

		using DecayedType = TDecay<T>;

		static constexpr const FTypeInfo SelectedTypeInfo(InPlaceType<DecayedType>);
		TypeInfo = reinterpret_cast<uintptr>(&SelectedTypeInfo);

		if constexpr (CEmpty<DecayedType>) return;

		constexpr bool bIsInlineStorable = sizeof(DecayedType) <= sizeof(InternalStorage) && alignof(DecayedType) <= alignof(TFunctionStorage);
		constexpr bool bIsTriviallyStorable = bIsInlineStorable && CTrivial<DecayedType> && CTriviallyCopyable<DecayedType>;

		if constexpr (bIsTriviallyStorable)
		{
			new (&InternalStorage) DecayedType(Forward<Ts>(Args)...);
			TypeInfo |= static_cast<uintptr>(ERepresentation::Trivial);
		}
		else if constexpr (bIsInlineStorable)
		{
			new (&InternalStorage) DecayedType(Forward<Ts>(Args)...);
			TypeInfo |= static_cast<uintptr>(ERepresentation::Small);
		}
		else
		{
			ExternalStorage = new DecayedType(Forward<Ts>(Args)...);
			TypeInfo |= static_cast<uintptr>(ERepresentation::Big);
		}

	}

	FORCEINLINE void Swap(TFunctionStorage& InValue)
	{
		if (!IsValid() && !InValue.IsValid()) return;

		if (IsValid() && !InValue.IsValid())
		{
			InValue = MoveTemp(*this);
			Destroy();
			Invalidate();
		}
		else if (InValue.IsValid() && !IsValid())
		{
			*this = MoveTemp(InValue);
			InValue.Destroy();
			InValue.Invalidate();
		}
		else
		{
			TFunctionStorage Temp = MoveTemp(*this);
			*this = MoveTemp(InValue);
			InValue = MoveTemp(Temp);
		}
	}

private:

	union
	{
		uint8 InternalStorage[64 - sizeof(uintptr) - sizeof(uintptr)];
		void* ExternalStorage;
	};

	uintptr TypeInfo;
	uintptr Callable;

	struct FMovableTypeInfo
	{
		const size_t TypeSize;
		const size_t TypeAlignment;

		using FMoveConstruct = void(*)(void*, void*);
		using FDestruct      = void(*)(void*       );
		
		const FMoveConstruct MoveConstruct;
		const FDestruct      Destruct;

		template <typename T>
		FORCEINLINE constexpr FMovableTypeInfo(TInPlaceType<T>)
			: TypeSize(sizeof(T)), TypeAlignment(alignof(T))
			, MoveConstruct(
				[](void* A, void* B)
				{
					new (A) T(*reinterpret_cast<T*>(B));
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
	
	struct FCopyableTypeInfo : public FMovableTypeInfo
	{
		using FCopyConstruct = void(*)(void*, const void*);

		const FCopyConstruct CopyConstruct;

		template <typename T>
		FORCEINLINE constexpr FCopyableTypeInfo(TInPlaceType<T>)
			: FMovableTypeInfo(InPlaceType<T>)
			, CopyConstruct(
				[](void* A, const void* B)
				{
					new (A) T(*reinterpret_cast<const T*>(B));
				}
			)
		{ }
	};

	using FTypeInfo = TConditional<bIsUnique, FMovableTypeInfo, FCopyableTypeInfo>;
	
	static_assert(alignof(FTypeInfo) >= 4);

	static constexpr uintptr_t RepresentationMask = 3;

	enum class ERepresentation : uintptr
	{
		Empty   = 0, // EmptyType
		Trivial = 1, // Trivial & Internal
		Small   = 2, // InternalStorage
		Big     = 3, // ExternalStorage
	};

	FORCEINLINE constexpr ERepresentation  GetRepresentation() const { return        static_cast<ERepresentation>(TypeInfo &  RepresentationMask); }
	FORCEINLINE constexpr const FTypeInfo& GetTypeInfo()       const { return *reinterpret_cast<const FTypeInfo*>(TypeInfo & ~RepresentationMask); }

	FORCEINLINE constexpr       void* GetStorage()       { return GetRepresentation() == ERepresentation::Trivial || GetRepresentation() == ERepresentation::Small ? InternalStorage : ExternalStorage; }
	FORCEINLINE constexpr const void* GetStorage() const { return GetRepresentation() == ERepresentation::Trivial || GetRepresentation() == ERepresentation::Small ? InternalStorage : ExternalStorage; }
	
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

	FORCEINLINE ResultType operator()(Ts... Args)         requires (CSameAs<CVRef,       int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &       requires (CSameAs<CVRef,       int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) &&      requires (CSameAs<CVRef,       int&&>) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const   requires (CSameAs<CVRef, const int  >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&  requires (CSameAs<CVRef, const int& >) { return CallImpl(Forward<Ts>(Args)...); }
	FORCEINLINE ResultType operator()(Ts... Args) const&& requires (CSameAs<CVRef, const int&&>) { return CallImpl(Forward<Ts>(Args)...); }

	FORCEINLINE constexpr bool           IsValid() const { return Storage.IsValid(); }
	FORCEINLINE constexpr explicit operator bool() const { return Storage.IsValid(); }

	FORCEINLINE constexpr void Swap(TFunctionImpl& InValue) { Storage.Swap(InValue.Storage); }

private:

	using CallableType = ResultType(*)(uintptr, Ts&&...);

	TFunctionStorage<bIsRef, bIsUnique> Storage;

	FORCEINLINE ResultType CallImpl(Ts&&... Args) const
	{
		checkf(IsValid(), TEXT("Attempting to call an unbound TFunction!"));
		CallableType Callable = reinterpret_cast<CallableType>(Storage.GetCallable());
		return Callable(Storage.GetValuePtr(), Forward<Ts>(Args)...);
	}

protected: // These functions should not be used by user-defined class

	// Use Invalidate() to invalidate the storage or use Emplace<T>() to emplace a new object after destruction.
	FORCEINLINE constexpr void Destroy() { Storage.Destroy(); }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	FORCEINLINE constexpr void Invalidate() { Storage.Invalidate(); }

	// Make sure you call this function after you have destroyed the held object using Destroy().
	template <typename T, typename... ArgTypes>
	FORCEINLINE constexpr TDecay<T>& Emplace(ArgTypes&&... Args)
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
			Forward<ArgTypes>(Args)...
		);

		return *reinterpret_cast<DecayedType*>(Storage.GetValuePtr());
	}

};

NAMESPACE_PRIVATE_END

template <CFunction F>
class TFunctionRef
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

	FORCEINLINE constexpr TFunctionRef() = delete;

	FORCEINLINE constexpr TFunctionRef(const TFunctionRef& InValue) = default;
	FORCEINLINE constexpr TFunctionRef(TFunctionRef&& InValue)      = default;

	// We delete the assignment operators because we don't want it to be confused with being related to
	// regular C++ reference assignment - i.e. calling the assignment operator of whatever the reference
	// is bound to - because that's not what TFunctionRef does, nor is it even capable of doing that.
	FORCEINLINE constexpr TFunctionRef& operator=(const TFunctionRef& InValue) = delete;
	FORCEINLINE constexpr TFunctionRef& operator=(TFunctionRef&& InValue)      = delete;

	template <typename T> requires (!CTFunctionRef<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE constexpr TFunctionRef(T&& InValue)
	{
		checkf(NAMESPACE_PRIVATE::FunctionIsBound(InValue), TEXT("Cannot bind a null/unbound callable to a TFunctionRef"));
		Impl::template Emplace<T>(Forward<T>(InValue));
	}

};

template <CFunction F>
class TFunction 
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

	FORCEINLINE constexpr TFunction(nullptr_t = nullptr) { Impl::Invalidate(); }

	FORCEINLINE TFunction(const TFunction& InValue)            = default;
	FORCEINLINE TFunction(TFunction&& InValue)                 = default;
	FORCEINLINE TFunction& operator=(const TFunction& InValue) = default;
	FORCEINLINE TFunction& operator=(TFunction&& InValue)      = default;

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
	
	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		Impl::template Emplace<T>(Forward<ArgTypes>(Args)...);
	}

	FORCEINLINE constexpr TFunction& operator=(nullptr_t) { Reset(); return *this; }

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

	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CCopyConstructible<TDecay<T>>
		&& CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(ArgTypes&&... Args)
	{
		Impl::Destroy();
		return Impl::template Emplace<T>(Forward<ArgTypes>(Args)...);
	}

	FORCEINLINE constexpr void Reset() { Impl::Destroy(); Impl::Invalidate(); }

};

template <CFunction F>
class TUniqueFunction
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

	FORCEINLINE constexpr TUniqueFunction(nullptr_t = nullptr) { Impl::Invalidate(); }

	FORCEINLINE TUniqueFunction(const TUniqueFunction& InValue)            = delete;
	FORCEINLINE TUniqueFunction(TUniqueFunction&& InValue)                 = default;
	FORCEINLINE TUniqueFunction& operator=(const TUniqueFunction& InValue) = delete;
	FORCEINLINE TUniqueFunction& operator=(TUniqueFunction&& InValue)      = default;

	FORCEINLINE TUniqueFunction(const TFunction<F>& InValue)
	{
		new (this) TFunction<F>(InValue);
	}

	FORCEINLINE TUniqueFunction(TFunction<F>&& InValue)
	{
		new (this) TFunction<F>(MoveTemp(InValue));
	}

	FORCEINLINE TUniqueFunction& operator=(const TFunction<F>& InValue)
	{
		*reinterpret_cast<TFunction<F>*>(this) = InValue;
		return *this;
	}

	FORCEINLINE TUniqueFunction& operator=(TFunction<F>&& InValue)
	{
		*reinterpret_cast<TFunction<F>*>(this) = MoveTemp(InValue);
		return *this;
	}

	template <typename T> requires (!CTInPlaceType<TDecay<T>>
		&& !CTFunctionRef<TDecay<T>> && !CTFunction<TDecay<T>> && !CTUniqueFunction<TDecay<T>>
		&& CConstructibleFrom<TDecay<T>, T&&> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>
		&& NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value)
	FORCEINLINE TUniqueFunction(T&& InValue)
	{
		if (!NAMESPACE_PRIVATE::FunctionIsBound(InValue)) Impl::Invalidate();
		else Impl::template Emplace<T>(Forward<T>(InValue));
	}

	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE explicit TUniqueFunction(TInPlaceType<T>, ArgTypes&&... Args)
	{
		Impl::template Emplace<T>(Forward<ArgTypes>(Args)...);
	}

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
	
	template <typename T, typename... ArgTypes> requires (NAMESPACE_PRIVATE::TIsInvocableSignature<F, TDecay<T>>::Value
		&& CConstructibleFrom<TDecay<T>, ArgTypes...> && CMoveConstructible<TDecay<T>> && CDestructible<TDecay<T>>)
	FORCEINLINE TDecay<T>& Emplace(ArgTypes&&... Args)
	{
		Impl::Destroy();
		using DecayedType = TDecay<T>;
		return Impl::template Emplace<T>(Forward<ArgTypes>(Args)...);
	}

	FORCEINLINE constexpr void Reset() { Impl::Destroy(); Impl::Invalidate(); }

};

template <CFunction F>
FORCEINLINE constexpr bool operator==(const TFunctionRef<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <CFunction F>
FORCEINLINE constexpr bool operator==(const TFunction<F>& LHS, nullptr_t)
{
	return !LHS;
}

template <CFunction F>
FORCEINLINE constexpr bool operator==(const TUniqueFunction<F>& LHS, nullptr_t)
{
	return !LHS;
}

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

template <typename F> requires (CConstructibleFrom<F, F&&>)
FORCEINLINE constexpr NAMESPACE_PRIVATE::TNotFunction<TDecay<F>> NotFn(F&& Func)
{
	return { Forward<F>(Func) };
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
