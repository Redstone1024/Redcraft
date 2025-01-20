#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Iterators/Utility.h"
#include "Strings/Formatting.h"
#include "Strings/StringView.h"
#include "Strings/String.h"
#include "Miscellaneous/BitwiseEnum.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** An enumeration that defines the color of the console. */
enum class EColor : uint8
{
	Default = 0xFF,

	Black     = 0b0000,
	Red       = 0b0001,
	Green     = 0b0010,
	Blue      = 0b0100,
	Intensity = 0b1000,

	Cyan    = Green | Blue,
	Magenta = Blue  | Red,
	Yellow  = Red   | Green,

	White = Red | Green | Blue,

	BrightBlack   = Intensity | Black,
	BrightRed     = Intensity | Red,
	BrightGreen   = Intensity | Green,
	BrightBlue    = Intensity | Blue,
	BrightYellow  = Intensity | Yellow,
	BrightMagenta = Intensity | Magenta,
	BrightCyan    = Intensity | Cyan,
	BrightWhite   = Intensity | White
};

ENABLE_ENUM_CLASS_BITWISE_OPERATIONS(EColor)

/** @return The color of the console. */
NODISCARD REDCRAFTUTILITY_API EColor GetForegroundColor();
NODISCARD REDCRAFTUTILITY_API EColor GetBackgroundColor();

/** Set the color of the console. Returns the color that was successfully set. */
REDCRAFTUTILITY_API EColor SetForegroundColor(EColor InColor);
REDCRAFTUTILITY_API EColor SetBackgroundColor(EColor InColor);

/** @return The size of the console window. */
NODISCARD REDCRAFTUTILITY_API uint GetWindowWidth();
NODISCARD REDCRAFTUTILITY_API uint GetWindowHeight();

/** @return true if the standard stream is redirected. */
NODISCARD REDCRAFTUTILITY_API bool IsInputRedirected();
NODISCARD REDCRAFTUTILITY_API bool IsOutputRedirected();
NODISCARD REDCRAFTUTILITY_API bool IsErrorRedirected();

/** Clear the console screen. */
REDCRAFTUTILITY_API void Clear();

/** @return The input character from the standard input. */
NODISCARD REDCRAFTUTILITY_API char Input(bool bEcho = true);

/** @return The input line from the standard input. */
NODISCARD REDCRAFTUTILITY_API FString InputLn(bool bEcho = true);

/** Print the character to the standard output. */
REDCRAFTUTILITY_API bool Print(char Char);

/** Print the formatted string to the standard output. */
template <CFormattable... Ts>
FORCEINLINE bool Print(FStringView Fmt, Ts&&... Args)
{
	struct FStandardOutputIterator
	{
		FORCEINLINE constexpr FStandardOutputIterator& operator=(char Char)
		{
			bError |= !Print(Char);
			return *this;
		}

		FORCEINLINE constexpr bool operator==(FDefaultSentinel) const { return bError; }

		FORCEINLINE constexpr FStandardOutputIterator& operator*()     { return *this; }
		FORCEINLINE constexpr FStandardOutputIterator& operator++()    { return *this; }
		FORCEINLINE constexpr FStandardOutputIterator& operator++(int) { return *this; }

		bool bError = false;
	};

	static_assert(COutputIterator<FStandardOutputIterator, char>);

	FStandardOutputIterator Iter;

	auto Range = Ranges::View(Iter, DefaultSentinel);

	Iter = Algorithms::Format(Range, Fmt, Forward<Ts>(Args)...);

	return Iter != DefaultSentinel;
}

/** Print the value to the standard output. */
template <CFormattable T>
FORCEINLINE bool Print(T&& Value)
{
	if constexpr (CSameAs<TRemoveCVRef<T>, char>)
	{
		return Print(static_cast<char>(Value));
	}

	else if constexpr (CConvertibleTo<T, FStringView>)
	{
		return Print(static_cast<FStringView>(Value));
	}

	else return Print(TEXT("{0}"), Forward<T>(Value));
}

/** Print the newline character to the standard output. */
FORCEINLINE bool PrintLn()
{
	return Print(TEXT("\n"));
}

/** Print the string to the standard output and append the newline character. */
template <CFormattable... Ts>
FORCEINLINE bool PrintLn(FStringView Fmt, Ts&&... Args)
{
	return Print(Fmt, Forward<Ts>(Args)...) && Print(TEXT("\n"));
}

/** Print the value to the standard output and append the newline character. */
template <CFormattable T>
FORCEINLINE bool PrintLn(T&& Value)
{
	return Print(Forward<T>(Value)) && Print(TEXT("\n"));
}

/** Print the character to the standard error. */
REDCRAFTUTILITY_API bool Error(char Char);

/** Print the formatted string to the standard error. */
template <CFormattable... Ts>
FORCEINLINE bool Error(FStringView Fmt, Ts&&... Args)
{
	struct FStandardOutputIterator
	{
		FORCEINLINE constexpr FStandardOutputIterator& operator=(char Char)
		{
			bError |= !Error(Char);
			return *this;
		}

		FORCEINLINE constexpr bool operator==(FDefaultSentinel) const { return bError; }

		FORCEINLINE constexpr FStandardOutputIterator& operator*()     { return *this; }
		FORCEINLINE constexpr FStandardOutputIterator& operator++()    { return *this; }
		FORCEINLINE constexpr FStandardOutputIterator& operator++(int) { return *this; }

		bool bError = false;
	};

	static_assert(COutputIterator<FStandardOutputIterator, char>);

	FStandardOutputIterator Iter;

	auto Range = Ranges::View(Iter, DefaultSentinel);

	Iter = Algorithms::Format(Range, Fmt, Forward<Ts>(Args)...);

	return Iter != DefaultSentinel;
}

/** Print the value to the standard error. */
template <CFormattable T>
FORCEINLINE bool Error(T&& Value)
{
	if constexpr (CSameAs<TRemoveCVRef<T>, char>)
	{
		return Error(static_cast<char>(Value));
	}

	else if constexpr (CConvertibleTo<T, FStringView>)
	{
		return Error(static_cast<FStringView>(Value));
	}

	else return Error(TEXT("{0}"), Forward<T>(Value));
}

/** Print the newline character to the standard error. */
FORCEINLINE bool ErrorLn()
{
	return Error(TEXT("\n"));
}

/** Print the string to the standard error and append the newline character. */
template <CFormattable... Ts>
FORCEINLINE bool ErrorLn(FStringView Fmt, Ts&&... Args)
{
	return Error(Fmt, Forward<Ts>(Args)...) && Error(TEXT("\n"));
}

/** Print the value to the standard error and append the newline character. */
template <CFormattable T>
FORCEINLINE bool ErrorLn(T&& Value)
{
	return Error(Forward<T>(Value)) && Error(TEXT("\n"));
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
