#include "Testing/ConceptsTesting.h"
#include "Misc/AssertionMacros.h"
#include "Concepts/Concepts.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Warning: The test here is not a complete test, it is only used to determine whether the environment supports the concepts

NAMESPACE_UNNAMED_BEGIN

int32 TestObject;
void TestFunction() { }

struct FTestStructA { };
struct FTestStructB : FTestStructA { int32 Member; };
struct FTestStructC { FTestStructC() { } };
struct FTestStructD { FTestStructD(const FTestStructD&) { } };
struct FTestStructE { virtual void Member() = 0; };
struct FTestStructF { int32 MemberA; private: int32 MemberB; };
struct FTestStructG { char MemberA; float MemberB; short MemberC; int MemberD; };
struct FTestStructH final : FTestStructE { virtual void Member() override { } };
struct FTestStructI { int32 MemberA; double MemberB; FTestStructI(int32 A, double B) { } FTestStructI& operator=(int32) { return *this; };  };
struct FTestStructJ { int32 MemberA; double MemberB; FTestStructJ() { }; };
struct FTestStructK { int32 MemberA; double MemberB; FTestStructK() = default; };
struct FTestStructL { int32 MemberA; double MemberB; FTestStructL() = delete; };
struct FTestStructM { int32 MemberA; double MemberB; FTestStructM(const FTestStructM&) { }; FTestStructM& operator=(const FTestStructM&) { return *this; }; };
struct FTestStructN { int32 MemberA; double MemberB; FTestStructN(const FTestStructN&) = default; FTestStructN& operator=(const FTestStructN&) = default;  };
struct FTestStructO { int32 MemberA; double MemberB; FTestStructO(const FTestStructO&) = delete; FTestStructO& operator=(const FTestStructO&) = delete;  };
struct FTestStructP { int32 MemberA; double MemberB; FTestStructP(FTestStructP&&) { }; FTestStructP& operator=(FTestStructP&&) { return *this; }; };
struct FTestStructQ { int32 MemberA; double MemberB; FTestStructQ(FTestStructQ&&) = default; FTestStructQ& operator=(FTestStructQ&&) = default; };
struct FTestStructR { int32 MemberA; double MemberB; FTestStructR(FTestStructR&&) = delete; FTestStructR& operator=(FTestStructR&&) = delete; };
struct FTestStructS { int32 MemberA; double MemberB; ~FTestStructS() { } };
struct FTestStructT { int32 MemberA; double MemberB; ~FTestStructT() = default; };
struct FTestStructU { int32 MemberA; double MemberB; ~FTestStructU() = delete; };
struct FTestStructV { int32 MemberA; double MemberB; virtual ~FTestStructV() { }; };
struct FTestStructW { int32 MemberA; double MemberB; operator FTestStructV() { return FTestStructV(); } };

enum ETestEnum { };
enum class ETestEnumClass { };
enum class ETestEnumClass8 : uint8 { };
enum class ETestEnumClass32 : uint32 { };
enum class ETestEnumClass64 : uint64 { };

union FTestUnion { };

NAMESPACE_UNNAMED_END

void TestConcepts()
{
	// Same.h

	always_check(!(CSameAs<int32, int64>));
	always_check((CSameAs<int32, int32>));

	// Destructible.h

	always_check(CDestructible<FTestStructS>);
	always_check(CDestructible<FTestStructT>);
	always_check(!CDestructible<FTestStructU>);

	// Derived.h

	always_check(!(CDerivedFrom<FTestStructH, FTestStructD>));
	always_check((CDerivedFrom<FTestStructH, FTestStructE>));
	always_check(!(CDerivedFrom<FTestStructE, FTestStructH>));

	// Convertible.h

	always_check((CConvertibleTo<int32, uint32>));
	always_check(!(CConvertibleTo<FTestStructH*, FTestStructD*>));
	always_check((CConvertibleTo<FTestStructH*, FTestStructE*>));
	always_check(!(CConvertibleTo<FTestStructE*, FTestStructH*>));
	always_check((CConvertibleTo<FTestStructW, FTestStructV>));

	// Constructible.h

	always_check((CConstructibleFrom<FTestStructJ>));
	always_check((CConstructibleFrom<FTestStructK>));
	always_check(!(CConstructibleFrom<FTestStructI, int32>));
	always_check((CConstructibleFrom<FTestStructI, FTestStructI&>));
	always_check((CConstructibleFrom<FTestStructI, int32, double>));

	always_check(!CDefaultInitializable<FTestStructI>);
	always_check(CDefaultInitializable<FTestStructJ>);
	always_check(CDefaultInitializable<FTestStructK>);
	always_check(!CDefaultInitializable<FTestStructL>);

	always_check(CMoveConstructible<FTestStructP>);
	always_check(CMoveConstructible<FTestStructQ>);
	always_check(!CMoveConstructible<FTestStructR>);

	always_check(CCopyConstructible<FTestStructM>);
	always_check(CCopyConstructible<FTestStructN>);
	always_check(!CCopyConstructible<FTestStructO>);

	// BuiltinType.h

	always_check(CIntegral<bool>);
	always_check(CIntegral<int32>);
	always_check(!CIntegral<float>);

	always_check(CSignedIntegral<signed>);
	always_check(!CSignedIntegral<unsigned>);

	always_check(!CUnsignedIntegral<signed>);
	always_check(CUnsignedIntegral<unsigned>);

	always_check(!CNonBooleanIntegral<bool>);
	always_check(CNonBooleanIntegral<int32>);
	always_check(!CNonBooleanIntegral<float>);

	always_check(!CFloatingPoint<int32>);
	always_check(CFloatingPoint<float>);

}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END