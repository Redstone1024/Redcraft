#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Meta.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <typename... Ts> requires (sizeof...(Ts) > 0 && (true && ... && CDestructible<Ts>))
class TVariant;

NAMESPACE_PRIVATE_BEGIN

template <typename    T > struct TIsTVariant                  : FFalse { };
template <typename... Ts> struct TIsTVariant<TVariant<Ts...>> : FTrue  { };

template <typename VariantType>
struct TVariantNumImpl;

template <typename... Ts>
struct TVariantNumImpl<TVariant<Ts...>> : TConstant<size_t, Meta::TSize<TTypeSequence<Ts...>>> { };

template <typename T, typename VariantType>
struct TVariantIndexImpl;

template <typename T, typename... Ts>
struct TVariantIndexImpl<T, TVariant<Ts...>> : TConstant<size_t, Meta::TIndex<T, TTypeSequence<Ts...>>> { };

template <size_t I, typename VariantType>
struct TVariantAlternativeImpl;

template <size_t I, typename... Ts>
struct TVariantAlternativeImpl<I, TVariant<Ts...>>
{
	using Type = Meta::TType<I, TTypeSequence<Ts...>>;
};

template <typename T, typename TSequence>
struct TVariantOverloadType
{
	using FrontType = Meta::TFront<TSequence>;
	using NextSequence = Meta::TPop<TSequence>;
	using NextUniqueSequence = typename TVariantOverloadType<T, NextSequence>::Type;

	// T_i x[] = { Forward<T>(t) };
	static constexpr bool bConditional = requires { DeclVal<void(FrontType(&&)[1])>()({ DeclVal<T>() }); };

	using Type = TConditional<bConditional, Meta::TPush<FrontType, NextUniqueSequence>, NextUniqueSequence>;
};

template <typename T>
struct TVariantOverloadType<T, TTypeSequence<>>
{
	using Type = TTypeSequence<>;
};

template <typename T, typename... Ts>
using TVariantSelectedType = Meta::TOverloadResolution<T, typename NAMESPACE_PRIVATE::TVariantOverloadType<T, TTypeSequence<Ts...>>::Type>;

NAMESPACE_PRIVATE_END

template <typename T>
concept CTVariant = NAMESPACE_PRIVATE::TIsTVariant<TRemoveCV<T>>::Value;

template <CTVariant T>
inline constexpr size_t TVariantNum = NAMESPACE_PRIVATE::TVariantNumImpl<TRemoveCV<T>>::Value;

template <typename T, CTVariant U>
inline constexpr size_t TVariantIndex = NAMESPACE_PRIVATE::TVariantIndexImpl<T, TRemoveCV<U>>::Value;

template <size_t I, CTVariant U>
using TVariantAlternative = TCopyCV<U, typename NAMESPACE_PRIVATE::TVariantAlternativeImpl<I, TRemoveCV<U>>::Type>;

template <typename... Ts> requires (sizeof...(Ts) > 0 && (true && ... && CDestructible<Ts>))
class TVariant
{
public:

	FORCEINLINE constexpr TVariant() : TypeIndex(0xFF) { };

	FORCEINLINE constexpr TVariant(FInvalid) : TVariant() { };

	FORCEINLINE constexpr TVariant(const TVariant& InValue) requires (true && ... && CTriviallyCopyConstructible<Ts>) = default;

	FORCEINLINE constexpr TVariant(const TVariant& InValue) requires ((true && ... && CCopyConstructible<Ts>) && !(true && ... && CTriviallyCopyConstructible<Ts>))
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	FORCEINLINE constexpr TVariant(TVariant&& InValue) requires (true && ... && CTriviallyMoveConstructible<Ts>) = default;

	FORCEINLINE constexpr TVariant(TVariant&& InValue) requires ((true && ... && CMoveConstructible<Ts>) && !(true && ... && CTriviallyMoveConstructible<Ts>))
		: TypeIndex(static_cast<uint8>(InValue.GetIndex()))
	{
		if (IsValid()) MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
	}

	template <size_t I, typename... ArgTypes> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, ArgTypes...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceIndex<I>, ArgTypes&&... Args)
		: TypeIndex(I)
	{
		using SelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		new (&Value) SelectedType(Forward<ArgTypes>(Args)...);
	}

	template <typename T, typename... ArgTypes> requires (CConstructibleFrom<T, ArgTypes...>)
	FORCEINLINE constexpr explicit TVariant(TInPlaceType<T>, ArgTypes&&... Args)
		: TVariant(InPlaceIndex<TVariantIndex<T, TVariant<Ts...>>>, Forward<ArgTypes>(Args)...)
	{ }

	template <typename T> requires (requires { typename NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>; }
		&& !CTInPlaceType<TRemoveCVRef<T>> && !CTInPlaceIndex<TRemoveCVRef<T>>
		&& !CBaseOf<TVariant, TRemoveCVRef<T>>)
	FORCEINLINE constexpr TVariant(T&& InValue) : TVariant(InPlaceType<NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>>, Forward<T>(InValue))
	{ }

	FORCEINLINE constexpr ~TVariant() requires (true && ... && CTriviallyDestructible<Ts>) = default;

	FORCEINLINE constexpr ~TVariant() requires (!(true && ... && CTriviallyDestructible<Ts>))
	{
		Reset();
	}

	FORCEINLINE constexpr TVariant& operator=(const TVariant& InValue) requires (true && ... && (CTriviallyCopyConstructible<Ts> && CTriviallyCopyAssignable<Ts>)) = default;

	constexpr TVariant& operator=(const TVariant& InValue) requires ((true && ... && (CCopyConstructible<Ts> && CCopyAssignable<Ts>))
		&& !(true && ... && (CTriviallyCopyConstructible<Ts> && CTriviallyCopyAssignable<Ts>)))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) CopyAssignImpl[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{	
			Reset();
			CopyConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	FORCEINLINE constexpr TVariant& operator=(TVariant&& InValue) requires (true && ... && (CTriviallyMoveConstructible<Ts> && CTriviallyMoveAssignable<Ts>)) = default;

	constexpr TVariant& operator=(TVariant&& InValue) requires ((true && ... && (CMoveConstructible<Ts> && CMoveAssignable<Ts>))
		&& !(true && ... && (CTriviallyMoveConstructible<Ts> && CTriviallyMoveAssignable<Ts>)))
	{
		if (&InValue == this) return *this;

		if (!InValue.IsValid())
		{
			Reset();
			return *this;
		}

		if (GetIndex() == InValue.GetIndex()) MoveAssignImpl[InValue.GetIndex()](&Value, &InValue.Value);
		else
		{
			Reset();
			MoveConstructImpl[InValue.GetIndex()](&Value, &InValue.Value);
			TypeIndex = static_cast<uint8>(InValue.GetIndex());
		}

		return *this;
	}

	template <typename T> requires (requires { typename NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>; })
	FORCEINLINE constexpr TVariant& operator=(T&& InValue)
	{
		using SelectedType = NAMESPACE_PRIVATE::TVariantSelectedType<T, Ts...>;

		if (GetIndex() == TVariantIndex<SelectedType, TVariant<Ts...>>) GetValue<SelectedType>() = Forward<T>(InValue);
		else
		{
			Reset();
			new (&Value) SelectedType(Forward<T>(InValue));
			TypeIndex = TVariantIndex<SelectedType, TVariant<Ts...>>;
		}

		return *this;
	}
	
	friend constexpr bool operator==(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CEqualityComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return false;
		if (LHS.IsValid() == false) return true;

		using FCompareImpl = bool(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> bool { return *reinterpret_cast<const Ts*>(LHS) == *reinterpret_cast<const Ts*>(RHS); }... };

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}

	friend constexpr partial_ordering operator<=>(const TVariant& LHS, const TVariant& RHS) requires (true && ... && CSynthThreeWayComparable<Ts>)
	{
		if (LHS.GetIndex() != RHS.GetIndex()) return partial_ordering::unordered;
		if (LHS.IsValid() == false) return partial_ordering::equivalent;

		using FCompareImpl = partial_ordering(*)(const void*, const void*);
		constexpr FCompareImpl CompareImpl[] = { [](const void* LHS, const void* RHS) -> partial_ordering { return SynthThreeWayCompare(*reinterpret_cast<const Ts*>(LHS), *reinterpret_cast<const Ts*>(RHS)); }...};

		return CompareImpl[LHS.GetIndex()](&LHS.Value, &RHS.Value);
	}
	
	template <typename T> requires (!CBaseOf<TVariant, T> && CEqualityComparable<T>)
	FORCEINLINE constexpr bool operator==(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? GetValue<T>() == InValue : false;
	}
	
	template <typename T> requires (!CBaseOf<TVariant, T> && CEqualityComparable<T>)
	FORCEINLINE constexpr partial_ordering operator<=>(const T& InValue) const&
	{
		return HoldsAlternative<T>() ? SynthThreeWayCompare(GetValue<T>(), InValue) : partial_ordering::unordered;
	}
	
	FORCEINLINE constexpr bool operator==(FInvalid) const& { return !IsValid(); }

	template <size_t I, typename... ArgTypes> requires (I < sizeof...(Ts)
		&& CConstructibleFrom<TVariantAlternative<I, TVariant<Ts...>>, ArgTypes...>)
	FORCEINLINE constexpr TVariantAlternative<I, TVariant<Ts...>>& Emplace(ArgTypes&&... Args)
	{
		Reset();

		using SelectedType = TVariantAlternative<I, TVariant<Ts...>>;
		SelectedType* Result = new (&Value) SelectedType(Forward<ArgTypes>(Args)...);
		TypeIndex = I;

		return *Result;
	}

	template <typename T, typename... ArgTypes> requires (CConstructibleFrom<T, ArgTypes...>)
	FORCEINLINE constexpr T& Emplace(ArgTypes&&... Args)
	{
		return Emplace<TVariantIndex<T, TVariant<Ts...>>>(Forward<ArgTypes>(Args)...);
	}

	FORCEINLINE constexpr const type_info& GetTypeInfo() const { return IsValid() ? *TypeInfos[GetIndex()] : typeid(void); }

	FORCEINLINE constexpr size_t GetIndex()        const { return TypeIndex != 0xFF ? TypeIndex : INDEX_NONE; }
	FORCEINLINE constexpr bool IsValid()           const { return TypeIndex != 0xFF; }
	FORCEINLINE constexpr explicit operator bool() const { return TypeIndex != 0xFF; }

	template <size_t   I> FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == I                                 : false; }
	template <typename T> FORCEINLINE constexpr bool HoldsAlternative() const { return IsValid() ? GetIndex() == TVariantIndex<T, TVariant<Ts...>> : false; }

	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value);  }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<I>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const TVariantAlternative<I, TVariant<Ts...>>*>(&Value)); }

	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() &       { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<      T*>(&Value);  }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() &&      { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<      T*>(&Value)); }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const&  { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return          *reinterpret_cast<const T*>(&Value);  }
	template <typename T> FORCEINLINE constexpr decltype(auto) GetValue() const&& { checkf(HoldsAlternative<T>(), TEXT("It is an error to call GetValue() on an wrong TVariant. Please either check HoldsAlternative() or use Get(DefaultValue) instead.")); return MoveTemp(*reinterpret_cast<const T*>(&Value)); }

	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) Get(      TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) &      { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }
	template <size_t I> requires (I < sizeof...(Ts)) FORCEINLINE constexpr decltype(auto) Get(const TVariantAlternative<I, TVariant<Ts...>>& DefaultValue) const& { return HoldsAlternative<I>() ? GetValue<I>() : DefaultValue; }

	template <typename T> FORCEINLINE constexpr decltype(auto) Get(      T& DefaultValue) &      { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }
	template <typename T> FORCEINLINE constexpr decltype(auto) Get(const T& DefaultValue) const& { return HoldsAlternative<T>() ? GetValue<T>() : DefaultValue; }

	FORCEINLINE constexpr void Reset()
	{
		if (GetIndex() == INDEX_NONE) return;

		if constexpr (!(true && ... && CTriviallyDestructible<Ts>))
		{
			DestroyImpl[GetIndex()](&Value);
		}

		TypeIndex = static_cast<uint8>(INDEX_NONE);
	}

	FORCEINLINE constexpr size_t GetTypeHash() const requires (true && ... && CHashable<Ts>)
	{
		if (!IsValid()) return 114514;

		using NAMESPACE_REDCRAFT::GetTypeHash;

		using FHashImpl = size_t(*)(const void*);
		constexpr FHashImpl HashImpl[] = { [](const void* This) -> size_t { return GetTypeHash(*reinterpret_cast<const Ts*>(This)); }... };

		return HashCombine(GetTypeHash(GetIndex()), HashImpl[GetIndex()](&Value));
	}

	constexpr void Swap(TVariant& InValue) requires (true && ... && (CMoveConstructible<Ts> && CSwappable<Ts>))
	{
		if (!IsValid() && !InValue.IsValid()) return;

		if (IsValid() && !InValue.IsValid())
		{
			InValue = MoveTemp(*this);
			Reset();
			return;
		}

		if (InValue.IsValid() && !IsValid())
		{
			*this = MoveTemp(InValue);
			InValue.Reset();
			return;
		}

		if (GetIndex() == InValue.GetIndex())
		{
			using NAMESPACE_REDCRAFT::Swap;

			using FSwapImpl = void(*)(void*, void*);
			constexpr FSwapImpl SwapImpl[] = { [](void* A, void* B) { Swap(*reinterpret_cast<Ts*>(A), *reinterpret_cast<Ts*>(B)); }... };

			SwapImpl[GetIndex()](&Value, &InValue.Value);

			return;
		}

		TVariant Temp = MoveTemp(*this);
		*this = MoveTemp(InValue);
		InValue = MoveTemp(Temp);
	}

private:
	
	static constexpr const type_info* TypeInfos[] = { &typeid(Ts)... };

	using FCopyConstructImpl = void(*)(void*, const void*);
	using FMoveConstructImpl = void(*)(void*,       void*);
	using FCopyAssignImpl    = void(*)(void*, const void*);
	using FMoveAssignImpl    = void(*)(void*,       void*);
	using FDestroyImpl       = void(*)(void*             );

	static constexpr FCopyConstructImpl CopyConstructImpl[] = { [](void* A, const void* B) { if constexpr (requires(Ts* A, const Ts* B) { Memory::CopyConstruct (A, B); }) Memory::CopyConstruct (reinterpret_cast<Ts*>(A), reinterpret_cast<const Ts*>(B)); else checkf(false, TEXT("The type '%s' is not copy constructible."), typeid(Ts).name()); }... };
	static constexpr FMoveConstructImpl MoveConstructImpl[] = { [](void* A,       void* B) { if constexpr (requires(Ts* A,       Ts* B) { Memory::MoveConstruct (A, B); }) Memory::MoveConstruct (reinterpret_cast<Ts*>(A), reinterpret_cast<      Ts*>(B)); else checkf(false, TEXT("The type '%s' is not move constructible."), typeid(Ts).name()); }... };
	static constexpr FCopyAssignImpl    CopyAssignImpl[]    = { [](void* A, const void* B) { if constexpr (requires(Ts* A, const Ts* B) { Memory::CopyAssign    (A, B); }) Memory::CopyAssign    (reinterpret_cast<Ts*>(A), reinterpret_cast<const Ts*>(B)); else checkf(false, TEXT("The type '%s' is not copy assignable."),    typeid(Ts).name()); }... };
	static constexpr FMoveAssignImpl    MoveAssignImpl[]    = { [](void* A,       void* B) { if constexpr (requires(Ts* A,       Ts* B) { Memory::MoveAssign    (A, B); }) Memory::MoveAssign    (reinterpret_cast<Ts*>(A), reinterpret_cast<      Ts*>(B)); else checkf(false, TEXT("The type '%s' is not move assignable."),    typeid(Ts).name()); }... };
	static constexpr FDestroyImpl       DestroyImpl[]       = { [](void* A               ) { if constexpr (requires(Ts* A             ) { Memory::Destruct      (A   ); }) Memory::Destruct      (reinterpret_cast<Ts*>(A)                                   ); else checkf(false, TEXT("The type '%s' is not destructible."),       typeid(Ts).name()); }... };

	TAlignedUnion<1, Ts...> Value;
	uint8 TypeIndex;

};

NAMESPACE_PRIVATE_BEGIN

template <typename F, typename... VariantTypes>
struct TVariantVisitImpl
{
	struct GetTotalNum
	{
		FORCEINLINE static constexpr size_t Do()
		{
			if (sizeof...(VariantTypes) == 0) return 0;

			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			size_t Result = 1;

			for (size_t Index = 0; Index < sizeof...(VariantTypes); ++Index)
			{
				Result *= VariantNums[Index];
			}

			return Result;
		};
	};

	struct EncodeIndices
	{
		FORCEINLINE static constexpr size_t Do(initializer_list<size_t> Indices)
		{
			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			size_t Result = 0;

			for (size_t Index = 0; Index < sizeof...(VariantTypes); ++Index)
			{
				Result *= VariantNums[Index];
				Result += GetData(Indices)[Index];
			}

			return Result;
		};
	};

	struct DecodeExtent
	{
		FORCEINLINE static constexpr size_t Do(size_t EncodedIndex, size_t Extent)
		{
			constexpr size_t VariantNums[] = { TVariantNum<TRemoveReference<VariantTypes>>... };

			for (size_t Index = Extent + 1; Index < sizeof...(VariantTypes); ++Index)
			{
				EncodedIndex /= VariantNums[Index];
			}

			return EncodedIndex % VariantNums[Extent];
		};
	};

	template <size_t EncodedIndex, typename>
	struct InvokeEncoded;

	template <size_t EncodedIndex, size_t... ExtentIndices>
	struct InvokeEncoded<EncodedIndex, TIndexSequence<ExtentIndices...>>
	{
		FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
		{
			return Invoke(Forward<F>(Func), Forward<VariantTypes>(Variants).template GetValue<DecodeExtent::Do(EncodedIndex, ExtentIndices)>()...);
		}

		template <typename Ret>
		struct Result
		{
			FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
			{
				return InvokeResult<Ret>(Forward<F>(Func), Forward<VariantTypes>(Variants).template GetValue<DecodeExtent::Do(EncodedIndex, ExtentIndices)>()...);
			}
		};
	};

	template <typename>
	struct InvokeVariant;

	template <size_t... EncodedIndices>
	struct InvokeVariant<TIndexSequence<EncodedIndices...>>
	{
		FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
		{
			using ExtentIndices = TIndexSequenceFor<VariantTypes...>;

			using ResultType = TCommonType<decltype(InvokeEncoded<EncodedIndices, ExtentIndices>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...))...>;
			
			using InvokeImplType = ResultType(*)(F&&, VariantTypes&&...);

			constexpr InvokeImplType InvokeImpl[] = { InvokeEncoded<EncodedIndices, ExtentIndices>::template Result<ResultType>::Do... };

			return InvokeImpl[EncodeIndices::Do({ Variants.GetIndex()... })](Forward<F>(Func), Forward<VariantTypes>(Variants)...);
		}

		template <typename Ret>
		struct Result
		{
			FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
			{
				using ExtentIndices = TIndexSequenceFor<VariantTypes...>;

				using InvokeImplType = Ret(*)(F&&, VariantTypes&&...);

				constexpr InvokeImplType InvokeImpl[] = { InvokeEncoded<EncodedIndices, ExtentIndices>::template Result<Ret>::Do... };

				return InvokeImpl[EncodeIndices::Do({ Variants.GetIndex()... })](Forward<F>(Func), Forward<VariantTypes>(Variants)...);
			}
		};
	};

	FORCEINLINE static constexpr decltype(auto) Do(F&& Func, VariantTypes&&... Variants)
	{
		return InvokeVariant<TMakeIndexSequence<GetTotalNum::Do()>>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...);
	}

	template <typename Ret>
	struct Result
	{
		FORCEINLINE static constexpr Ret Do(F&& Func, VariantTypes&&... Variants)
		{
			return InvokeVariant<TMakeIndexSequence<GetTotalNum::Do()>>::template Result<Ret>::Do(Forward<F>(Func), Forward<VariantTypes>(Variants)...);
		}
	};
};

NAMESPACE_PRIVATE_END

template <typename F, typename FirstVariantType, typename... VariantTypes>
	requires (CTVariant<TRemoveReference<FirstVariantType>> && (true && ... && CTVariant<TRemoveReference<VariantTypes>>))
constexpr decltype(auto) Visit(F&& Func, FirstVariantType&& FirstVariant, VariantTypes&&... Variants)
{
	checkf((true && ... && Variants.IsValid()), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
	return NAMESPACE_PRIVATE::TVariantVisitImpl<F, FirstVariantType, VariantTypes...>::Do(Forward<F>(Func), Forward<FirstVariantType>(FirstVariant), Forward<VariantTypes>(Variants)...);
}

template <typename Ret, typename F, typename FirstVariantType, typename... VariantTypes>
	requires (CTVariant<TRemoveReference<FirstVariantType>> && (true && ... && CTVariant<TRemoveReference<VariantTypes>>))
constexpr Ret Visit(F&& Func, FirstVariantType&& FirstVariant, VariantTypes&&... Variants)
{
	checkf((true && ... && Variants.IsValid()), TEXT("It is an error to call Visit() on an wrong TVariant. Please either check IsValid()."));
	return NAMESPACE_PRIVATE::TVariantVisitImpl<F, FirstVariantType, VariantTypes...>::template Result<Ret>::Do(Forward<F>(Func), Forward<FirstVariantType>(FirstVariant), Forward<VariantTypes>(Variants)...);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
