#pragma once

#include "CoreTypes.h"

#include <cassert>

NS_REDCRAFT_BEGIN

NS_PRIVATE_BEGIN

class FRecursionScopeMarker
{
public:

	FRecursionScopeMarker(uint8& InCounter) : Counter(InCounter) { ++Counter; }
	~FRecursionScopeMarker() { --Counter; }

private:

	uint8& Counter;

};

NS_PRIVATE_END

#if BUILD_DEBUG

	#define check_code(InCode)				do { InCode; } while (false)
	#define check(InExpr)					assert(InExpr)
	#define checkf(InExpr, InFormat, ...)	assert(InExpr)
	#define check_no_entry()				checkf(false, "Enclosing block should never be called.")
	#define check_no_reentry()				{ static bool bBeenHere##__LINE__ = false; checkf(!bBeenHere##__LINE__, "Enclosing block was called more than once."); bBeenHere##__LINE__ = true; }
	#define check_no_recursion()			static uint8 RecursionCounter##__LINE__ = 0; checkf(RecursionCounter##__LINE__ == 0, "Enclosing block was entered recursively."); const NS_PRIVATE::FRecursionScopeMarker ScopeMarker##__LINE__(RecursionCounter##__LINE__)
	#define verify(InExpr)					assert(InExpr)
	#define verifyf(InExpr, InFormat, ...)	assert(InExpr)
	#define unimplemented()					check(false, "Unimplemented function called.")

#else

	#define check_code(...)
	#define check(InExpr)
	#define checkf(InExpr, InFormat, ...)
	#define check_no_entry()
	#define check_no_reentry()
	#define check_no_recursion()
	#define verify(InExpr)					{ if(InExpr) { } }
	#define verifyf(InExpr, InFormat, ...)	{ if(InExpr) { } }
	#define unimplemented()

#endif

NS_REDCRAFT_END
