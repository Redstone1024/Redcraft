#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4455)
#elif PLATFORM_COMPILER_GCC || PLATFORM_COMPILER_CLANG
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wliteral-suffix"
#endif

FORCEINLINE constexpr int8  operator""i8 (unsigned long long int Value) { return static_cast<int8 >(Value); }
FORCEINLINE constexpr int16 operator""i16(unsigned long long int Value) { return static_cast<int16>(Value); }
FORCEINLINE constexpr int32 operator""i32(unsigned long long int Value) { return static_cast<int32>(Value); }
FORCEINLINE constexpr int64 operator""i64(unsigned long long int Value) { return static_cast<int64>(Value); }

FORCEINLINE constexpr int8  operator""I8 (unsigned long long int Value) { return static_cast<int8 >(Value); }
FORCEINLINE constexpr int16 operator""I16(unsigned long long int Value) { return static_cast<int16>(Value); }
FORCEINLINE constexpr int32 operator""I32(unsigned long long int Value) { return static_cast<int32>(Value); }
FORCEINLINE constexpr int64 operator""I64(unsigned long long int Value) { return static_cast<int64>(Value); }

FORCEINLINE constexpr uint8  operator""u8 (unsigned long long int Value) { return static_cast<uint8 >(Value); }
FORCEINLINE constexpr uint16 operator""u16(unsigned long long int Value) { return static_cast<uint16>(Value); }
FORCEINLINE constexpr uint32 operator""u32(unsigned long long int Value) { return static_cast<uint32>(Value); }
FORCEINLINE constexpr uint64 operator""u64(unsigned long long int Value) { return static_cast<uint64>(Value); }

FORCEINLINE constexpr uint8  operator""U8 (unsigned long long int Value) { return static_cast<uint8 >(Value); }
FORCEINLINE constexpr uint16 operator""U16(unsigned long long int Value) { return static_cast<uint16>(Value); }
FORCEINLINE constexpr uint32 operator""U32(unsigned long long int Value) { return static_cast<uint32>(Value); }
FORCEINLINE constexpr uint64 operator""U64(unsigned long long int Value) { return static_cast<uint64>(Value); }

#if PLATFORM_HAS_INT128

FORCEINLINE constexpr  int128 operator""i128(const char* Str);
FORCEINLINE constexpr uint128 operator""u128(const char* Str);

FORCEINLINE constexpr  int128 operator""I128(const char* Str) { return operator""i128(Str); }
FORCEINLINE constexpr uint128 operator""U128(const char* Str) { return operator""u128(Str); }

FORCEINLINE constexpr  int128 operator""i128(const char* Str)
{
	return static_cast<int128>(operator""u128(Str));
}

FORCEINLINE constexpr uint128 operator""u128(const char* Str)
{
	constexpr uint8 DigitFromChar[] =
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	};

	uint128 Result = 0;

	uint Base = 10;

	size_t BeginIndex = 0;

	if (Str[0] == '0')
	{
		if (Str[1] == 'x' || Str[1] == 'X')
		{
			Base = 16;
			BeginIndex += 2;
		}
		else if (Str[1] == 'b' || Str[1] == 'B')
		{
			Base = 2;
			BeginIndex += 2;
		}
		else Base = 8;
	}

	for (size_t I = BeginIndex; Str[I]; ++I)
	{
		Result = Result * Base + DigitFromChar[Str[I]];
	}

	return Result;
}

#endif

#if PLATFORM_HAS_INT128
FORCEINLINE constexpr  intmax operator""imax(const char* Str) { return operator""i128(Str); }
FORCEINLINE constexpr  intmax operator""IMAX(const char* Str) { return operator""I128(Str); }
FORCEINLINE constexpr uintmax operator""umax(const char* Str) { return operator""u128(Str); }
FORCEINLINE constexpr uintmax operator""UMAX(const char* Str) { return operator""U128(Str); }
#else
FORCEINLINE constexpr  intmax operator""imax(unsigned long long int Value) { return operator""i64(Value); }
FORCEINLINE constexpr  intmax operator""IMAX(unsigned long long int Value) { return operator""I64(Value); }
FORCEINLINE constexpr uintmax operator""umax(unsigned long long int Value) { return operator""u64(Value); }
FORCEINLINE constexpr uintmax operator""UMAX(unsigned long long int Value) { return operator""U64(Value); }
#endif

#ifndef __STDCPP_FLOAT32_T__
FORCEINLINE constexpr float32 operator""f32(long double Value) { return static_cast<float32>(Value); }
FORCEINLINE constexpr float32 operator""F32(long double Value) { return static_cast<float32>(Value); }
#endif

#ifndef __STDCPP_FLOAT64_T__
FORCEINLINE constexpr float64 operator""f64(long double Value) { return static_cast<float64>(Value); }
FORCEINLINE constexpr float64 operator""F64(long double Value) { return static_cast<float64>(Value); }
#endif

#if PLATFORM_COMPILER_MSVC
#	pragma warning(pop)
#elif PLATFORM_COMPILER_GCC || PLATFORM_COMPILER_CLANG
#	pragma GCC diagnostic pop
#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
