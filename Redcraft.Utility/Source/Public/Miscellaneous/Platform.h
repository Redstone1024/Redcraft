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

// Build information macro

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

// Whether the CPU is x86/x64 (i.e. both 32 and 64-bit variants)

#ifndef PLATFORM_CPU_X86_FAMILY
#	if (defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__amd64__) || defined(__x86_64__))
#		define PLATFORM_CPU_X86_FAMILY	1
#	else
#		define PLATFORM_CPU_X86_FAMILY	0
#	endif
#endif

// Whether the CPU is AArch32/AArch64 (i.e. both 32 and 64-bit variants)

#ifndef PLATFORM_CPU_ARM_FAMILY
#	if (defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64))
#		define PLATFORM_CPU_ARM_FAMILY	1
#	else
#		define PLATFORM_CPU_ARM_FAMILY	0
#	endif
#endif

// Function type macros

#if PLATFORM_WINDOWS

#	define VARARGS              __cdecl
#	define CDECL                __cdecl
#	define STDCALL              __stdcall
#	define FORCEINLINE          __forceinline
#	define FORCENOINLINE        __declspec(noinline)
#	define RESTRICT             __restrict

#elif PLATFORM_LINUX

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

#if PLATFORM_WINDOWS
#	define DLLEXPORT    __declspec(dllexport)
#	define DLLIMPORT    __declspec(dllimport)
#elif PLATFORM_LINUX
#	define DLLEXPORT    __attribute__((visibility("default")))
#	define DLLIMPORT    __attribute__((visibility("default")))
#else
#	error "DLL export and import macros must be defined."
#endif

// Optimization macros

#if !defined(__clang__)
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL _Pragma("optimize(\"\", off)")
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  _Pragma("optimize(\"\", on)")
#elif defined(_MSC_VER)
#	define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize off")
#	define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  _Pragma("clang optimize on")
#elif defined(__GNUC__ )
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

#ifdef __SIZEOF_INT128__
using int128 = __int128;
#else
using int128 = void;
#endif

using int8_least   = NAMESPACE_STD::int_least8_t;
using int16_least  = NAMESPACE_STD::int_least16_t;
using int32_least  = NAMESPACE_STD::int_least32_t;
using int64_least  = NAMESPACE_STD::int_least64_t;
using int128_least = int128;

using int8_fast   = NAMESPACE_STD::int_fast8_t;
using int16_fast  = NAMESPACE_STD::int_fast16_t;
using int32_fast  = NAMESPACE_STD::int_fast32_t;
using int64_fast  = NAMESPACE_STD::int_fast64_t;
using int128_fast = int128;

#ifdef __SIZEOF_INT128__
using intmax = int128;
#else
using intmax = int64;
#endif

// Unsigned integral types

using uint8  = NAMESPACE_STD::uint8_t;
using uint16 = NAMESPACE_STD::uint16_t;
using uint32 = NAMESPACE_STD::uint32_t;
using uint64 = NAMESPACE_STD::uint64_t;

#ifdef __SIZEOF_INT128__
using uint128 = unsigned __int128;
#else
using uint128 = void;
#endif

using uint8_least   = NAMESPACE_STD::uint_least8_t;
using uint16_least  = NAMESPACE_STD::uint_least16_t;
using uint32_least  = NAMESPACE_STD::uint_least32_t;
using uint64_least  = NAMESPACE_STD::uint_least64_t;
using uint128_least = uint128;

using uint8_fast   = NAMESPACE_STD::int_fast8_t;
using uint16_fast  = NAMESPACE_STD::int_fast16_t;
using uint32_fast  = NAMESPACE_STD::int_fast32_t;
using uint64_fast  = NAMESPACE_STD::int_fast64_t;
using uint128_fast = uint128;

#ifdef __SIZEOF_INT128__
using uintmax = uint128;
#else
using uintmax = uint64;
#endif

// Floating point types

#if defined(__STDCPP_FLOAT16_T__)
using float16 = NAMESPACE_STD::float16_t;
#else
using float16 = void;
#endif

#if defined(__STDCPP_FLOAT32_T__)
using float32 = NAMESPACE_STD::float32_t;
#else
static_assert(sizeof(float) == 4);
using float32 = float;
#endif

#if defined(__STDCPP_FLOAT64_T__)
using float64 = NAMESPACE_STD::float64_t;
#else
static_assert(sizeof(double) == 8);
using float64 = double;
#endif

#if defined(__STDCPP_FLOAT128_T__)
using float128 = NAMESPACE_STD::float128_t;
#else
using float128 = void;
#endif

#if defined(__STDCPP_BFLOAT16_T__)
using bfloat16 = NAMESPACE_STD::bfloat16_t;
#else
using bfloat16 = void;
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
using intptr  = NAMESPACE_STD::intptr_t;
using ptrdiff = NAMESPACE_STD::ptrdiff_t;
using size_t  = NAMESPACE_STD::size_t;
using ssize_t = intptr_t;

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
