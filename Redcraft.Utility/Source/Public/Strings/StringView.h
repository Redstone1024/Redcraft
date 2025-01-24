#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Noncopyable.h"
#include "Memory/Allocators.h"
#include "Memory/MemoryOperator.h"
#include "Containers/ArrayView.h"
#include "Iterators/Utility.h"
#include "Iterators/BasicIterator.h"
#include "Iterators/Sentinel.h"
#include "Strings/Char.h"
#include "Strings/Convert.h"
#include "Strings/Formatting.h"
#include "Miscellaneous/AssertionMacros.h"

#include <cstring>
#include <cwchar>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CCharType T>
class TStringView;

template <CCharType T, CAllocator<T> Allocator>
class TString;

NAMESPACE_PRIVATE_BEGIN

template <typename T> struct TIsTStringView                 : FFalse { };
template <typename T> struct TIsTStringView<TStringView<T>> : FTrue  { };

template <typename T>
class TCStringFromTStringView final : FNoncopyable
{
public:

	FORCEINLINE TCStringFromTStringView(const T* InPtr, bool bInDelete)
		: Ptr(InPtr), bDelete(bInDelete)
	{ }

	FORCEINLINE TCStringFromTStringView(TCStringFromTStringView&& InValue)
		: Ptr(InValue.Ptr), bDelete(Exchange(InValue.bDelete, false))
	{ }

	FORCEINLINE ~TCStringFromTStringView()
	{
		if (bDelete) delete[] Ptr;
	}

	FORCEINLINE TCStringFromTStringView& operator=(TCStringFromTStringView&& InValue)
	{
		if (bDelete) delete[] Ptr;

		Ptr = InValue.Ptr;

		bDelete = Exchange(InValue.bDelete, false);

		return *this;
	}

	NODISCARD FORCEINLINE operator const T*() const { return Ptr; }

private:

	const T* Ptr;
	bool bDelete;

};

NAMESPACE_PRIVATE_END

template <typename T> concept CTStringView = NAMESPACE_PRIVATE::TIsTStringView<TRemoveCV<T>>::Value;

/**
 * The class template TStringView describes an object that can refer to a constant contiguous sequence of char-like objects
 * with the first element of the sequence at position zero. Provides a set of convenient string processing functions.
 */
template <CCharType T>
class TStringView : public TArrayView<const T>
{
private:

	using FSuper = TArrayView<const T>;

public:

	using FElementType = T;

	using FReference = typename FSuper::FReference;

	using        FIterator = typename FSuper::       FIterator;
	using FReverseIterator = typename FSuper::FReverseIterator;

	static_assert(CContiguousIterator<FIterator>);

	/** Constructs an empty string view. */
	FORCEINLINE constexpr TStringView() = default;

	/** Constructs a string view that is a view over the range ['InFirst', 'InFirst' + 'Count'). */
	template <CContiguousIterator I> requires (CConvertibleTo<TIteratorReference<I>, T> && CSameAs<TRemoveCVRef<TIteratorReference<I>>, TRemoveCVRef<T>>)
	FORCEINLINE constexpr TStringView(I InFirst, size_t InCount) : FSuper(InFirst, InCount) { }

	/** Constructs a string view that is a view over the range ['InFirst', 'InLast'). */
	template <CContiguousIterator I, CSizedSentinelFor<I> S> requires (CConvertibleTo<TIteratorReference<I>, T> && CSameAs<TRemoveCVRef<TIteratorReference<I>>, TRemoveCVRef<T>>)
	FORCEINLINE constexpr TStringView(I InFirst, S InLast) : FSuper(InFirst, InLast) { }

	/** Constructs a string view that is a view over the string 'InString'. */
	template <typename Allocator>
	FORCEINLINE constexpr TStringView(const TString<FElementType, Allocator>& InString);

	/** Constructs a string view that is a view over the range ['InPtr', 'InPtr' + 'Count'). */
	FORCEINLINE constexpr TStringView(const FElementType* InPtr, size_t Count) : FSuper(InPtr, Count)
	{
		checkf(InPtr != nullptr, TEXT("TStringView cannot be initialized by nullptr. Please check the pointer."));
	}

	FORCEINLINE constexpr TStringView(nullptr_t, size_t) = delete;

	/** Constructs a string view that is a view over the range ['InPtr', '\0'). */
	FORCEINLINE constexpr TStringView(const FElementType* InPtr)
	{
		checkf(InPtr != nullptr, TEXT("TStringView cannot be initialized by nullptr. Please check the pointer."));

		size_t Length = 0;

		if constexpr (CSameAs<FElementType, char>)
		{
			Length = NAMESPACE_STD::strlen(InPtr);
		}
		else if constexpr (CSameAs<FElementType, wchar>)
		{
			Length = NAMESPACE_STD::wcslen(InPtr);
		}
		else
		{
			while (InPtr[Length] != LITERAL(FElementType, '\0')) ++Length;
		}

		*this = TStringView(InPtr, Length);
	}

	FORCEINLINE constexpr TStringView(nullptr_t) = delete;

	/** Defaulted copy constructor copies the size and data pointer. */
	FORCEINLINE constexpr TStringView(const TStringView&) = default;

	/** Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and the size. */
	FORCEINLINE constexpr TStringView& operator=(const TStringView&) noexcept = default;

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr bool operator==(TStringView LHS, TStringView RHS) { return static_cast<FSuper>(LHS) == static_cast<FSuper>(RHS); }

	/** Compares the contents of a string view and a character. */
	NODISCARD friend constexpr bool operator==(TStringView  LHS, FElementType RHS) { return LHS == TStringView(&RHS, 1); }
	NODISCARD friend constexpr bool operator==(FElementType LHS, TStringView  RHS) { return TStringView(&LHS, 1) == RHS; }

	/** Compares the contents of two string views. */
	NODISCARD friend constexpr auto operator<=>(TStringView LHS, TStringView RHS) { return static_cast<FSuper>(LHS) <=> static_cast<FSuper>(RHS); }

	/** Compares the contents of a string view and a character. */
	NODISCARD friend constexpr auto operator<=>(TStringView  LHS, FElementType RHS) { return LHS <=> TStringView(&RHS, 1); }
	NODISCARD friend constexpr auto operator<=>(FElementType LHS, TStringView  RHS) { return TStringView(&LHS, 1) <=> RHS; }

public:

	/** Shrinks the view by moving its start forward. */
	FORCEINLINE constexpr TStringView& RemovePrefix(size_t Count)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		*this = Substr(Count);

		return *this;
	}

	/** Shrinks the view by moving its end backward. */
	FORCEINLINE constexpr TStringView& RemoveSuffix(size_t Count)
	{
		checkf(Count <= this->Num(), TEXT("Illegal subview range. Please check Count."));

		*this = Substr(0, this->Num() - Count);

		return *this;
	}

	/** Removes whitespace characters from the start of this string. */
	FORCEINLINE constexpr TStringView& TrimStart()
	{
		auto Index = Find([](FElementType Char) { return !TChar<FElementType>::IsSpace(Char); });

		if (Index != INDEX_NONE)
		{
			RemovePrefix(Index);
		}
		else *this = TStringView();

		return *this;
	}

	/** Removes whitespace characters from the end of this string. */
	FORCEINLINE constexpr TStringView& TrimEnd()
	{
		auto Index = RFind([](FElementType Char) { return !TChar<FElementType>::IsSpace(Char); });

		if (Index != INDEX_NONE)
		{
			RemoveSuffix(this->Num() - Index - 1);
		}
		else *this = TStringView();

		return *this;
	}

	/** Removes whitespace characters from the start and end of this string. */
	FORCEINLINE constexpr TStringView& TrimStartAndEnd()
	{
		TrimStart();
		TrimEnd();

		return *this;
	}

	/** Removes characters after the first null-terminator. */
	FORCEINLINE constexpr TStringView& TrimToNullTerminator()
	{
		auto Index = Find(LITERAL(FElementType, '\0'));

		if (Index != INDEX_NONE)
		{
			*this = Substr(0, Index);
		}

		return *this;
	}

public:

	/** Copies the elements of this string view to the destination buffer without null-termination. */
	FORCEINLINE constexpr size_t Copy(FElementType* Dest, size_t Count = DynamicExtent, size_t Offset = 0) const
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

		FSuper Temp = this->Subview(Offset, Count);

		return TStringView(Temp.GetData(), Temp.Num());
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(TStringView Prefix) const
	{
		return this->Num() >= Prefix.Num() && Substr(0, Prefix.Num()) == Prefix;
	}

	/** @return true if the string view starts with the given prefix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool StartsWith(FElementType Prefix) const
	{
		return this->Num() >= 1 && this->Front() == Prefix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(TStringView Suffix) const
	{
		return this->Num() >= Suffix.Num() && Substr(this->Num() - Suffix.Num(), Suffix.Num()) == Suffix;
	}

	/** @return true if the string view ends with the given suffix, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool EndsWith(FElementType Suffix) const
	{
		return this->Num() >= 1 && this->Back() == Suffix;
	}

	/** @return true if the string view contains the given substring, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(TStringView View) const
	{
		return Find(View) != INDEX_NONE;
	}

	/** @return true if the string view contains the given character, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool Contains(FElementType Char) const
	{
		return Find(Char) != INDEX_NONE;
	}

	/** @return true if the string view contains character that satisfy the given predicate, false otherwise. */
	template <CPredicate<FElementType> F>
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
	NODISCARD constexpr size_t Find(FElementType Char, size_t Index = 0) const
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
	template <CPredicate<FElementType> F>
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
	NODISCARD constexpr size_t RFind(FElementType Char, size_t Index = INDEX_NONE) const
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
	template <CPredicate<FElementType> F>
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
		return Find([View](FElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the first occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstOf(FElementType Char, size_t Index = 0) const
	{
		return Find(Char, Index);
	}

	/** @return The index of the last occurrence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		return RFind([View](FElementType Char) { return View.Contains(Char); }, Index);
	}

	/** @return The index of the last occurrence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastOf(FElementType Char, size_t Index = INDEX_NONE) const
	{
		return RFind(Char, Index);
	}

	/** @return The index of the first absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(TStringView View, size_t Index = 0) const
	{
		return Find([View](FElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the first absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindFirstNotOf(FElementType Char, size_t Index = 0) const
	{
		return Find([Char](FElementType C) { return C != Char; }, Index);
	}

	/** @return The index of the last absence of the character contained in the given view, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(TStringView View, size_t Index = INDEX_NONE) const
	{
		return RFind([View](FElementType Char) { return !View.Contains(Char); }, Index);
	}

	/** @return The index of the last absence of the given character, or INDEX_NONE if not found. */
	NODISCARD FORCEINLINE constexpr size_t FindLastNotOf(FElementType Char, size_t Index = INDEX_NONE) const
	{
		return RFind([Char](FElementType C) { return C != Char; }, Index);
	}

public:

	/** @return The non-modifiable standard C character string version of the string view. */
	NODISCARD FORCEINLINE auto operator*() const
	{
		if (EndsWith(LITERAL(FElementType, '\0')) || Contains(LITERAL(FElementType, '\0')))
		{
			return NAMESPACE_PRIVATE::TCStringFromTStringView<FElementType>(this->GetData(), false);
		}

		FElementType* Buffer = new FElementType[this->Num() + 1];

		Copy(Buffer);

		Buffer[this->Num()] = LITERAL(FElementType, '\0');

		return NAMESPACE_PRIVATE::TCStringFromTStringView<FElementType>(Buffer, true);
	}

public:

	/** @return true if the string only contains valid characters, false otherwise. */
	NODISCARD constexpr bool IsValid() const
	{
		for (FElementType Char : *this)
		{
			if (!TChar<FElementType>::IsValid(Char)) return false;
		}

		return true;
	}

	/** @return true if the string only contains ASCII characters, false otherwise. */
	NODISCARD constexpr bool IsASCII() const
	{
		for (FElementType Char : *this)
		{
			if (!TChar<FElementType>::IsASCII(Char)) return false;
		}

		return true;
	}

	/** @return true if the string can be converted to a boolean value, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsBoolean() const
	{
		bool Temp;

		return Algorithms::Parse(*this, Temp);
	}

	/** @return true if the string can be converted to an integer value, false otherwise. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && CSameAs<TRemoveCVRef<U>, U>)
	NODISCARD FORCEINLINE constexpr bool IsInteger(uint Base = 0) const
	{
		U Temp;

		return Algorithms::Parse(*this, Temp, Base);
	}

	/** @return true if the string can be converted to a floating-point value, false otherwise. */
	template <CFloatingPoint U = float> requires (!CSameAs<U, bool> && CSameAs<TRemoveCVRef<U>, U>)
	NODISCARD FORCEINLINE constexpr bool IsFloatingPoint(bool bFixed = true, bool bScientific = true, bool bHex = true) const
	{
		U Temp;

		return Algorithms::Parse(*this, Temp, bFixed, bScientific, bHex);
	}

	/** Converts the string into a boolean value. */
	NODISCARD FORCEINLINE constexpr bool ToBool() const
	{
		bool Result;

		verifyf(Algorithms::Parse(*this, Result), TEXT("Illegal conversion. Please check the IsBoolean()."));

		return Result;
	}

	/** Converts the string into an integer value. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr U ToInt(uint Base = 0) const
	{
		U Result;

		verifyf(Algorithms::Parse(*this, Result, Base), TEXT("Illegal conversion. Please check the IsInteger()."));

		return Result;
	}

	/** Converts the string into a floating-point value. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr U ToFloat(bool bFixed = true, bool bScientific = true, bool bHex = true) const
	{
		U Result;

		verifyf(Algorithms::Parse(*this, Result, bFixed, bScientific, bHex), TEXT("Illegal conversion. Please check the IsFloatingPoint()."));

		return Result;
	}

	/** Parse the string into a boolean value. */
	NODISCARD FORCEINLINE constexpr bool Parse(bool& Value)
	{
		return Algorithms::Parse(*this, Value);
	}

	/** Parse the string into an integer value. */
	template <CIntegral U = int> requires (!CSameAs<U, bool> && !CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr bool Parse(U& Value, uint Base = 0)
	{
		return Algorithms::Parse(*this, Value, Base);
	}

	/** Parse the string into a floating-point value. */
	template <CFloatingPoint U = float> requires (!CConst<U> && !CVolatile<U>)
	NODISCARD FORCEINLINE constexpr bool Parse(U& Value, bool bFixed = true, bool bScientific = true, bool bHex = true)
	{
		return Algorithms::Parse(*this, Value, bFixed, bScientific, bHex);
	}

public:

	/** Overloads the GetTypeHash algorithm for TStringView. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(TStringView A) { return GetTypeHash(static_cast<FSuper>(A)); }

};

template <CPointer I>
TStringView(I) -> TStringView<TIteratorElement<I>>;

template <typename I, typename S>
TStringView(I, S) -> TStringView<TIteratorElement<I>>;

template<typename T, typename Allocator>
TStringView(TString<T, Allocator>) -> TStringView<T>;

using FStringView        = TStringView<char>;
using FWStringView       = TStringView<wchar>;
using FU8StringView      = TStringView<u8char>;
using FU16StringView     = TStringView<u16char>;
using FU32StringView     = TStringView<u32char>;
using FUnicodeStringView = TStringView<unicodechar>;

// ReSharper disable CppInconsistentNaming

#define TEXT_VIEW(X)        TStringView(TEXT(X))
#define WTEXT_VIEW(X)       TStringView(WTEXT(X))
#define U8TEXT_VIEW(X)      TStringView(U8TEXT(X))
#define U16TEXT_VIEW(X)     TStringView(U16TEXT(X))
#define U32TEXT_VIEW(X)     TStringView(U32TEXT(X))
#define UNICODETEXT_VIEW(X) TStringView(UNICODETEXT(X))

#define LITERAL_VIEW(T, X)  TStringView(LITERAL(T, X))

// ReSharper restore CppInconsistentNaming

/**
 * A formatter for TStringView.
 *
 * The syntax of format specifications is:
 *
 *	[Fill And Align] [Width] [Precision] [Type] [!] [?]
 *
 * 1. The fill and align part:
 *
 *	[Fill Character] <Align Option>
 *
 *	i.   Fill Character: The character is used to fill width of the object. It is optional and cannot be '{' or '}'.
 *	                     It should be representable as a single unicode otherwise it is undefined behavior.
 *
 *  ii.  Align Option: The character is used to indicate the direction of alignment.
 *
 *		- '<': Align the formatted argument to the left of the available space
 *		       by inserting n fill characters after the formatted argument.
 *		       This is default option.
 *		- '^': Align the formatted argument to the center of the available space
 *		       by inserting n fill characters around the formatted argument.
 *		       If cannot absolute centering, offset to the left.
 *		- '>': Align the formatted argument ro the right of the available space
 *		       by inserting n fill characters before the formatted argument.
 *
 * 2. The width part:
 *
 *	- 'N':   The number is used to specify the minimum field width of the object.
 *	         N should be an unsigned non-zero decimal number.
 *	- '{N}': Dynamically determine the minimum field width of the object.
 *	         N should be a valid index of the format integral argument.
 *	         N is optional, and the default value is automatic indexing.
 *
 * 3. The precision part:
 *
 *	- '.N':   The number is used to specify the maximum field width of the object.
 *	          N should be an unsigned non-zero decimal number.
 *	- '.{N}': Dynamically determine the maximum field width of the object.
 *	          N should be a valid index of the format integral argument.
 *	          N is optional, and the default value is automatic indexing.
 *
 * 4. The type indicator part:
 *
 *	- none: Indicates the as-is formatting.
 *	- 'S':  Indicates the as-is formatting.
 *	- 's':  Indicates lowercase formatting.
 *
 * 5. The case indicators part:
 *
 *	- '!': Indicates capitalize the entire string.
 *
 * 6. The escape indicators part:
 *
 *	- '?': Indicates the escape formatting.
 *
 */
template <CCharType T>
class TFormatter<TStringView<T>, T>
{
private:

	using FCharType      = T;
	using FCharTraits    = TChar<FCharType>;
	using FFillCharacter = TStaticArray<FCharType, FCharTraits::MaxCodeUnitLength>;

public:

	template <CFormatStringContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Parse(CTX& Context)
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		// Set the default values.
		{
			FillUnitLength   = 1;
			FillCharacter[0] = LITERAL(FCharType, ' ');
			AlignOption      = LITERAL(FCharType, '<');

			MinFieldWidth =  0;
			MaxFieldWidth = -1;

			bDynamicMin = false;
			bDynamicMax = false;

			bLowercase = false;
			bUppercase = false;
			bEscape    = false;
		}

		// If the format description string is empty.
		if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

		FCharType Char = *Iter; ++Iter;

		// Try to parse the fill and align part.
		// This code assumes that the format string does not contain multi-unit characters, except for fill character.

		// If the fill character is multi-unit.
		if (!FCharTraits::IsValid(Char))
		{
			FillUnitLength   = 1;
			FillCharacter[0] = Char;

			while (true)
			{
				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;

				// If the fill character ends.
				if (FillUnitLength == FCharTraits::MaxCodeUnitLength || FCharTraits::IsValid(Char)) break;

				FillCharacter[FillUnitLength++] = Char;
			}

			if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. The fill character is not representable as a single unicode."));

				return Iter;
			}

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// If the fill character is single-unit.
		else do
		{
			if (Iter == Sent) break;

			// If the fill character is specified.
			if (*Iter == LITERAL(FCharType, '<') || *Iter == LITERAL(FCharType, '^') || *Iter == LITERAL(FCharType, '>'))
			{
				FillUnitLength   = 1;
				FillCharacter[0] = Char;

				Char = *Iter; ++Iter;
			}

			// If the fill character is not specified and the align option is not specified.
			else if (Char != LITERAL(FCharType, '<') && Char != LITERAL(FCharType, '^') && Char != LITERAL(FCharType, '>')) break;

			AlignOption = Char;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}
		while (false);

		// Try to parse the width part.
		{
			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicMin   = true;
				MinFieldWidth = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicMin || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				MinFieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicMin, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicMin && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					MinFieldWidth = MinFieldWidth * 10 + Digit;
				}
			}

			if (bDynamicMin)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (MinFieldWidth == INDEX_NONE)
					{
						MinFieldWidth = Context.GetNextIndex();

						if (MinFieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the field width."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(MinFieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the field width."));
					}

					else break;

					bDynamicMin   = false;
					MinFieldWidth = 0;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the precision part.
		if (Char == LITERAL(FCharType, '.'))
		{
			if (Iter == Sent) UNLIKELY
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			Char = *Iter; ++Iter;

			if (Char == LITERAL(FCharType, '{'))
			{
				bDynamicMax   = true;
				MaxFieldWidth = INDEX_NONE;

				if (Iter == Sent) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				Char = *Iter; ++Iter;
			}

			if ((bDynamicMax || Char != LITERAL(FCharType, '0')) && FCharTraits::IsDigit(Char))
			{
				MaxFieldWidth = FCharTraits::ToDigit(Char);

				while (true)
				{
					if (Iter == Sent)
					{
						checkf(!bDynamicMax, TEXT("Illegal format string. Missing '}' in format string."));

						return Iter;
					}

					if (!bDynamicMax && *Iter == LITERAL(FCharType, '}')) return Iter;

					Char = *Iter; ++Iter;

					const uint Digit = FCharTraits::ToDigit(Char);

					if (Digit >= 10) break;

					MaxFieldWidth = MaxFieldWidth * 10 + Digit;
				}
			}

			else if (!bDynamicMax)
			{
				checkf(false, TEXT("Illegal format string. Missing precision in format string."));

				return Iter;
			}

			if (bDynamicMax)
			{
				if (Char != LITERAL(FCharType, '}')) UNLIKELY
				{
					checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

					return Iter;
				}

				do
				{
					// Try to automatic indexing.
					if (MaxFieldWidth == INDEX_NONE)
					{
						MaxFieldWidth = Context.GetNextIndex();

						if (MaxFieldWidth == INDEX_NONE) UNLIKELY
						{
							checkf(false, TEXT("Illegal index. Please check the precision."));
						}
						else break;
					}

					// Try to manual indexing.
					else if (!Context.CheckIndex(MaxFieldWidth)) UNLIKELY
					{
						checkf(false, TEXT("Illegal index. Please check the precision."));
					}

					else break;

					bDynamicMax   = false;
					MaxFieldWidth = -1;
				}
				while (false);

				if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

				Char = *Iter; ++Iter;
			}
		}

		// Try to parse the type indicators part.

		switch (Char)
		{
		case LITERAL(FCharType, 's'): bLowercase = true; break;
		default: { }
		}

		switch (Char)
		{
		case LITERAL(FCharType, 'S'):
		case LITERAL(FCharType, 's'): if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter; Char = *Iter; ++Iter; break;
		default: { }
		}

		// Try to parse the case indicators part.
		if (Char == LITERAL(FCharType, '!'))
		{
			bUppercase = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		// Try to parse the escape indicators part.
		if (Char == LITERAL(FCharType, '?'))
		{
			bEscape = true;

			if (Iter == Sent || *Iter == LITERAL(FCharType, '}')) return Iter;

			Char = *Iter; ++Iter;
		}

		checkf(false, TEXT("Illegal format string. Missing '}' in format string."));

		return Iter;
	}

	template <CFormatObjectContext<FCharType> CTX>
	constexpr TRangeIterator<CTX> Format(TStringView<FCharType> Object, CTX& Context) const
	{
		auto Iter = Ranges::Begin(Context);
		auto Sent = Ranges::End  (Context);

		size_t MinDynamicField = MinFieldWidth;
		size_t MaxDynamicField = MaxFieldWidth;

		// Visit the dynamic width argument.
		if (bDynamicMin)
		{
			MinDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic width argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic width argument must be an integral."));

					return 0;
				}
			}
			, MinFieldWidth);
		}

		// Visit the dynamic precision argument.
		if (bDynamicMax)
		{
			MaxDynamicField = Context.Visit([]<typename U>(U&& Value) -> size_t
			{
				using FDecayU = TRemoveCVRef<U>;

				if constexpr (CIntegral<FDecayU> && !CSameAs<FDecayU, bool>)
				{
					checkf(Value > 0, TEXT("Illegal format argument. The dynamic precision argument must be a unsigned non-zero number."));

					return Math::Max(Value, 1);
				}
				else
				{
					checkf(false, TEXT("Illegal format argument. The dynamic precision argument must be an integral."));

					return 0;
				}
			}
			, MaxFieldWidth);
		}

		size_t LeftPadding  = 0;
		size_t RightPadding = 0;

		// Estimate the field width.
		if (MinDynamicField != 0)
		{
			// If escape formatting is enabled, add quotes characters.
			size_t FieldWidth = bEscape ? 2 : 0;

			for (auto ObjectIter = Object.Begin(); ObjectIter != Object.End(); ++ObjectIter)
			{
				if (bEscape)
				{
					switch (const FCharType Char = *ObjectIter)
					{
					case LITERAL(FCharType, '\"'):
					case LITERAL(FCharType, '\\'):
					case LITERAL(FCharType, '\a'):
					case LITERAL(FCharType, '\b'):
					case LITERAL(FCharType, '\f'):
					case LITERAL(FCharType, '\n'):
					case LITERAL(FCharType, '\r'):
					case LITERAL(FCharType, '\t'):
					case LITERAL(FCharType, '\v'): FieldWidth += 2; break;
					default:
						{
							// Use '\x00' format for other non-printable characters.
							if (!FCharTraits::IsASCII(Char) || !FCharTraits::IsPrint(Char))
							{
								FieldWidth += 2 + sizeof(FCharType) * 2;
							}

							else ++FieldWidth;
						}
					}
				}

				else ++FieldWidth;
			}

			const size_t PaddingWidth = MinDynamicField - Math::Min(FieldWidth, MinDynamicField, MaxDynamicField);

			switch (AlignOption)
			{
			default:
			case LITERAL(FCharType, '<'): RightPadding = PaddingWidth; break;
			case LITERAL(FCharType, '>'): LeftPadding  = PaddingWidth; break;
			case LITERAL(FCharType, '^'):
				LeftPadding  = Math::DivAndFloor(PaddingWidth, 2);
				RightPadding = PaddingWidth - LeftPadding;
			}
		}

		// Write the left padding.
		for (size_t Index = 0; Index != LeftPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		// Write the left quote.
		if (bEscape)
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = LITERAL(FCharType, '\"');
		}

		auto ObjectIter = Object.Begin();

		bool bComplete = false;

		// Write the object, include escaped quotes in the counter.
		for (size_t Index = bEscape ? 1 : 0; Index != MaxDynamicField; ++Index)
		{
			if (ObjectIter == Object.End())
			{
				bComplete = true;

				break;
			}

			FCharType Char = *ObjectIter++;

			if (Iter == Sent) UNLIKELY return Iter;

			// Convert the character case.
			if (bLowercase) Char = FCharTraits::ToLower(Char);
			if (bUppercase) Char = FCharTraits::ToUpper(Char);

			if (bEscape)
			{
				switch (Char)
				{
				case LITERAL(FCharType, '\"'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, '\"'); break;
				case LITERAL(FCharType, '\\'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, '\\'); break;
				case LITERAL(FCharType, '\a'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'a');  break;
				case LITERAL(FCharType, '\b'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'b');  break;
				case LITERAL(FCharType, '\f'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'f');  break;
				case LITERAL(FCharType, '\n'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'n');  break;
				case LITERAL(FCharType, '\r'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'r');  break;
				case LITERAL(FCharType, '\t'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 't');  break;
				case LITERAL(FCharType, '\v'): *Iter++ = LITERAL(FCharType, '\\'); *Iter++ = LITERAL(FCharType, 'v');  break;
				default:
					{
						// Use '\x00' format for other non-printable characters.
						if (!FCharTraits::IsASCII(Char) || !FCharTraits::IsPrint(Char))
						{
							*Iter++ = LITERAL(FCharType, '\\');
							*Iter++ = LITERAL(FCharType, 'x' );

							using FUnsignedT = TMakeUnsigned<FCharType>;

							constexpr size_t DigitNum = sizeof(FCharType) * 2;

							FUnsignedT IntValue = static_cast<FUnsignedT>(Char);

							TStaticArray<FCharType, DigitNum> Buffer;

							for (size_t Jndex = 0; Jndex != DigitNum; ++Jndex)
							{
								Buffer[DigitNum - Jndex - 1] = FCharTraits::FromDigit(IntValue & 0xF);

								IntValue >>= 4;
							}

							check(IntValue == 0);

							for (size_t Jndex = 0; Jndex != DigitNum; ++Jndex)
							{
								if (Iter == Sent) UNLIKELY return Iter;

								*Iter++ = Buffer[Jndex];
							}
						}

						else *Iter++ = Char;
					}
				}
			}

			else *Iter++ = Char;
		}

		// Write the right quote, if the field width is enough.
		if (bEscape && bComplete)
		{
			if (Iter == Sent) UNLIKELY return Iter;

			*Iter++ = LITERAL(FCharType, '\"');
		}

		// Write the right padding.
		for (size_t Index = 0; Index != RightPadding; ++Index)
		{
			for (size_t Jndex = 0; Jndex != FillUnitLength; ++Jndex)
			{
				if (Iter == Sent) UNLIKELY return Iter;

				*Iter++ = FillCharacter[Jndex];
			}
		}

		return Iter;
	}

private:

	size_t         FillUnitLength = 1;
	FFillCharacter FillCharacter  = { LITERAL(FCharType, ' ') };
	FCharType      AlignOption    =   LITERAL(FCharType, '<');

	size_t MinFieldWidth =  0;
	size_t MaxFieldWidth = -1;

	bool bDynamicMin = false;
	bool bDynamicMax = false;

	bool bLowercase = false;
	bool bUppercase = false;
	bool bEscape    = false;

};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
