#pragma once

#include "CoreTypes.h"
#include "String/Char.h"
#include "Containers/Array.h"
#include "String/StringView.h"
#include "Templates/Utility.h"
#include "Templates/Optional.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Noncopyable.h"
#include "Miscellaneous/Iterator.h"
#include "Miscellaneous/Container.h"
#include "Miscellaneous/AssertionMacros.h"

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

	using Super = TArray<T, Allocator>;

public:

	using ElementType   = typename Super::ElementType;
	using AllocatorType = typename Super::AllocatorType;

	using      Reference = typename Super::     Reference;
	using ConstReference = typename Super::ConstReference;

	using      Iterator = typename Super::     Iterator;
	using ConstIterator = typename Super::ConstIterator;

	using      ReverseIterator = typename Super::     ReverseIterator;
	using ConstReverseIterator = typename Super::ConstReverseIterator;

	static_assert(CContiguousIterator<     Iterator>);
	static_assert(CContiguousIterator<ConstIterator>);

	/** Default constructor. Constructs an empty string. */
	FORCEINLINE TString() = default;

	/** Constructs the string with 'Count' copies of characters with 'InValue'. */
	FORCEINLINE TString(size_t Count, ElementType InChar) : Super(Count, InChar) { }

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
	FORCEINLINE TString(I First, S Last) : Super(MoveTemp(First), MoveTemp(Last)) { }

	/** Copy constructor. Constructs the string with the copy of the contents of 'InValue'. */
	FORCEINLINE TString(const TString&) = default;

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString(TString&&) = default;

	/** Constructs the string with the contents of the initializer list. */
	FORCEINLINE TString(initializer_list<ElementType> IL) : TString(Iteration::Begin(IL), Iteration::End(IL)) { }

	/** Copy assignment operator. Replaces the contents with a copy of the contents of 'InValue'. */
	FORCEINLINE TString& operator=(const TString&) = default;

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE TString& operator=(TString&&) = default;

	/** Compares the contents of two strings. */
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS, const TString& RHS) { return TStringView<ElementType>(LHS) == TStringView<ElementType>(RHS); }

	/** Compares the contents of a string and a character. */
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS,       ElementType  RHS) { return TStringView<ElementType>(LHS) == RHS; }
	NODISCARD friend FORCEINLINE bool operator==(const TString& LHS, const ElementType* RHS) { return TStringView<ElementType>(LHS) == RHS; }
	NODISCARD friend FORCEINLINE bool operator==(      ElementType  LHS, const TString& RHS) { return LHS == TStringView<ElementType>(RHS); }
	NODISCARD friend FORCEINLINE bool operator==(const ElementType* LHS, const TString& RHS) { return LHS == TStringView<ElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS, const TString& RHS) { return TStringView<ElementType>(LHS) <=> TStringView<ElementType>(RHS); }

	/** Compares the contents of 'LHS' and 'RHS' lexicographically. */
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS,       ElementType  RHS) { return TStringView<ElementType>(LHS) <=> RHS; }
	NODISCARD friend FORCEINLINE auto operator<=>(const TString& LHS, const ElementType* RHS) { return TStringView<ElementType>(LHS) <=> RHS; }
	NODISCARD friend FORCEINLINE auto operator<=>(      ElementType  LHS, const TString& RHS) { return LHS <=> TStringView<ElementType>(RHS); }
	NODISCARD friend FORCEINLINE auto operator<=>(const ElementType* LHS, const TString& RHS) { return LHS <=> TStringView<ElementType>(RHS); }

public:

	/** Inserts 'InValue' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, ElementType InValue)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, InValue);
	}

	/** Inserts 'InValue' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, ElementType InValue)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::Insert(Iter, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, size_t Count, ElementType InValue)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, Count, InValue);
	}

	/** Inserts 'Count' copies of the 'InValue' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, size_t Count, ElementType InValue)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::Insert(Iter, Count, InValue);
	}

	/** Inserts characters from the 'View' before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, TStringView<ElementType> View)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, View);
	}

	/** Inserts characters from the 'View' before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, TStringView<ElementType> View)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Insert(Iter, View.Begin(), View.End());
	}

	/** Inserts characters from the range ['First', 'Last') before 'Index' in the string. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE Iterator Insert(size_t Index, I First, S Last)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the range ['First', 'Last') before 'Iter'. */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE Iterator Insert(ConstIterator Iter, I First, S Last)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::Insert(Iter, MoveTemp(First), MoveTemp(Last));
	}

	/** Inserts characters from the initializer list before 'Index' in the string. */
	FORCEINLINE Iterator Insert(size_t Index, initializer_list<ElementType> IL)
	{
		checkf(Index <= this->Num(), TEXT("Illegal index. Please check Index <= Num()."));

		return Insert(this->Begin() + Index, IL);
	}

	/** Inserts characters from the initializer list before 'Iter' in the string. */
	FORCEINLINE Iterator Insert(ConstIterator Iter, initializer_list<ElementType> IL)
	{
		checkf(this->IsValidIterator(Iter), TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::Insert(Iter, IL);
	}

	/** Erases the character at 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(size_t Index, bool bAllowShrinking = true)
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index < Num()."));

		return Erase(this->Begin() + Index, bAllowShrinking);
	}

	/** Erases the character at 'Iter' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(ConstIterator Iter, bool bAllowShrinking = true)
	{
		checkf(this->IsValidIterator(Iter) && Iter != this->End(), TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::StableErase(Iter, bAllowShrinking);
	}

	/** Erases 'CountToErase' characters starting from 'Index' in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(size_t Index, size_t CountToErase, bool bAllowShrinking = true)
	{
		checkf(Index <= this->Num() && Index + CountToErase <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToErase."));

		return Erase(this->Begin() + Index, this->Begin() + Index + CountToErase, bAllowShrinking);

	}

	/** Erases the characters in the range ['First', 'Last') in the string. But it may change the order of characters. */
	FORCEINLINE Iterator Erase(ConstIterator First, ConstIterator Last, bool bAllowShrinking = true)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Super::StableErase(First, Last, bAllowShrinking);
	}

	/** Here, the 'Erase' is already stable and there is no need to provide 'StableErase'. */
	void StableErase(...) = delete;

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

			const size_t CurrentNum = this->Num();

			this->SetNum(CurrentNum + Count);

			for (size_t Index = CurrentNum; Index != CurrentNum + Count; ++Index)
			{
				(*this)[Index] = ElementType(*First++);
			}
		}
		else
		{
			Insert(this->End(), MoveTemp(First), MoveTemp(Last));
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
		auto Index = Find([](ElementType Char) { return !TChar<ElementType>::IsSpace(Char); });

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
		auto Index = RFind([](ElementType Char) { return !TChar<ElementType>::IsSpace(Char); });

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
		auto Index = Find(LITERAL(ElementType, '\0'));

		if (Index != INDEX_NONE)
		{
			this->SetNum(Index, bAllowShrinking);
		}

		return *this;
	}

public:

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE bool StartsWith(TStringView<ElementType> Prefix) const
	{
		return TStringView<ElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE bool StartsWith(ElementType Prefix) const
	{
		return TStringView<ElementType>(*this).StartsWith(Prefix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE bool EndsWith(TStringView<ElementType> Suffix) const
	{
		return TStringView<ElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE bool EndsWith(ElementType Suffix) const
	{
		return TStringView<ElementType>(*this).EndsWith(Suffix);
	}

	/** @return true if the string view contains the given substring, false otherwise. */
	NODISCARD FORCEINLINE bool Contains(TStringView<ElementType> View) const
	{
		return TStringView<ElementType>(*this).Contains(View);
	}

	/** @return true if the string view contains the given character, false otherwise. */
	NODISCARD FORCEINLINE bool Contains(ElementType Char) const
	{
		return TStringView<ElementType>(*this).Contains(Char);
	}

	/** @return true if the string view contains character that satisfy the given predicate, false otherwise. */
	template <CPredicate<ElementType> F>
	NODISCARD FORCEINLINE bool Contains(F&& InPredicate) const
	{
		return TStringView<ElementType>(*this).Contains(Forward<F>(InPredicate));
	}

	/** Replace the substring [Index, Index + CountToReplace) with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, size_t Count, ElementType InChar)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, Count, InChar);
	}

	/** Replace the substring ['First', 'Last') with 'Count' copies of the 'InChar'. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, size_t Count, ElementType InChar)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, MakeCountedConstantIterator(InChar, Count), DefaultSentinel);
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the 'View'. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, TStringView<ElementType> View)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, View);
	}

	/** Replace the substring ['First', 'Last') with the contents of the 'View'. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, TStringView<ElementType> View)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, View.Begin(), View.End());
	}

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, I InString, S Sentinel)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, MoveTemp(InString), MoveTemp(Sentinel));
	}

	/** Replace the substring ['First', 'Last') with the contents of the range ['InString', 'Sentinel'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<ElementType, TIteratorReferenceType<I>>)
	TString& Replace(ConstIterator First, ConstIterator Last, I InString, S Sentinel)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t InsertIndex = First - this->Begin();

			const size_t RemoveCount = Iteration::Distance(First, Last);
			const size_t InsertCount = Iteration::Distance(InString, Sentinel);

			const size_t NumToReset = this->Num() - RemoveCount + InsertCount;

			if (InsertCount < RemoveCount)
			{
				for (size_t Index = InsertIndex; Index != InsertIndex + InsertCount; ++Index)
				{
					(*this)[Index] = ElementType(*InString++);
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
					(*this)[Index] = ElementType(*InString++);
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

	/** Replace the substring [Index, Index + CountToReplace) with the contents of the initializer list. */
	FORCEINLINE TString& Replace(size_t Index, size_t CountToReplace, initializer_list<ElementType> IL)
	{
		checkf(Index <= this->Num() && Index + CountToReplace <= this->Num(), TEXT("Illegal substring range. Please check Index and CountToReplace."));

		return Replace(this->Begin() + Index, this->Begin() + Index + CountToReplace, IL);
	}

	/** Replace the substring ['First', 'Last') with the contents of the initializer list. */
	FORCEINLINE TString& Replace(ConstIterator First, ConstIterator Last, initializer_list<ElementType> IL)
	{
		checkf(this->IsValidIterator(First) && this->IsValidIterator(Last) && First <= Last, TEXT("Read access violation. Please check IsValidIterator()."));

		return Replace(First, Last, Iteration::Begin(IL), Iteration::End(IL));
	}

	/** Obtains a string that is a view over the 'Count' characters of this string view starting at 'Offset'.  */
	FORCEINLINE TString Substr(size_t Offset = 0, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= this->Num() && Offset + Count <= this->Num(), TEXT("Illegal substring range. Please check Offset and Count."));

		return TStringView<ElementType>(*this).Substr(Offset, Count);
	}

	/** Copies the characters of this string to the destination buffer without null-termination. */
	FORCEINLINE size_t Copy(ElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
	{
		checkf(Dest != nullptr, TEXT("Illegal destination buffer. Please check the pointer."));

		checkf(Offset <= this->Num() && (Count == DynamicExtent || Offset + Count <= this->Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		return TStringView<ElementType>(*this).Copy(Dest, Count, Offset);
	}

	FORCEINLINE size_t Copy(nullptr_t, size_t = DynamicExtent, size_t = 0) const = delete;

	/** @return The index of the first occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD size_t Find(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD size_t Find(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(Char, Index);
	}

	/** @return The index of the first occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD size_t Find(F&& InPredicate, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).Find(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the last occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD size_t RFind(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD size_t RFind(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(Char, Index);
	}

	/** @return The index of the last occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD size_t RFind(F&& InPredicate, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).RFind(Forward<F>(InPredicate), Index);
	}

	/** @return The index of the first occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstOf(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstOf(View, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstOf(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastOf(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastOf(View, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastOf(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstNotOf(TStringView<ElementType> View, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstNotOf(View, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindFirstNotOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindFirstNotOf(Char, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastNotOf(TStringView<ElementType> View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastNotOf(View, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE size_t FindLastNotOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < this->Num(), TEXT("Illegal index. Please check Index."));

		return TStringView<ElementType>(*this).FindLastNotOf(Char, Index);
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
					const auto Result = Facet.in(State, BeginFrom, EndFrom, NextFrom, Iteration::Begin(Buffer), Iteration::End(Buffer), NextTo);

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
					const auto Result = Facet.out(State, BeginFrom, EndFrom, NextFrom, Iteration::Begin(Buffer), Iteration::End(Buffer), NextTo);

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
			const_cast<ElementType*>(this->GetData())[this->Num()] = LITERAL(ElementType, '\0');

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
		return TStringView<ElementType>(*this).IsValid();
	}

	/** @return true if the string only contains ASCII characters, false otherwise. */
	NODISCARD FORCEINLINE bool IsASCII() const
	{
		return TStringView<ElementType>(*this).IsASCII();
	}

	/** @return true if the string can be fully represented as a boolean value, false otherwise. */
	NODISCARD FORCEINLINE bool IsBoolean() const
	{
		return TStringView<ElementType>(*this).IsBoolean();
	}

	/** @return true if the string can be fully represented as an integer value, false otherwise. */
	NODISCARD FORCEINLINE bool IsInteger(unsigned Base = 10, bool bSigned = true) const
	{
		return TStringView<ElementType>(*this).IsInteger(Base, bSigned);
	}

	/** @return true if the string can be fully represented as a floating-point value, false otherwise. */
	NODISCARD FORCEINLINE bool IsFloatingPoint(bool bFixed = true, bool bScientific = true, bool bSigned = true) const
	{
		return TStringView<ElementType>(*this).IsFloatingPoint(bFixed, bScientific, bSigned);
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
	NODISCARD static FORCEINLINE TString FromInt(U Value, unsigned Base = 10)
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
	NODISCARD static FORCEINLINE TString FromFloat(U Value, bool bFixed, bool bScientific, unsigned Precision)
	{
		TString Result;

		Result.AppendFloat(Value, bFixed, bScientific, Precision);

		return Result;
	}

	/** Converts a boolean value into a string and appends it to the string. */
	void AppendBool(bool Value);

	/** Converts an integer value into a string and appends it to the string. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	void AppendInt(U Value, unsigned Base = 10);

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	void AppendFloat(U Value);

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	void AppendFloat(U Value, bool bFixed, bool bScientific);

	/** Converts a floating-point value into a string and appends it to the string. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	void AppendFloat(U Value, bool bFixed, bool bScientific, unsigned Precision);

	/**
	 * Converts a string into a boolean value.
	 *
	 * - "True"  and non-zero integers become true.
	 * - "False" and unparsable values become false.
	 *
	 * @return The boolean value.
	 */
	NODISCARD FORCEINLINE bool ToBool() const
	{
		return TStringView<ElementType>(*this).ToBool();
	}

	/**
	 * Converts a string into an integer value.
	 *
	 * - "0x" or "0X" prefixes are not recognized if base is 16.
	 * - Only the minus sign is recognized (not the plus sign), and only for signed integer types of value.
	 * - Leading whitespace is not ignored.
	 *
	 * Ensure that the entire string can be parsed if IsNumeric(Base, false, true, false) is true.
	 *
	 * @param Base - The base of the number, between [2, 36].
	 *
	 * @return The integer value.
	 */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE U ToInt(unsigned Base = 10) const
	{
		checkf(Base >= 2 && Base <= 36, TEXT("Illegal base. Please check the base."));

		return TStringView<ElementType>(*this).template ToInt<U>(Base);
	}

	/**
	 * Converts a string into a floating-point value.
	 *
	 * - "0x" or "0X" prefixes are not recognized if base is 16.
	 * - The plus sign is not recognized outside the exponent (only the minus sign is permitted at the beginning).
	 * - Leading whitespace is not ignored.
	 *
	 * Ensure that the entire string can be parsed if bFixed and IsNumeric(10, false) is true.
	 * Parsers hex floating-point values if bFixed and bScientific are false.
	 *
	 * @param bFixed      - The fixed-point format.
	 * @param bScientific - The scientific notation.
	 *
	 * @return The floating-point value.
	 */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE U ToFloat(bool bFixed = true, bool bScientific = false) const
	{
		return TStringView<ElementType>(*this).template ToFloat<U>(bFixed, bScientific);
	}

	/** Converts a string into a boolean value and remove the parsed substring. */
	NODISCARD FORCEINLINE bool ToBoolAndTrim()
	{
		TStringView<ElementType> View = *this;

		bool Result = View.ToBoolAndTrim();

		size_t TrimNum = this->Num() - View.Num();

		if (TrimNum > 0) Erase(0, TrimNum);

		return Result;
	}

	/** Converts a string into an integer value and remove the parsed substring. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE U ToIntAndTrim(unsigned Base = 10)
	{
		TStringView<ElementType> View = *this;

		U Result = View.template ToIntAndTrim<U>(Base);

		size_t TrimNum = this->Num() - View.Num();

		if (TrimNum > 0) Erase(0, TrimNum);

		return Result;
	}

	/** Converts a string into a floating-point value and remove the parsed substring. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE U ToFloatAndTrim(bool bFixed = true, bool bScientific = true)
	{
		TStringView<ElementType> View = *this;

		U Result = View.template ToFloatAndTrim<U>(bFixed, bScientific);

		size_t TrimNum = this->Num() - View.Num();

		if (TrimNum > 0) Erase(0, TrimNum);

		return Result;
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
	NODISCARD static FORCEINLINE TString Format(TStringView<ElementType> Fmt, const Ts&... Args)
	{
		TString Result;

		Result.AppendFormat(Fmt, Args...);

		return Result;
	}

	/** Format some objects using a format string and append to the string. */
	template <typename... Ts>
	void AppendFormat(TStringView<ElementType> Fmt, const Ts&... Args);

	/**
	 * Parse a string using a format string to objects.
	 *
	 * @param Fmt  - The format string.
	 * @param Args - The objects to parse.
	 *
	 * @return The number of objects successfully parsed.
	 */
	template <typename... Ts>
	FORCEINLINE size_t Parse(TStringView<ElementType> Fmt, Ts&... Args) const
	{
		return TStringView(*this).Parse(Fmt, Args...);
	}

	/** Parse a string using a format string to objects and remove the parsed substring. */
	template <typename... Ts>
	FORCEINLINE size_t ParseAndTrim(TStringView<ElementType> Fmt, Ts&... Args)
	{
		TStringView<ElementType> View = *this;

		size_t Result = View.ParseAndTrim(Fmt, Args...);

		size_t TrimNum = this->Num() - View.Num();

		if (TrimNum > 0) Erase(0, TrimNum);

		return Result;
	}

public:

	/** Overloads the GetTypeHash algorithm for TString. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TString& A) { return GetTypeHash(TStringView<ElementType>(A)); }

	/** Overloads the Swap algorithm for TString. */
	friend FORCEINLINE void Swap(TString& A, TString& B) { Swap(static_cast<Super&>(A), static_cast<Super&>(B)); }

};

template<typename T>
TString(const T*) -> TString<T>;

template<typename T>
TString(TStringView<T>) -> TString<T>;

template<CForwardIterator I, typename S>
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
	: TStringView(InString.GetData(), InString.Num()) { }

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#include "String/Conversion.h.inl"
