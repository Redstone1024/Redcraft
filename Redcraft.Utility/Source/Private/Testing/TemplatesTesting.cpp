#include "Testing/TemplatesTesting.h"

#include "Miscellaneous/AssertionMacros.h"
#include "Miscellaneous/Compare.h"
#include "Templates/Templates.h"

#pragma warning(disable : 4930)
#pragma warning(disable : 4101)
#pragma warning(disable : 4244)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestTemplates()
{
	TestInvoke();
	TestReferenceWrapper();
	TestOptional();
	TestVariant();
	TestAny();
	TestMiscTemplates();
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

void TestOptional()
{
	TOptional<int32> TempA;
	TOptional<int32> TempB(Invalid);
	TOptional<int32> TempC(InPlace, 0);
	TOptional<int32> TempD(0);
	TOptional<int32> TempE(0l);
	TOptional<int32> TempF(0.0);
	TOptional<int32> TempG(TempA);
	TOptional<int32> TempH(TempD);
	TOptional<int32> TempI(MakeOptional<int32>(0));
	TOptional<int32> TempJ(MakeOptional<int32>(Invalid));

	TOptional<int32> TempK, TempL, TempM, TempN;
	TempK = TempA;
	TempL = TempD;
	TempM = MakeOptional<int32>(0);
	TempN = MakeOptional<int32>(Invalid);

	*TempL = 303;
	*TempM = 404;

	TOptional<int32> TempO;
	TempO.Emplace(404);

	always_check(TempO);
	always_check(TempO.IsValid());

	always_check(*TempO == 404);
	always_check(TempO.GetValue() == 404);
	always_check(TempO.Get(500) == 404);

	TempO.Reset();
	always_check(TempO == TempO);
	always_check(!(TempO != TempO));
	always_check(TempO.Get(500) == 500);

	int32 TempP = 200;
	TempO = TempP;
	TempO = 300;

	always_check(TempO != TempA);
	always_check(TempO != TempD);
	always_check(TempO == TempO);
	always_check(TempO == 300);
	always_check(300 == TempO);

	int16 TempQ = 1024;
	TOptional<int16> TempR = TempQ;

	TOptional<int32> TempS(InPlace, TempQ);
	TOptional<int32> TempT(TempQ);
	TOptional<int32> TempU(TempR);
	TOptional<int32> TempV(MakeOptional<int16>(2048));

	TOptional<int32> TempW, TempX, TempY;
	TempW = TempQ;
	TempX = TempR;
	TempY = MakeOptional<int16>(2048);

	struct FTracker
	{
		FTracker() { }
		FTracker(const FTracker& InValue) { always_check_no_entry(); }
		FTracker(FTracker&& InValue) { }
		FTracker& operator=(const FTracker& InValue) { always_check_no_entry(); return *this; }
		FTracker& operator=(FTracker&& InValue) { return *this; }
	};

	TOptional<FTracker> TempZ(MakeOptional<FTracker>());
	TempZ = MakeOptional<FTracker>();
	TempZ = FTracker();
}

void TestVariant()
{
	TVariant<int32> TempA;
	TVariant<int32> TempB(Invalid);
	TVariant<int32> TempC(InPlaceType<int32>, 0);
	TVariant<int32> TempD(0);
	TVariant<int32> TempE(0l);
	TVariant<int32> TempF(0.0);
	TVariant<int32> TempG(TempA);
	TVariant<int32> TempH(TempD);
	TVariant<int32> TempI(TVariant<int32>(0));
	TVariant<int32> TempJ(TVariant<int32>(Invalid));

	TVariant<int32> TempK, TempL, TempM, TempN;
	TempK = TempA;
	TempL = TempD;
	TempM = TVariant<int32>(0);
	TempN = TVariant<int32>(Invalid);

	TempL = 303;
	TempM = 404;

	TVariant<int32> TempO;
	TempO.Emplace<int32>(202);
	TempO.Emplace<0>(404);

	always_check(TempO);
	always_check(TempO.IsValid());

	always_check(TempO == 404);
	always_check(TempO.GetValue<int32>() == 404);
	always_check(TempO.Get<0>(500) == 404);

	TempO.Reset();
	always_check(TempO == TempO);
	always_check(!(TempO != TempO));
	always_check(TempO.Get<int32>(500) == 500);

	int32 TempP = 200;
	TempO = TempP;
	TempO = 300;

	always_check(TempO != TempA);
	always_check(TempO != TempD);
	always_check(TempO == TempO);
	always_check(TempO == 300);
	always_check(300 == TempO);

	Swap(TempD, TempA);

	int16 TempQ = 1024;
	TVariant<int16, int32> TempR = TempQ;

	TVariant<int16, int32> TempS(InPlaceType<int32>, TempQ);
	TVariant<int16, int32> TempT(TempQ);
	TVariant<int16, int32> TempU(TempR);
	TVariant<int16, int32> TempV(TVariant<int16, int32>(2048));

	TVariant<int16, int32> TempW, TempX, TempY;
	TempW = TempQ;
	TempX = TempR;
	TempY = TVariant<int16, int32>(2048);

	Swap(TempW, TempX);
	Swap(TempW, TempX);

	struct FTracker
	{
		FTracker() { }
		FTracker(const FTracker& InValue) { always_check_no_entry(); }
		FTracker(FTracker&& InValue) { }
		FTracker& operator=(const FTracker& InValue) { always_check_no_entry(); return *this; }
		FTracker& operator=(FTracker&& InValue) { return *this; }
	};

	TVariant<FTracker> TempZ(Invalid);
	TempZ = TVariant<FTracker>();
	TempZ = FTracker();

	always_check((TIsSame<int32, TVariantAlternativeType<0, TVariant<int32, float>>::Type>::Value));
	always_check((TIsSame<float, TVariantAlternativeType<1, TVariant<int32, float>>::Type>::Value));
	always_check((TIsSame<const int32, TVariantAlternativeType<0, const TVariant<int32, float>>::Type>::Value));

	always_check((TVariantAlternativeIndex<int32, TVariant<int32, float>>::Value == 0));
	always_check((TVariantAlternativeIndex<float, TVariant<int32, float>>::Value == 1));

	bool bIsConst;
	bool bIsLValue;
	bool bIsRValue;

	auto TestQualifiers = [&bIsConst, &bIsLValue, &bIsRValue](auto&& Arg) -> int32
	{
		using T = decltype(Arg);
		always_check(Arg                                                 == 10);
		always_check(TIsConst<typename TRemoveReference<T>::Type>::Value == bIsConst);
		always_check(TIsLValueReference<T>::Value                        == bIsLValue);
		always_check(TIsRValueReference<T>::Value                        == bIsRValue);
		return 0;
	};

	bIsConst  = false;
	bIsLValue = true;
	bIsRValue = false;

	TVariant<int32> TempLA = 10;
	auto ReturnLA = TempLA.Visit(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnLA)>::Value));

	bIsConst  = true;
	bIsLValue = true;
	bIsRValue = false;

	const TVariant<int32> TempLB = TempLA;
	auto ReturnLB = TempLB.Visit(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnLB)>::Value));
	
	bIsConst  = false;
	bIsLValue = false;
	bIsRValue = true;

	TVariant<int32> TempRA = 10;
	auto ReturnRA = MoveTemp(TempRA).Visit(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnRA)>::Value));
	
	bIsConst  = true;
	bIsLValue = false;
	bIsRValue = true;

	const TVariant<int32> TempRB = TempLA;
	auto ReturnRB = MoveTemp(TempRB).Visit(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnRB)>::Value));

	bIsConst = false;
	bIsLValue = true;
	bIsRValue = false;

	TVariant<int32> TempLC = 10;
	auto ReturnLC = TempLC.Visit<int32>(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnLC)>::Value));

	bIsConst = true;
	bIsLValue = true;
	bIsRValue = false;

	const TVariant<int32> TempLD = TempLC;
	auto ReturnLD = TempLD.Visit<int32>(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnLD)>::Value));

	bIsConst = false;
	bIsLValue = false;
	bIsRValue = true;
	
	TVariant<int32> TempRC = 10;
	auto ReturnRC = MoveTemp(TempRC).Visit<int32>(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnRC)>::Value));
	
	bIsConst = true;
	bIsLValue = false;
	bIsRValue = true;

	const TVariant<int32> TempRD = TempLC;
	auto ReturnRD = MoveTemp(TempRD).Visit<int32>(TestQualifiers);
	always_check((TIsSame<int32, decltype(ReturnRD)>::Value));

}

void TestAny()
{
	struct FIntegral
	{
		int32 A;
		FIntegral() { }
		FIntegral(int32 InA) : A(InA) { }
		bool operator==(FIntegral RHS) const { return A == RHS.A; }
	};

	struct FFloating
	{
		double A;
		uint8 Pad[64];
		FFloating() { }
		FFloating(double InA) : A(InA) { }
		bool operator==(FFloating RHS) const { return A == RHS.A; }
	};

	struct FTracker
	{
		FTracker() { }
		FTracker(const FTracker& InValue) { always_check_no_entry(); }
		FTracker(FTracker&& InValue) { }
		FTracker& operator=(const FTracker& InValue) { always_check_no_entry(); return *this; }
		FTracker& operator=(FTracker&& InValue) { return *this; }
	};

	{
		FAny TempA;
		FAny TempB(Invalid);
		FAny TempC(0);
		FAny TempD(InPlaceType<int32>, 0);
		FAny TempG(TempA);
		FAny TempH(TempC);

		FAny TempK, TempL, TempM, TempN;
		TempK = TempA;
		TempL = TempD;
		TempM = FAny(0);
		TempN = FAny(Invalid);

		TempL = 303;
		TempM = 404;

		FAny TempO;
		TempO.Emplace<int32>(202);
		TempO.Emplace<int32>(404);

		always_check(TempO);
		always_check(TempO.IsValid());

		always_check(TempO == 404);
		always_check(TempO.GetValue<int32>() == 404);
		always_check(TempO.Get<int32>(500) == 404);

		TempO.Reset();
		always_check(TempO.Get<int32>(500) == 500);

		int32 TempP = 200;
		TempO = TempP;
		TempO = 300;

		always_check(TempO == 300);
		always_check(300 == TempO);

		Swap(TempD, TempA);

		always_check(!TempD.IsValid());
		always_check(0 == TempA);
	}

	{
		FAny TempA;
		FAny TempB(Invalid);
		FAny TempC(FIntegral(0));
		FAny TempD(InPlaceType<FIntegral>, 0);
		FAny TempG(TempA);
		FAny TempH(TempC);

		FAny TempK, TempL, TempM, TempN;
		TempK = TempA;
		TempL = TempD;
		TempM = FAny(FIntegral(0));
		TempN = FAny(Invalid);

		TempL = FIntegral(303);
		TempM = FIntegral(404);

		FAny TempO;
		TempO.Emplace<FIntegral>(202);
		TempO.Emplace<FIntegral>(404);

		always_check(TempO);
		always_check(TempO.IsValid());

		always_check(TempO == FIntegral(404));
		always_check(TempO.GetValue<FIntegral>() == FIntegral(404));
		always_check(TempO.Get<FIntegral>(500) == FIntegral(404));

		TempO.Reset();
		always_check(TempO.Get<FIntegral>(500) == FIntegral(500));

		FIntegral TempP = FIntegral(200);
		TempO = TempP;
		TempO = FIntegral(300);

		always_check(TempO == FIntegral(300));
		always_check(FIntegral(300) == TempO);

		Swap(TempD, TempA);

		always_check(!TempD.IsValid());
		always_check(FIntegral(0) == TempA);
	}

	{
		FAny TempA;
		FAny TempB(Invalid);
		FAny TempC(FFloating(0.0));
		FAny TempD(InPlaceType<FFloating>, 0.0);
		FAny TempG(TempA);
		FAny TempH(TempC);

		FAny TempK, TempL, TempM, TempN;
		TempK = TempA;
		TempL = TempD;
		TempM = FAny(FFloating(0.0));
		TempN = FAny(Invalid);

		TempL = FFloating(303.0);
		TempM = FFloating(404.0);

		FAny TempO;
		TempO.Emplace<FFloating>(202.0);
		TempO.Emplace<FFloating>(404.0);

		always_check(TempO);
		always_check(TempO.IsValid());

		always_check(TempO == FFloating(404.0));
		always_check(TempO.GetValue<FFloating>() == FFloating(404.0));
		always_check(TempO.Get<FFloating>(500.0) == FFloating(404.0));

		TempO.Reset();
		always_check(TempO.Get<FFloating>(500.0) == FFloating(500.0));

		FFloating TempP = FFloating(200.0);
		TempO = TempP;
		TempO = FFloating(300.0);

		always_check(TempO == FFloating(300.0));
		always_check(FFloating(300.0) == TempO);

		Swap(TempD, TempA);

		always_check(!TempD.IsValid());
		always_check(FFloating(0.0) == TempA);
	}

	{
		FAny TempA;
		FAny TempB(InPlaceType<int32>, 0);
		FAny TempC(InPlaceType<FIntegral>, 0);
		FAny TempD(InPlaceType<FFloating>, 0.0);
		FAny TempE(InPlaceType<FTracker>);

		Swap(TempA, TempB);
		Swap(TempA, TempC);
		Swap(TempA, TempD);
		Swap(TempA, TempE);

		Swap(TempB, TempA);
		Swap(TempB, TempC);
		Swap(TempB, TempD);
		Swap(TempB, TempE);

		Swap(TempC, TempA);
		Swap(TempC, TempB);
		Swap(TempC, TempD);
		Swap(TempC, TempE);

		Swap(TempD, TempA);
		Swap(TempD, TempB);
		Swap(TempD, TempC);
		Swap(TempD, TempE);

		Swap(TempE, TempA);
		Swap(TempE, TempB);
		Swap(TempE, TempC);
		Swap(TempE, TempD);

		always_check(TempA == FIntegral(0));
		always_check(TempB == FFloating(0.0));
		always_check(TempC.HoldsAlternative<FTracker>());
		always_check(TempD == Invalid);
		always_check(TempE == int32(0));

		FAny TempZ(Invalid);
		TempZ = FAny();
		TempZ = FTracker();
	}
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

void TestMiscTemplates()
{
	TTestStructA<int32> ObjectA(new int32(3));
	always_check(TestFunctionB(&ObjectA) == 1);
	always_check(TestFunctionB(AddressOf(ObjectA)) == 0);
	always_check(AddressOf(TestMiscTemplates) == &TestMiscTemplates);
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
