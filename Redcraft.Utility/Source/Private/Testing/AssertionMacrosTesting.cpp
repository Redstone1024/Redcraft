#include "Testing/AssertionMacrosTesting.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

void TestNoEntry()
{
	check_no_entry();
	always_check_no_entry();
}

void TestNoReentry()
{
	check_no_reentry();
	always_check_no_reentry();
}

void TestNoRecursion(int32 Depth)
{
	if (Depth < 0) return;
	
	check_no_recursion();
	always_check_no_recursion();

	TestNoRecursion(--Depth);
}

void TestUnimplemented()
{
	unimplemented();
	always_unimplemented();
}

void TestAssertionMacros()
{
	check(true);
	//check(false);
	checkf(true, "True!");
	//checkf(false, "False!");

	always_check(true);
	//always_check(false);
	always_checkf(true, "True!");
	//always_checkf(false, "False!");

	//TestNoEntry();

	TestNoReentry();
	//TestNoReentry();

	TestNoRecursion(0);
	TestNoRecursion(0);
	//TestNoRecursion(1);

	//TestUnimplemented();

	verify(true);
	//verify(false);

	int32 A = 1;
	int32 B = 0;
	verify(B = A);
	always_check(B == A);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
