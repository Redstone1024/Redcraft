#pragma once

#include "CoreTypes.h"
#include "String/Char.h"
#include "Containers/Array.h"
#include "String/StringView.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Memory/ObserverPointer.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CCharType T>
using TDefaultStringAllocator = TInlineAllocator<(40 - 3 * sizeof(size_t)) / sizeof(T)>;

template <CCharType T, CAllocator<T> Allocator = TDefaultStringAllocator<T>>
class TString final
{
public:

	using ElementType   = typename TArray<T, Allocator>::ElementType;
	using AllocatorType = typename TArray<T, Allocator>::AllocatorType;

	using      Reference = typename TArray<T, Allocator>::     Reference;
	using ConstReference = typename TArray<T, Allocator>::ConstReference;

	using      Iterator = typename TArray<T, Allocator>::     Iterator;
	using ConstIterator = typename TArray<T, Allocator>::ConstIterator;

	using      ReverseIterator = typename TArray<T, Allocator>::     ReverseIterator;
	using ConstReverseIterator = typename TArray<T, Allocator>::ConstReverseIterator;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Default constructor. Constructs an empty string. */
	FORCEINLINE TString() : NativeData({ LITERAL(ElementType, '\0') }) { }

	/** Constructs the string with 'Count' copies of characters with 'InValue'. */
	FORCEINLINE TString(size_t Count, ElementType InChar) : TString(MakeCountedConstantIterator(InChar, Count), DefaultSentinel) { }

	/** Constructs a string with the contents of the range ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE TString(const ElementType* InPtr, size_t Count) : TString(TStringView<ElementType>(InPtr, Count))
	{
		checkf(InPtr != nullptr, TEXT("TString cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE TString(nullptr_t, size_t) = delete;

	/** Constructs a string with the contents of the range ['InPtr', '\0'). */
	FORCEINLINE TString(const ElementType* InPtr) : TString(TStringView<ElementType>(InPtr))
	{
		checkf(InPtr != nullptr, TEXT("TString cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE TString(nullptr_t) = delete;

	/** Constructs the string with the contents of the 'View'. */
	FORCEINLINE TString(TStringView<ElementType> View) : TString(View.Begin(), View.End()) { }

	/** Constructs the string with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	TString(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			if constexpr (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t Count = Iteration::Distance(First, Last);

			NativeData.SetNum(Count + 1);

			for (size_t Index = 0; Index != Count; ++Index)
			{
				NativeData[Index] = ElementType(*First++);
			}

			NativeData.Back() = LITERAL(ElementType, '\0');
		}
		else
		{
			while (First != Last)
			{
				NativeData.PushBack(ElementType(*First));
				++First;
			}

			NativeData.PushBack(LITERAL(ElementType, '\0'));
		}
	}

	/** Copy constructor. Constructs the string with the copy of the contents of 'InValue'. */
	FORCEINLINE TString(const TString&) = default;

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString(TString&& InValue) : NativeData(MoveTemp(InValue.NativeData)) { InValue.NativeData.PushBack(LITERAL(ElementType, '\0')); }

	/** Constructs the string with the contents of the initializer list. */
	FORCEINLINE TString(initializer_list<ElementType> IL) : TString(Iteration::Begin(IL), Iteration::End(IL)) { }

	/** Destructs the string. The destructors of the characters are called and the used storage is deallocated. */
	FORCEINLINE ~TString() = default;

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	FORCEINLINE TString& operator=(const TString&) = default;

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString& operator=(TString&& InValue) { NativeData = MoveTemp(InValue.NativeData); InValue.NativeData.PushBack(LITERAL(ElementType, '\0')); return *this; }

	/** Compares the contents of two strings. */
	FORCEINLINE NODISCARD friend bool operator==(const TString& LHS, const TString& RHS) { return TStringView<ElementType>(LHS) == TStringView<ElementType>(RHS); }

	/** Compares the contents of a string and a character. */
	FORCEINLINE NODISCARD friend bool operator==(const TString& LHS,       ElementType  RHS) { return TStringView<ElementType>(LHS) == RHS; }
	FORCEINLINE NODISCARD friend bool operator==(const TString& LHS, const ElementType* RHS) { return TStringView<ElementType>(LHS) == RHS; }
	FORCEINLINE NODISCARD friend bool operator==(      ElementType  LHS, const TString& RHS) { return LHS == TStringView<ElementType>(RHS); }
	FORCEINLINE NODISCARD friend bool operator==(const ElementType* LHS, const TString& RHS) { return LHS == TStringView<ElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	FORCEINLINE NODISCARD friend auto operator<=>(const TString& LHS, const TString& RHS) { return TStringView<ElementType>(LHS) <=> TStringView<ElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	FORCEINLINE NODISCARD friend auto operator<=>(const TString& LHS,       ElementType  RHS) { return TStringView<ElementType>(LHS) <=> RHS; }
	FORCEINLINE NODISCARD friend auto operator<=>(const TString& LHS, const ElementType* RHS) { return TStringView<ElementType>(LHS) <=> RHS; }
	FORCEINLINE NODISCARD friend auto operator<=>(      ElementType  LHS, const TString& RHS) { return LHS <=> TStringView<ElementType>(RHS); }
	FORCEINLINE NODISCARD friend auto operator<=>(const ElementType* LHS, const TString& RHS) { return LHS <=> TStringView<ElementType>(RHS); }

	/** Inserts 'InValue' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, ElementType InValue)
	{
		checkf(Index <= Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(Begin() + Index, InValue);
	}

	/** Inserts 'InValue' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, ElementType InValue)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.Insert(Iter, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, size_t Count, ElementType InValue)
	{
		checkf(Index <= Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(Begin() + Index, Count, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, size_t Count, ElementType InValue)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.Insert(Iter, Count, InValue);
	}

	/** Inserts characters from the 'View' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, TStringView<ElementType> View)
	{
		checkf(Index <= Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(Begin() + Index, View);
	}

	/** Inserts characters from the 'View' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, TStringView<ElementType> View)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Insert(Iter, View.Begin(), View.End());
	}

	/** Inserts characters from the range ['First', 'Last') before 'Index' in the string. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE Iterator Insert(size_t Index, I First, S Last)
	{
		checkf(Index <= Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(Begin() + Index, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE Iterator Insert(ConstIterator Iter, I First, S Last)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.Insert(Iter, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the initializer list before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, initializer_list<ElementType> IL)
	{
		checkf(Index <= Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(Begin() + Index, IL);
	}

	/** Inserts characters from the initializer list before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, initializer_list<ElementType> IL)
	{
		checkf(IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.Insert(Iter, IL);
	}

	/** Erases the character at 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(size_t Index, bool bAllowShrinking = true)
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index < Num()."));

		return Erase(Begin() + Index, bAllowShrinking);
	}

	/** Erases the character at 'Iter' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(ConstIterator Iter, bool bAllowShrinking = true)
	{
		checkf(IsValidIterator(Iter) && Iter != End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.StableErase(Iter, bAllowShrinking);
	}

	/** Erases 'CountToErase' characters starting from 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(size_t Index, size_t CountToErase, bool bAllowShrinking = true)
	{
		checkf(Index <= Num() && Index + CountToErase <= Num(), TEXT("Illegal substring range. Please check Index and CountToErase."));

		return Erase(Begin() + Index, Begin() + Index + CountToErase, bAllowShrinking);

	}

	/** Erases the characters in the range ['First', 'Last') in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(ConstIterator First, ConstIterator Last, bool bAllowShrinking = true)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return NativeData.StableErase(First, Last, bAllowShrinking);
	}

	/** Appends the given character value to the end of the string. */
	FORCEINLINE void PushBack(ElementType InValue) { NativeData.Back() = InValue; NativeData.PushBack(LITERAL(ElementType, '\0')); }

	/** Removes the last character of the string. The string cannot be empty. */
	FORCEINLINE void PopBack(bool bAllowShrinking = true) { NativeData.PopBack(bAllowShrinking); NativeData.Back() = LITERAL(ElementType, '\0'); }

	/** Appends 'Count' copies of the 'InValue' to the end of the string. */
	TString& Append(size_t Count, ElementType InChar) { return Append(MakeCountedConstantIterator(InChar, Count), DefaultSentinel); }

	/** Appends the contents of the 'View' to the end of the string. */
	FORCEINLINE TString& Append(TStringView<ElementType> View) { return Append(View.Begin(), View.End()); }

	/** Appends the contents of the range ['First', 'Last') to the end of the string. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	TString& Append(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			if constexpr (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t Count = Iteration::Distance(First, Last);

			const size_t CurrentNum = Num();

			NativeData.SetNum(CurrentNum + Count + 1);

			for (size_t Index = CurrentNum; Index != CurrentNum + Count; ++Index)
			{
				NativeData[Index] = ElementType(*First++);
			}

			NativeData.Back() = LITERAL(ElementType, '\0');
		}
		else
		{
			NativeData.Insert(NativeData.End() - 1, MoveTemp(First), MoveTemp(Last));

			NativeData.PushBack(LITERAL(ElementType, '\0'));
		}

		return *this;
	}

	/** Appends the contents of the initializer list to the end of the string. */
	FORCEINLINE TString& Append(initializer_list<ElementType> IL) { return Append(Iteration::Begin(IL), Iteration::End(IL)); }

	/** Appends the given character value to the end of the string. */
	FORCEINLINE TString& operator+=(ElementType InChar) { return Append(1, InChar); }

	/** Appends the contents of the 'View' to the end of the string. */
	FORCEINLINE TString& operator+=(TStringView<ElementType> View) { return Append(View); }

	/** Appends the contents of the range ['First', 'Last') to the end of the string. */
	FORCEINLINE TString& operator+=(initializer_list<ElementType> IL) { return Append(IL); }

	/** Concatenates two strings. */
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS, const TString& RHS) { return TString(LHS).Append(RHS); }

	/** Concatenates the string with the given character value. */
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS,             ElementType  RHS) { return TString(LHS).Append(1, RHS); }
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS,       const ElementType* RHS) { return TString(LHS).Append(   RHS); }
	NODISCARD friend FORCEINLINE TString operator+(const TString& LHS, TStringView<ElementType> RHS) { return TString(LHS).Append(   RHS); }
	NODISCARD friend FORCEINLINE TString operator+(            ElementType  LHS, const TString& RHS) { return TString(1, LHS).Append(RHS); }
	NODISCARD friend FORCEINLINE TString operator+(      const ElementType* LHS, const TString& RHS) { return TString(   LHS).Append(RHS); }
	NODISCARD friend FORCEINLINE TString operator+(TStringView<ElementType> LHS, const TString& RHS) { return TString(   LHS).Append(RHS); }

	/** Concatenates two strings. The rvalue maybe modified. */
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS, TString&& RHS) { LHS.Append(MoveTemp(RHS)); return LHS; }

	/** Concatenates two strings. The rvalue maybe modified. */
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,               ElementType   RHS) { LHS.Append(1, RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,         const ElementType*  RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS,   TStringView<ElementType>  RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(TString&& LHS, const TString<ElementType>& RHS) { LHS.Append(   RHS); return LHS; }
	NODISCARD friend FORCEINLINE TString operator+(              ElementType   LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(        const ElementType*  LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(  TStringView<ElementType>  LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }
	NODISCARD friend FORCEINLINE TString operator+(const TString<ElementType>& LHS, TString&& RHS) { RHS.Insert(0, LHS); return RHS; }

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(TStringView<ElementType> Prefix) const
	{
		return TStringView<ElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(ElementType Prefix) const
	{
		return TStringView<ElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(TStringView<ElementType> Suffix) const
	{
		return TStringView<ElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(ElementType Suffix) const
	{
		return TStringView<ElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view contains the given substring, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(TStringView<ElementType> View) const
	{
		return TStringView<ElementType>(*this).Contains(View);
	}

	/** @return true if the string view contains the given character, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(ElementType Char) const
	{
		return TStringView<ElementType>(*this).Contains(Char);
	}

	/** @return true if the string view contains character that satisfy the given predicate, false otherwise. */
	template <CPredicate<ElementType> F>
	NODISCARD FORCEINLINE constexpr bool Contains(F&& InPredicate) const
	{
		return TStringView<ElementType>(*this).Contains(Forward<F>(InPredicate));
	}

	/** Replace the substring [Index, Index + CountToReplace) with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, size_t Count, ElementType InChar)
	{
		checkf(Index <= Num() && Index + CountToReplace <= Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(Begin() + Index, Begin() + Index + CountToReplace, Count, InChar);
	}

	/** Replace the substring ['First', 'Last') with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, size_t Count, ElementType InChar)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, MakeCountedConstantIterator(InChar, Count), DefaultSentinel);
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the 'View'. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, TStringView<ElementType> View)
	{
		checkf(Index <= Num() && Index + CountToReplace <= Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(Begin() + Index, Begin() + Index + CountToReplace, View);
	}

	/** Replace the substring ['First', 'Last') with the contents of the 'View'. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, TStringView<ElementType> View)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, View.Begin(), View.End());
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, I InString, S Sentinel)
	{
		checkf(Index <= Num() && Index + CountToReplace <= Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(Begin() + Index, Begin() + Index + CountToReplace, MoveTemp(InString), MoveTemp(Sentinel));
	}

	/** Replace the substring ['First', 'Last') with the contents of the range ['InString', 'Sentinel'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	TString& Replace(ConstIterator First, ConstIterator Last, I InString, S Sentinel)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t InsertIndex = First - Begin();

			const size_t RemoveCount = Iteration::Distance(First, Last);
			const size_t InsertCount = Iteration::Distance(InString, Sentinel);

			const size_t NumToReset = Num() - RemoveCount + InsertCount;

			if (InsertCount < RemoveCount)
			{
				for (size_t Index = InsertIndex; Index != InsertIndex + InsertCount; ++Index)
				{
					NativeData[Index] = ElementType(*InString++);
				}

				for (size_t Index = InsertIndex + InsertCount; Index != NumToReset; ++Index)
				{
					NativeData[Index] = NativeData[Index + (RemoveCount - InsertCount)];
				}

				NativeData.SetNum(NumToReset + 1);
			}
			else
			{
				NativeData.SetNum(NumToReset + 1);

				for (size_t Index = Num(); Index != InsertIndex + InsertCount - 1; --Index)
				{
					NativeData[Index - 1] = NativeData[Index + (RemoveCount - InsertCount) - 1];
				}

				for (size_t Index = InsertIndex; Index != InsertIndex + InsertCount; ++Index)
				{
					NativeData[Index] = ElementType(*InString++);
				}
			}

			NativeData.Back() = LITERAL(ElementType, '\0');
		}
		else
		{
			TString Temp(MoveTemp(First), MoveTemp(Last));
			return Replace(First, Last, Temp.Begin(), Temp.End());
		}

		return *this;
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the initializer list. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, initializer_list<ElementType> IL)
	{
		checkf(Index <= Num() && Index + CountToReplace <= Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(Begin() + Index, Begin() + Index + CountToReplace, IL);
	}

	/** Replace the substring ['First', 'Last') with the contents of the initializer list. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, initializer_list<ElementType> IL)
	{
		checkf(IsValidIterator(First) && IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, Iteration::Begin(IL), Iteration::End(IL));
	}

	/** Obtains a string that is a view over the 'Count' characters of this string view starting at 'Offset'.  */
	FORCEINLINE TString Substr(size_t Offset = 0, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= Num() && Offset + Count <= Num(), TEXT("Illegal substring range. Please check Offset and Count."));

		return TStringView<ElementType>(*this).Substr(Offset, Count);
	}

	/** Copies the characters of this string to the destination buffer without null-termination. */
	FORCEINLINE constexpr size_t Copy(ElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
	{
		checkf(Dest != nullptr, TEXT("Illegal destination buffer. Please check the pointer."));

		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		return TStringView<ElementType>(*this).Copy(Dest, Count, Offset);
	}

	FORCEINLINE constexpr size_t Copy(nullptr_t, size_t Count = DynamicExtent, size_t Offset = 0) const = delete;

	/** @return The index of the first occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t Find(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t Find(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(Char, Index);
	}

	/** @return The index of the first occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD constexpr size_t Find(F&& InPredicate, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the last occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t RFind(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t RFind(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(Char, Index);
	}

	/** @return The index of the last occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD constexpr size_t RFind(F&& InPredicate, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the first occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstOf(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstOf(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastOf(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastOf(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstNotOf(View, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstNotOf(Char, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastNotOf(View, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastNotOf(Char, Index);
	}

	/** Resizes the string to contain 'Count' characters. Additional null characters are appended. */
	FORCEINLINE void SetNum(size_t Count, bool bAllowShrinking = true) { SetNum(Count, LITERAL(ElementType, '\0'), bAllowShrinking); }

	/** Resizes the string to contain 'Count' characters. Additional copies of 'InValue' are appended. */
	FORCEINLINE void SetNum(size_t Count, ElementType InValue, bool bAllowShrinking = true) { NativeData.SetNum(Count + 1, InValue, bAllowShrinking); NativeData.Back() = LITERAL(ElementType, '\0'); }

	/** Increase the max capacity of the string to a value that's greater or equal to 'Count'. */
	FORCEINLINE void Reserve(size_t Count) { NativeData.Reserve(Count + 1); }

	/** Requests the removal of unused capacity. */
	FORCEINLINE void Shrink() { NativeData.Shrink(); }

	/** @return The pointer to the underlying character storage. */
	NODISCARD FORCEINLINE TObserverPtr<      ElementType[]> GetData()       { return NativeData.GetData(); }
	NODISCARD FORCEINLINE TObserverPtr<const ElementType[]> GetData() const { return NativeData.GetData(); }

	/** @return The non-modifiable standard C character string version of the string. */
	NODISCARD FORCEINLINE const ElementType* ToCString() const { return NativeData.GetData().Get(); }

	/** @return The iterator to the first or end character. */
	NODISCARD FORCEINLINE      Iterator Begin()       { return NativeData.Begin(); }
	NODISCARD FORCEINLINE ConstIterator Begin() const { return NativeData.Begin(); }
	NODISCARD FORCEINLINE      Iterator End()         { return --NativeData.End(); }
	NODISCARD FORCEINLINE ConstIterator End()   const { return --NativeData.End(); }

	/** @return The reverse iterator to the first or end character. */
	NODISCARD FORCEINLINE      ReverseIterator RBegin()       { return ++NativeData.RBegin(); }
	NODISCARD FORCEINLINE ConstReverseIterator RBegin() const { return ++NativeData.RBegin(); }
	NODISCARD FORCEINLINE      ReverseIterator REnd()         { return NativeData.REnd();     }
	NODISCARD FORCEINLINE ConstReverseIterator REnd()   const { return NativeData.REnd();     }

	/** @return The number of characters in the string. */
	NODISCARD FORCEINLINE size_t Num() const { return NativeData.Num() - 1; }

	/** @return The number of characters that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t Max() const { return NativeData.Max() - 1; }

	/** @return true if the string is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested character. */
	NODISCARD FORCEINLINE       ElementType& operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return NativeData[Index]; }
	NODISCARD FORCEINLINE const ElementType& operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return NativeData[Index]; }

	/** @return The reference to the first or last character. */
	NODISCARD FORCEINLINE       ElementType& Front()       { return *Begin();     }
	NODISCARD FORCEINLINE const ElementType& Front() const { return *Begin();     }
	NODISCARD FORCEINLINE       ElementType& Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE const ElementType& Back()  const { return *(End() - 1); }

	/** Erases all characters from the string. After this call, Num() returns zero. */
	FORCEINLINE void Reset(bool bAllowShrinking = true)
	{
		NativeData.Reset(bAllowShrinking);
		NativeData.PushBack(LITERAL(ElementType, '\0'));
	}

	/** Overloads the GetTypeHash algorithm for TString. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TString& A) { return GetTypeHash(A.NativeData); }

	/** Overloads the Swap algorithm for TString. */
	friend FORCEINLINE void Swap(TString& A, TString& B) { Swap(A.NativeData, B.NativeData); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	TArray<T, Allocator> NativeData;

};

template<typename T>
TString(size_t, T) -> TString<T>;

template<typename T>
TString(const T*) -> TString<T>;

template<typename T>
TString(TStringView<T>) -> TString<T>;

template<typename I, typename S>
TString(I, S) -> TString<TIteratorElementType<I>>;

template <typename T>
TString(initializer_list<T>) -> TString<T>;

using FString        = TString<char>;
using FWString       = TString<wchar>;
using FU8String      = TString<u8char>;
using FU16String     = TString<u16char>;
using FU32String     = TString<u32char>;
using FUnicodeString = TString<unicodechar>;

template <CCharType T> template <typename Allocator> constexpr TStringView<T>::TStringView(const TString<ElementType, Allocator>& InString)
	: TStringView(InString.GetData().Get(), InString.Num()) { }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
