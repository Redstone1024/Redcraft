#include "Testing/TemplatesTesting.h"
#include "Miscellaneous/AssertionMacros.h"
#include "Templates/Templates.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

void TestTemplates()
{
	TestInvoke();
	TestReferenceWrapper();
	TestMiscellaneous();
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

void TestReferenceWrapper()
{
	typedef int32(*FuncType)(int32, int32, int32);
	FuncType TempA = [](int32 A, int32 B, int32 C) -> int32 { return A * B * C; };
	TReferenceWrapper<FuncType> TempB(TempA);
	always_check(TempB(1, 1, 1) == 1);
	TempB.Get() = &TestFunctionA;
	always_check(TempA(1, 1, 1) == 3);
	
	int32 ArrayA[3] = { 1, 2, 3 };
	TReferenceWrapper<int32> ArrayB[3] = { ArrayA[1], ArrayA[0], ArrayA[2] };
	always_check(ArrayB[0] == 2);
	always_check(ArrayB[1] == 1);
	always_check(ArrayB[2] == 3);
	for (int32& Element : ArrayB) Element *= 2;
	always_check(ArrayA[0] == 2);
	always_check(ArrayA[1] == 4);
	always_check(ArrayA[2] == 6);
}

NAMESPACE_UNNAMED_BEGIN

template <typename T>
struct TTestStructA
{
	T* Pad;
	T* Data;

	TTestStructA(T* InData) : Pad(nullptr), Data(InData) { }
	~TTestStructA() { delete Data; }
	T** operator&() { return &Data; }
};

template <typename T>
int32 TestFunctionB(TTestStructA<T>* Ptr)
{
	return 0;
}

template <typename T>
int32 TestFunctionB(T** Ptr)
{
	return 1;
}

NAMESPACE_UNNAMED_END

void TestMiscellaneous()
{
	TTestStructA<int32> ObjectA(new int32(3));
	always_check(TestFunctionB(&ObjectA) == 1);
	always_check(TestFunctionB(AddressOf(ObjectA)) == 0);
	always_check(AddressOf(TestMiscellaneous) == &TestMiscellaneous);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
