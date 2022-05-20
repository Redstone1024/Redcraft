#include "Testing/TypeTraitsTesting.h"

#include "Miscellaneous/AssertionMacros.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Templates.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

// WARNING: The test here is not a complete test, it is only used to determine whether the environment supports the traits

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

void TestTypeTraits()
{
	// HelperClasses.h

	always_check((TConstant<int32, 1>::Value == 1));
	always_check(static_cast<int32>(TConstant<int32, 2>::Value) == 2);
	always_check((TConstant<int32, 3>() == 3));

	always_check(!FFalse::Value);
	always_check(FTrue::Value);

	// PrimaryType.h

	always_check(!CVoid<int32>);
	always_check(CVoid<void>);
	always_check(CVoid<const void>);
	always_check(CVoid<const volatile void>);
	always_check(CVoid<volatile void>);

	always_check(!CNullPointer<int32>);
	always_check(CNullPointer<nullptr_t>);

	always_check(CIntegral<int32>);
	always_check(!CIntegral<float>);

	always_check(!CFloatingPoint<int32>);
	always_check(CFloatingPoint<float>);

	always_check(!CArray<int32>);
	always_check(CArray<int32[]>);
	always_check(CArray<int32[10]>);

	always_check(!CPointer<int32>);
	always_check(CPointer<int32*>);

	always_check(!CLValueReference<int32>);
	always_check(CLValueReference<int32&>);
	always_check(!CLValueReference<int32&&>);
	
	always_check(!CRValueReference<int32>);
	always_check(!CRValueReference<int32&>);
	always_check(CRValueReference<int32&&>);

	always_check(CMemberObjectPointer<int32(FTestStructA::*)>);
	always_check(!CMemberObjectPointer<int32(FTestStructA::*)()>);

	always_check(!CMemberFunctionPointer<int32(FTestStructA::*)>);
	always_check(CMemberFunctionPointer<int32(FTestStructA::*)()>);

	always_check(!CEnum<int32>);
	always_check(!CEnum<FTestStructA>);
	always_check(CEnum<ETestEnum>);
	always_check(CEnum<ETestEnumClass>);

	always_check(!CUnion<int32>);
	always_check(!CUnion<FTestStructA>);
	always_check(CUnion<FTestUnion>);

	always_check(!CUnion<int32>);
	always_check(!CUnion<FTestStructA>);
	always_check(CUnion<FTestUnion>);

	always_check(!CFunction<int32>);
	always_check(!CFunction<FTestStructA>);
	always_check(!CFunction<FTestUnion>);
	always_check(CFunction<int32(int32)>);

	// CompositeType.h

	always_check(!CFundamental<FTestStructA>);
	always_check(CFundamental<int32>);
	always_check(CFundamental<float>);
	always_check(!CFundamental<int32*>);
	always_check(CFundamental<void>);

	always_check(!CArithmetic<FTestStructA>);
	always_check(CArithmetic<int32>);
	always_check(CArithmetic<float>);
	always_check(!CArithmetic<int32*>);
	always_check(!CArithmetic<void>);

	always_check(!CScalar<FTestStructA>);
	always_check(CScalar<int32>);
	always_check(CScalar<float>);
	always_check(CScalar<int32*>);
	always_check(!CScalar<void>);

	always_check(CObject<FTestStructA>);
	always_check(!CObject<FTestStructA&>);
	always_check(CObject<int32>);
	always_check(CObject<int32*>);
	always_check(!CObject<int32&>);

	always_check(CCompound<FTestStructA>);
	always_check(CCompound<FTestStructA&>);
	always_check(!CCompound<int32>);
	always_check(CCompound<int32*>);
	always_check(CCompound<int32&>);

	always_check(!CReference<int32>);
	always_check(!CReference<int32*>);
	always_check(CReference<int32&>);
	always_check(CReference<int32&&>);

	always_check(!CMemberPointer<FTestStructA>);
	always_check(CMemberPointer<int32(FTestStructA::*)>);

	always_check(CSignedIntegral<signed>);
	always_check(!CSignedIntegral<unsigned>);

	always_check(!CUnsignedIntegral<signed>);
	always_check(CUnsignedIntegral<unsigned>);

	// TypeProperties.h

	always_check(!CConst<int32>);
	always_check(CConst<const int32>);
	always_check(!CConst<volatile int32>);
	always_check(CConst<const volatile int32>);

	always_check(!CVolatile<int32>);
	always_check(!CVolatile<const int32>);
	always_check(CVolatile<volatile int32>);
	always_check(CVolatile<const volatile int32>);

	always_check(CTrivial<FTestStructB>);
	always_check(!CTrivial<FTestStructC>);

	always_check(CTriviallyCopyable<FTestStructB>);
	always_check(!CTriviallyCopyable<FTestStructD>);
	always_check(!CTriviallyCopyable<FTestStructE>);

	always_check(CStandardLayout<FTestStructB>);
	always_check(!CStandardLayout<FTestStructE>);
	always_check(!CStandardLayout<FTestStructF>);

	always_check(CUniqueObjectRepresentible<FTestStructF>);
	always_check(!CUniqueObjectRepresentible<FTestStructG>);

	always_check(CEmpty<FTestStructA>);
	always_check(!CEmpty<FTestStructB>);
	always_check(CEmpty<FTestStructC>);
	always_check(CEmpty<FTestStructD>);
	always_check(!CEmpty<FTestStructE>);
	always_check(!CEmpty<FTestStructF>);

	always_check(CPolymorphic<FTestStructE>);
	always_check(!CPolymorphic<FTestStructF>);

	always_check(CAbstract<FTestStructE>);
	always_check(!CAbstract<FTestStructH>);

	always_check(!CFinal<FTestStructE>);
	always_check(CFinal<FTestStructH>);

	always_check(!CAggregate<int32>);
	always_check(CAggregate<int32[64]>);
	always_check(CAggregate<FTestStructB>);
	always_check(!CAggregate<FTestStructF>);

	always_check(CSigned<signed>);
	always_check(!CSigned<unsigned>);

	always_check(!CUnsigned<signed>);
	always_check(CUnsigned<unsigned>);

	always_check(!CBoundedArray<int32>);
	always_check(!CBoundedArray<int32[]>);
	always_check(CBoundedArray<int32[64]>);

	always_check(!CUnboundedArray<int32>);
	always_check(CUnboundedArray<int32[]>);
	always_check(!CUnboundedArray<int32[64]>);

	always_check(!CScopedEnum<ETestEnum>);
	always_check(CScopedEnum<ETestEnumClass>);

	// SupportedOperations.h

	always_check(!CDefaultConstructible<FTestStructI>);
	always_check(CDefaultConstructible<FTestStructJ>);
	always_check(CDefaultConstructible<FTestStructK>);
	always_check(!CDefaultConstructible<FTestStructL>);

	always_check(!CTriviallyDefaultConstructible<FTestStructI>);
	always_check(!CTriviallyDefaultConstructible<FTestStructJ>);
	always_check(CTriviallyDefaultConstructible<FTestStructK>);
	always_check(!CTriviallyDefaultConstructible<FTestStructL>);

	always_check(!(CConstructibleFrom<FTestStructI, int32>));
	always_check((CConstructibleFrom<FTestStructI, FTestStructI&>));
	always_check((CConstructibleFrom<FTestStructI, int32, double>));

	always_check(!(CTriviallyConstructibleFrom<FTestStructI, int32>));
	always_check((CTriviallyConstructibleFrom<FTestStructI, FTestStructI&>));
	always_check(!(CTriviallyConstructibleFrom<FTestStructI, int32, double>));

	always_check(CCopyConstructible<FTestStructM>);
	always_check(CCopyConstructible<FTestStructN>);
	always_check(!CCopyConstructible<FTestStructO>);

	always_check(!CTriviallyCopyConstructible<FTestStructM>);
	always_check(CTriviallyCopyConstructible<FTestStructN>);
	always_check(!CTriviallyCopyConstructible<FTestStructO>);

	always_check(CMoveConstructible<FTestStructP>);
	always_check(CMoveConstructible<FTestStructQ>);
	always_check(!CMoveConstructible<FTestStructR>);

	always_check(!CTriviallyMoveConstructible<FTestStructP>);
	always_check(CTriviallyMoveConstructible<FTestStructQ>);
	always_check(!CTriviallyMoveConstructible<FTestStructR>);

	always_check(!(CAssignableFrom<FTestStructI, FTestStructH>));
	always_check((CAssignableFrom<FTestStructI, FTestStructI&>));
	always_check((CAssignableFrom<FTestStructI, int32>));

	always_check(!(CTriviallyAssignableFrom<FTestStructI, FTestStructH>));
	always_check((CTriviallyAssignableFrom<FTestStructI, FTestStructI&>));
	always_check(!(CTriviallyAssignableFrom<FTestStructI, int32>));

	always_check(CCopyAssignable<FTestStructM>);
	always_check(CCopyAssignable<FTestStructN>);
	always_check(!CCopyAssignable<FTestStructO>);

	always_check(!CTriviallyCopyAssignable<FTestStructM>);
	always_check(CTriviallyCopyAssignable<FTestStructN>);
	always_check(!CTriviallyCopyAssignable<FTestStructO>);

	always_check(CMoveAssignable<FTestStructP>);
	always_check(CMoveAssignable<FTestStructQ>);
	always_check(!CMoveAssignable<FTestStructR>);

	always_check(!CTriviallyMoveAssignable<FTestStructP>);
	always_check(CTriviallyMoveAssignable<FTestStructQ>);
	always_check(!CTriviallyMoveAssignable<FTestStructR>);

	always_check(CDestructible<FTestStructS>);
	always_check(CDestructible<FTestStructT>);
	always_check(!CDestructible<FTestStructU>);

	always_check(!CTriviallyDestructible<FTestStructS>);
	always_check(CTriviallyDestructible<FTestStructT>);
	always_check(!CTriviallyDestructible<FTestStructU>);

	always_check(!CVirtualDestructible<FTestStructT>);
	always_check(CVirtualDestructible<FTestStructV>);

	// Miscellaneous.h

	always_check(ArrayRank<int32[1][2][3]> == 3);
	always_check(ArrayRank<int32[1][2][3][4]> == 4);
	always_check(ArrayRank<int32> == 0);

	always_check(ArrayExtent<int32[1][2][3]> == 1);
	always_check((ArrayExtent<int32[1][2][3][4], 1> == 2));
	always_check(ArrayExtent<int32[]> == 0);

	always_check(!(CSameAs<int32, int64>));
	always_check((CSameAs<int32, int32>));

	always_check(!(CBaseOf<FTestStructH, FTestStructD>));
	always_check(!(CBaseOf<FTestStructH, FTestStructE>));
	always_check((CBaseOf<FTestStructE, FTestStructH>));

	always_check((CConvertibleTo<int32, uint32>));
	always_check(!(CConvertibleTo<FTestStructH*, FTestStructD*>));
	always_check((CConvertibleTo<FTestStructH*, FTestStructE*>));
	always_check(!(CConvertibleTo<FTestStructE*, FTestStructH*>));
	always_check((CConvertibleTo<FTestStructW, FTestStructV>));

	always_check((CSameAs<int32, TRemoveConst<int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<int32*>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<int32&>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<int32&&>::Type>));
	always_check((CSameAs<int32, TRemoveConst<const int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<const volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveConst<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemoveVolatile<int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<int32*>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<int32&>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<int32&&>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<const int32>::Type>));
	always_check((CSameAs<int32, TRemoveVolatile<volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<const volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveVolatile<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemoveCV<int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveCV<int32*>::Type>));
	always_check(!(CSameAs<int32, TRemoveCV<int32&>::Type>));
	always_check(!(CSameAs<int32, TRemoveCV<int32&&>::Type>));
	always_check((CSameAs<int32, TRemoveCV<const int32>::Type>));
	always_check((CSameAs<int32, TRemoveCV<volatile int32>::Type>));
	always_check((CSameAs<int32, TRemoveCV<const volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveCV<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemovePointer<int32>::Type>));
	always_check((CSameAs<int32, TRemovePointer<int32*>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<int32&>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<int32&&>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<const int32>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<const volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemovePointer<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemoveReference<int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveReference<int32*>::Type>));
	always_check((CSameAs<int32, TRemoveReference<int32&>::Type>));
	always_check((CSameAs<int32, TRemoveReference<int32&&>::Type>));
	always_check(!(CSameAs<int32, TRemoveReference<const int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveReference<volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveReference<const volatile int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveReference<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemoveCVRef<int32>::Type>));
	always_check(!(CSameAs<int32, TRemoveCVRef<int32*>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<int32&>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<int32&&>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<const int32>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<volatile int32>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<const volatile int32>::Type>));
	always_check((CSameAs<int32, TRemoveCVRef<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TRemoveExtent<int32[1]>::Type>));
	always_check(!(CSameAs<int32, TRemoveExtent<int32[1][2]>::Type>));
	always_check((CSameAs<int32[2][3], TRemoveExtent<int32[1][2][3]>::Type>));

	always_check((CSameAs<int32, TRemoveAllExtents<int32[1]>::Type>));
	always_check((CSameAs<int32, TRemoveAllExtents<int32[1][2]>::Type>));
	always_check((CSameAs<int32, TRemoveAllExtents<int32[1][2][3]>::Type>));

	always_check((CSameAs<int32, TMakeSigned<int32>::Type>));
	always_check((CSameAs<int32, TMakeSigned<uint32>::Type>));

	always_check((CSameAs<uint32, TMakeUnsigned<int32>::Type>));
	always_check((CSameAs<uint32, TMakeUnsigned<uint32>::Type>));

	TAlignedStorage<32, 4>::Type Aligned4;
	TAlignedStorage<32, 8>::Type Aligned8;
	TAlignedStorage<32, 16>::Type Aligned16;
	TAlignedStorage<32, 32>::Type Aligned32;
	always_check((int64)(&Aligned4) % 4 == 0);
	always_check((int64)(&Aligned8) % 8 == 0);
	always_check((int64)(&Aligned16) % 16 == 0);
	always_check((int64)(&Aligned32) % 32 == 0);

	always_check(sizeof(TAlignedUnion<8, int32, int32>::Type) == 8);
	always_check(sizeof(TAlignedUnion<0, int8, int32>::Type) == 4);
	always_check(sizeof(TAlignedUnion<0, int32, int64>::Type) == 8);
	always_check(sizeof(TAlignedUnion<0, int32, double>::Type) == 8);

	always_check((CSameAs<int32, TDecay<int32>::Type>));
	always_check((CSameAs<int32*, TDecay<int32*>::Type>));
	always_check((CSameAs<int32*, TDecay<int32[]>::Type>));
	always_check((CSameAs<int32, TDecay<int32&>::Type>));
	always_check((CSameAs<int32, TDecay<int32&&>::Type>));
	always_check((CSameAs<int32, TDecay<const int32>::Type>));
	always_check((CSameAs<int32, TDecay<volatile int32>::Type>));
	always_check((CSameAs<int32, TDecay<const volatile int32>::Type>));
	always_check((CSameAs<int32, TDecay<const volatile int32&>::Type>));

	always_check((CSameAs<int32, TConditional<true, int32, int64>::Type>));
	always_check((CSameAs<int64, TConditional<false, int32, int64>::Type>));

	always_check((CSameAs<int, TUnderlyingType<ETestEnumClass>::Type>));
	always_check((CSameAs<uint8, TUnderlyingType<ETestEnumClass8>::Type>));
	always_check((CSameAs<uint32, TUnderlyingType<ETestEnumClass32>::Type>));
	always_check((CSameAs<uint64, TUnderlyingType<ETestEnumClass64>::Type>));

	always_check((CSameAs<void, TVoid<int32>::Type>));
	always_check((CSameAs<void, TVoid<int32, int64>::Type>));
	
	// Invocable.h
	
	always_check((CInvocable<int32()>));
	always_check((CInvocable<int32(int32), int32>));
	always_check(!(CInvocable<int32(int32), FTestStructA>));
	always_check((CInvocable<int32(int32), int32>));

	always_check((CInvocableResult<void, int32()>));
	always_check((CInvocableResult<int32, int32()>));
	always_check((CInvocableResult<int32, int32(int32), int32>));
	always_check(!(CInvocableResult<int32, int32(int32), FTestStructA>));
	always_check(!(CInvocableResult<FTestStructA, int32(int32), int32>));

	always_check((CSameAs<int32, TInvokeResult<int32()>::Type>));
	always_check((CSameAs<int32, TInvokeResult<int32(int32), int32>::Type>));
//	always_check((CSameAs<char(&)[2], TInvokeResult<char(&())[2]>::Type>));

	always_check((CInvocable          <decltype([](                         ) -> void  {                          })                      >));
	always_check((CRegularInvocable   <decltype([](int32 A                  ) -> int32 { return A;                }), int32               >));
	always_check((CPredicate          <decltype([](int32 A, int32 B, int32 C) -> bool  { return (A + B + C) == 0; }), int32, int32, int32 >));
	always_check((CRelation           <decltype([](int32 A, int32 B         ) -> bool  { return (A ^ B) == 0;     }), int32, int32        >));
	always_check((CEquivalenceRelation<decltype([](int32 A, int32 B         ) -> bool  { return A == B;           }), int32, int32        >));
	always_check((CStrictWeakOrder    <decltype([](int32 A, int32 B         ) -> bool  { return A < B;            }), int32, int32        >));

	// Common.h

	always_check((CSameAs<int32, TCommonType<int8, int32>::Type>));
	always_check((CSameAs<int64, TCommonType<int8, int32, int64>::Type>));
	always_check((CSameAs<double, TCommonType<float, double>::Type>));

	always_check((CSameAs<int32, TCommonReference<int8, int32>::Type>));
	always_check((CSameAs<int64, TCommonReference<int8, int32, int64>::Type>));
	always_check((CSameAs<double, TCommonReference<float, double>::Type>));
	
	always_check((CCommonWith<int32, int32>));
	always_check((CCommonWith<int8, int32>));
	always_check((CCommonWith<float, double>));
	always_check(!(CCommonWith<FTestStructA, int32>));

	always_check((CCommonReferenceWith<int8, int32>));
	always_check((CCommonReferenceWith<float, int32>));
	always_check((CCommonReferenceWith<float, double>));
	always_check(!(CCommonReferenceWith<FTestStructA, double>));

	// Swappable.h

	always_check(CSwappable<int32>);
	always_check(CSwappable<FTestStructG>);
	always_check(CSwappable<FTestStructN>);
	always_check(!CSwappable<FSingleton>);

	always_check((CSwappableWith<int32&, int32&>));

	// CopyQualifiers.h

	always_check((CSameAs<               int32, TCopyConst<               int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyConst<const          int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyConst<const volatile int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyConst<               int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyConst<const          int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyConst<const volatile int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyConst<               int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyConst<const          int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyConst<const volatile int32, const volatile int32>::Type>));
	
	always_check((CSameAs<               int32, TCopyVolatile<               int32,                int32>::Type>));
	always_check((CSameAs<               int32, TCopyVolatile<const          int32,                int32>::Type>));
	always_check((CSameAs<      volatile int32, TCopyVolatile<const volatile int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyVolatile<               int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyVolatile<const          int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyVolatile<const volatile int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyVolatile<               int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyVolatile<const          int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyVolatile<const volatile int32, const volatile int32>::Type>));
	
	always_check((CSameAs<               int32, TCopyCV<               int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCV<const          int32,                int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCV<const volatile int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCV<               int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCV<const          int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCV<const volatile int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCV<               int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCV<const          int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCV<const volatile int32, const volatile int32>::Type>));
	
	always_check((CSameAs<int32,   TCopyReference<int32,   int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyReference<int32,   int32& >::Type>));
	always_check((CSameAs<int32&&, TCopyReference<int32,   int32&&>::Type>));
	always_check((CSameAs<int32&,  TCopyReference<int32&,  int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyReference<int32&,  int32& >::Type>));
	always_check((CSameAs<int32&,  TCopyReference<int32&,  int32&&>::Type>));
	always_check((CSameAs<int32&&, TCopyReference<int32&&, int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyReference<int32&&, int32& >::Type>));
	always_check((CSameAs<int32&&, TCopyReference<int32&&, int32&&>::Type>));

	always_check((CSameAs<               int32, TCopyCVRef<               int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<const          int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<               int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<const          int32, const          int32>::Type>));
	always_check((CSameAs<      volatile int32, TCopyCVRef<      volatile int32,                int32>::Type>));
	always_check((CSameAs<      volatile int32, TCopyCVRef<               int32,       volatile int32>::Type>));
	always_check((CSameAs<      volatile int32, TCopyCVRef<      volatile int32,       volatile int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<const          int32,                int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCVRef<const volatile int32,                int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<               int32, const          int32>::Type>));
	always_check((CSameAs<const          int32, TCopyCVRef<const          int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCVRef<const volatile int32, const          int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCVRef<               int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCVRef<const          int32, const volatile int32>::Type>));
	always_check((CSameAs<const volatile int32, TCopyCVRef<const volatile int32, const volatile int32>::Type>));
	
	always_check((CSameAs<int32,   TCopyCVRef<int32,   int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyCVRef<int32,   int32& >::Type>));
	always_check((CSameAs<int32&&, TCopyCVRef<int32,   int32&&>::Type>));
	always_check((CSameAs<int32&,  TCopyCVRef<int32&,  int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyCVRef<int32&,  int32& >::Type>));
	always_check((CSameAs<int32&,  TCopyCVRef<int32&,  int32&&>::Type>));
	always_check((CSameAs<int32&&, TCopyCVRef<int32&&, int32  >::Type>));
	always_check((CSameAs<int32&,  TCopyCVRef<int32&&, int32& >::Type>));
	always_check((CSameAs<int32&&, TCopyCVRef<int32&&, int32&&>::Type>));

	always_check((CSameAs<const           int32,   TCopyCVRef<const          int32,         int32  >::Type>));
	always_check((CSameAs<const           int32&,  TCopyCVRef<               int32,   const int32& >::Type>));
	always_check((CSameAs<const volatile  int32&&, TCopyCVRef<const volatile int32,   const int32&&>::Type>));
	always_check((CSameAs<const           int32&,  TCopyCVRef<const          int32&,        int32  >::Type>));
	always_check((CSameAs<const           int32&,  TCopyCVRef<const          int32&,  const int32& >::Type>));
	always_check((CSameAs<const volatile  int32&,  TCopyCVRef<      volatile int32&,  const int32&&>::Type>));
	always_check((CSameAs<const           int32&&, TCopyCVRef<const          int32&&,       int32  >::Type>));
	always_check((CSameAs<const           int32&,  TCopyCVRef<const          int32&&, const int32& >::Type>));
	always_check((CSameAs<const volatile  int32&&, TCopyCVRef<const volatile int32&&, const int32&&>::Type>));

	// BooleanTestable.h

	always_check(CBooleanTestable<bool>);
	always_check(CBooleanTestable<int32>);
	always_check(CBooleanTestable<float>);
	always_check(!CBooleanTestable<FTestStructA>);

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

}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
