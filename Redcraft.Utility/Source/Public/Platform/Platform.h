#pragma once

#include "Misc/CoreDefines.h"

#include <cstdint>
#include <cstdlib>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Build information macro.

#ifndef PLATFORM_NAME
#	error "PLATFORM_NAME must be defined"
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
#	error "BUILD_TYPE must be defined"
#endif

#ifndef BUILD_DEBUG
#	define BUILD_DEBUG 0
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

// Function type macros.

#if PLATFORM_WINDOWS

#	define VARARGS				__cdecl
#	define CDECL				__cdecl
#	define STDCALL				__stdcall
#	define FORCEINLINE			__forceinline
#	define FORCENOINLINE		__declspec(noinline)
#	define RESTRICT				__restrict

#elif PLATFORM_LINUX

#	define VARARGS
#	define CDECL
#	define STDCALL
#	define FORCENOINLINE		__attribute__((noinline))
#	define RESTRICT				__restrict

#	if BUILD_DEBUG
#		define FORCEINLINE		inline
#	else
#		define FORCEINLINE		inline __attribute__ ((always_inline))
#	endif

#else

#	define VARARGS
#	define CDECL
#	define STDCALL
#	define FORCEINLINE
#	define FORCENOINLINE
#	define RESTRICT				__restrict

#endif

// DLL export and import definitions.

#if PLATFORM_WINDOWS

#	define DLLEXPORT	__declspec(dllexport)
#	define DLLIMPORT	__declspec(dllimport)

#elif PLATFORM_LINUX

#	define DLLEXPORT	__attribute__((visibility("default")))
#	define DLLIMPORT	__attribute__((visibility("default")))

#else

#	define DLLEXPORT
#	define DLLIMPORT

#endif

// Unsigned base types.

typedef NAMESPACE_STD::uint8_t		 uint8;
typedef NAMESPACE_STD::uint16_t		uint16;
typedef NAMESPACE_STD::uint32_t		uint32;
typedef NAMESPACE_STD::uint64_t		uint64;

// Signed base types.

typedef NAMESPACE_STD::int8_t		 int8;
typedef NAMESPACE_STD::int16_t		int16;
typedef NAMESPACE_STD::int32_t		int32;
typedef NAMESPACE_STD::int64_t		int64;

// Character types.

typedef char						ANSICHAR;
typedef wchar_t						WIDECHAR;
typedef WIDECHAR					TCHAR;

// Pointer types.

typedef NAMESPACE_STD::uintptr_t	uintptr_t;
typedef NAMESPACE_STD::intptr_t		intptr_t;
typedef NAMESPACE_STD::size_t		size_t;
typedef intptr_t					ssize_t;

// Null types.

typedef decltype(NULL)				null_t;
typedef NAMESPACE_STD::nullptr_t	nullptr_t;

#if PLATFORM_LINUX
#	define PLATFORM_TCHAR_IS_CHAR16 1
#else
#	define PLATFORM_TCHAR_IS_CHAR16 0
#endif

// Define the TEXT macro.

#if PLATFORM_TCHAR_IS_CHAR16
#	define TEXT_PASTE(x) u ## x
#else
#	define TEXT_PASTE(x) L ## x
#endif
#define TEXT(x) TEXT_PASTE(x)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
