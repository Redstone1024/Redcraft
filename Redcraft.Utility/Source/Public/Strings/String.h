#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Optional.h"
#include "Memory/Allocators.h"
#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Iterators/Utility.h"
#include "Iterators/Sentinel.h"
#include "Iterators/BasicIterator.h"
#include "Iterators/InsertIterator.h"
#include "Ranges/Utility.h"
#include "Ranges/Factory.h"
#include "Strings/Char.h"
#include "Strings/StringView.h"
#include "Strings/Formatting.h"
#include "Miscellaneous/AssertionMacros.h"

#include <locale>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <typename T            > struct TIsTString                : FFalse { };
template <typename T, typename A> struct TIsTString<TString<T, A>> : FTrue  { };

NAMESPACE_PRIVATE_END

template <typename T> concept CTString = NAMESPACE_PRIVATE::TIsTString<TRemoveCV<T>>::Value;

/** The default string allocator that uses SSO and can be placed right into FAny without dynamically allocating memory. */
template <CCharType T>
using TDefaultStringAllocator = TInlineAllocator<(40 - 3 * sizeof(size_t)) / sizeof(T)>;

/** A string class that stores and manipulates sequences of characters. Unlike std::basic_string, it is not null-terminated. */
template <CCharType T, CAllocator<T> Allocator = TDefaultStringAllocator<T>>
class TString : public TArray<T, Allocator>
{
private:

	using FSuper = TArray<T, Allocator>;

public:

	using FElementType   = typename FSuper::FElementType;
	using FAllocatorType = typename FSuper::FAllocatorType;

	using      FReference = typename FSuper::     FReference;
	using FConstReference = typename FSuper::FConstReference;

	using      FIterator = typename FSuper::     FIterator;
	using FConstIterator = typename FSuper::FConstIterator;

	using      FReverseIterator = typename FSuper::     FReverseIterator;
	using FConstReverseIterator = typename FSuper::FConstReverseIterator;

	static_assert(CContiguousIterator<     FIterator>);
	static_assert(CContiguousIterator<FConstIterator>);

	/** Default constructor. Constructs an empty string. */
	FORCEINLINE TString() = default;

	/** Constructs the string with 'Count' copies of characters with 'InValue'. */
	FORCEINLINE TString(size_t Count, FElementType InChar) : FSuper(Count, InChar) { }

	/** Constructs a string with the contents of the range ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE TString(const FElementType* InPtr, size_t Count) : TString(TStringView<FElementType>(InPtr, Count))
	{
		checkf(InPtr != nullptr, TEXT("TString cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE TString(nullptr_t, size_t) = delete;

	/** Constructs a string with the contents of the range ['InPtr', '\0'). */
	FORCEINLINE TString(const FElementType* InPtr) : TString(TStringView<FElementType>(InPtr))
	{
		checkf(InPtr != nullptr, TEXT("TString cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE TString(nullptr_t) = delete;

	/** Constructs the string with the contents of the 'View'. */
	FORCEINLINE TString(TStringView<FElementType> View) : TString(View.Begin(), View.End()) { }

	/** Constructs the string with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	FORCEINLINE TString(I First, S Last) : FSuper(MoveTemp(First), MoveTemp(Last)) { }

	/** Constructs the string with the contents of the range. */
	template <CInputRange R> requires (!CSameAs<TRemoveCVRef<R>, TString>
		&& !CSameAs<TRemoveCVRef<R>, TStringView<FElementType>> && CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE explicit TString(R&& Range) : TString(Ranges::Begin(Range), Ranges::End(Range)) { }

	/** Copy constructor. Constructs the string with the copy of the contents of 'InValue'. */
	FORCEINLINE TString(const TString&) = default;

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString(TString&&) = default;

	/** Constructs the string with the contents of the initializer list. */
	FORCEINLINE TString(initializer_list<FElementType> IL) : TString(Ranges::Begin(IL), Ranges::End(IL)) { }

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	FORCEINLINE TString& operator=(const TString&) = default;

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString& operator=(TString&&) = default;

	/** Compares the contents of two strings. */
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS, const TString& RHS) { return TStringView<FElementType>(LHS) == TStringView<FElementType>(RHS); }

	/** Compares the contents of a string and a character. */
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS,       FElementType  RHS) { return TStringView<FElementType>(LHS) == RHS; }
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS, const FElementType* RHS) { return TStringView<FElementType>(LHS) == RHS; }
	NODISCARD friend FORCEINLINE bool operator==(      FElementType  LHS, const TString& RHS) { return LHS == TStringView<FElementType>(RHS); }
	NODISCARD friend FORCEINLINE bool operator==(const FElementType* LHS, const TString& RHS) { return LHS == TStringView<FElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS, const TString& RHS) { return TStringView<FElementType>(LHS) <=> TStringView<FElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS,       FElementType  RHS) { return TStringView<FElementType>(LHS) <=> RHS; }
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS, const FElementType* RHS) { return TStringView<FElementType>(LHS) <=> RHS; }
	NODISCARD friend FORCEINLINE auto operator<=>(      FElementType  LHS, const TString& RHS) { return LHS <=> TStringView<FElementType>(RHS); }
	NODISCARD friend FORCEINLINE auto operator<=>(const FElementType* LHS, const TString& RHS) { return LHS <=> TStringView<FElementType>(RHS); }

public:

	/** Inserts 'InValue' before 'Index' in the string. */
	FORCEINLINE FIterator Insert(size_t Index, FElementType InValue)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, InValue);
	}

	/** Inserts 'InValue' before 'Iter' in the string. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, FElementType InValue)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::Insert(Iter, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Index' in the string. */
	FORCEINLINE FIterator Insert(size_t Index, size_t Count, FElementType InValue)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, Count, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the string. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, size_t Count, FElementType InValue)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::Insert(Iter, Count, InValue);
	}

	/** Inserts characters from the 'View' before 'Index' in the string. */
	FORCEINLINE FIterator Insert(size_t Index, TStringView<FElementType> View)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, View);
	}

	/** Inserts characters from the 'View' before 'Iter' in the string. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, TStringView<FElementType> View)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Insert(Iter, View.Begin(), View.End());
	}

	/** Inserts characters from the range ['First', 'Last') before 'Index' in the string. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	FORCEINLINE FIterator Insert(size_t Index, I First, S Last)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	FORCEINLINE FIterator Insert(FConstIterator Iter, I First, S Last)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::Insert(Iter, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the initializer list before 'Index' in the string. */
	FORCEINLINE FIterator Insert(size_t Index, initializer_list<FElementType> IL)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, IL);
	}

	/** Inserts characters from the initializer list before 'Iter' in the string. */
	FORCEINLINE FIterator Insert(FConstIterator Iter, initializer_list<FElementType> IL)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::Insert(Iter, IL);
	}

	/** Erases the character at 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE FIterator Erase(size_t Index, bool bAllowShrinking = true)
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index < Num()."));

		return Erase(this->Begin() + Index, bAllowShrinking);
	}

	/** Erases the character at 'Iter' in the string. But it may change the order of characters. */
	FORCEINLINE FIterator Erase(FConstIterator Iter, bool bAllowShrinking = true)
	{
		checkf(this->IsValidIterator(Iter) && Iter != this->End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::StableErase(Iter, bAllowShrinking);
	}

	/** Erases 'CountToErase' characters starting from 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE FIterator Erase(size_t Index, size_t CountToErase, bool bAllowShrinking = true)
	{
		checkf(Index <= this->Num() && Index + CountToErase <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToErase."));

		return Erase(this->Begin() + Index, this->Begin() + Index + CountToErase, bAllowShrinking);

	}

	/** Erases the characters in the range ['First', 'Last') in the string. But it may change the order of characters. */
	FORCEINLINE FIterator Erase(FConstIterator First, FConstIterator Last, bool bAllowShrinking = true)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return FSuper::StableErase(First, Last, bAllowShrinking);
	}

	/** Here, the 'Erase' is already stable and there is no need to provide 'StableErase'. */
	void StableErase(...) = delete;

	/** Appends 'Count' copies of the 'InValue' to the end of the string. */
	TString& Append(size_t Count, FElementType InChar) { return Append(Ranges::Repeat(InChar, Count)); }

	/** Appends the contents of the range ['InPtr', 'InPtr' + 'Count') to the end of the string. */
	FORCEINLINE TString& Append(const FElementType* InPtr, size_t Count) { return Append(TStringView<FElementType>(InPtr, Count)); }

	FORCEINLINE TString& Append(nullptr_t, size_t) = delete;

	/** Constructs a string with the contents of the range ['InPtr', '\0'). */
	FORCEINLINE TString& Append(const FElementType* InPtr) { return Append(TStringView<FElementType>(InPtr)); }

	FORCEINLINE TString& Append(nullptr_t) = delete;

	/** Appends the contents of the range ['First', 'Last') to the end of the string. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	TString& Append(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			size_t Count = 0;

			if constexpr (CSizedSentinelFor<S, I>)
			{
				checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));

				Count = Last - First;
			}
			else for (I Iter = First; Iter != Last; ++Iter) ++Count;

			const size_t CurrentNum = this->Num();

			this->SetNum(CurrentNum + Count);

			for (size_t Index = CurrentNum; Index != CurrentNum + Count; ++Index)
			{
				(*this)[Index] = FElementType(*First++);
			}
		}
		else
		{
			Insert(this->End(), MoveTemp(First), MoveTemp(Last));
		}

		return *this;
	}

	/** Appends the contents of the range to the end of the string. */
	template <CInputRange R> requires (CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE TString& Append(R&& Range) { return Append(Ranges::Begin(Range), Ranges::End(Range)); }

	/** Appends the contents of the initializer list to the end of the string. */
	FORCEINLINE TString& Append(initializer_list<FElementType> IL) { return Append(Ranges::Begin(IL), Ranges::End(IL)); }

	/** Appends the given character value to the end of the string. */
	FORCEINLINE TString& operator+=(FElementType InChar) { return Append(1, InChar); }

	/** Appends the contents of the range ['InPtr', '\0') to the end of the string. */
	FORCEINLINE TString& operator+=(const FElementType* InPtr) { return Append(InPtr); }

	FORCEINLINE TString& operator+=(nullptr_t) = delete;

	/** Appends the contents of the range to the end of the string. */
	template <CInputRange R> requires (CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE TString& operator+=(R&& Range) { return Append(Range); }

	/** Appends the contents of the range ['First', 'Last') to the end of the string. */
	FORCEINLINE TString& operator+=(initializer_list<FElementType> IL) { return Append(IL); }

	/** Concatenates two strings. */
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS, const TString& RHS) { return TString(LHS).Append(RHS); }

	/** Concatenates the string with the given character value. */
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS,             FElementType  RHS) { return TString(LHS).Append(1, RHS); }
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS,       const FElementType* RHS) { return TString(LHS).Append(   RHS); }
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS, TStringView<FElementType> RHS) { return TString(LHS).Append(   RHS); }
	NODISCARD friend FORCEINLINE TString operator+(            FElementType  LHS, const TString& RHS) { return TString(1, LHS).Append(RHS); }
	NODISCARD friend FORCEINLINE TString operator+(      const FElementType* LHS, const TString& RHS) { return TString(   LHS).Append(RHS); }
	NODISCARD friend FORCEINLINE TString operator+(TStringView<FElementType> LHS, const TString& RHS) { return TString(   LHS).Append(RHS); }

	/** Concatenates two strings. The rvalue maybe modified. */
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS, TString&& RHS) { LHS.Append(MoveTemp(RHS)); return LHS; }

	/** Concatenates two strings. The rvalue maybe modified. */
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,               FElementType   RHS) { LHS.Append(1, RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,         const FElementType*  RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,   TStringView<FElementType>  RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS, const TString<FElementType>& RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(              FElementType   LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(        const FElementType*  LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(  TStringView<FElementType>  LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(const TString<FElementType>& LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }

public:

	/** Shrinks the view by moving its start forward. */
	FORCEINLINE constexpr TString& RemovePrefix(size_t Count, bool bAllowShrinking = true)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		Erase(0, Count, bAllowShrinking);

		return *this;
	}

	/** Shrinks the view by moving its end backward. */
	FORCEINLINE constexpr TString& RemoveSuffix(size_t Count, bool bAllowShrinking = true)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		this->SetNum(this->Num() - Count, bAllowShrinking);

		return *this;
	}

	/** Removes whitespace characters from the start of this string. */
	FORCEINLINE constexpr TString& TrimStart(bool bAllowShrinking = true)
	{
		auto Index = Find([](FElementType Char) { return !TChar<FElementType>::IsSpace(Char); });

		if (Index != INDEX_NONE)
		{
			RemovePrefix(Index, bAllowShrinking);
		}
		else this->Reset(bAllowShrinking);

		return *this;
	}

	/** Removes whitespace characters from the end of this string. */
	FORCEINLINE constexpr TString& TrimEnd(bool bAllowShrinking = true)
	{
		auto Index = RFind([](FElementType Char) { return !TChar<FElementType>::IsSpace(Char); });

		if (Index != INDEX_NONE)
		{
			this->SetNum(Index + 1, bAllowShrinking);
		}
		else this->Reset(bAllowShrinking);

		return *this;
	}

	/** Removes whitespace characters from the start and end of this string. */
	FORCEINLINE constexpr TString& TrimStartAndEnd(bool bAllowShrinking = true)
	{
		TrimStart(false);
		TrimEnd(bAllowShrinking);

		return *this;
	}

	/** Removes characters after the first null-terminator. */
	FORCEINLINE constexpr TString& TrimToNullTerminator(bool bAllowShrinking = true)
	{
		auto Index = Find(LITERAL(FElementType, '\0'));

		if (Index != INDEX_NONE)
		{
			this->SetNum(Index, bAllowShrinking);
		}

		return *this;
	}

public:

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE bool StartsWith(TStringView<FElementType> Prefix) const
	{
		return TStringView<FElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE bool StartsWith(FElementType Prefix) const
	{
		return TStringView<FElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE bool EndsWith(TStringView<FElementType> Suffix) const
	{
		return TStringView<FElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE bool EndsWith(FElementType Suffix) const
	{
		return TStringView<FElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view contains the given substring, false otherwise. */
	NODISCARD FORCEINLINE bool Contains(TStringView<FElementType> View) const
	{
		return TStringView<FElementType>(*this).Contains(View);
	}

	/** @return true if the string view contains the given character, false otherwise. */
	NODISCARD FORCEINLINE bool Contains(FElementType Char) const
	{
		return TStringView<FElementType>(*this).Contains(Char);
	}

	/** @return true if the string view contains character that satisfy the given predicate, false otherwise. */
	template <CPredicate<FElementType> F>
	NODISCARD FORCEINLINE bool Contains(F&& InPredicate) const
	{
		return TStringView<FElementType>(*this).Contains(Forward<F>(InPredicate));
	}

	/** Replace the substring [Index, Index + CountToReplace) with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, size_t Count, FElementType InChar)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, Count, InChar);
	}

	/** Replace the substring ['First', 'Last') with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(FConstIterator First, FConstIterator Last, size_t Count, FElementType InChar)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, Ranges::Repeat(InChar, Count));
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, const FElementType* InPtr, size_t Count)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, InPtr, InPtr + Count);
	}

	FORCEINLINE TString& Replace(size_t, size_t, nullptr_t, size_t) = delete;

	/** Replace the substring ['First', 'Last') with the contents of the ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE TString& Replace(FConstIterator First, FConstIterator Last, const FElementType* InPtr, size_t Count)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, TStringView<FElementType>(InPtr, Count));
	}

	FORCEINLINE TString& Replace(FConstIterator, FConstIterator, nullptr_t, size_t) = delete;

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the ['InPtr', '\0'). */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, const FElementType* InPtr)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, InPtr);
	}

	FORCEINLINE TString& Replace(size_t, size_t, nullptr_t) = delete;

	/** Replace the substring ['First', 'Last') with the contents of the ['InPtr', '\0'). */
	FORCEINLINE TString& Replace(FConstIterator First, FConstIterator Last, const FElementType* InPtr)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, TStringView<FElementType>(InPtr));
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, I InString, S Sentinel)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, MoveTemp(InString), MoveTemp(Sentinel));
	}

	/** Replace the substring ['First', 'Last') with the contents of the range ['InString', 'Sentinel'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<FElementType, TIteratorReference<I>>)
	TString& Replace(FConstIterator First, FConstIterator Last, I InString, S Sentinel)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));

			const size_t InsertIndex = First - this->Begin();
			const size_t RemoveCount = Last - First;

			size_t InsertCount = 0;

			if constexpr (CSizedSentinelFor<S, I>)
			{
				checkf(InString - Sentinel <= 0, TEXT("Illegal range iterator. Please check InString <= Sentinel."));

				InsertCount = Sentinel - InString;
			}
			else for (I Iter = InString; Iter != Sentinel; ++Iter) ++InsertCount;

			const size_t NumToReset = this->Num() - RemoveCount + InsertCount;

			if (InsertCount < RemoveCount)
			{
				for (size_t Index = InsertIndex; Index != InsertIndex + InsertCount; ++Index)
				{
					(*this)[Index] = FElementType(*InString++);
				}

				for (size_t Index = InsertIndex + InsertCount; Index != NumToReset; ++Index)
				{
					(*this)[Index] = (*this)[Index + (RemoveCount - InsertCount)];
				}

				this->SetNum(NumToReset);
			}
			else
			{
				this->SetNum(NumToReset);

				for (size_t Index = this->Num(); Index != InsertIndex + InsertCount - 1; --Index)
				{
					(*this)[Index - 1] = (*this)[Index + (RemoveCount - InsertCount) - 1];
				}

				for (size_t Index = InsertIndex; Index != InsertIndex + InsertCount; ++Index)
				{
					(*this)[Index] = FElementType(*InString++);
				}
			}
		}
		else
		{
			TString Temp(MoveTemp(First), MoveTemp(Last));

			return Replace(First, Last, Temp.Begin(), Temp.End());
		}

		return *this;
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the range. */
	template <CInputRange R> requires (CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, R&& Range)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, Forward<R>(Range));
	}

	/** Replace the substring ['First', 'Last') with the contents of the range. */
	template <CInputRange R> requires (CConstructibleFrom<FElementType, TRangeReference<R>>)
	FORCEINLINE TString& Replace(FConstIterator First, FConstIterator Last, R&& Range)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, Ranges::Begin(Range), Ranges::End(Range));
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the initializer list. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, initializer_list<FElementType> IL)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, IL);
	}

	/** Replace the substring ['First', 'Last') with the contents of the initializer list. */
	FORCEINLINE TString& Replace(FConstIterator First, FConstIterator Last, initializer_list<FElementType> IL)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, Ranges::Begin(IL), Ranges::End(IL));
	}

	/** Obtains a string that is a view over the 'Count' characters of this string view starting at 'Offset'.  */
	FORCEINLINE TString Substr(size_t Offset = 0, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= this->Num() && Offset + Count <= this->Num(), TEXT("Illegal substring range. Please check Offset and Count."));

		return TString(TStringView<FElementType>(*this).Substr(Offset, Count));
	}

	/** Copies the characters of this string to the destination buffer without null-termination. */
	FORCEINLINE size_t Copy(FElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
	{
		checkf(Dest != nullptr, TEXT("Illegal destination buffer. Please check the pointer."));

		checkf(Offset <= this->Num() && (Count == DynamicExtent || Offset + Count <= this->Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		return TStringView<FElementType>(*this).Copy(Dest, Count, Offset);
	}

	FORCEINLINE size_t Copy(nullptr_t, size_t = DynamicExtent, size_t = 0) const = delete;

	/** @return The index of the first occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD size_t Find(TStringView<FElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).Find(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD size_t Find(FElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).Find(Char, Index);
	}

	/** @return The index of the first occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<FElementType> F>
	NODISCARD size_t Find(F&& InPredicate, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).Find(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the last occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD size_t RFind(TStringView<FElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).RFind(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD size_t RFind(FElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).RFind(Char, Index);
	}

	/** @return The index of the last occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<FElementType> F>
	NODISCARD size_t RFind(F&& InPredicate, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).RFind(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the first occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstOf(TStringView<FElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindFirstOf(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstOf(FElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindFirstOf(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastOf(TStringView<FElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindLastOf(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastOf(FElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindLastOf(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstNotOf(TStringView<FElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindFirstNotOf(View, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstNotOf(FElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindFirstNotOf(Char, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastNotOf(TStringView<FElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindLastNotOf(View, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastNotOf(FElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<FElementType>(*this).FindLastNotOf(Char, Index);
	}

public:

	/** Try to decode the given character using the U-encoded to a string using the T-encoded. */
	template <CCharType U>
	bool DecodeFrom(U Char, bool bAllowShrinking = true)
	{
		return DecodeFrom(TStringView(&Char, 1), bAllowShrinking);
	}

	/** Try to decode the given string using the U-encoded to a string using the T-encoded. */
	template <CCharType U, CAllocator<U> A>
	bool DecodeFrom(const TString<U, A>& String, bool bAllowShrinking = true)
	{
		return DecodeFrom(TStringView(String), bAllowShrinking);
	}

	/** Try to decode the given string view using the U-encoded to a string using the T-encoded. */
	template <CCharType U>
	bool DecodeFrom(TStringView<U> View, bool bAllowShrinking = true)
	{
		this->Reset(false);

		auto AppendToResult = [this]<typename W>(auto& Self, TStringView<W> View) -> bool
		{
			//  char ->  char
			// wchar -> wchar
			if constexpr (CSameAs<W, char> && CSameAs<T, char> || CSameAs<W, wchar> && CSameAs<T, wchar>)
			{
				// Unable to determine whether the user-preferred locale encoded character is valid or not, it is assumed to be valid.
				Insert(this->End(), View.Begin(), View.End());

				return true;
			}

			// char -> wchar
			// char -> wchar -> ...
			else if constexpr (CSameAs<W, char>)
			{
				NAMESPACE_STD::locale Loc = NAMESPACE_STD::locale("");

				check((NAMESPACE_STD::has_facet<NAMESPACE_STD::codecvt<wchar_t, char, NAMESPACE_STD::mbstate_t>>(Loc)));

				const auto& Facet = NAMESPACE_STD::use_facet<NAMESPACE_STD::codecvt<wchar, char, NAMESPACE_STD::mbstate_t>>(Loc);

				NAMESPACE_STD::mbstate_t State = NAMESPACE_STD::mbstate_t();

				const char* BeginFrom = ToAddress(View.Begin());
				const char* EndFrom   = ToAddress(View.End());

				wchar Buffer[FWChar::MaxCodeUnitLength];

				const char* NextFrom;
				wchar* NextTo;

				do
				{
					const auto Result = Facet.in(State, BeginFrom, EndFrom, NextFrom, Ranges::Begin(Buffer), Ranges::End(Buffer), NextTo);

					if (BeginFrom == NextFrom) return false;

					if (Result == NAMESPACE_STD::codecvt_base::error)  return false;
					if (Result == NAMESPACE_STD::codecvt_base::noconv) return false;

					// char -> wchar
					if constexpr (CSameAs<T, wchar>)
					{
						for (wchar* Iter = Buffer; Iter != NextTo; ++Iter)
						{
							this->PushBack(*Iter);
						}
					}
					else
					{
						if (!Self(Self, TStringView(Buffer, NextTo))) return false;
					}

					BeginFrom = NextFrom;
				}
				while (BeginFrom != EndFrom);

				return true;
			}

			// wchar -> char
			else if constexpr (CSameAs<W, wchar> && CSameAs<T, char>)
			{
				NAMESPACE_STD::locale Loc = NAMESPACE_STD::locale("");

				check((NAMESPACE_STD::has_facet<NAMESPACE_STD::codecvt<wchar_t, char, NAMESPACE_STD::mbstate_t>>(Loc)));

				const auto& Facet = NAMESPACE_STD::use_facet<NAMESPACE_STD::codecvt<wchar, char, NAMESPACE_STD::mbstate_t>>(Loc);

				NAMESPACE_STD::mbstate_t State = NAMESPACE_STD::mbstate_t();

				const wchar* BeginFrom = ToAddress(View.Begin());
				const wchar* EndFrom   = ToAddress(View.End());

				char Buffer[FChar::MaxCodeUnitLength];

				const wchar* NextFrom;
				char* NextTo;

				do
				{
					const auto Result = Facet.out(State, BeginFrom, EndFrom, NextFrom, Ranges::Begin(Buffer), Ranges::End(Buffer), NextTo);

					if (BeginFrom == NextFrom) return false;

					if (Result == NAMESPACE_STD::codecvt_base::error)  return false;
					if (Result == NAMESPACE_STD::codecvt_base::noconv) return false;

					for (char* Iter = Buffer; Iter != NextTo; ++Iter)
					{
						this->PushBack(*Iter);
					}

					BeginFrom = NextFrom;
				}
				while (BeginFrom != EndFrom);

				return true;
			}

			// u8char -> unicodechar -> ...
			else if constexpr (CSameAs<W, u8char>)
			{
				auto Iter = View.Begin();

				while (Iter != View.End())
				{
					unicodechar Temp = static_cast<unicodechar>(*Iter++);

					unicodechar Unicode;

					if      ((Temp & 0b10000000) == 0b00000000) // 0XXXXXXX
					{
						Unicode = Temp;
					}

					else if ((Temp & 0b11100000) == 0b11000000) // 110XXXXX 10XXXXXX
					{
						if (Iter + 1 > View.End()) return false;

						Unicode = (Temp & 0b00011111) << 6;

						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |= Temp & 0b00111111;
					}

					else if ((Temp & 0b11110000) == 0b11100000) // 1110XXXX 10XXXXXX 10XXXXXX
					{
						if (Iter + 2 > View.End()) return false;

						Unicode = (Temp & 0b00001111) << 12;

						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |= (Temp & 0b00111111) << 6;
						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |=  Temp & 0b00111111;
					}

					else if ((Temp & 0b11111000) == 0b11110000) // 11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
					{
						if (Iter + 3 > View.End()) return false;

						Unicode = (Temp & 0b00000111) << 18;

						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |= (Temp & 0b00111111) << 12;
						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |= (Temp & 0b00111111) <<  6;
						Temp = static_cast<unicodechar>(*Iter++); if ((Temp & 0b11000000) != 0b10000000) return false; else Unicode |=  Temp & 0b00111111;
					}

					else return false;

					if (!Self(Self, TStringView(&Unicode, 1))) return false;
				}

				return true;
			}

			// u16char -> unicodechar -> ...
			//   wchar -> unicodechar -> ... for Windows
			else if constexpr (CSameAs<W, u16char> || PLATFORM_WINDOWS && CSameAs<W, wchar>)
			{
				auto Iter = View.Begin();

				while (Iter != View.End())
				{
					unicodechar Temp = static_cast<unicodechar>(*Iter++);

					unicodechar Unicode;

					// High Surrogate <UD800>..<UDBFF>;
					// Low Surrogate  <UDC00>..<UDFFF>;

					if (Temp >= 0xD800 && Temp <= 0xDBFF)
					{
						if (Iter == View.End()) return false;

						Unicode = (Temp & 0b00000011'11111111) << 10;

						Temp = static_cast<unicodechar>(*Iter++);

						if (Temp >= 0xDC00 && Temp <= 0xDFFF)
						{
							Unicode |= Temp & 0b00000011'11111111;

							Unicode += 0x10000;
						}
						else return false;
					}
					else Unicode = Temp;

					if (!Self(Self, TStringView(&Unicode, 1))) return false;
				}

				return true;
			}

			//   wchar -> unicodechar -> ... for Linux
			else if constexpr (PLATFORM_LINUX && CSameAs<W, wchar>)
			{
				return Self(Self, TStringView(reinterpret_cast<const u32char*>(View.GetData()), View.Num()));
			}

			// unicodechar u32char -> u8char
			else if constexpr (CSameAs<W, unicodechar> && CSameAs<T, u8char>)
			{
				for (unicodechar Char : View)
				{
					if (!FUnicodeChar::IsValid(Char)) return false;

					if      (!(Char & ~0b0000000'00000000'00000000'01111111)) // 0XXXXXXX
					{
						this->PushBack(static_cast<u8char>(Char));
					}
					else if (!(Char & ~0b0000000'00000000'00000111'11111111)) // 110XXXXX 10XXXXXX
					{
						this->PushBack(static_cast<u8char>(0b11000000 | (Char >> 6 & 0b00011111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char & 0b00111111)));
					}
					else if (!(Char & ~0b0000000'00000000'11111111'11111111)) // 1110XXXX 10XXXXXX 10XXXXXX
					{
						this->PushBack(static_cast<u8char>(0b11100000 | (Char >> 12 & 0b00001111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char >>  6 & 0b00111111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char       & 0b00111111)));
					}
					else if (!(Char & ~0b0000000'11111111'11111111'11111111)) // 11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
					{
						this->PushBack(static_cast<u8char>(0b11110000 | (Char >> 18 & 0b00000111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char >> 12 & 0b00111111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char >>  6 & 0b00111111)));
						this->PushBack(static_cast<u8char>(0b10000000 | (Char       & 0b00111111)));
					}
					else check_no_entry();
				}

				return true;
			}

			// unicodechar u32char -> u16char
			// unicodechar u32char -> wchar         for Windows
			// unicodechar u32char -> wchar -> char for Windows
			else if constexpr (CSameAs<W, unicodechar> && (CSameAs<T, u16char> || PLATFORM_WINDOWS && (CSameAs<T, char> || CSameAs<T, wchar>)))
			{
				for (unicodechar Char : View)
				{
					if (!FUnicodeChar::IsValid(Char)) return false;

					if      (!(Char & ~0b0000000'00000000'11111111'11111111)) // XXXXXXXX'XXXXXXXX
					{
						if constexpr (PLATFORM_WINDOWS && (CSameAs<T, char> || CSameAs<T, wchar>))
						{
							wchar WChar = static_cast<wchar>(Char);

							if (!Self(Self, TStringView(&WChar, 1))) return false;
						}
						else this->PushBack(static_cast<u16char>(Char));
					}
					else if (!(Char & ~0b0000000'00011111'11111111'11111111)) // 110110XX'XXXXXXXX 110111XX'XXXXXXXX
					{
						Char -= 0x10000;

						u16char Buffer[] = {
							static_cast<u16char>(0b11011000'00000000 | (Char >> 10 & 0b00000011'11111111)),
							static_cast<u16char>(0b11011100'00000000 | (Char       & 0b00000011'11111111))
						};

						if constexpr (PLATFORM_WINDOWS && (CSameAs<T, char> || CSameAs<T, wchar>))
						{
							if (!Self(Self, TStringView(reinterpret_cast<const wchar*>(Buffer), 2))) return false;
						}
						else
						{
							this->PushBack(Buffer[0]);
							this->PushBack(Buffer[1]);
						}
					}
					else check_no_entry();
				}

				return true;
			}

			// unicodechar u32char -> unicodechar u32char
			// unicodechar u32char -> wchar         for Linux
			// unicodechar u32char -> wchar -> char for Linux
			else if constexpr (CSameAs<W, unicodechar> && (CSameAs<T, unicodechar> || PLATFORM_LINUX && (CSameAs<T, char> || CSameAs<T, wchar>)))
			{
				for (unicodechar Char : View)
				{
					if (!FUnicodeChar::IsValid(Char)) return false;
				}

				if constexpr (PLATFORM_LINUX && (CSameAs<T, char> || CSameAs<T, wchar>))
				{
					return Self(Self, TStringView(reinterpret_cast<const wchar*>(View.GetData()), View.Num()));
				}
				else Insert(this->End(), View.Begin(), View.End());

				return true;
			}

			else static_assert(sizeof(W) == -1, "Unsupported character type");

			return false;
		};

		bool bIsValid = AppendToResult(AppendToResult, View);

		if (!bIsValid) this->Reset(bAllowShrinking);
		else if (bAllowShrinking) this->Shrink();

		return bIsValid;
	}

	/** Try to encode a T-encoded string to a U-encoded string. */
	template <CCharType U, CAllocator<U> A = TDefaultStringAllocator<T>>
	NODISCARD TOptional<TString<U, A>> EncodeTo() const
	{
		TString<U, A> Result;

		bool bIsValid = Result.DecodeFrom(*this);

		if (!bIsValid) return Invalid;

		return Result;
	}

	/** @return The target-encoded string from the T-encoded string. */
	NODISCARD FORCEINLINE auto ToString()        const { return EncodeTo<char>();        }
	NODISCARD FORCEINLINE auto ToWString()       const { return EncodeTo<wchar>();       }
	NODISCARD FORCEINLINE auto ToU8String()      const { return EncodeTo<u8char>();      }
	NODISCARD FORCEINLINE auto ToU16String()     const { return EncodeTo<u16char>();     }
	NODISCARD FORCEINLINE auto ToU32String()     const { return EncodeTo<u32char>();     }
	NODISCARD FORCEINLINE auto ToUnicodeString() const { return EncodeTo<unicodechar>(); }

	/** @return The non-modifiable standard C character string version of the string. */
	NODISCARD FORCEINLINE auto operator*() const&
	{
		if (this->Max() >= this->Num() + 1)
		{
			const_cast<FElementType*>(this->GetData())[this->Num()] = LITERAL(FElementType, '\0');

			return *TStringView(this->GetData(), this->Num() + 1);
		}

		return *TStringView(*this);
	}

	/** @return The non-modifiable standard C character string version of the string. */
	NODISCARD FORCEINLINE auto operator*() &&
	{
		if (this->Back() != LITERAL(T, '\0'))
		{
			this->PushBack(LITERAL(T, '\0'));
		}

		return AsConst(*this).GetData();
	}

public:

	/** @return true if the string only contains valid characters, false otherwise. */
	NODISCARD FORCEINLINE bool IsValid() const
	{
		return TStringView<FElementType>(*this).IsValid();
	}

	/** @return true if the string only contains ASCII characters, false otherwise. */
	NODISCARD FORCEINLINE bool IsASCII() const
	{
		return TStringView<FElementType>(*this).IsASCII();
	}

	/** @return true if the string can be converted to a boolean value, false otherwise. */
	NODISCARD FORCEINLINE bool IsBoolean() const
	{
		return TStringView<FElementType>(*this).IsBoolean();
	}

	/** @return true if the string can be converted to an integer value, false otherwise. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && CSameAs<TRemoveCVRef<U>, U>)
	NODISCARD FORCEINLINE bool IsInteger(uint Base = 0) const
	{
		return TStringView<FElementType>(*this).template IsInteger<U>(Base);
	}

	/** @return true if the string can be converted to a floating-point value, false otherwise. */
	template <CFloatingPoint U = float> requires (!CSameAs<U, bool> && CSameAs<TRemoveCVRef<U>, U>)
	NODISCARD FORCEINLINE bool IsFloatingPoint(bool bFixed = true, bool bScientific = true, bool bHex = true) const
	{
		return TStringView<FElementType>(*this).template IsFloatingPoint<U>(bFixed, bScientific, bHex);
	}

	/** Converts the string into a boolean value. */
	NODISCARD FORCEINLINE constexpr bool ToBool() const
	{
		return TStringView<FElementType>(*this).ToBool();
	}

	/** Converts the string into an integer value. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr U ToInt(uint Base = 0) const
	{
		return TStringView<FElementType>(*this).template ToInt<U>(Base);
	}

	/** Converts the string into a floating-point value. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr U ToFloat(bool bFixed = true, bool bScientific = true, bool bHex = true) const
	{
		return TStringView<FElementType>(*this).template ToFloat<U>(bFixed, bScientific, bHex);
	}

	/** Parse the string into a boolean value. */
	NODISCARD FORCEINLINE constexpr bool Parse(bool& Value)
	{
		return TStringView<FElementType>(*this).Parse(Value);
	}

	/** Parse the string into an integer value. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr bool Parse(U& Value, uint Base = 0)
	{
		return TStringView<FElementType>(*this).Parse(Value, Base);
	}

	/** Parse the string into a floating-point value. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr bool Parse(U& Value, bool bFixed = true, bool bScientific = true, bool bHex = true)
	{
		return TStringView<FElementType>(*this).Parse(Value, bFixed, bScientific, bHex);
	}

public:

	/**
	 * Converts a boolean value into a string.
	 *
	 * - true  becomes "True".
	 * - false becomes "False".
	 *
	 * @return The string containing the boolean value.
	 */
	NODISCARD static FORCEINLINE TString FromBool(bool Value)
	{
		TString Result;

		Result.AppendBool(Value);

		return Result;
	}

	/**
	 * Converts an integer value into a string.
	 *
	 * @param Base - The base of the number, between [2, 36].
	 *
	 * @return The string containing the integer value.
	 */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD static FORCEINLINE TString FromInt(U Value, uint Base = 10)
	{
		checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

		TString Result;

		Result.AppendInt(Value, Base);

		return Result;
	}

	/**
	 * Converts a floating-point value into a string.
	 * The string is formatted using the shortest representation in fixed-point or scientific notation.
	 *
	 * @return The string containing the floating-point value.
	 */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD static FORCEINLINE TString FromFloat(U Value)
	{
		TString Result;

		Result.AppendFloat(Value);

		return Result;
	}

	/**
	 * Converts a floating-point value into a string.
	 * The string is formatted using the shortest representation in fixed-point or scientific notation.
	 * The string is formatted using the hex representation if bFixed and bScientific are false.
	 *
	 * @param bFixed      - The fixed-point format.
	 * @param bScientific - The scientific notation.
	 *
	 * @return The string containing the floating-point value.
	 */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD static FORCEINLINE TString FromFloat(U Value, bool bFixed, bool bScientific)
	{
		TString Result;

		Result.AppendFloat(Value, bFixed, bScientific);

		return Result;
	}

	/**
	 * Converts a floating-point value into a string.
	 * The string is formatted using the shortest representation in fixed-point or scientific notation.
	 * The string is formatted using the hex representation if bFixed and bScientific are false.
	 *
	 * @param bFixed      - The fixed-point format.
	 * @param bScientific - The scientific notation.
	 * @param Precision   - The number of digits after the decimal point.
	 * @return
	 */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD static FORCEINLINE TString FromFloat(U Value, bool bFixed, bool bScientific, uint Precision)
	{
		TString Result;

		Result.AppendFloat(Value, bFixed, bScientific, Precision);

		return Result;
	}

	/** Converts a boolean value into a string and appends it to the string. */
	FORCEINLINE void AppendBool(bool Value)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0}"), Value);
	}

	/** Converts an integer value into a string and appends it to the string. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	FORCEINLINE void AppendInt(U Value, uint Base = 10)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:_{1}I}"), Value, Base);
	}

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	FORCEINLINE void AppendFloat(U Value)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0}"), Value);
	}

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	FORCEINLINE void AppendFloat(U Value, bool bFixed, bool bScientific)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		if (bFixed && bScientific)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:G}"), Value);
		}

		else if (bFixed)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:F}"), Value);
		}

		else if (bScientific)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:E}"), Value);
		}

		else
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:A}"), Value);
		}
	}

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	FORCEINLINE void AppendFloat(U Value, bool bFixed, bool bScientific, uint Precision)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		if (bFixed && bScientific)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:.{1}G}"), Value, Precision);
		}

		else if (bFixed)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:.{1}F}"), Value, Precision);
		}

		else if (bScientific)
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:.{1}E}"), Value, Precision);
		}

		else
		{
			Algorithms::Format(Inserter, LITERAL_VIEW(FElementType, "{0:.{1}A}"), Value, Precision);
		}
	}

public:

	/**
	 * Format some objects using a format string.
	 *
	 * @param Fmt  - The format string.
	 * @param Args - The objects to format.
	 *
	 * @return The formatted string containing the objects.
	 */
	template <typename... Ts>
	NODISCARD static FORCEINLINE TString Format(TStringView<FElementType> Fmt, const Ts&... Args)
	{
		TString Result;

		Result.AppendFormat(Fmt, Args...);

		return Result;
	}

	/** Format some objects using a format string and append to the string. */
	template <typename... Ts>
	FORCEINLINE void AppendFormat(TStringView<FElementType> Fmt, const Ts&... Args)
	{
		auto Inserter = Ranges::View(MakeBackInserter(*this), UnreachableSentinel);

		Algorithms::Format(Inserter, Fmt, Args...);
	}

public:

	/** Overloads the GetTypeHash algorithm for TString. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TString& A) { return GetTypeHash(TStringView<FElementType>(A)); }

	/** Overloads the Swap algorithm for TString. */
	friend FORCEINLINE void Swap(TString& A, TString& B) { Swap(static_cast<FSuper&>(A), static_cast<FSuper&>(B)); }

};

template<typename T>
TString(const T*) -> TString<T>;

template<typename T>
TString(TStringView<T>) -> TString<T>;

template<typename I, typename S>
TString(I, S) -> TString<TIteratorElement<I>>;

template<typename R>
TString(R) -> TString<TRangeElement<R>>;

template <typename T>
TString(initializer_list<T>) -> TString<T>;

using FString        = TString<char>;
using FWString       = TString<wchar>;
using FU8String      = TString<u8char>;
using FU16String     = TString<u16char>;
using FU32String     = TString<u32char>;
using FUnicodeString = TString<unicodechar>;

template <CCharType T> template <typename Allocator> constexpr TStringView<T>::TStringView(const TString<FElementType, Allocator>& InString)
	: TStringView(InString.GetData(), InString.Num()) { }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
