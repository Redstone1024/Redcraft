#pragma once

#include "CoreTypes.h"
#include "Miscellaneous/ProgramSupport.h"
#include "Miscellaneous/PreprocessorHelpers.h"

#ifdef NDEBUG
#undef NDEBUG
#include <cassert>
#define NDEBUG 1
#else
#include <cassert>
#endif

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

class FRecursionScopeMarker
{
public:

	FRecursionScopeMarker(uint8& InCounter) : Counter(InCounter) { ++Counter; }
	~FRecursionScopeMarker() { --Counter; }

private:

	uint8& Counter;

};

#define RS_CHECK_IMPL(InExpr)                       assert(InExpr)
#define RS_CHECK_F_IMPL(InExpr, InFormat, ...)      assert((InFormat, InExpr))

NAMESPACE_PRIVATE_END

#if BUILD_DEBUG || BUILD_DEVELOPMENT
#	define DO_CHECK 1
#else
#	define DO_CHECK 0
#endif

// ReSharper disable CppInconsistentNaming

#define always_check(InExpr)                        RS_CHECK_IMPL(InExpr)
#define always_checkf(InExpr, InFormat, ...)        RS_CHECK_F_IMPL(InExpr, InFormat, ##__VA_ARGS__)
#define always_check_no_entry()                     always_checkf(false, "Enclosing block should never be called.")
#define always_check_no_reentry()                   { static bool PREPROCESSOR_JOIN(bBeenHere, __LINE__) = false; always_checkf(!PREPROCESSOR_JOIN(bBeenHere, __LINE__), "Enclosing block was called more than once."); PREPROCESSOR_JOIN(bBeenHere, __LINE__) = true; }
#define always_check_no_recursion()                 static uint8 PREPROCESSOR_JOIN(RecursionCounter, __LINE__) = 0; always_checkf(PREPROCESSOR_JOIN(RecursionCounter, __LINE__) == 0, "Enclosing block was entered recursively."); const NAMESPACE_REDCRAFT::NAMESPACE_PRIVATE::FRecursionScopeMarker PREPROCESSOR_JOIN(ScopeMarker, __LINE__)(PREPROCESSOR_JOIN(RecursionCounter, __LINE__))
#define always_unimplemented()                      always_checkf(false, "Unimplemented function called.")

#if DO_CHECK

#	define check_code(InCode)                       do InCode while(false)

#	define check(InExpr)                            check_code({ always_check(InExpr); })
#	define checkf(InExpr, InFormat, ...)            always_checkf(InExpr, InFormat, ##__VA_ARGS__)
#	define check_no_entry()                         always_check_no_entry()
#	define check_no_reentry()                       always_check_no_reentry()
#	define check_no_recursion()                     always_check_no_recursion()
#	define verify(InExpr)                           always_check(InExpr)
#	define verifyf(InExpr, InFormat, ...)           always_checkf(InExpr, InFormat, ##__VA_ARGS__)
#	define unimplemented()                          always_unimplemented()

#else

#	define check_code(InCode)

#	define check(InExpr)
#	define checkf(InExpr, InFormat, ...)
#	define check_no_entry()                         { Unreachable(); }
#	define check_no_reentry()
#	define check_no_recursion()
#	define verify(InExpr)                           { if(InExpr) { } }
#	define verifyf(InExpr, InFormat, ...)           { if(InExpr) { } }
#	define unimplemented()                          { Unreachable(); }

#endif

// ReSharper restore CppInconsistentNaming

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
