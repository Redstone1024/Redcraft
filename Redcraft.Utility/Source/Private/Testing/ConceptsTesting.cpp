#include "Testing/ConceptsTesting.h"

#include "Miscellaneous/AssertionMacros.h"
#include "Templates/Templates.h"
#include "Concepts/Concepts.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

// WARNING: The test here is not a complete test, it is only used to determine whether the environment supports the concepts

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

	// BooleanTestable.h

	always_check(CBooleanTestable<bool>);
	always_check(CBooleanTestable<int32>);
	always_check(CBooleanTestable<float>);
	always_check(!CBooleanTestable<FTestStructA>);

	// Assignable.h

	always_check((CAssignableFrom<int32&, int64>));
	always_check((CAssignableFrom<int32&, int32>));
	always_check((CAssignableFrom<int32&, int8>));
	always_check(!(CAssignableFrom<FTestStructI&, int32>));
	always_check(!(CAssignableFrom<FTestStructA&, void>));

	// Common.h

	always_check((CCommonWith<int32, int32>));
	always_check((CCommonWith<int8, int32>));
	always_check((CCommonWith<float, double>));
	always_check(!(CCommonWith<FTestStructA, int32>));

	always_check((CCommonReferenceWith<int8, int32>));
	always_check((CCommonReferenceWith<float, int32>));
	always_check((CCommonReferenceWith<float, double>));
	always_check(!(CCommonReferenceWith<FTestStructA, double>));

	// Comparable.h

	always_check((CEqualityComparable<int32>));
	always_check(!(CEqualityComparable<FTestStructA>));

	always_check((CEqualityComparableWith<int32, int32>));
	always_check((CEqualityComparableWith<int32, int64>));
	always_check(!(CEqualityComparableWith<FTestStructA, FTestStructA>));

	always_check((CTotallyOrdered<int32>));
	always_check(!(CTotallyOrdered<FTestStructA>));

	always_check((CTotallyOrderedWith<int32, int32>));
	always_check((CTotallyOrderedWith<int32, int64>));
	always_check(!(CTotallyOrderedWith<FTestStructA, FTestStructA>));

	// Objects.h

	always_check(CMovable<int32>);
	always_check(CCopyable<int32>);
	always_check(CSemiregular<int32>);
	always_check(CRegular<int32>);
	
	always_check(CMovable<FTestStructQ>);
	always_check(!CCopyable<FTestStructQ>);
	always_check(!CSemiregular<FTestStructQ>);
	always_check(!CRegular<FTestStructQ>);

	always_check(CMovable<FTestStructN>);
	always_check(CCopyable<FTestStructN>);
	always_check(!CSemiregular<FTestStructN>);
	always_check(!CRegular<FTestStructN>);
	
	// Swappable.h

	always_check(CSwappable<int32>);
	always_check(CSwappable<FTestStructG>);
	always_check(CSwappable<FTestStructN>);
	always_check(!CSwappable<FSingleton>);

	always_check((CSwappableWith<int32&, int32&>));

	// Invocable.h

	always_check((CInvocable          <decltype([](                         ) -> void  {                          })                      >));
	always_check((CRegularInvocable   <decltype([](int32 A                  ) -> int32 { return A;                }), int32               >));
	always_check((CPredicate          <decltype([](int32 A, int32 B, int32 C) -> bool  { return (A + B + C) == 0; }), int32, int32, int32 >));
	always_check((CRelation           <decltype([](int32 A, int32 B         ) -> bool  { return (A ^ B) == 0;     }), int32, int32        >));
	always_check((CEquivalenceRelation<decltype([](int32 A, int32 B         ) -> bool  { return A == B;           }), int32, int32        >));
	always_check((CStrictWeakOrder    <decltype([](int32 A, int32 B         ) -> bool  { return A < B;            }), int32, int32        >));

}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
