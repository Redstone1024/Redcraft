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
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/AssertionMacros.h"
#include "Miscellaneous/ConstantIterator.h"

#include <cstring>
#include <cwchar>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CCharType T, CAllocator<T> Allocator>
class TString;

/**
 * The class template TStringView describes an object that can refer to a constant contiguous sequence of char-like objects
 * with the first element of the sequence at position zero. Provides a set of convenient string processing functions.
 */
template <CCharType T>
class TStringView : public TArrayView<const T>
{
private:

	using Super = TArrayView<const T>;

public:

	using ElementType = T;

	using Reference = typename Super::Reference;

	using Iterator        = typename Super::Iterator;
	using ReverseIterator = typename Super::ReverseIterator;

	static_assert(CContiguousIterator<Iterator>);

	/** Constructs an empty string view. */
	FORCEINLINE constexpr TStringView() = default;

	/** Constructs a string view that is a view over the range ['InFirst', 'InFirst' + 'Count'). */
	template <CContiguousIterator I> requires (CConvertibleTo<TIteratorElementType<I>(*)[], const ElementType(*)[]>)
	FORCEINLINE constexpr TStringView(I InFirst, size_t InCount) : Super(InFirst, InCount) { }

	/** Constructs a string view that is a view over the range ['InFirst', 'InLast'). */
	template <CContiguousIterator I, CSizedSentinelFor<I> S> requires (CConvertibleTo<TIteratorElementType<I>(*)[], const ElementType(*)[]>)
	FORCEINLINE constexpr TStringView(I InFirst, S InLast) : Super(InFirst, InLast) { }

	/** Constructs a string view that is a view over the string 'InString'. */
	template <typename Allocator>
	FORCEINLINE constexpr TStringView(const TString<ElementType, Allocator>& InString);

	/** Constructs a string view that is a view over the range ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE constexpr TStringView(const ElementType* InPtr, size_t Count) : Super(InPtr, Count)
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

		*this = TStringView(InPtr, Length);
	}

	FORCEINLINE constexpr TStringView(nullptr_t) = delete;

	/** Defaulted copy constructor copies the size and data pointer. */
	FORCEINLINE constexpr TStringView(const TStringView&) = default;

	/** Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and the size. */
	FORCEINLINE constexpr TStringView& operator=(const TStringView&) noexcept = default;

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr bool operator==(TStringView LHS, TStringView RHS) { return static_cast<Super>(LHS) == static_cast<Super>(RHS); }

	/** Compares the contents of a string view and a character. */
	NODISCARD friend constexpr bool operator==(TStringView LHS, ElementType RHS) { return LHS == TStringView(&RHS, 1); }
	NODISCARD friend constexpr bool operator==(ElementType LHS, TStringView RHS) { return TStringView(&LHS, 1) == RHS; }

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr auto operator<=>(TStringView LHS, TStringView RHS) { return static_cast<Super>(LHS) <=> static_cast<Super>(RHS); }

	/** Compares the contents of a string view and a character. */
	NODISCARD friend constexpr auto operator<=>(TStringView LHS, ElementType RHS) { return LHS <=> TStringView(&RHS, 1); }
	NODISCARD friend constexpr auto operator<=>(ElementType LHS, TStringView RHS) { return TStringView(&LHS, 1) <=> RHS; }

	/** Shrinks the view by moving its start forward. */
	FORCEINLINE constexpr void RemovePrefix(size_t Count)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		*this = Substr(Count);
	}

	/** Shrinks the view by moving its end backward. */
	FORCEINLINE constexpr void RemoveSuffix(size_t Count)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		*this = Substr(0, this->Num() - Count);
	}

	/** Copies the elements of this string view to the destination buffer without null-termination. */
	FORCEINLINE constexpr size_t Copy(ElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
	{
		checkf(Dest != nullptr, TEXT("Illegal destination buffer. Please check the pointer."));

		checkf(Offset <= this->Num() && (Count == DynamicExtent || Offset + Count <= this->Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		if (Count == DynamicExtent)
		{
			Count = this->Num() - Offset;
		}

		Memory::CopyAssign(Dest, this->GetData() + Offset, Count);

		return Count;
	}

	FORCEINLINE constexpr size_t Copy(nullptr_t, size_t Count = DynamicExtent, size_t Offset = 0) const = delete;

	/** Obtains an array view that is a view over the first 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TStringView First(size_t Count) const
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		return Substr(0, Count);
	}

	/** Obtains an array view that is a view over the last 'Count' elements of this array view. */
	NODISCARD FORCEINLINE constexpr TStringView Last(size_t Count) const
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		return Substr(this->Num() - Count);
	}

	/** Obtains a string view that is a view over the 'Count' elements of this string view starting at 'Offset'.  */
	NODISCARD FORCEINLINE constexpr TStringView Substr(size_t Offset, size_t Count = DynamicExtent) const
	{
		checkf(Offset <= this->Num() && (Count == DynamicExtent || Offset + Count <= this->Num()), TEXT("Illegal subview range. Please check Offset and Count."));

		Super Temp = this->Subview(Offset, Count);

		return TStringView(Temp.GetData(), Temp.Num());
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(TStringView Prefix) const
	{
		return this->Num() >= Prefix.Num() && Substr(0, Prefix.Num()) == Prefix;
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(ElementType Prefix) const
	{
		return this->Num() >= 1 && this->Front() == Prefix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(TStringView Suffix) const
	{
		return this->Num() >= Suffix.Num() && Substr(this->Num() - Suffix.Num(), Suffix.Num()) == Suffix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(ElementType Suffix) const
	{
		return this->Num() >= 1 && this->Back() == Suffix;
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
	NODISCARD constexpr size_t Find(TStringView View, size_t Index = 0) const
	{
		if (Index >= this->Num()) return INDEX_NONE;

		if (View.Num() > this->Num()) return INDEX_NONE;

		if (View.Num() == 0) return Index;

		for (; Index != this->Num() - View.Num() + 1; ++Index)
		{
			if (Substr(Index).StartsWith(View))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t Find(ElementType Char, size_t Index = 0) const
	{
		if (Index >= this->Num()) return INDEX_NONE;

		for (; Index != this->Num(); ++Index)
		{
			if ((*this)[Index] == Char)
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD constexpr size_t Find(F&& InPredicate, size_t Index = 0) const
	{
		if (Index >= this->Num()) return INDEX_NONE;

		for (; Index != this->Num(); ++Index)
		{
			if (InvokeResult<bool>(Forward<F>(InPredicate), (*this)[Index]))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the last occurrence of the given substring, or INDEX_NONE if not found. */
	NODISCARD constexpr size_t RFind(TStringView View, size_t Index = INDEX_NONE) const
	{
		if (Index != INDEX_NONE && Index >= this->Num()) return INDEX_NONE;

		if (View.Num() > this->Num()) return INDEX_NONE;

		if (Index == INDEX_NONE) Index = this->Num();

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
	NODISCARD constexpr size_t RFind(ElementType Char, size_t Index = INDEX_NONE) const
	{
		if (Index != INDEX_NONE && Index >= this->Num()) return INDEX_NONE;

		if (Index == INDEX_NONE) Index = this->Num();

		for (; Index != 0; --Index)
		{
			if ((*this)[Index - 1] == Char)
			{
				return Index - 1;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the last occurrence of the character that satisfy the given predicate, or INDEX_NONE if not found. */
	template <CPredicate<ElementType> F>
	NODISCARD constexpr size_t RFind(F&& InPredicate, size_t Index = INDEX_NONE) const
	{
		if (Index != INDEX_NONE && Index >= this->Num()) return INDEX_NONE;

		if (Index == INDEX_NONE) Index = this->Num();

		for (; Index != 0; --Index)
		{
			if (InvokeResult<bool>(Forward<F>(InPredicate), (*this)[Index - 1]))
			{
				return Index - 1;
			}
		}

		return INDEX_NONE;
	}

	/** @return The index of the first occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(TStringView View, size_t Index = 0) const
	{
		return Find([View](ElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(ElementType Char, size_t Index = 0) const
	{
		return Find(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		return RFind([View](ElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		return RFind(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(TStringView View, size_t Index = 0) const
	{
		return Find([View](ElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(ElementType Char, size_t Index = 0) const
	{
		return Find([Char](ElementType C) { return C != Char; }, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		return RFind([View](ElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(ElementType Char, size_t Index = INDEX_NONE) const
	{
		return RFind([Char](ElementType C) { return C != Char; }, Index);
	}

public:

	/** @return true if the string only contains valid characters, false otherwise. */
	NODISCARD constexpr bool IsValid() const
	{
		for (ElementType Char : *this)
		{
			if (!TChar<ElementType>::IsValid(Char)) return false;
		}

		return true;
	}

	/** @return true if the string only contains ASCII characters, false otherwise. */
	NODISCARD constexpr bool IsASCII() const
	{
		for (ElementType Char : *this)
		{
			if (!TChar<ElementType>::IsASCII(Char)) return false;
		}

		return true;
	}

	/** @return true if the string only contains numeric characters, false otherwise. */
	NODISCARD constexpr bool IsNumeric() const
	{
		for (ElementType Char : *this)
		{
			if (!TChar<ElementType>::IsDigit(Char)) return false;
		}

		return true;
	}

	/** @return true if the string only contains numeric characters, false otherwise. */
	NODISCARD constexpr bool IsNumeric(unsigned Base) const
	{
		for (ElementType Char : *this)
		{
			if (!TChar<ElementType>::IsDigit(Char, Base)) return false;
		}

		return true;
	}

public:

	/**
	 * Parse a string using a format string to objects.
	 *
	 * @param Fmt  - The format string.
	 * @param Args - The objects to parse.
	 *
	 * @return The number of objects successfully parsed.
	 */
	template <typename... Ts>
	size_t Parse(TStringView Fmt, Ts&... Args) const;

public:

	/** Overloads the GetTypeHash algorithm for TStringView. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TStringView A) { return GetTypeHash(static_cast<Super>(A)); }

};

template <typename I, typename S>
TStringView(I, S) -> TStringView<TIteratorElementType<I>>;

template<typename T, typename Allocator>
TStringView(TString<T, Allocator>) -> TStringView<T>;

using FStringView        = TStringView<char>;
using FWStringView       = TStringView<wchar>;
using FU8StringView      = TStringView<u8char>;
using FU16StringView     = TStringView<u16char>;
using FU32StringView     = TStringView<u32char>;
using FUnicodeStringView = TStringView<unicodechar>;

#define TEXT_VIEW(X)        TStringView(TEXT(X))
#define WTEXT_VIEW(X)       TStringView(WTEXT(X))
#define U8TEXT_VIEW(X)      TStringView(U8TEXT(X))
#define U16TEXT_VIEW(X)     TStringView(U16TEXT(X))
#define U32TEXT_VIEW(X)     TStringView(U32TEXT(X))
#define UNICODETEXT_VIEW(X) TStringView(UNICODETEXT(X))

#define LITERAL_VIEW(T, X)  TStringView(LITERAL(T, X))

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
