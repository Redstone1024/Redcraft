#include "Testing/MiscellaneousTesting.h"

#include "Miscellaneous/AssertionMacros.h"
#include "Miscellaneous/Compare.h"
#include "Miscellaneous/VarArgs.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestMiscellaneous()
{
	TestAssertionMacros();
	TestCompare();
	TestVarArgs();
}

NAMESPACE_UNNAMED_BEGIN

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

NAMESPACE_UNNAMED_END

void TestAssertionMacros()
{
	check(true);
	//check(false);
	checkf(true, TEXT("True!"));
	//checkf(false, TEXT("False!"));

	always_check(true);
	//always_check(false);
	always_checkf(true, TEXT("True!"));
	//always_checkf(false, TEXT("False!"));

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

struct FTestSynth
{
	int32 A;
	FTestSynth(int32 InA) : A(InA) { }
	friend bool operator< (FTestSynth LHS, FTestSynth RHS) { return LHS.A <  RHS.A; }
	friend bool operator<=(FTestSynth LHS, FTestSynth RHS) { return LHS.A <= RHS.A; }
	friend bool operator==(FTestSynth LHS, FTestSynth RHS) { return LHS.A == RHS.A; }
	friend bool operator>=(FTestSynth LHS, FTestSynth RHS) { return LHS.A >= RHS.A; }
	friend bool operator> (FTestSynth LHS, FTestSynth RHS) { return LHS.A >  RHS.A; }
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

	always_check((CSameAs<TCommonComparisonCategory<strong_ordering                                 >, strong_ordering >));
	always_check((CSameAs<TCommonComparisonCategory<strong_ordering, weak_ordering                  >, weak_ordering   >));
	always_check((CSameAs<TCommonComparisonCategory<strong_ordering, weak_ordering, partial_ordering>, partial_ordering>));

	always_check(CThreeWayComparable<int32>);
	always_check(CThreeWayComparable<FTestPartialOrdering>);
	always_check(CThreeWayComparable<FTestWeakOrdering>);
	always_check(CThreeWayComparable<FTestStrongOrdering>);

	always_check((CThreeWayComparable<bool, bool>));
	always_check((CThreeWayComparable<int16, int32>));

	always_check((CSameAs<TCompareThreeWayResult<int32               >, strong_ordering >));
	always_check((CSameAs<TCompareThreeWayResult<float               >, partial_ordering>));
	always_check((CSameAs<TCompareThreeWayResult<FTestPartialOrdering>, partial_ordering>));
	always_check((CSameAs<TCompareThreeWayResult<FTestWeakOrdering   >, weak_ordering   >));
	always_check((CSameAs<TCompareThreeWayResult<FTestStrongOrdering >, strong_ordering >));

	always_check((SynthThreeWayCompare(0, 0)   == strong_ordering::equal));
	always_check((SynthThreeWayCompare(0, 0.0) == strong_ordering::equal));

	always_check(SynthThreeWayCompare(FTestPartialOrdering(-1), FTestPartialOrdering( 0)) == partial_ordering::less);
	always_check(SynthThreeWayCompare(FTestPartialOrdering( 0), FTestPartialOrdering( 0)) == partial_ordering::equivalent);
	always_check(SynthThreeWayCompare(FTestPartialOrdering( 0), FTestPartialOrdering(-1)) == partial_ordering::greater);

	always_check(SynthThreeWayCompare(FTestPartialOrdering( 0, true), FTestPartialOrdering( 0, false)) == partial_ordering::unordered);
	
	always_check(SynthThreeWayCompare(FTestSynth(-1), FTestSynth( 0)) == weak_ordering::less);
	always_check(SynthThreeWayCompare(FTestSynth( 0), FTestSynth( 0)) == weak_ordering::equivalent);
	always_check(SynthThreeWayCompare(FTestSynth( 0), FTestSynth(-1)) == weak_ordering::greater);
}

NAMESPACE_UNNAMED_BEGIN

enum class ETestVarArgs
{
	A = 0xA,
	B = 0xB,
};

struct FTestVarArgs
{
	int16 A;
	float32 B;

	friend bool operator==(const FTestVarArgs& LHS, const FTestVarArgs& RHS) { return LHS.A == RHS.A && LHS.B == RHS.B; }
};

void VARARGS TestVarArgs(int32 Count, ...)
{
	VARARGS_ACCESS_BEGIN(Context, Count);

//	always_check(VARARGS_ACCESS(Context,      bool) == true);
//	always_check(VARARGS_ACCESS(Context,      char) == 2);
//	always_check(VARARGS_ACCESS(Context,     short) == 3);
	always_check(VARARGS_ACCESS(Context,       int) == 4);
	always_check(VARARGS_ACCESS(Context, long long) == 5);

//	always_check(VARARGS_ACCESS(Context,       float) == 6.0f);
	always_check(VARARGS_ACCESS(Context,      double) == 7.0 );
	always_check(VARARGS_ACCESS(Context, long double) == 8.0l);

//	always_check(VARARGS_ACCESS(Context,             nullptr_t) == nullptr);
	always_check(VARARGS_ACCESS(Context,                 void*) == nullptr);
	always_check(VARARGS_ACCESS(Context, int32 FTestVarArgs::*) == nullptr);

	always_check(VARARGS_ACCESS(Context, ETestVarArgs) == ETestVarArgs::B);
	always_check(VARARGS_ACCESS(Context, FTestVarArgs) == FTestVarArgs({ 404, 5.0f }));

	VARARGS_ACCESS_END(Context);
};

NAMESPACE_UNNAMED_END

void TestVarArgs()
{
	TestVarArgs
	(
		7 - 5,
//		true,
//		static_cast<     char>(2),
//		static_cast<    short>(3),
		static_cast<      int>(4),
		static_cast<long long>(5),
//		6.0f,
		7.0,
		8.0l,
//		nullptr,
		static_cast<void*>(nullptr),
		static_cast<int32 FTestVarArgs::*>(nullptr),
		ETestVarArgs::B,
		FTestVarArgs({ 404, 5.0f })
	);
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
