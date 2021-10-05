#pragma once

#include "Misc/CoreDefines.h"

#include <new>
#include <cstdint>
#include <cstdlib>
#include <cstddef>

NS_REDCRAFT_BEGIN

// Build information macro.

#ifndef PLATFORM_NAME
	#define PLATFORM_NAME Unknown
#endif

#ifndef PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS 0
#endif

#ifndef PLATFORM_LINUX
	#define PLATFORM_LINUX 0
#endif

#ifndef PLATFORM_UNKNOWN
	#define PLATFORM_UNKNOWN 0
#endif

#ifndef BUILD_TYPE
	#define BUILD_TYPE Unknown
#endif

#ifndef BUILD_DEBUG
	#define BUILD_DEBUG 0
#endif

#ifndef BUILD_RELEASE
	#define BUILD_RELEASE 0
#endif

#ifndef BUILD_UNKNOWN
	#define BUILD_UNKNOWN 0
#endif

// Function type macros.

#if PLATFORM_WINDOWS

	#define VARARGS			__cdecl					
	#define CDECL			__cdecl					
	#define STDCALL			__stdcall				
	#define FORCEINLINE		__forceinline			
	#define FORCENOINLINE	__declspec(noinline)	
	#define RESTRICT		__restrict

#elif PLATFORM_LINUX

	#define VARARGS
	#define CDECL
	#define STDCALL
	#define FORCENOINLINE	__attribute__((noinline))
	#define RESTRICT		__restrict

	#if BUILD_DEBUG
		#define FORCEINLINE inline
	#else
		#define FORCEINLINE inline __attribute__ ((always_inline))
	#endif
	
#else

	#define VARARGS
	#define CDECL
	#define STDCALL
	#define FORCEINLINE
	#define FORCENOINLINE
	#define RESTRICT		__restrict

#endif

// Alignment.

#if PLATFORM_WINDOWS

	#if defined(__clang__)
	
		#define GCC_PACK(n)		__attribute__((packed,aligned(n)))
		#define GCC_ALIGN(n)	__attribute__((aligned(n)))
		
		#if defined(_MSC_VER)
			#define MS_ALIGN(n)	__declspec(align(n))
		#endif
	
	#else
	
		#define GCC_PACK(n)
		#define GCC_ALIGN(n)
		#define MS_ALIGN(n)		__declspec(align(n))
	
	#endif
	
#elif PLATFORM_LINUX

	#define GCC_PACK(n)			__attribute__((packed,aligned(n)))
	#define GCC_ALIGN(n)		__attribute__((aligned(n)))
	#define MS_ALIGN(n)

#else

	#define GCC_PACK(n)
	#define GCC_ALIGN(n)
	#define MS_ALIGN(n)

#endif

// DLL export and import definitions.

#if PLATFORM_WINDOWS

	#define DLLEXPORT	__declspec(dllexport)
	#define DLLIMPORT	__declspec(dllimport)

#elif PLATFORM_LINUX

	#define DLLEXPORT	__attribute__((visibility("default")))
	#define DLLIMPORT	__attribute__((visibility("default")))
	
#else

	#define DLLEXPORT
	#define DLLIMPORT

#endif

// Unsigned base types.

typedef std::uint8_t		uint8;
typedef std::uint16_t		uint16;
typedef std::uint32_t		uint32;
typedef std::uint64_t		uint64;

// Signed base types.

typedef	std::int8_t			int8;
typedef std::int16_t		int16;
typedef std::int32_t		int32;
typedef std::int64_t		int64;

// Character types.

typedef char				ANSICHAR;
typedef wchar_t				WIDECHAR;
typedef WIDECHAR			TCHAR;

// Pointer types.

typedef std::uintptr_t		uintptr_t;
typedef std::intptr_t		intptr_t;
typedef std::size_t			size_t;
typedef intptr_t			ssize_t;

// Null types.

typedef decltype(NULL)		null_t;
typedef std::nullptr_t		nullptr_t;

#if PLATFORM_LINUX
	#define PLATFORM_TCHAR_IS_CHAR16 1
#else
	#define PLATFORM_TCHAR_IS_CHAR16 0
#endif

// Define the TEXT macro.

#if PLATFORM_TCHAR_IS_CHAR16
	#define TEXT_PASTE(x) u ## x
#else
	#define TEXT_PASTE(x) L ## x
#endif
#define TEXT(x) TEXT_PASTE(x)

NS_REDCRAFT_END
