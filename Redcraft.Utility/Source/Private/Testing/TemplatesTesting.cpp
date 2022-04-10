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
	TestTuple();
	TestFunction();
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

	always_check((TIsSame<int32,  TUnwrapRefDecay<int32>::Type>::Value));
	always_check((TIsSame<int32&, TUnwrapRefDecay<TReferenceWrapper<int32>>::Type>::Value));
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

	always_check(GetTypeHash(MakeOptional<int32>(114)) == GetTypeHash(MakeOptional<int32>(114)));
	always_check(GetTypeHash(MakeOptional<int32>(114)) != GetTypeHash(MakeOptional<int32>(514)));
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

	always_check(GetTypeHash(TVariant<int32, float>(114)) == GetTypeHash(TVariant<int32, float>(114)));
	always_check(GetTypeHash(TVariant<int32, float>(114)) != GetTypeHash(TVariant<int32, float>(514)));
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

void TestTuple()
{
	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>::Value));

	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32, char>&&>().GetValue<0>()),                int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	
	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>::Value));

	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>::Value));

	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>::Value));

	always_check((TIsSame<decltype(DeclVal<               TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<               TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const          TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<      volatile TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>::Value));
	always_check((TIsSame<decltype(DeclVal<const volatile TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>::Value));

	always_check((TIsSame<TTupleElementType<0,                TTuple<double, float&, char&&>>::Type,                double>::Value));
	always_check((TIsSame<TTupleElementType<1,                TTuple<double, float&, char&&>>::Type,                float&>::Value));
	always_check((TIsSame<TTupleElementType<2,                TTuple<double, float&, char&&>>::Type,                char&&>::Value));
	always_check((TIsSame<TTupleElementType<0, const          TTuple<double, float&, char&&>>::Type, const          double>::Value));
	always_check((TIsSame<TTupleElementType<1, const          TTuple<double, float&, char&&>>::Type, const          float&>::Value));
	always_check((TIsSame<TTupleElementType<2, const          TTuple<double, float&, char&&>>::Type, const          char&&>::Value));
	always_check((TIsSame<TTupleElementType<0,       volatile TTuple<double, float&, char&&>>::Type,       volatile double>::Value));
	always_check((TIsSame<TTupleElementType<1,       volatile TTuple<double, float&, char&&>>::Type,       volatile float&>::Value));
	always_check((TIsSame<TTupleElementType<2,       volatile TTuple<double, float&, char&&>>::Type,       volatile char&&>::Value));
	always_check((TIsSame<TTupleElementType<0, const volatile TTuple<double, float&, char&&>>::Type, const volatile double>::Value));
	always_check((TIsSame<TTupleElementType<1, const volatile TTuple<double, float&, char&&>>::Type, const volatile float&>::Value));
	always_check((TIsSame<TTupleElementType<2, const volatile TTuple<double, float&, char&&>>::Type, const volatile char&&>::Value));
	
	always_check((TTupleElementIndex<double,                TTuple<double, float&, char&&>>::Value == 0));
	always_check((TTupleElementIndex<float&,                TTuple<double, float&, char&&>>::Value == 1));
	always_check((TTupleElementIndex<char&&,                TTuple<double, float&, char&&>>::Value == 2));
	always_check((TTupleElementIndex<double, const          TTuple<double, float&, char&&>>::Value == 0));
	always_check((TTupleElementIndex<float&, const          TTuple<double, float&, char&&>>::Value == 1));
	always_check((TTupleElementIndex<char&&, const          TTuple<double, float&, char&&>>::Value == 2));
	always_check((TTupleElementIndex<double,       volatile TTuple<double, float&, char&&>>::Value == 0));
	always_check((TTupleElementIndex<float&,       volatile TTuple<double, float&, char&&>>::Value == 1));
	always_check((TTupleElementIndex<char&&,       volatile TTuple<double, float&, char&&>>::Value == 2));
	always_check((TTupleElementIndex<double, const volatile TTuple<double, float&, char&&>>::Value == 0));
	always_check((TTupleElementIndex<float&, const volatile TTuple<double, float&, char&&>>::Value == 1));
	always_check((TTupleElementIndex<char&&, const volatile TTuple<double, float&, char&&>>::Value == 2));

	always_check((TTupleElementIndex<int32, TTuple<double, float&, char&&>>::Value == INDEX_NONE));

//	always_check((TIsSame<TTupleElementType<0, int32>::Type, double>::Value));

//	always_check((TTupleElementIndex<int32, int32>::Value == 0));

//	always_check((TIsSame<TTupleElementType<4, TTuple<double, float&, char&&>>::Type, double>::Value));

	{
		using Type = TTuple<int8, uint8, int16, uint16, int32, uint32, int64, uint64, int8, uint8, int16, uint16, int32, uint32, int64, uint64,
			int8, uint8, int16, uint16, int32, uint32, int64, uint64, int8, uint8, int16, uint16, int32, uint32, int64, uint64,
			int8, uint8, int16, uint16, int32, uint32, int64, uint64, int8, uint8, int16, uint16, int32, uint32, int64, uint64>;

		Type Temp;

		Temp.First = 0;
		Temp.Second = 0;
		Temp.Third = 0;
		Temp.Fourth = 0;
		Temp.Fifth = 0;
		Temp.Sixth = 0;
		Temp.Seventh = 0;
		Temp.Eighth = 0;
		Temp.Ninth = 0;
		Temp.Tenth = 0;
		Temp.Eleventh = 0;
		Temp.Twelfth = 0;
		Temp.Thirteenth = 0;
		Temp.Fourteenth = 0;
		Temp.Fifteenth = 0;
		Temp.Sixteenth = 0;
		
		always_check(TIsDefaultConstructible<Type>::Value);
		always_check(TIsTriviallyDefaultConstructible<Type>::Value);
		always_check(TIsConstructible<Type>::Value);
		always_check(TIsTriviallyConstructible<Type>::Value);
		always_check(TIsCopyConstructible<Type>::Value);
		always_check(TIsTriviallyCopyConstructible<Type>::Value);
		always_check(TIsMoveConstructible<Type>::Value);
		always_check(TIsTriviallyMoveConstructible<Type>::Value);
		always_check(TIsCopyAssignable<Type>::Value);
		always_check(TIsTriviallyCopyAssignable<Type>::Value);
		always_check(TIsMoveAssignable<Type>::Value);
		always_check(TIsTriviallyMoveAssignable<Type>::Value);
		always_check(TIsDestructible<Type>::Value);
		always_check(TIsTriviallyDestructible<Type>::Value);
	}

	{
		TTuple<int32, int32> TempA(0, 1);
		TTuple<int32, int32> TempB = { 0, 1 };
		TTuple<int64, double> TempC = TempB;
		TTuple<int64, double> TempD = MoveTemp(TempB);
		TTuple<double, int64> TempE, TempF;
		TempE = TempC;
		TempF = MoveTemp(TempD);
		always_check(TempC.GetValue<0>() == 0);
		always_check(TempC.GetValue<int64>() == 0);
	}

	{
		TTuple TempA = MakeTuple(1, 2, 3);
		int32 TempB;
		Tie(Ignore, TempB, Ignore) = TempA;
		always_check(TempB == 2);
		TTuple TempC = ForwardAsTuple(TempB);
		TempC.GetValue<0>() = 4;
		always_check(TempB == 4);
	}

	struct FTracker
	{
		int8 Flag;
		FTracker(int8 InFlag) : Flag(InFlag) { }
		FTracker(const FTracker& InValue) { Flag = InValue.Flag - 1; always_check(!Flag); }
		FTracker(FTracker&& InValue) { Flag = InValue.Flag + 1; always_check(!Flag); }
		FTracker& operator=(const FTracker& InValue) { Flag = InValue.Flag - 1; always_check(!Flag); return *this; }
		FTracker& operator=(FTracker&& InValue) { Flag = InValue.Flag + 1; always_check(!Flag); return *this; }
	};

	{
		TTuple<int32, FTracker> TempA(404, -1);
		TTuple<double, FTracker> TempB(3.14, 1);
		TTuple<float, FTracker> TempC(1.42f, -1);
		TTuple<> TempD = { };
		auto TempE = TupleCat(MoveTemp(TempA), TempB, MoveTemp(TempC), MoveTemp(TempD));
		always_check(TempE.GetValue<int32>() == 404);
		always_check(TempE.GetValue<double>() == 3.14);
		always_check(TempE.GetValue<float>() == 1.42f);
		always_check((TIsSame<decltype(TempE), TTuple<int32, FTracker, double, FTracker, float, FTracker>>::Value));
		always_check((TIsSame<decltype(TempE), typename TTupleCatResult<TTuple<int32, FTracker>, TTuple<double, FTracker>, TTuple<float, FTracker>>::Type>::Value));
	}

	{
		always_check(MakeTuple(10, 0.0) == MakeTuple(10.0, 0));
		always_check(MakeTuple(10, 0.0) != MakeTuple(10.1, 0));

		always_check((MakeTuple(10, 0.0) <=> MakeTuple(10.0, 0)) == 0);
		always_check((MakeTuple(10, 1.0) <=> MakeTuple(10.0, 0)) >  0);
		always_check((MakeTuple(10, 0.0) <=> MakeTuple(10.1, 0)) <  0);
		always_check((MakeTuple(10, 0.0) <=> MakeTuple(10.1, 0)) != 0);
	}

	{
		double TempB = 0.0;
		TTuple<int32, double&> TempC(10, TempB);
		int16 TempD = 10;
		FTracker TempE(0);
		TTuple<int16&, FTracker&&> TempF(TempD, MoveTemp(TempE));
		auto TempG = TupleCat(TempC, TempF);
		TempG.GetValue<1>() = 3.14;
		always_check(TempB == 3.14);
		always_check(TempG.GetValue<0>() == 10);
		always_check(TempG.GetValue<2>() == 10);
		always_check((TIsSame<decltype(TempG), TTuple<int32, double&, int16&, FTracker&&>>::Value));
		always_check((TIsSame<decltype(TempG), typename TTupleCatResult<TTuple<int32, double&>, TTuple<int16&, FTracker&&>>::Type>::Value));
	}

	{
		int32 TempO = 15;
		TTuple<int32&&, const int64> TempA = { MoveTemp(TempO), 514 };
		
		TempA.Apply(
			[]<typename T, typename U> (T&& A, U&& B)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check((TIsSame<T&&, int32&>::Value));
				always_check((TIsSame<U&&, const int64&>::Value));
			}
		);
		
		MoveTemp(TempA).Apply(
			[]<typename T, typename U> (T&& A, U&& B)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check((TIsSame<T&&, int32&&>::Value));
				always_check((TIsSame<U&&, const int64&&>::Value));
			}
		);
		
		TempA.ApplyAfter(
			[]<typename T, typename U, typename V> (T&& A, U&& B, V&&C)
			{
				always_check(A == '-');
				always_check(B == 15);
				always_check(C == 514);
				always_check((TIsSame<T&&, char&&>::Value));
				always_check((TIsSame<U&&, int32&>::Value));
				always_check((TIsSame<V&&, const int64&>::Value));
			},
			'-'
		);
		
		MoveTemp(TempA).ApplyAfter(
			[]<typename T, typename U, typename V> (T&& A, U&& B, V&&C)
			{
				always_check(A == '-');
				always_check(B == 15);
				always_check(C == 514);
				always_check((TIsSame<T&&, char&&>::Value));
				always_check((TIsSame<U&&, int32&&>::Value));
				always_check((TIsSame<V&&, const int64&&>::Value));
			},
			'-'
		);
		
		TempA.ApplyBefore(
			[]<typename T, typename U, typename V> (T&& A, U&& B, V&&C)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check(C == '-');
				always_check((TIsSame<T&&, int32&>::Value));
				always_check((TIsSame<U&&, const int64&>::Value));
				always_check((TIsSame<V&&, char&&>::Value));
			},
			'-'
		);
		
		MoveTemp(TempA).ApplyBefore(
			[]<typename T, typename U, typename V> (T&& A, U&& B, V&&C)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check(C == '-');
				always_check((TIsSame<T&&, int32&&>::Value));
				always_check((TIsSame<U&&, const int64&&>::Value));
				always_check((TIsSame<V&&, char&&>::Value));
			},
			'-'
		);
	}

	{
		TTuple<int32, char> TempA = { 1, 'A' };
		TTuple<int32, char> TempB = TempA.Transform([](auto&& InValue) { return InValue + 1; });

		VisitTuple(
			[]<typename T> (T&& A)
			{
				if constexpr (TIsSame<T&&, int32&>::Value) always_check(A == 2);
				else if constexpr (TIsSame<T&&, char&>::Value) always_check(A == 'B');
				else always_check_no_entry();
			},
			TempB
		);

		VisitTuple([](auto&& A) { A++; }, TempB);
		
		VisitTuple(
			[]<typename T> (T&& A)
			{
				if constexpr (TIsSame<T&&, int32&>::Value) always_check(A == 3);
				else if constexpr (TIsSame<T&&, char&>::Value) always_check(A == 'C');
				else always_check_no_entry();
			},
			TempB
		);
	}

	{
		struct FTest
		{
			FTest(int32 A, float B, char C)
			{
				always_check(A == 1);
				always_check(B == 1.2f);
				always_check(C == 'A');
			}
		};
		MakeTuple(1, 1.2f, 'A').Construct<FTest>();
	}

	{
		auto Func = [] { return MakeTuple(1, 2.3, 'A'); };
		auto [A, B, C] = Func();
		always_check(A == 1);
		always_check(B == 2.3);
		always_check(C == 'A');
		always_check((TIsSame<decltype(C), char>::Value));
	}

	always_check(GetTypeHash(MakeTuple(114, 1.0f)) == GetTypeHash(MakeTuple(114, 1.0f)));
	always_check(GetTypeHash(MakeTuple(114, 1.0f)) != GetTypeHash(MakeTuple(514, 1.0f)));
}

NAMESPACE_UNNAMED_BEGIN

struct FFunctionDebug
{
	int32 Index = 0;
	int32 Output[12];
	void Print(int32 In) { Output[Index++] = In; }
};

FFunctionDebug FunctionDebug;

struct FPrintAdd
{
	FPrintAdd(int32 InNum) : Num(InNum) { }
	void F(int32 I) const { FunctionDebug.Print(Num + I); }
	int32 Num;
};

void PrintNum(int32 I)
{
	FunctionDebug.Print(I);
}

struct FPrintNum
{
	void operator()(int32 I) const
	{
		FunctionDebug.Print(I);
	}
};

NAMESPACE_UNNAMED_END

void TestFunction()
{
	{
//		TFunctionRef<void()> TempA;
		TFunction<void()> TempB;
		TUniqueFunction<void()> TempC;
	}

	{
		struct FFunctor
		{
			int32 operator()() &       { return 0; }
			int32 operator()() &&      { return 1; }
			int32 operator()() const&  { return 2; }
			int32 operator()() const&& { return 3; }
		};

		FFunctor Functor;

		TFunctionRef<int32()        > TempA = Functor;
		TFunctionRef<int32() &      > TempB = Functor;
		TFunctionRef<int32() &&     > TempC = Functor;
		TFunctionRef<int32() const  > TempD = Functor;
		TFunctionRef<int32() const& > TempE = Functor;
		TFunctionRef<int32() const&&> TempF = Functor;

		always_check(         TempA()  == 0);
		always_check(         TempB()  == 0);
		always_check(MoveTemp(TempC)() == 1);
		always_check(         TempD()  == 2);
		always_check(         TempE()  == 2);
		always_check(MoveTemp(TempF)() == 3);
	}

	{
		int32 Offset = 0xFA00;
		auto FuncA = [&Offset](int32 In) { return In + Offset; };

		TFunctionRef<int32(int32)> TempA = FuncA;
		Offset = 0xFB00;
		always_check(TempA(0xAA) == 0xFBAA);

		TFunction<int32(int32)> TempB = FuncA;
		Offset = 0xFC00;
		always_check(TempB(0xAB) == 0xFCAB);

		TUniqueFunction<int32(int32)> TempC = FuncA;
		Offset = 0xFD00;
		always_check(TempC(0xAC) == 0xFDAC);
	}

	{
		struct FFunctor
		{
			int32 A;
			FFunctor(int32 InA) : A(InA) { }
			int32 operator()() const { return A; }
		};

		TFunction<void()> TempA = FFunctor(0xAA);
		TFunction<void()> TempB(InPlaceType<FFunctor>, 0xBB);

		TempA();
		TempB();

		TFunction<int32()> TempC = FFunctor(0xAA);
		TFunction<int32()> TempD(InPlaceType<FFunctor>, 0xBB);

		always_check(TempC() == 0xAA);
		always_check(TempD() == 0xBB);

		TempA = nullptr;
		TempB = nullptr;

		always_check(!TempA.IsValid());
		always_check(!TempB.IsValid());

		TempA = FFunctor(0xCC);
		TempB.Emplace<FFunctor>(0xDD);

		always_check(TempA.IsValid());
		always_check(TempB.IsValid());

		TempA();
		TempB();

		TempC.Reset();
		TempD.Reset();

		always_check(!TempC.IsValid());
		always_check(!TempD.IsValid());

		TempC = FFunctor(0xEE);
		TempD.Emplace<FFunctor>(0xFF);

		always_check(TempC.IsValid());
		always_check(TempD.IsValid());

		always_check(TempC() == 0xEE);
		always_check(TempD() == 0xFF);

		always_check(TempC.TargetType() == Typeid(FFunctor));
		always_check(TempD.TargetType() == Typeid(FFunctor));
	}

	{
		TFunctionRef<void()>       RefA = [] { };
		TFunction<void()>       ObjectA = [] { };
		TUniqueFunction<void()> UniqueA = [] { };

		TFunctionRef<void()>       RefB = RefA;
//		TFunction<void()>       ObjectB = RefA;
//		TUniqueFunction<void()> UniqueB = RefA;

		TFunctionRef<void()>       RefC = ObjectA;
		TFunction<void()>       ObjectC = ObjectA;
		TUniqueFunction<void()> UniqueC = ObjectA;

		TFunctionRef<void()>       RefD = UniqueA;
//		TFunction<void()>       ObjectD = UniqueA;
//		TUniqueFunction<void()> UniqueD = UniqueA;

		TFunctionRef<void()>       RefE = MoveTemp(RefA);
//		TFunction<void()>       ObjectE = MoveTemp(RefA);
//		TUniqueFunction<void()> UniqueE = MoveTemp(RefA);

		TFunctionRef<void()>       RefF = MoveTemp(ObjectA);
		TFunction<void()>       ObjectF = MoveTemp(ObjectA);
		TUniqueFunction<void()> UniqueF = MoveTemp(ObjectA);

		TFunctionRef<void()>       RefG = MoveTemp(UniqueA);
//		TFunction<void()>       ObjectG = MoveTemp(UniqueA);
		TUniqueFunction<void()> UniqueG = MoveTemp(UniqueA);
	}
	
	{
		TFunctionRef<void()>       RefA = [] { };
		TFunction<void()>       ObjectA = [] { };
		TUniqueFunction<void()> UniqueA = [] { };

//		TFunctionRef<void()>       RefB;    RefB = RefA;
//		TFunction<void()>       ObjectB; ObjectB = RefA;
//		TUniqueFunction<void()> UniqueB; UniqueB = RefA;

//		TFunctionRef<void()>       RefC;    RefC = ObjectA;
		TFunction<void()>       ObjectC; ObjectC = ObjectA;
		TUniqueFunction<void()> UniqueC; UniqueC = ObjectA;

//		TFunctionRef<void()>       RefD;    RefD = UniqueA;
//		TFunction<void()>       ObjectD; ObjectD = UniqueA;
//		TUniqueFunction<void()> UniqueD; UniqueD = UniqueA;
		
//		TFunctionRef<void()>       RefE;    RefE = MoveTemp(RefA);
//		TFunction<void()>       ObjectE; ObjectE = MoveTemp(RefA);
//		TUniqueFunction<void()> UniqueE; UniqueE = MoveTemp(RefA);

//		TFunctionRef<void()>       RefF;    RefF = MoveTemp(ObjectA);
		TFunction<void()>       ObjectF; ObjectF = MoveTemp(ObjectA);
		TUniqueFunction<void()> UniqueF; UniqueF = MoveTemp(ObjectA);

//		TFunctionRef<void()>       RefG;    RefG = MoveTemp(UniqueA);
//		TFunction<void()>       ObjectG; ObjectG = MoveTemp(UniqueA);
		TUniqueFunction<void()> UniqueG; UniqueG = MoveTemp(UniqueA);
	}

	{
		TFunction<void(int32)> Display = PrintNum;
		Display(-9);

		TFunction<void()> Display42 = [] { PrintNum(42); };
		Display42();

		TFunction<void()> Display31337 = [] { PrintNum(31337); };
		Display31337();

		TFunction<void(const FPrintAdd&, int32)> AddDisplay = &FPrintAdd::F;
		const FPrintAdd Foo(314159);
		AddDisplay(Foo, 1);
		AddDisplay(314159, 1);

		TFunction<int32(FPrintAdd const&)> Num = &FPrintAdd::Num;
		FunctionDebug.Print(Num(Foo));

		TFunction<void(int32)> AddDisplay2 = [Foo](int32 A) { Foo.F(A); };
		AddDisplay2(2);

		TFunction<void(int32)> AddDisplay3 = [Ptr = &Foo](int32 A) { Ptr->F(A); };
		AddDisplay3(3);

		TFunction<void(int32)> DisplayObject = FPrintNum();
		DisplayObject(18);

		auto Factorial = [](int32 N) {
			TFunction<int32(int32)> Fac = [&](int32 N) { return (N < 2) ? 1 : N * Fac(N - 1); };
			return Fac(N);
		};

		for (int32 I = 5; I < 8; ++I) FunctionDebug.Print(Factorial(I));

		always_check(FunctionDebug.Index == 12);
		always_check(FunctionDebug.Output[0] == -9);
		always_check(FunctionDebug.Output[1] == 42);
		always_check(FunctionDebug.Output[2] == 31337);
		always_check(FunctionDebug.Output[3] == 314160);
		always_check(FunctionDebug.Output[4] == 314160);
		always_check(FunctionDebug.Output[5] == 314159);
		always_check(FunctionDebug.Output[6] == 314161);
		always_check(FunctionDebug.Output[7] == 314162);
		always_check(FunctionDebug.Output[8] == 18);
		always_check(FunctionDebug.Output[9] == 120);
		always_check(FunctionDebug.Output[10] == 720);
		always_check(FunctionDebug.Output[11] == 5040);
	}

	{
		TFunction<bool(bool)> Identity = TIdentity<>();
		TFunction<bool(bool)> NotIdentity = NotFn(Identity);
	
		always_check(Identity(true));
		always_check(NotIdentity(false));
	}

	{
		always_check(TPromote   <int32>()(4   ) ==  4);
		always_check(TNegate    <int32>()(4   ) == -4);
		always_check(TPlus      <int32>()(4, 2) ==  6);
		always_check(TMinus     <int32>()(4, 2) ==  2);
		always_check(TMultiplies<int32>()(4, 2) ==  8);
		always_check(TDivides   <int32>()(4, 2) ==  2);
		always_check(TModulus   <int32>()(4, 2) ==  0);
		
		always_check(TBitNot<int32>()(4   ) == -5);
		always_check(TBitAnd<int32>()(4, 2) ==  0);
		always_check(TBitOr <int32>()(4, 2) ==  6);
		always_check(TBitXor<int32>()(4, 2) ==  6);
		always_check(TBitLsh<int32>()(4, 2) == 16);
		always_check(TBitRsh<int32>()(4, 2) ==  1);
		
		always_check(TLogicalAnd<int32>()(4, 2) == true);
		always_check(TLogicalOr <int32>()(4, 2) == true);
		always_check(TLogicalNot<int32>()(4   ) == false);

		always_check(TEqualTo     <int32>()(4, 2) == false);
		always_check(TNotEqualTo  <int32>()(4, 2) == true);
		always_check(TGreater     <int32>()(4, 2) == true);
		always_check(TLess        <int32>()(4, 2) == false);
		always_check(TGreaterEqual<int32>()(4, 2) == true);
		always_check(TLessEqual   <int32>()(4, 2) == false);
	}
	
	{
		TFunction<int32(int32, int32)> TempA = TPlus      <>();
		TFunction<int32(int32, int32)> TempB = TMinus     <>();
		TFunction<int32(int32, int32)> TempC = TMultiplies<>();
		TFunction<int32(int32, int32)> TempD = TDivides   <>();
		TFunction<int32(int32, int32)> TempE = TModulus   <>();
		TFunction<int32(int32       )> TempF = TNegate    <>();
		
		always_check(TempA(4, 2) == 6);
		always_check(TempB(4, 2) == 2);
		always_check(TempC(4, 2) == 8);
		always_check(TempD(4, 2) == 2);
		always_check(TempE(4, 2) == 0);
		always_check(TempF(4   ) == -4);
		
		TFunction<bool(int32, int32)> TempG = TEqualTo     <>();
		TFunction<bool(int32, int32)> TempH = TNotEqualTo  <>();
		TFunction<bool(int32, int32)> TempI = TGreater     <>();
		TFunction<bool(int32, int32)> TempJ = TLess        <>();
		TFunction<bool(int32, int32)> TempK = TGreaterEqual<>();
		TFunction<bool(int32, int32)> TempL = TLessEqual   <>();
		
		always_check(TempG(4, 2) == false);
		always_check(TempH(4, 2) == true);
		always_check(TempI(4, 2) == true);
		always_check(TempJ(4, 2) == false);
		always_check(TempK(4, 2) == true);
		always_check(TempL(4, 2) == false);
		
		TFunction<bool(int32, int32)> TempM = TLogicalAnd<>();
		TFunction<bool(int32, int32)> TempN = TLogicalOr <>();
		TFunction<bool(int32       )> TempO = TLogicalNot<>();
	
		always_check(TempM(4, 2) == true);
		always_check(TempN(4, 2) == true);
		always_check(TempO(4   ) == false);
		
		TFunction<int32(int32, int32)> TempP = TBitAnd<>();
		TFunction<int32(int32, int32)> TempQ = TBitOr <>();
		TFunction<int32(int32, int32)> TempR = TBitXor<>();
		TFunction<int32(int32       )> TempS = TBitNot<>();
	
		always_check(TempP(4, 2) == 0);
		always_check(TempQ(4, 2) == 6);
		always_check(TempR(4, 2) == 6);
		always_check(TempS(4   ) == -5);
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
