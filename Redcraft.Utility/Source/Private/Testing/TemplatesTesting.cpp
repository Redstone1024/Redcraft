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
	TestCompare();
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

struct FTestPartialOrdering
{
	int32 Num;
	bool bIsValid;
	FTestPartialOrdering(int32 InNum, bool bInIsValid = true) : Num(InNum), bIsValid(bInIsValid) { }
	friend bool operator==(FTestPartialOrdering LHS, FTestPartialOrdering RHS) { return LHS.bIsValid && RHS.bIsValid ? LHS.Num == RHS.Num : false; }
	friend partial_ordering operator<=>(FTestPartialOrdering LHS, FTestPartialOrdering RHS) { return LHS.bIsValid && RHS.bIsValid ? LHS.Num <=> RHS.Num : partial_ordering::unordered; }
};

struct FTestWeakOrdering
{
	int32 Num;
	FTestWeakOrdering(int32 InNum) : Num(InNum) { }
	friend bool operator==(FTestWeakOrdering LHS, FTestWeakOrdering RHS) { return LHS.Num == RHS.Num; }
	friend weak_ordering operator<=>(FTestWeakOrdering LHS, FTestWeakOrdering RHS) { return LHS.Num <=> RHS.Num; }
};

struct FTestStrongOrdering
{
	int32 Num;
	FTestStrongOrdering(int32 InNum) : Num(InNum) { }
	friend bool operator==(FTestStrongOrdering LHS, FTestStrongOrdering RHS) { return LHS.Num == RHS.Num; }
	friend strong_ordering operator<=>(FTestStrongOrdering LHS, FTestStrongOrdering RHS) { return LHS.Num <=> RHS.Num; }
};

NAMESPACE_UNNAMED_END

void TestCompare()
{
	always_check((-1 <=>  0) == strong_ordering::less);
	always_check(( 0 <=>  0) == strong_ordering::equivalent);
	always_check(( 0 <=>  0) == strong_ordering::equal);
	always_check(( 0 <=> -1) == strong_ordering::greater);
	
	always_check((-1 <=>  0) <  0);
	always_check((-1 <=>  0) <= 0);
	always_check(( 0 <=>  0) <= 0);
	always_check(( 0 <=>  0) == 0);
	always_check(( 0 <=>  0) >= 0);
	always_check(( 0 <=> -1) >= 0);
	always_check(( 0 <=> -1) >  0);
	always_check((-1 <=>  1) != 0);

	int64 NaNBit = 0xFFF8000000000000;
	double NaN = *reinterpret_cast<double*>(&NaNBit);

	always_check((-1.0 <=>  0.0) == partial_ordering::less);
	always_check(( 0.0 <=>  0.0) == partial_ordering::equivalent);
	always_check(( 0.0 <=> -1.0) == partial_ordering::greater);
	always_check(( 0.0 <=>  NaN) == partial_ordering::unordered);

	always_check((-1.0 <=>  0.0) == weak_ordering::less);
	always_check(( 0.0 <=>  0.0) == weak_ordering::equivalent);
	always_check(( 0.0 <=> -1.0) == weak_ordering::greater);
	
	always_check((-1.0 <=>  0.0) == strong_ordering::less);
	always_check(( 0.0 <=>  0.0) == strong_ordering::equivalent);
	always_check(( 0.0 <=>  0.0) == strong_ordering::equal);
	always_check(( 0.0 <=> -1.0) == strong_ordering::greater);
	
	always_check((-1.0 <=>  0.0) <  0);
	always_check((-1.0 <=>  0.0) <= 0);
	always_check(( 0.0 <=>  0.0) <= 0);
	always_check(( 0.0 <=>  0.0) == 0);
	always_check(( 0.0 <=>  0.0) >= 0);
	always_check(( 0.0 <=> -1.0) >= 0);
	always_check(( 0.0 <=> -1.0) >  0);
	always_check((-1.0 <=>  1.0) != 0);
	
	always_check((FTestPartialOrdering(-1) <=> FTestPartialOrdering( 0)) == partial_ordering::less);
	always_check((FTestPartialOrdering( 0) <=> FTestPartialOrdering( 0)) == partial_ordering::equivalent);
	always_check((FTestPartialOrdering( 0) <=> FTestPartialOrdering(-1)) == partial_ordering::greater);

	always_check((FTestPartialOrdering( 0, true) <=> FTestPartialOrdering( 0, false)) == partial_ordering::unordered);
	
	always_check((FTestWeakOrdering(-1) <=> FTestWeakOrdering( 0)) == weak_ordering::less);
	always_check((FTestWeakOrdering( 0) <=> FTestWeakOrdering( 0)) == weak_ordering::equivalent);
	always_check((FTestWeakOrdering( 0) <=> FTestWeakOrdering(-1)) == weak_ordering::greater);
	
	always_check((FTestStrongOrdering(-1) <=> FTestStrongOrdering( 0)) == strong_ordering::less);
	always_check((FTestStrongOrdering( 0) <=> FTestStrongOrdering( 0)) == strong_ordering::equivalent);
	always_check((FTestStrongOrdering( 0) <=> FTestStrongOrdering( 0)) == strong_ordering::equal);
	always_check((FTestStrongOrdering( 0) <=> FTestStrongOrdering(-1)) == strong_ordering::greater);
	
	always_check((FTestPartialOrdering(-1) <  FTestPartialOrdering( 0)));
	always_check((FTestPartialOrdering( 0) == FTestPartialOrdering( 0)));
	always_check((FTestPartialOrdering( 0) >  FTestPartialOrdering(-1)));
	
	always_check((FTestWeakOrdering(-1) <  FTestWeakOrdering( 0)));
	always_check((FTestWeakOrdering( 0) == FTestWeakOrdering( 0)));
	always_check((FTestWeakOrdering( 0) >  FTestWeakOrdering(-1)));

	always_check((FTestStrongOrdering(-1) <  FTestStrongOrdering( 0)));
	always_check((FTestStrongOrdering( 0) == FTestStrongOrdering( 0)));
	always_check((FTestStrongOrdering( 0) >  FTestStrongOrdering(-1)));

	always_check((TIsSame<TCommonComparisonCategory<strong_ordering                                 >::Type, strong_ordering >::Value));
	always_check((TIsSame<TCommonComparisonCategory<strong_ordering, weak_ordering                  >::Type, weak_ordering   >::Value));
	always_check((TIsSame<TCommonComparisonCategory<strong_ordering, weak_ordering, partial_ordering>::Type, partial_ordering>::Value));

	always_check(CThreeWayComparable<int32>);
	always_check(CThreeWayComparable<FTestPartialOrdering>);
	always_check(CThreeWayComparable<FTestWeakOrdering>);
	always_check(CThreeWayComparable<FTestStrongOrdering>);

	always_check((CThreeWayComparableWith<bool, bool>));
	always_check((CThreeWayComparableWith<int16, int32>));

	always_check((TIsSame<TCompareThreeWayResult<int32               >::Type, strong_ordering >::Value));
	always_check((TIsSame<TCompareThreeWayResult<float               >::Type, partial_ordering>::Value));
	always_check((TIsSame<TCompareThreeWayResult<FTestPartialOrdering>::Type, partial_ordering>::Value));
	always_check((TIsSame<TCompareThreeWayResult<FTestWeakOrdering   >::Type, weak_ordering   >::Value));
	always_check((TIsSame<TCompareThreeWayResult<FTestStrongOrdering >::Type, strong_ordering >::Value));

	always_check((TCompareThreeWay<int32>()(0, 0)   == strong_ordering::equal));
	always_check((TCompareThreeWay<void>() (0, 0.0) == strong_ordering::equal));

	always_check((StrongOrder(0, 0)                 == strong_ordering::equal));
	always_check((WeakOrder(0, 0)                   == strong_ordering::equal));
	always_check((PartialOrder(0, 0)                == strong_ordering::equal));
	always_check((CompareStrongOrderFallback(0, 0)  == strong_ordering::equal));
	always_check((CompareWeakOrderFallback(0, 0)    == strong_ordering::equal));
	always_check((ComparePartialOrderFallback(0, 0) == strong_ordering::equal));

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
