#pragma once

#include "Miscellaneous/CoreDefines.h"

#include <cstdint>
#include <cstdlib>
#include <cstddef>

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

// Unsigned integral types

using uint8  = NAMESPACE_STD::uint8_t;
using uint16 = NAMESPACE_STD::uint16_t;
using uint32 = NAMESPACE_STD::uint32_t;
using uint64 = NAMESPACE_STD::uint64_t;

// Signed integral types

using int8  = NAMESPACE_STD::int8_t;
using int16 = NAMESPACE_STD::int16_t;
using int32 = NAMESPACE_STD::int32_t;
using int64 = NAMESPACE_STD::int64_t;

// Floating point types

using float32 = float;
using float64 = double;

// Character types

using chara  = char;
using charw  = wchar_t;
using chart  = charw;
using char8  = char8_t;
using char16 = char16_t;
using char32 = char32_t;

// Pointer types

using uintptr = NAMESPACE_STD::uintptr_t;
using intptr  = NAMESPACE_STD::intptr_t;
using ptrdiff = NAMESPACE_STD::ptrdiff_t;
using size_t  = NAMESPACE_STD::size_t;
using ssize_t = intptr_t;

// Null types

using null_t    = decltype(NULL);
using nullptr_t = NAMESPACE_STD::nullptr_t;

#if PLATFORM_LINUX
#	define PLATFORM_TCHAR_IS_CHAR16 1
#else
#	define PLATFORM_TCHAR_IS_CHAR16 0
#endif

// Define the TEXT macro

#if PLATFORM_TCHAR_IS_CHAR16
#	define TEXT_PASTE(x) u ## x
#else
#	define TEXT_PASTE(x) L ## x
#endif
#define TEXT(x) TEXT_PASTE(x)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
