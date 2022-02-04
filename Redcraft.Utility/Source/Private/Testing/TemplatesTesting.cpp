#include "Testing/TemplatesTesting.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Templates.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

void TestTemplates()
{
	TestInvoke();
}

NAMESPACE_UNNAMED_BEGIN

int32 TestFunctionA(int32 A, int32 B, int32 C)
{
	return A + B + C;
}

struct FTestStructA
{
	int32 Num;
	FTestStructA(int32 InNum) : Num(InNum) { }
	int32 Add(int32 A) const { return Num + A; }
};

NAMESPACE_UNNAMED_END

void TestInvoke()
{
	Invoke([=]() { });
	FTestStructA TempA(123);
	always_check(Invoke(TestFunctionA, 1, 2, 3) == 6);
	always_check(Invoke(&FTestStructA::Add, TempA, 1) == 124);
	always_check(Invoke(&FTestStructA::Add, &TempA, 1) == 124);
	int32 TempB = Invoke(&FTestStructA::Num, &TempA);
	int32 TempC = Invoke(&FTestStructA::Num, TempA);
	always_check(TempB == 123);
	always_check(TempC == 123);
	int64 TempD = InvokeResult<int64>(&FTestStructA::Num, &TempA);
	int64 TempE = InvokeResult<int64>(&FTestStructA::Num, TempA);
	always_check(TempD == 123);
	always_check(TempE == 123);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
