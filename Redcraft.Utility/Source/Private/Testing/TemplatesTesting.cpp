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
	TestAtomic();
	TestScopeHelper();
	TestPropagateConst();
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

	always_check((CSameAs<int32,  TUnwrapRefDecay<int32>>));
	always_check((CSameAs<int32&, TUnwrapRefDecay<TReferenceWrapper<int32>>>));
}

void TestOptional()
{
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
		always_check(TempO >= 200);
		always_check(400 >= TempO);

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

	{
		TOptional<uint8> TempA = Invalid;
		TOptional<int16> TempB = 16;
		TOptional<int64> TempC = 32;

		always_check(TempA != TempB);
		always_check(TempB != TempC);
		always_check(TempB <= TempC);
		always_check(TempA <=> TempB == partial_ordering::unordered);
	}

	{
		struct FTest { FTest(initializer_list<int32>, int32) { } };

		TOptional<FTest> Temp(InPlace, { 0, 1, 2 }, 3);
		Temp.Emplace({ 0, 1, 2 }, 3);
	}
}

void TestVariant()
{
	{
		TVariant<int32> TempA;
		TVariant<int32> TempB(Invalid);
		TVariant<int32> TempC(InPlaceType<int32>, 0);
		TVariant<int32> TempD(0);
//		TVariant<int32> TempE(0ll);
//		TVariant<int32> TempF(0.0);
		TVariant<int32> TempG(TempA);
		TVariant<int32> TempH(TempD);
		TVariant<int32> TempI = TVariant<int32>(0);
		TVariant<int32> TempJ = TVariant<int32>(Invalid);

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
		always_check(TempO >= 200);
		always_check(400 >= TempO);

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
	}

	{
		using VariantType = TVariant<int32, int64, float64>;
		VariantType TempArray[] = { 10, 15ll, 1.5 };

		for(auto&& TempA : TempArray)
		{
			Visit(
				[](auto&& A)
				{
					using T = TRemoveCVRef<decltype(A)>;
					if constexpr (CSameAs<T, int32>) always_check(A == 10);
					else if constexpr (CSameAs<T, int64>) always_check(A == 15ll);
					else if constexpr (CSameAs<T, float64>) always_check(A == 1.5);
					else always_check_no_entry();
				},
				TempA
			);

			VariantType TempB = Visit([](auto&& A) -> VariantType { return A + A; }, TempA);

			Visit(
				[](auto&& A, auto&& B)
				{
					using T = TRemoveCVRef<decltype(A)>;
					if constexpr (CSameAs<T, int32>) always_check(A == 10 && B == 20);
					else if constexpr (CSameAs<T, int64>) always_check(A == 15ll && B == 30ll);
					else if constexpr (CSameAs<T, float64>) always_check(A == 1.5 && B == 3.0);
					else always_check_no_entry();
				},
				TempA, TempB
			);

			Visit([](auto&& A) { A *= 2; }, TempA);

			Visit(
				[](auto&& A)
				{
					using T = TRemoveCVRef<decltype(A)>;
					if constexpr (CSameAs<T, int32>) always_check(A == 20);
					else if constexpr (CSameAs<T, int64>) always_check(A == 30ll);
					else if constexpr (CSameAs<T, float64>) always_check(A == 3.0);
					else always_check_no_entry();
				},
				TempA
			);
		}

		for (auto&& TempA : TempArray) {
			Visit(
				TOverloaded
				{
					[](int32 A)   { always_check(A == 20);   },
					[](int64 A)   { always_check(A == 30ll); },
					[](float64 A) { always_check(A == 3.0);  },
					[](auto A)    { always_check_no_entry(); },
				},
				TempA
			);
		}
	}

	{
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

		always_check((CSameAs<int32, TVariantAlternative<0, TVariant<int32, float>>>));
		always_check((CSameAs<float, TVariantAlternative<1, TVariant<int32, float>>>));
		always_check((CSameAs<const int32, TVariantAlternative<0, const TVariant<int32, float>>>));

		always_check((TVariantIndex<int32, TVariant<int32, float>> == 0));
		always_check((TVariantIndex<float, TVariant<int32, float>> == 1));

		bool bIsConst;
		bool bIsLValue;
		bool bIsRValue;

		auto TestQualifiers = [&bIsConst, &bIsLValue, &bIsRValue](auto&& A) -> int32
		{
			using T = decltype(A);
			always_check(A == 10);
			always_check(CConst<TRemoveReference<T>> == bIsConst);
			always_check(CLValueReference<T> == bIsLValue);
			always_check(CRValueReference<T> == bIsRValue);
			return 0;
		};

		bIsConst = false;
		bIsLValue = true;
		bIsRValue = false;

		TVariant<int32> TempLA = 10;
		auto ReturnLA = Visit(TestQualifiers, TempLA);
		always_check((CSameAs<int32, decltype(ReturnLA)>));

		bIsConst = true;
		bIsLValue = true;
		bIsRValue = false;

		const TVariant<int32> TempLB = TempLA;
		auto ReturnLB = Visit(TestQualifiers, TempLB);
		always_check((CSameAs<int32, decltype(ReturnLB)>));

		bIsConst = false;
		bIsLValue = false;
		bIsRValue = true;

		TVariant<int32> TempRA = 10;
		auto ReturnRA = Visit(TestQualifiers, MoveTemp(TempRA));
		always_check((CSameAs<int32, decltype(ReturnRA)>));

		bIsConst = true;
		bIsLValue = false;
		bIsRValue = true;

		const TVariant<int32> TempRB = TempLA;
		auto ReturnRB = Visit(TestQualifiers, MoveTemp(TempRB));
		always_check((CSameAs<int32, decltype(ReturnRB)>));

		bIsConst = false;
		bIsLValue = true;
		bIsRValue = false;

		TVariant<int32> TempLC = 10;
		auto ReturnLC = Visit<int32>(TestQualifiers, TempLC);
		always_check((CSameAs<int32, decltype(ReturnLC)>));

		bIsConst = true;
		bIsLValue = true;
		bIsRValue = false;

		const TVariant<int32> TempLD = TempLC;
		auto ReturnLD = Visit<int32>(TestQualifiers, TempLD);
		always_check((CSameAs<int32, decltype(ReturnLD)>));

		bIsConst = false;
		bIsLValue = false;
		bIsRValue = true;

		TVariant<int32> TempRC = 10;
		auto ReturnRC = Visit<int32>(TestQualifiers, MoveTemp(TempRC));
		always_check((CSameAs<int32, decltype(ReturnRC)>));

		bIsConst = true;
		bIsLValue = false;
		bIsRValue = true;

		const TVariant<int32> TempRD = TempLC;
		auto ReturnRD = Visit<int32>(TestQualifiers, MoveTemp(TempRD));
		always_check((CSameAs<int32, decltype(ReturnRD)>));
	}

	{
		always_check(GetTypeHash(TVariant<int32, float>(114)) == GetTypeHash(TVariant<int32, float>(114)));
		always_check(GetTypeHash(TVariant<int32, float>(114)) != GetTypeHash(TVariant<int32, float>(514)));
	}

	{
		TVariant<uint8, int16, int32> TempA = Invalid;
		TVariant<uint8, int16, int32> TempB = static_cast<int16>(16);
		TVariant<uint8, int16, int32> TempC = static_cast<int32>(16);
		TVariant<uint8, int16, int32> TempD = static_cast<int32>(32);

		always_check(TempA != TempB);
		always_check(TempB != TempC);
		always_check(TempB != TempC);
		always_check(TempD >= TempC);
		always_check(TempA <=> TempB == partial_ordering::unordered);
	}

	{
		struct FTest { FTest(initializer_list<int32>, int32) { } };

		TVariant<FTest> TempA(InPlaceIndex<0>, { 0, 1, 2 }, 3);
		TempA.Emplace<0>({ 0, 1, 2 }, 3);

		TVariant<FTest> TempB(InPlaceType<FTest>, { 0, 1, 2 }, 3);
		TempB.Emplace<FTest>({ 0, 1, 2 }, 3);
	}
}

void TestAny()
{
	struct FIntegral
	{
		int32 A;
		FIntegral() { }
		FIntegral(int32 InA) : A(InA) { }
		bool operator==(FIntegral RHS) const& { return A == RHS.A; }
	};

	struct FFloating
	{
		double A;
		uint8 Pad[64];
		FFloating() { }
		FFloating(double InA) : A(InA) { }
		bool operator==(FFloating RHS) const& { return A == RHS.A; }
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
		always_check(TempO >= 400);
		always_check(500 >= TempO);
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

	{
		struct FTest { FTest(initializer_list<int32>, int32) { } };

		FAny Temp(InPlaceType<FTest>, { 0, 1, 2 }, 3);
		Temp.Emplace<FTest>({ 0, 1, 2 }, 3);
	}
}

void TestTuple()
{
	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32, char>&>().GetValue<0>()), const volatile int32&>));

	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32, char>&&>().GetValue<0>()),                int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32, char>&&>().GetValue<0>()), const volatile int32&&>));

	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32&, char>&>().GetValue<0>()), const volatile int32&>));

	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32&, char>&&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32&, char>&&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32&, char>&&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32&, char>&&>().GetValue<0>()), const volatile int32&>));

	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32&&, char>&>().GetValue<0>()),                int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32&&, char>&>().GetValue<0>()), const          int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32&&, char>&>().GetValue<0>()),       volatile int32&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32&&, char>&>().GetValue<0>()), const volatile int32&>));

	always_check((CSameAs<decltype(DeclVal<               TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<               TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const          TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<      volatile TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<               int32&&, char>&&>().GetValue<0>()),                int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const          int32&&, char>&&>().GetValue<0>()), const          int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<      volatile int32&&, char>&&>().GetValue<0>()),       volatile int32&&>));
	always_check((CSameAs<decltype(DeclVal<const volatile TTuple<const volatile int32&&, char>&&>().GetValue<0>()), const volatile int32&&>));

	always_check((CSameAs<TTupleElement<0,                TTuple<double, float&, char&&>>,                double>));
	always_check((CSameAs<TTupleElement<1,                TTuple<double, float&, char&&>>,                float&>));
	always_check((CSameAs<TTupleElement<2,                TTuple<double, float&, char&&>>,                char&&>));
	always_check((CSameAs<TTupleElement<0, const          TTuple<double, float&, char&&>>, const          double>));
	always_check((CSameAs<TTupleElement<1, const          TTuple<double, float&, char&&>>,                float&>));
	always_check((CSameAs<TTupleElement<2, const          TTuple<double, float&, char&&>>,                char&&>));
	always_check((CSameAs<TTupleElement<0,       volatile TTuple<double, float&, char&&>>,       volatile double>));
	always_check((CSameAs<TTupleElement<1,       volatile TTuple<double, float&, char&&>>,                float&>));
	always_check((CSameAs<TTupleElement<2,       volatile TTuple<double, float&, char&&>>,                char&&>));
	always_check((CSameAs<TTupleElement<0, const volatile TTuple<double, float&, char&&>>, const volatile double>));
	always_check((CSameAs<TTupleElement<1, const volatile TTuple<double, float&, char&&>>,                float&>));
	always_check((CSameAs<TTupleElement<2, const volatile TTuple<double, float&, char&&>>,                char&&>));

	always_check((TTupleIndex<double,                TTuple<double, float&, char&&>> == 0));
	always_check((TTupleIndex<float&,                TTuple<double, float&, char&&>> == 1));
	always_check((TTupleIndex<char&&,                TTuple<double, float&, char&&>> == 2));
	always_check((TTupleIndex<double, const          TTuple<double, float&, char&&>> == 0));
	always_check((TTupleIndex<float&, const          TTuple<double, float&, char&&>> == 1));
	always_check((TTupleIndex<char&&, const          TTuple<double, float&, char&&>> == 2));
	always_check((TTupleIndex<double, volatile       TTuple<double, float&, char&&>> == 0));
	always_check((TTupleIndex<float&, volatile       TTuple<double, float&, char&&>> == 1));
	always_check((TTupleIndex<char&&, volatile       TTuple<double, float&, char&&>> == 2));
	always_check((TTupleIndex<double, const volatile TTuple<double, float&, char&&>> == 0));
	always_check((TTupleIndex<float&, const volatile TTuple<double, float&, char&&>> == 1));
	always_check((TTupleIndex<char&&, const volatile TTuple<double, float&, char&&>> == 2));

//	always_check((CSameAs<TTupleElement<0, int32>, double>));

//	always_check((TTupleIndex<int32, int32> == 0));

//	always_check((CSameAs<TTupleElement<4, TTuple<double, float&, char&&>>, double>));

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

		always_check(CDefaultConstructible<Type>);
		always_check(CTriviallyDefaultConstructible<Type>);
		always_check(CConstructibleFrom<Type>);
		always_check(CTriviallyConstructibleFrom<Type>);
		always_check(CCopyConstructible<Type>);
		always_check(CTriviallyCopyConstructible<Type>);
		always_check(CMoveConstructible<Type>);
		always_check(CTriviallyMoveConstructible<Type>);
		always_check(CCopyAssignable<Type>);
		always_check(CTriviallyCopyAssignable<Type>);
		always_check(CMoveAssignable<Type>);
		always_check(CTriviallyMoveAssignable<Type>);
		always_check(CDestructible<Type>);
		always_check(CTriviallyDestructible<Type>);
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
		FTracker(const FTracker& InValue)            { Flag = InValue.Flag - 1; always_check(!Flag);               }
		FTracker(FTracker&& InValue)                 { Flag = InValue.Flag + 1; always_check(!Flag);               }
		FTracker& operator=(const FTracker& InValue) { Flag = InValue.Flag - 1; always_check(!Flag); return *this; }
		FTracker& operator=(FTracker&& InValue)      { Flag = InValue.Flag + 1; always_check(!Flag); return *this; }
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
		always_check((CSameAs<decltype(TempE), TTuple<int32, FTracker, double, FTracker, float, FTracker>>));
		always_check((CSameAs<decltype(TempE), TTupleCatResult<TTuple<int32, FTracker>, TTuple<double, FTracker>, TTuple<float, FTracker>>>));
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
		always_check((CSameAs<decltype(TempG), TTuple<int32, double&, int16&, FTracker&&>>));
		always_check((CSameAs<decltype(TempG), TTupleCatResult<TTuple<int32, double&>, TTuple<int16&, FTracker&&>>>));
	}

	{
		int32 TempO = 15;
		TTuple<int32&&, const int64> TempA = { MoveTemp(TempO), 514 };

		TempA.Apply(
			[](auto&& A, auto&& B)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check((CSameAs<decltype(A), int32&>));
				always_check((CSameAs<decltype(B), const int64&>));
			}
		);

		MoveTemp(TempA).Apply(
			[](auto&& A, auto&& B)
			{
				always_check(A == 15);
				always_check(B == 514);
				always_check((CSameAs<decltype(A), int32&&>));
				always_check((CSameAs<decltype(B), const int64&&>));
			}
		);
	}

	{
		TTuple<int32, char> TempA = { 1, 'A' };
		TTuple<int32, char> TempB = TempA.Transform([](auto&& InValue) { return InValue + 1; });

		VisitTuple(
			[]<typename T> (T&& A)
			{
				if constexpr (CSameAs<T&&, int32&>) always_check(A == 2);
				else if constexpr (CSameAs<T&&, char&>) always_check(A == 'B');
				else always_check_no_entry();
			},
			TempB
		);

		VisitTuple([](auto&& A) { A++; }, TempB);

		VisitTuple(
			[]<typename T> (T&& A)
			{
				if constexpr (CSameAs<T&&, int32&>) always_check(A == 3);
				else if constexpr (CSameAs<T&&, char&>) always_check(A == 'C');
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

		Ignore = MakeTuple(1, 1.2f, 'A').Construct<FTest>();
	}

	{
		auto Func = [] { return MakeTuple(1, 2.3, 'A'); };
		auto [A, B, C] = Func();
		always_check(A == 1);
		always_check(B == 2.3);
		always_check(C == 'A');
		always_check((CSameAs<decltype(C), char>));
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
		struct FFunctor
		{
			int32 A;
			FFunctor(int32 InA) : A(InA) { }
			int32 operator()() const { return A; }
		};

		FFunctor Functor(0xCC);

//		TFunctionRef<void()>       RefA;
		TFunction<void()>       ObjectA;
		TUniqueFunction<void()> UniqueA;

//		   RefA = Functor;
		ObjectA = Functor;
		UniqueA = Functor;

//		   RefA.Emplace<FFunctor>(0xCC);
		ObjectA.Emplace<FFunctor>(0xCC);
		UniqueA.Emplace<FFunctor>(0xCC);

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
		TFunction<bool(bool)> Identity = [](bool In) { return In; };
		TFunction<bool(bool)> NotIdentity = NotFn(Identity);

		always_check(NotFn(Identity)(false));

		always_check(Identity(true));
		always_check(NotIdentity(false));
	}

	{
		struct FTest
		{
			FTest(initializer_list<int32>, int32) { }
			void operator()() { }
		};

		TFunction<void()> TempA(InPlaceType<FTest>, { 0, 1, 2 }, 3);
		TempA.Emplace<FTest>({ 0, 1, 2 }, 3);

		TUniqueFunction<void()> TempB(InPlaceType<FTest>, { 0, 1, 2 }, 3);
		TempB.Emplace<FTest>({ 0, 1, 2 }, 3);
	}
}

void TestAtomic()
{
	{
		TAtomic<int32> TempA;

		always_check(TempA.bIsAlwaysLockFree);
		always_check((TempA = 11) == 11);
		TempA.Store(12);
		always_check(TempA.Load() == 12);
		always_check((int32)TempA == 12);
		always_check(TempA.Exchange(13) == 12);
		int32 TempB = 13;
		always_check(TempA.CompareExchange(TempB, 15) == true);
		always_check(TempA.CompareExchange(TempB, 15) == false);
		always_check(TempA.CompareExchange(TempB, 15) == true);
		TempA.Wait(13);
		TempA.Notify();
		always_check(TempA.FetchAdd(1) == 15);
		always_check(TempA.FetchSub(1) == 16);
		always_check(TempA.FetchMul(3) == 15);
		always_check(TempA.FetchDiv(3) == 45);
		always_check(TempA.FetchMod(16) == 15);
		always_check(TempA.FetchAnd(0xFF) == 15);
		always_check(TempA.FetchOr(0xFFFF) == 0xF);
		always_check(TempA.FetchXor(0xFF) == 0xFFFF);
		always_check(TempA.FetchLsh(4) == 0xFF00);
		always_check(TempA.FetchRsh(4) == 0xFF000);
		always_check(++TempA == 0xFF01);
		always_check(TempA++ == 0xFF01);
		always_check(--TempA == 0xFF01);
		always_check(TempA-- == 0xFF01);
		always_check((TempA += 1) == 0xFF01);
		always_check((TempA -= 1) == 0xFF00);
		always_check((TempA *= 16) == 0xFF000);
		always_check((TempA /= 16) == 0xFF00);
		always_check((TempA %= 0x1000) == 0xF00);
		always_check((TempA &= 1) == 0x0);
		always_check((TempA |= 1) == 0x1);
		always_check((TempA ^= 0xF) == 0xE);
		always_check((TempA <<= 4) == 0xE0);
		always_check((TempA >>= 4) == 0xE);
	}

	{
		int32 A;
		TAtomicRef<int32> TempA(A);

		always_check(TempA.bIsAlwaysLockFree);
		always_check((TempA = 11) == 11);
		TempA.Store(12);
		always_check(TempA.Load() == 12);
		always_check((int32)TempA == 12);
		always_check(TempA.Exchange(13) == 12);
		int32 TempB = 13;
		always_check(TempA.CompareExchange(TempB, 15) == true);
		always_check(TempA.CompareExchange(TempB, 15) == false);
		always_check(TempA.CompareExchange(TempB, 15) == true);
		TempA.Wait(13);
		TempA.Notify();
		always_check(TempA.FetchAdd(1) == 15);
		always_check(TempA.FetchSub(1) == 16);
		always_check(TempA.FetchMul(3) == 15);
		always_check(TempA.FetchDiv(3) == 45);
		always_check(TempA.FetchMod(16) == 15);
		always_check(TempA.FetchAnd(0xFF) == 15);
		always_check(TempA.FetchOr(0xFFFF) == 0xF);
		always_check(TempA.FetchXor(0xFF) == 0xFFFF);
		always_check(TempA.FetchLsh(4) == 0xFF00);
		always_check(TempA.FetchRsh(4) == 0xFF000);
		always_check(++TempA == 0xFF01);
		always_check(TempA++ == 0xFF01);
		always_check(--TempA == 0xFF01);
		always_check(TempA-- == 0xFF01);
		always_check((TempA += 1) == 0xFF01);
		always_check((TempA -= 1) == 0xFF00);
		always_check((TempA *= 16) == 0xFF000);
		always_check((TempA /= 16) == 0xFF00);
		always_check((TempA %= 0x1000) == 0xF00);
		always_check((TempA &= 1) == 0x0);
		always_check((TempA |= 1) == 0x1);
		always_check((TempA ^= 0xF) == 0xE);
		always_check((TempA <<= 4) == 0xE0);
		always_check((TempA >>= 4) == 0xE);
	}

	{
		FAtomicFlag Flag;

		always_check(Flag.TestAndSet() == false);
		always_check(Flag.Test() == true);
		Flag.Clear();
		always_check(Flag.Test() == false);
		Flag.Wait(true);
		Flag.Notify();
	}

	{
		int32 TempA = 10;
		int32 TempB = KillDependency(TempA);
		always_check(TempB == 10);
	}

	{
		AtomicThreadFence();
		AtomicSignalFence();
	}
}

void TestScopeHelper()
{
	{
		int32 CheckNum = 0;
		{
			TScopeCallback ScopeCallback([&]() { CheckNum = 2; });
			always_check(CheckNum == 0);
			CheckNum = 1;
			always_check(CheckNum == 1);
		}
		always_check(CheckNum == 2);
	}

	{
		int32 CheckNum = 0;
		{
			TScopeCallback ScopeCallback([&]() { CheckNum = 2; });
			always_check(CheckNum == 0);
			CheckNum = 1;
			always_check(CheckNum == 1);
			ScopeCallback.Release();
		}
		always_check(CheckNum == 1);
	}

	{
		int32 CheckNum = 0;
		{
			TScopeCallback ScopeCallbackA([&]() { CheckNum = 2; });
			TScopeCallback ScopeCallbackB(MoveTemp(ScopeCallbackA));
			always_check(CheckNum == 0);
			CheckNum = 1;
			always_check(CheckNum == 1);
		}
		always_check(CheckNum == 2);
	}

	{
		int32 CheckNum = 1;
		{
			TGuardValue GuardValue(CheckNum);
			CheckNum = 2;
			always_check(CheckNum == 2);
		}
		always_check(CheckNum == 1);
	}

	{
		int32 CheckNum = 1;
		{
			TGuardValue GuardValue(CheckNum, 2);
			always_check(CheckNum == 2);
		}
		always_check(CheckNum == 1);
	}

	{
		int32 CheckNum = 1;
		{
			TGuardValue GuardValue(CheckNum, 2);
			always_check(CheckNum == 2);
			GuardValue.Release();
		}
		always_check(CheckNum == 2);
	}

	{
		int32 CheckNum = 1;
		{
			TGuardValue GuardValueA(CheckNum, 2);
			TGuardValue GuardValueB(MoveTemp(GuardValueA));
			always_check(CheckNum == 2);
		}
		always_check(CheckNum == 1);
	}

	{
		int32 CheckNum = 1;
		{
			TScopeCounter GuardValue(CheckNum);
			always_check(CheckNum == 2);
		}
		always_check(CheckNum == 1);
	}

}

void TestPropagateConst()
{
	{
		struct FTestA
		{
			void Check(bool bFlag)       { always_check(!bFlag); }
			void Check(bool bFlag) const { always_check( bFlag); }
		};

		struct FTestB
		{
			FTestB() { Ptr = &Object; }
			FTestA Object;
			TPropagateConst<FTestA*> Ptr;
		};

		      FTestB TempA;
		const FTestB TempB;

		TempA.Ptr->Check(false);
		TempB.Ptr->Check(true);
	}

	{
		int64 IntA;
		int64 IntB;

		TPropagateConst<int64*> TempA;
		TPropagateConst<int64*> TempB = &IntA;
		TPropagateConst<int64*> TempC = &IntB;

		TempA = TempB;
		TempB = TempC;

		always_check(TempA.IsValid());
		always_check(TempA == &IntA);
		always_check(TempB == TempC);
	}
}

void TestMiscTemplates()
{
	struct FTestRetainedRef { explicit FTestRetainedRef(TRetainedRef<const int64> InRef) { } };

	int64 IntA;
	FTestRetainedRef TempA(IntA);
//	FTestRetainedRef TempB(114514);

}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
