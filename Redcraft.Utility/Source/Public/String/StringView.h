#pragma once

#include "CoreTypes.h"
#include "String/Char.h"
#include "Memory/Allocator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "Containers/ArrayView.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/ObserverPointer.h"
#include "Miscellaneous/AssertionMacros.h"

#include <cstring>
#include <cwchar>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/**
 * The class template TStringView describes an object that can refer to a constant contiguous sequence of char-like objects
 * with the first element of the sequence at position zero. Provides a set of convenient string processing functions.
 */
template <CCharType T>
class TStringView final
{
public:

	using ElementType = T;

	using Reference = typename TArrayView<const ElementType>::Reference;

	using Iterator        = typename TArrayView<const ElementType>::Iterator;
	using ReverseIterator = typename TArrayView<const ElementType>::ReverseIterator;

	static_assert(CContiguousIterator<Iterator>);

	/** Constructs an empty string view. */
	FORCEINLINE constexpr TStringView() = default;

	/** Constructs a string view that is a view over the range ['InFirst', 'InFirst' + 'Count'). */
	template <CContiguousIterator I> requires (CConvertibleTo<TIteratorElementType<I>(*)[], const ElementType(*)[]>)
	FORCEINLINE constexpr TStringView(I InFirst, size_t InCount) : NativeData(InFirst, InCount) { }

	/** Constructs a string view that is a view over the range ['InFirst', 'InLast'). */
	template <CContiguousIterator I, CSizedSentinelFor<I> S> requires (CConvertibleTo<TIteratorElementType<I>(*)[], const ElementType(*)[]>)
	FORCEINLINE constexpr TStringView(I InFirst, S InLast) : NativeData(InFirst, InLast) { }

	/** Constructs a string view that is a view over the range ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE constexpr TStringView(const ElementType* InPtr, size_t Count) : NativeData(InPtr, Count)
	{
		checkf(InPtr != nullptr, TEXT("TStringView cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE constexpr TStringView(nullptr_t, size_t) = delete;

	/** Constructs a string view that is a view over the range ['InPtr', '\0'). */
	FORCEINLINE constexpr TStringView(const ElementType* InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TStringView cannot be initialized by nullptr. Please check the pointer."));

		size_t Length = 0;

		if constexpr (CSameAs<ElementType, char>)
		{
			Length = NAMESPACE_STD::strlen(InPtr);
		}
		else if constexpr (CSameAs<ElementType, wchar>)
		{
			Length = NAMESPACE_STD::wcslen(InPtr);
		}
		else
		{
			while (InPtr[Length] != LITERAL(ElementType, '\0')) ++Length;
		}

		NativeData = TArrayView<const ElementType>(InPtr, Length);
	}

	FORCEINLINE constexpr TStringView(nullptr_t) = delete;

	/** Defaulted copy constructor copies the size and data pointer. */
	FORCEINLINE constexpr TStringView(const TStringView&) = default;

	/** Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and the size. */
	FORCEINLINE constexpr TStringView& operator=(const TStringView&) noexcept = default;

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr bool operator==(TStringView LHS, TStringView RHS) { return LHS.NativeData == RHS.NativeData; }

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr auto operator<=>(TStringView LHS, TStringView RHS) { return LHS.NativeData <=> RHS.NativeData; }

	/** Shrinks the view by moving its start forward. */
	FORCEINLINE constexpr void RemovePrefix(size_t Count)
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		NativeData = NativeData.Subview(Count);
	}

	/** Shrinks the view by moving its end backward. */
	FORCEINLINE constexpr void RemoveSuffix(size_t Count)
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		NativeData = NativeData.Subview(0, Num() - Count);
	}

	/** Obtains a string view that is a view over the first 'Count' elements of this string view. */
	NODISCARD FORCEINLINE constexpr TStringView First(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TStringView(Begin(), Count);
	}

	/** Obtains a string view that is a view over the last 'Count' elements of this string view. */
	NODISCARD FORCEINLINE constexpr TStringView Last(size_t Count) const
	{
		checkf(Count <= Num(), TEXT("Illegal subview range. Please check Count."));

		return TStringView(End() - Count, Count);
	}

	/** Copies the elements of this string view to the destination buffer without null-termination. */
	FORCEINLINE constexpr size_t Copy(ElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		if (Count == DynamicExtent)
		{
			Count = Num() - Offset;
		}

		Memory::Memcpy(Dest, GetData().Get() + Offset, Count * sizeof(ElementType));

		return Count;
	}

	/** Obtains a string view that is a view over the 'Count' elements of this string view starting at 'Offset'.  */
	NODISCARD FORCEINLINE constexpr TStringView Substr(size_t Offset, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		return Subview(Offset, Count);
	}

	/** Obtains a string view that is a view over the 'Count' elements of this string view starting at 'Offset'.  */
	NODISCARD FORCEINLINE constexpr TStringView Subview(size_t Offset, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= Num() && (Count == DynamicExtent || Offset + Count <= Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		if (Count != DynamicExtent)
		{
			return TStringView(Begin() + Offset, Count);
		}
		else
		{
			return TStringView(Begin() + Offset, Num() - Offset);
		}
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(TStringView Prefix) const
	{
		return Num() >= Prefix.Num() && Substr(0, Prefix.Num()) == Prefix;
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(ElementType Prefix) const
	{
		return Num() >= 1 && Front() == Prefix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(TStringView Suffix) const
	{
		return Num() >= Suffix.Num() && Substr(Num() - Suffix.Num(), Suffix.Num()) == Suffix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(ElementType Suffix) const
	{
		return Num() >= 1 && Back() == Suffix;
	}

	/** @return true if the string view contains the given substring, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(TStringView View) const
	{
		return Find(View) != INDEX_NONE;
	}

	/** @return true if the string view contains the given character, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(ElementType Char) const
	{
		return Find(Char) != INDEX_NONE;
	}

	/** @return true if the string view contains character that satisfy the given predicate, false otherwise. */
	template <CPredicate<ElementType> F>
	NODISCARD FORCEINLINE constexpr bool Contains(F&& InPredicate) const
	{
		return Find(Forward<F>(InPredicate)) != INDEX_NONE;
	}

	/** @return The index of the first occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t Find(TStringView View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		if (View.Num() > Num()) return INDEX_NONE;

		if (View.Num() == 0) return Index;

		for (; Index != Num() - View.Num() + 1; ++Index)
		{
			if (Substr(Index).StartsWith(View))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t Find(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		for (; Index != Num(); ++Index)
		{
			if (NativeData[Index] == Char)
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD FORCEINLINE constexpr size_t Find(F&& InPredicate, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		for (; Index != Num(); ++Index)
		{
			if (InvokeResult<bool>(Forward<F>(InPredicate), NativeData[Index]))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the last occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t RFind(TStringView View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		if (View.Num() > Num()) return INDEX_NONE;

		if (Index == INDEX_NONE) Index = Num();

		if (View.Num() == 0) return Index;

		for (; Index != View.Num() - 1; --Index)
		{
			if (Substr(0, Index).EndsWith(View))
			{
				return Index - View.Num();
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t RFind(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		if (Index == INDEX_NONE) Index = Num();

		for (; Index != 0; --Index)
		{
			if (NativeData[Index - 1] == Char)
			{
				return Index - 1;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the last occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD FORCEINLINE constexpr size_t RFind(F&& InPredicate, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		if (Index == INDEX_NONE) Index = Num();

		for (; Index != 0; --Index)
		{
			if (InvokeResult<bool>(Forward<F>(InPredicate), NativeData[Index - 1]))
			{
				return Index - 1;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(TStringView View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return Find([View](ElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return Find(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return RFind([View](ElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return RFind(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(TStringView View, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return Find([View](ElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(ElementType Char, size_t Index = 0) const
	{
		checkf(Index < Num(), TEXT("Illegal index. Please check Index."));

		return Find([Char](ElementType C) { return C != Char; }, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return RFind([View](ElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		checkf(Index == INDEX_NONE || Index < Num(), TEXT("Illegal index. Please check Index."));

		return RFind([Char](ElementType C) { return C != Char; }, Index);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr TObserverPtr<const ElementType[]> GetData() const { return NativeData.GetData(); }

	/** @return The iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr Iterator Begin() const { return NativeData.Begin(); }
	NODISCARD FORCEINLINE constexpr Iterator End()   const { return NativeData.End();   }

	/** @return The reverse iterator to the first or end element. */
	NODISCARD FORCEINLINE constexpr ReverseIterator RBegin() const { return NativeData.RBegin(); }
	NODISCARD FORCEINLINE constexpr ReverseIterator REnd()   const { return NativeData.REnd();   }

	/** @return The number of elements in the container. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { return NativeData.Num(); }

	/** @return true if the container is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(Iterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested element. */
	NODISCARD FORCEINLINE constexpr Reference operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return NativeData[Index]; }

	/** @return The reference to the first or last element. */
	NODISCARD FORCEINLINE constexpr Reference Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr Reference Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TStringView. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TStringView A) { return GetTypeHash(A.NativeData); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	TArrayView<const ElementType> NativeData;

};

template <typename I, typename S>
TStringView(I, S) -> TStringView<TIteratorElementType<I>>;

using FStringView        = TStringView<char>;
using FWStringView       = TStringView<wchar>;
using FU8StringView      = TStringView<u8char>;
using FU16StringView     = TStringView<u16char>;
using FU32StringView     = TStringView<u32char>;
using FUnicodeStringView = TStringView<unicodechar>;

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
