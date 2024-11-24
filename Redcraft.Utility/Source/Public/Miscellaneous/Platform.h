#pragma once

#include "Miscellaneous/CoreDefines.h"

#include <cstdint>
#include <cstdlib>
#include <cstddef>

#if __cplusplus >= 202300L
#	include <stdfloat>
#endif

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Platform information macro

#ifndef PLATFORM_NAME
#	error "PLATFORM_NAME must be defined."
#endif

#ifndef PLATFORM_WINDOWS
#	define PLATFORM_WINDOWS 0
#endif

#ifndef PLATFORM_LINUX
#	define PLATFORM_LINUX 0
#endif

#ifndef PLATFORM_UNKNOWN
#	define PLATFORM_UNKNOWN 0
#endif

// Build information macro

#ifndef BUILD_TYPE
#	error "BUILD_TYPE must be defined."
#endif

#ifndef BUILD_DEBUG
#	define BUILD_DEBUG 0
#endif

#ifndef BUILD_DEVELOPMENT
#	define BUILD_DEVELOPMENT 0
#endif

#ifndef BUILD_RELEASE
#	define BUILD_RELEASE 0
#endif

#ifndef BUILD_UNKNOWN
#	define BUILD_UNKNOWN 0
#endif

// Compiler information macro

#ifndef PLATFORM_COMPILER_NAME
#	error "COMPILER_NAME must be defined."
#endif

#ifndef PLATFORM_COMPILER_MSVC
#	define PLATFORM_COMPILER_MSVC 0
#endif

#ifndef PLATFORM_COMPILER_CLANG
#	define PLATFORM_COMPILER_CLANG 0
#endif

#ifndef PLATFORM_COMPILER_GCC
#	define PLATFORM_COMPILER_GCC 0
#endif

#ifndef PLATFORM_COMPILER_UNKNOWN
#	define PLATFORM_COMPILER_UNKNOWN 0
#endif

// Architecture information macro

#ifndef PLATFORM_CPU_X86_FAMILY
#	if (defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__amd64__) || defined(__x86_64__))
#		define PLATFORM_CPU_X86_FAMILY	1
#	else
#		define PLATFORM_CPU_X86_FAMILY	0
#	endif
#endif

#ifndef PLATFORM_CPU_ARM_FAMILY
#	if (defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64))
#		define PLATFORM_CPU_ARM_FAMILY	1
#	else
#		define PLATFORM_CPU_ARM_FAMILY	0
#	endif
#endif

#ifndef PLATFORM_CPU_UNKNOWN_FAMILY
#	define PLATFORM_CPU_UNKNOWN_FAMILY	(!(PLATFORM_CPU_X86_FAMILY || PLATFORM_CPU_ARM_FAMILY))
#endif

// CPU bits information macro

#ifndef PLATFORM_CPU_BITS
#	if (defined(_M_X64) || defined(__amd64__) || defined(__x86_64__) || defined(__aarch64__) || defined(_M_ARM64))
#		define PLATFORM_CPU_BITS	64
#	elif (defined(_M_IX86) || defined(__i386__) || defined(__arm__) || defined(_M_ARM))
#		define PLATFORM_CPU_BITS	32
#	else
#		define PLATFORM_CPU_BITS	0
#	endif
#endif

// Endian information macro

#if !defined(PLATFORM_LITTLE_ENDIAN) && !defined(PLATFORM_BIG_ENDIAN)
#	if PLATFORM_COMPILER_MSVC
#		define PLATFORM_LITTLE_ENDIAN 1
#		define PLATFORM_BIG_ENDIAN    0
#	elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#		if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#			define PLATFORM_LITTLE_ENDIAN 1
#			define PLATFORM_BIG_ENDIAN    0
#		elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#			define PLATFORM_LITTLE_ENDIAN 0
#			define PLATFORM_BIG_ENDIAN    1
#		else
#			define PLATFORM_LITTLE_ENDIAN 0
#			define PLATFORM_BIG_ENDIAN    0
#		endif
#	endif
#endif

#ifndef PLATFORM_LITTLE_ENDIAN
#	define PLATFORM_LITTLE_ENDIAN 0
#endif

#ifndef PLATFORM_BIG_ENDIAN
#	define PLATFORM_BIG_ENDIAN    0
#endif

#ifndef PLATFORM_CPU_MIXED_ENDIAN
#	define PLATFORM_MIXED_ENDIAN  (!(PLATFORM_LITTLE_ENDIAN || PLATFORM_BIG_ENDIAN))
#endif

// Integral type information macro

#ifndef PLATFORM_HAS_INT8
#	define PLATFORM_HAS_INT8  1
#endif

#ifndef PLATFORM_HAS_INT16
#	define PLATFORM_HAS_INT16 1
#endif

#ifndef PLATFORM_HAS_INT32
#	define PLATFORM_HAS_INT32 1
#endif

#ifndef PLATFORM_HAS_INT64
#	define PLATFORM_HAS_INT64 1
#endif

#ifndef PLATFORM_HAS_INT128
#	if defined(__SIZEOF_INT128__)
#		define PLATFORM_HAS_INT128 (__SIZEOF_INT128__ == 16)
#	else
#		define PLATFORM_HAS_INT128 0
#	endif
#endif

#ifndef PLATFORM_INTMAX_BITS
#	if PLATFORM_HAS_INT128
#		define PLATFORM_INTMAX_BITS 128
#	else
#		define PLATFORM_INTMAX_BITS 64
#	endif
#endif

#if !(PLATFORM_HAS_INT8 && PLATFORM_HAS_INT16 && PLATFORM_HAS_INT32 && PLATFORM_HAS_INT64)
#	error "64-bit and below integral types must be supported."
#endif

// Floating point type information macro

#ifndef PLATFORM_HAS_FLOAT16
#	if defined(__STDCPP_FLOAT16_T__)
#		define PLATFORM_HAS_FLOAT16 1
#	else
#		define PLATFORM_HAS_FLOAT16 0
#	endif
#endif

#ifndef PLATFORM_HAS_FLOAT32
#	define PLATFORM_HAS_FLOAT32 1
#endif

#ifndef PLATFORM_HAS_FLOAT64
#	define PLATFORM_HAS_FLOAT64 1
#endif

#ifndef PLATFORM_HAS_FLOAT128
#	if defined(__STDCPP_FLOAT128_T__)
#		define PLATFORM_HAS_FLOAT128 1
#	else
#		define PLATFORM_HAS_FLOAT128 0
#	endif
#endif

#ifndef PLATFORM_HAS_BFLOAT16
#	if defined(__STDCPP_BFLOAT16_T__)
#		define PLATFORM_HAS_BFLOAT16 1
#	else
#		define PLATFORM_HAS_BFLOAT16 0
#	endif
#endif

// Character type information macro

#ifndef PLATFORM_HAS_WCHAR
#	define PLATFORM_HAS_WCHAR 1
#endif

#ifndef PLATFORM_HAS_U8CHAR
#	ifdef __cpp_char8_t
#		define PLATFORM_HAS_U8CHAR 1
#	else
#		define PLATFORM_HAS_U8CHAR 0
#	endif
#endif

#ifndef PLATFORM_HAS_U16CHAR
#	ifdef __cpp_unicode_characters
#		define PLATFORM_HAS_U16CHAR 1
#	else
#		define PLATFORM_HAS_U16CHAR 0
#	endif
#endif

#ifndef PLATFORM_HAS_U32CHAR
#	ifdef __cpp_unicode_characters
#		define PLATFORM_HAS_U32CHAR 1
#	else
#		define PLATFORM_HAS_U32CHAR 0
#	endif
#endif

#ifndef PLATFORM_HAS_UNICODECHAR
#	ifdef __cpp_unicode_characters
#		define PLATFORM_HAS_UNICODECHAR 1
#	else
#		define PLATFORM_HAS_UNICODECHAR 0
#	endif
#endif

#if !(PLATFORM_HAS_WCHAR && PLATFORM_HAS_U8CHAR && PLATFORM_HAS_U16CHAR && PLATFORM_HAS_U32CHAR)
#	error "All character types must be supported."
#endif

// Function type macros

#if PLATFORM_COMPILER_MSVC

#	define VARARGS              __cdecl
#	define CDECL                __cdecl
#	define STDCALL              __stdcall
#	define FORCEINLINE          __forceinline
#	define FORCENOINLINE        __declspec(noinline)
#	define RESTRICT             __restrict

#elif PLATFORM_COMPILER_CLANG || PLATFORM_COMPILER_GCC

#	define VARARGS
#	define CDECL
#	define STDCALL
#	define FORCENOINLINE        __attribute__((noinline))
#	define RESTRICT             __restrict

#	if BUILD_DEBUG
#		define FORCEINLINE      inline
#	else
#		define FORCEINLINE      inline __attribute__((always_inline))
#	endif

#else

#	define VARARGS
#	define CDECL
#	define STDCALL
#	define FORCEINLINE
#	define FORCENOINLINE
#	define RESTRICT             __restrict

#endif

// DLL export and import macros

#if PLATFORM_COMPILER_MSVC
#	define DLLEXPORT    __declspec(dllexport)
#	define DLLIMPORT    __declspec(dllimport)
#elif PLATFORM_COMPILER_CLANG || PLATFORM_COMPILER_GCC
#	define DLLEXPORT    __attribute__((visibility("default")))
#	define DLLIMPORT    __attribute__((visibility("default")))
#else
#	error "DLL export and import macros must be defined."
#endif

// Optimization macros

#if PLATFORM_COMPILER_MSVC
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL _Pragma("optimize(\"\", off)")
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  _Pragma("optimize(\"\", on)")
#elif PLATFORM_COMPILER_CLANG
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize off")
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  _Pragma("clang optimize on")
#elif PLATFORM_COMPILER_GCC
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL _Pragma("GCC push_options") _Pragma("GCC optimize (\"O0\")")
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  _Pragma("GCC pop_options")
#else
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL
#endif

#if BUILD_DEBUG
#	define PRAGMA_DISABLE_OPTIMIZATION
#	define PRAGMA_ENABLE_OPTIMIZATION
#else
#	define PRAGMA_DISABLE_OPTIMIZATION PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
#	define PRAGMA_ENABLE_OPTIMIZATION  PRAGMA_ENABLE_OPTIMIZATION_ACTUAL
#endif

// Signed integral types

using int8  = NAMESPACE_STD::int8_t;
using int16 = NAMESPACE_STD::int16_t;
using int32 = NAMESPACE_STD::int32_t;
using int64 = NAMESPACE_STD::int64_t;

#if PLATFORM_HAS_INT128
using int128 = __int128;
#endif

using int8_least  = NAMESPACE_STD::int_least8_t;
using int16_least = NAMESPACE_STD::int_least16_t;
using int32_least = NAMESPACE_STD::int_least32_t;
using int64_least = NAMESPACE_STD::int_least64_t;

#if PLATFORM_HAS_INT128
using int128_least = int128;
#endif

using int8_fast  = NAMESPACE_STD::int_fast8_t;
using int16_fast = NAMESPACE_STD::int_fast16_t;
using int32_fast = NAMESPACE_STD::int_fast32_t;
using int64_fast = NAMESPACE_STD::int_fast64_t;

#if PLATFORM_HAS_INT128
using int128_fast = int128;
#endif

#if PLATFORM_HAS_INT128
using intmax = int128;
#else
using intmax = int64;
#endif

static_assert(sizeof(int8)  == 1, "int8 must be 1 byte");
static_assert(sizeof(int16) == 2, "int16 must be 2 bytes");
static_assert(sizeof(int32) == 4, "int32 must be 4 bytes");
static_assert(sizeof(int64) == 8, "int64 must be 8 bytes");

static_assert(sizeof(int8_least)  >= 1, "int8_least must be at least 1 byte");
static_assert(sizeof(int16_least) >= 2, "int16_least must be at least 2 bytes");
static_assert(sizeof(int32_least) >= 4, "int32_least must be at least 4 bytes");
static_assert(sizeof(int64_least) >= 8, "int64_least must be at least 8 bytes");

static_assert(sizeof(int8_fast)  >= 1, "int8_fast must be at least 1 byte");
static_assert(sizeof(int16_fast) >= 2, "int16_fast must be at least 2 bytes");
static_assert(sizeof(int32_fast) >= 4, "int32_fast must be at least 4 bytes");
static_assert(sizeof(int64_fast) >= 8, "int64_fast must be at least 8 bytes");

static_assert(static_cast<int8>(0xFF)                == -1, "int8 use two's complement");
static_assert(static_cast<int16>(0xFFFF)             == -1, "int16 use two's complement");
static_assert(static_cast<int32>(0xFFFFFFFF)         == -1, "int32 use two's complement");
static_assert(static_cast<int64>(0xFFFFFFFFFFFFFFFF) == -1, "int64 use two's complement");

// Unsigned integral types

using uint = unsigned int;
using byte = unsigned char;

using uint8  = NAMESPACE_STD::uint8_t;
using uint16 = NAMESPACE_STD::uint16_t;
using uint32 = NAMESPACE_STD::uint32_t;
using uint64 = NAMESPACE_STD::uint64_t;

#if PLATFORM_HAS_INT128
using uint128 = unsigned __int128;
#endif

using uint8_least  = NAMESPACE_STD::uint_least8_t;
using uint16_least = NAMESPACE_STD::uint_least16_t;
using uint32_least = NAMESPACE_STD::uint_least32_t;
using uint64_least = NAMESPACE_STD::uint_least64_t;

#if PLATFORM_HAS_INT128
using uint128_least = uint128;
#endif

using uint8_fast  = NAMESPACE_STD::int_fast8_t;
using uint16_fast = NAMESPACE_STD::int_fast16_t;
using uint32_fast = NAMESPACE_STD::int_fast32_t;
using uint64_fast = NAMESPACE_STD::int_fast64_t;

#if PLATFORM_HAS_INT128
using uint128_fast = uint128;
#endif

#if PLATFORM_HAS_INT128
using uintmax = uint128;
#else
using uintmax = uint64;
#endif

static_assert(sizeof(uint8)  == 1,  "uint8 must be 1 byte");
static_assert(sizeof(uint16) == 2, "uint16 must be 2 bytes");
static_assert(sizeof(uint32) == 4, "uint32 must be 4 bytes");
static_assert(sizeof(uint64) == 8, "uint64 must be 8 bytes");

static_assert(sizeof(uint8_least)  >= 1,  "uint8_least must be at least 1 byte");
static_assert(sizeof(uint16_least) >= 2, "uint16_least must be at least 2 bytes");
static_assert(sizeof(uint32_least) >= 4, "uint32_least must be at least 4 bytes");
static_assert(sizeof(uint64_least) >= 8, "uint64_least must be at least 8 bytes");

static_assert(sizeof(uint8_fast)  >= 1,  "uint8_fast must be at least 1 byte");
static_assert(sizeof(uint16_fast) >= 2, "uint16_fast must be at least 2 bytes");
static_assert(sizeof(uint32_fast) >= 4, "uint32_fast must be at least 4 bytes");
static_assert(sizeof(uint64_fast) >= 8, "uint64_fast must be at least 8 bytes");

static_assert(static_cast<uint8 >(-1) > static_cast< uint8>(0),  "uint8 must be unsigned");
static_assert(static_cast<uint16>(-1) > static_cast<uint16>(0), "uint16 must be unsigned");
static_assert(static_cast<uint32>(-1) > static_cast<uint32>(0), "uint32 must be unsigned");
static_assert(static_cast<uint64>(-1) > static_cast<uint64>(0), "uint64 must be unsigned");

// Floating point types

#if PLATFORM_HAS_FLOAT16
using float16 = NAMESPACE_STD::float16_t;
#endif

#if PLATFORM_HAS_FLOAT32
#	ifdef __STDCPP_FLOAT32_T__
using float32 = NAMESPACE_STD::float32_t;
#	else
using float32 = float;
#	endif
#endif

#if PLATFORM_HAS_FLOAT64
#	ifdef __STDCPP_FLOAT64_T__
using float64 = NAMESPACE_STD::float64_t;
#	else
using float64 = double;
#	endif
#endif

#if PLATFORM_HAS_FLOAT128
using float128 = NAMESPACE_STD::float128_t;
#endif

#if PLATFORM_HAS_BFLOAT16
using bfloat16 = NAMESPACE_STD::bfloat16_t;
#endif

// Character types

// The 'char'        typically represents the user-preferred locale narrow encoded character set.
// The 'wchar'       typically represents the user-preferred locale wide encoded character set.
// The 'u8char'      typically represents the UTF-8 encoded unicode character set.
// The 'u16char'     typically represents the UTF-16 encoded unicode character set.
// The 'u32char'     typically represents the UTF-32 encoded unicode character set.
// The 'unicodechar' typically represents the fixed-length encoded unicode character set.

// The literals should preferentially use unicode character set instead of user-preferred locale character set.

using wchar       = wchar_t;
using u8char      = char8_t;
using u16char     = char16_t;
using u32char     = char32_t;
using unicodechar = char32_t;

static_assert(!PLATFORM_WINDOWS || sizeof(wchar) == sizeof(u16char), "wchar represents UTF-16 on Windows");
static_assert(!PLATFORM_LINUX   || sizeof(wchar) == sizeof(u32char), "wchar represents UTF-32 on Linux");

// Pointer types

using uintptr = NAMESPACE_STD::uintptr_t;
using ptrdiff = NAMESPACE_STD::ptrdiff_t;
using size_t  = NAMESPACE_STD::size_t;

static_assert(sizeof(uintptr) == sizeof(void*), "uintptr must be the same size as a pointer");
static_assert(sizeof(ptrdiff) == sizeof(void*), "ptrdiff must be the same size as a pointer");
static_assert(sizeof(size_t)  == sizeof(void*),  "size_t must be the same size as a pointer");
static_assert(static_cast<uintptr>(-1) > static_cast<uintptr>(0), "uintptr must be unsigned");

// Null types

using null_t    = decltype(NULL);
using nullptr_t = NAMESPACE_STD::nullptr_t;

// Define the TEXT macro

NAMESPACE_PRIVATE_BEGIN

#define TEXT_PASTE(X)    X
#define WTEXT_PASTE(X)   L##X
#define U8TEXT_PASTE(X)  u8##X
#define U16TEXT_PASTE(X) u##X
#define U32TEXT_PASTE(X) U##X

NAMESPACE_PRIVATE_END

#define TEXT(X)  TEXT_PASTE(X)
#define WTEXT(X) WTEXT_PASTE(X)

#define U8TEXT(X)      U8TEXT_PASTE(X)
#define U16TEXT(X)     U16TEXT_PASTE(X)
#define U32TEXT(X)     U32TEXT_PASTE(X)
#define UNICODETEXT(X) U32TEXT_PASTE(X)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
