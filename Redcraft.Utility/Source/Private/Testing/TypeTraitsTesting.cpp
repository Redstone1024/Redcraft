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

	always_check(!TIsFundamental<FTestStructA>::Value);
	always_check(TIsFundamental<int32>::Value);
	always_check(TIsFundamental<float>::Value);
	always_check(!TIsFundamental<int32*>::Value);
	always_check(TIsFundamental<void>::Value);

	always_check(!TIsArithmetic<FTestStructA>::Value);
	always_check(TIsArithmetic<int32>::Value);
	always_check(TIsArithmetic<float>::Value);
	always_check(!TIsArithmetic<int32*>::Value);
	always_check(!TIsArithmetic<void>::Value);

	always_check(!TIsScalar<FTestStructA>::Value);
	always_check(TIsScalar<int32>::Value);
	always_check(TIsScalar<float>::Value);
	always_check(TIsScalar<int32*>::Value);
	always_check(!TIsScalar<void>::Value);

	always_check(TIsObject<FTestStructA>::Value);
	always_check(!TIsObject<FTestStructA&>::Value);
	always_check(TIsObject<int32>::Value);
	always_check(TIsObject<int32*>::Value);
	always_check(!TIsObject<int32&>::Value);

	always_check(TIsCompound<FTestStructA>::Value);
	always_check(TIsCompound<FTestStructA&>::Value);
	always_check(!TIsCompound<int32>::Value);
	always_check(TIsCompound<int32*>::Value);
	always_check(TIsCompound<int32&>::Value);

	always_check(!TIsReference<int32>::Value);
	always_check(!TIsReference<int32*>::Value);
	always_check(TIsReference<int32&>::Value);
	always_check(TIsReference<int32&&>::Value);

	always_check(!TIsMemberPointer<FTestStructA>::Value);
	always_check(TIsMemberPointer<int32(FTestStructA::*)>::Value);

	// TypeProperties.h

	always_check(!TIsConst<int32>::Value);
	always_check(TIsConst<const int32>::Value);
	always_check(!TIsConst<volatile int32>::Value);
	always_check(TIsConst<const volatile int32>::Value);

	always_check(!TIsVolatile<int32>::Value);
	always_check(!TIsVolatile<const int32>::Value);
	always_check(TIsVolatile<volatile int32>::Value);
	always_check(TIsVolatile<const volatile int32>::Value);

	always_check(TIsTrivial<FTestStructB>::Value);
	always_check(!TIsTrivial<FTestStructC>::Value);

	always_check(TIsTriviallyCopyable<FTestStructB>::Value);
	always_check(!TIsTriviallyCopyable<FTestStructD>::Value);
	always_check(!TIsTriviallyCopyable<FTestStructE>::Value);

	always_check(TIsStandardLayout<FTestStructB>::Value);
	always_check(!TIsStandardLayout<FTestStructE>::Value);
	always_check(!TIsStandardLayout<FTestStructF>::Value);

	always_check(THasUniqueObjectRepresentations<FTestStructF>::Value);
	always_check(!THasUniqueObjectRepresentations<FTestStructG>::Value);

	always_check(TIsEmpty<FTestStructA>::Value);
	always_check(!TIsEmpty<FTestStructB>::Value);
	always_check(TIsEmpty<FTestStructC>::Value);
	always_check(TIsEmpty<FTestStructD>::Value);
	always_check(!TIsEmpty<FTestStructE>::Value);
	always_check(!TIsEmpty<FTestStructF>::Value);

	always_check(TIsPolymorphic<FTestStructE>::Value);
	always_check(!TIsPolymorphic<FTestStructF>::Value);

	always_check(TIsAbstract<FTestStructE>::Value);
	always_check(!TIsAbstract<FTestStructH>::Value);

	always_check(!TIsFinal<FTestStructE>::Value);
	always_check(TIsFinal<FTestStructH>::Value);

	always_check(!TIsAggregate<int32>::Value);
	always_check(TIsAggregate<int32[64]>::Value);
	always_check(TIsAggregate<FTestStructB>::Value);
	always_check(!TIsAggregate<FTestStructF>::Value);

	always_check(TIsSigned<signed>::Value);
	always_check(!TIsSigned<unsigned>::Value);

	always_check(!TIsUnsigned<signed>::Value);
	always_check(TIsUnsigned<unsigned>::Value);

	always_check(!TIsBoundedArray<int32>::Value);
	always_check(!TIsBoundedArray<int32[]>::Value);
	always_check(TIsBoundedArray<int32[64]>::Value);

	always_check(!TIsUnboundedArray<int32>::Value);
	always_check(TIsUnboundedArray<int32[]>::Value);
	always_check(!TIsUnboundedArray<int32[64]>::Value);

	always_check(!TIsScopedEnum<ETestEnum>::Value);
	always_check(TIsScopedEnum<ETestEnumClass>::Value);

	// SupportedOperations.h

	always_check(!TIsDefaultConstructible<FTestStructI>::Value);
	always_check(TIsDefaultConstructible<FTestStructJ>::Value);
	always_check(TIsDefaultConstructible<FTestStructK>::Value);
	always_check(!TIsDefaultConstructible<FTestStructL>::Value);

	always_check(!TIsTriviallyDefaultConstructible<FTestStructI>::Value);
	always_check(!TIsTriviallyDefaultConstructible<FTestStructJ>::Value);
	always_check(TIsTriviallyDefaultConstructible<FTestStructK>::Value);
	always_check(!TIsTriviallyDefaultConstructible<FTestStructL>::Value);

	always_check(!(TIsConstructible<FTestStructI, int32>::Value));
	always_check((TIsConstructible<FTestStructI, FTestStructI&>::Value));
	always_check((TIsConstructible<FTestStructI, int32, double>::Value));

	always_check(!(TIsTriviallyConstructible<FTestStructI, int32>::Value));
	always_check((TIsTriviallyConstructible<FTestStructI, FTestStructI&>::Value));
	always_check(!(TIsTriviallyConstructible<FTestStructI, int32, double>::Value));

	always_check(TIsCopyConstructible<FTestStructM>::Value);
	always_check(TIsCopyConstructible<FTestStructN>::Value);
	always_check(!TIsCopyConstructible<FTestStructO>::Value);

	always_check(!TIsTriviallyCopyConstructible<FTestStructM>::Value);
	always_check(TIsTriviallyCopyConstructible<FTestStructN>::Value);
	always_check(!TIsTriviallyCopyConstructible<FTestStructO>::Value);

	always_check(TIsMoveConstructible<FTestStructP>::Value);
	always_check(TIsMoveConstructible<FTestStructQ>::Value);
	always_check(!TIsMoveConstructible<FTestStructR>::Value);

	always_check(!TIsTriviallyMoveConstructible<FTestStructP>::Value);
	always_check(TIsTriviallyMoveConstructible<FTestStructQ>::Value);
	always_check(!TIsTriviallyMoveConstructible<FTestStructR>::Value);

	always_check(!(TIsAssignable<FTestStructI, FTestStructH>::Value));
	always_check((TIsAssignable<FTestStructI, FTestStructI&>::Value));
	always_check((TIsAssignable<FTestStructI, int32>::Value));

	always_check(!(TIsTriviallyAssignable<FTestStructI, FTestStructH>::Value));
	always_check((TIsTriviallyAssignable<FTestStructI, FTestStructI&>::Value));
	always_check(!(TIsTriviallyAssignable<FTestStructI, int32>::Value));

	always_check(TIsCopyAssignable<FTestStructM>::Value);
	always_check(TIsCopyAssignable<FTestStructN>::Value);
	always_check(!TIsCopyAssignable<FTestStructO>::Value);

	always_check(!TIsTriviallyCopyAssignable<FTestStructM>::Value);
	always_check(TIsTriviallyCopyAssignable<FTestStructN>::Value);
	always_check(!TIsTriviallyCopyAssignable<FTestStructO>::Value);

	always_check(TIsMoveAssignable<FTestStructP>::Value);
	always_check(TIsMoveAssignable<FTestStructQ>::Value);
	always_check(!TIsMoveAssignable<FTestStructR>::Value);

	always_check(!TIsTriviallyMoveAssignable<FTestStructP>::Value);
	always_check(TIsTriviallyMoveAssignable<FTestStructQ>::Value);
	always_check(!TIsTriviallyMoveAssignable<FTestStructR>::Value);

	always_check(TIsDestructible<FTestStructS>::Value);
	always_check(TIsDestructible<FTestStructT>::Value);
	always_check(!TIsDestructible<FTestStructU>::Value);

	always_check(!TIsTriviallyDestructible<FTestStructS>::Value);
	always_check(TIsTriviallyDestructible<FTestStructT>::Value);
	always_check(!TIsTriviallyDestructible<FTestStructU>::Value);

	always_check(!THasVirtualDestructor<FTestStructT>::Value);
	always_check(THasVirtualDestructor<FTestStructV>::Value);

	// Miscellaneous.h

	always_check(TRank<int32[1][2][3]>::Value == 3);
	always_check(TRank<int32[1][2][3][4]>::Value == 4);
	always_check(TRank<int32>::Value == 0);

	always_check(TExtent<int32[1][2][3]>::Value == 1);
	always_check((TExtent<int32[1][2][3][4], 1>::Value == 2));
	always_check(TExtent<int32[]>::Value == 0);

	always_check(!(TIsSame<int32, int64>::Value));
	always_check((TIsSame<int32, int32>::Value));

	always_check(!(TIsBaseOf<FTestStructH, FTestStructD>::Value));
	always_check(!(TIsBaseOf<FTestStructH, FTestStructE>::Value));
	always_check((TIsBaseOf<FTestStructE, FTestStructH>::Value));

	always_check((TIsConvertible<int32, uint32>::Value));
	always_check(!(TIsConvertible<FTestStructH*, FTestStructD*>::Value));
	always_check((TIsConvertible<FTestStructH*, FTestStructE*>::Value));
	always_check(!(TIsConvertible<FTestStructE*, FTestStructH*>::Value));
	always_check((TIsConvertible<FTestStructW, FTestStructV>::Value));

	always_check((TIsInvocable<int32()>::Value));
	always_check((TIsInvocable<int32(int32), int32>::Value));
	always_check(!(TIsInvocable<int32(int32), FTestStructA>::Value));
	always_check((TIsInvocable<int32(int32), int32>::Value));

	always_check((TIsInvocableResult<void, int32()>::Value));
	always_check((TIsInvocableResult<int32, int32()>::Value));
	always_check((TIsInvocableResult<int32, int32(int32), int32>::Value));
	always_check(!(TIsInvocableResult<int32, int32(int32), FTestStructA>::Value));
	always_check(!(TIsInvocableResult<FTestStructA, int32(int32), int32>::Value));

	always_check((TIsSame<int32, TRemoveConst<int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<int32*>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<int32&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<int32&&>::Type>::Value));
	always_check((TIsSame<int32, TRemoveConst<const int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<const volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveConst<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemoveVolatile<int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<int32*>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<int32&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<int32&&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<const int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveVolatile<volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<const volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveVolatile<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemoveCV<int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveCV<int32*>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveCV<int32&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveCV<int32&&>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCV<const int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCV<volatile int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCV<const volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveCV<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemovePointer<int32>::Type>::Value));
	always_check((TIsSame<int32, TRemovePointer<int32*>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<int32&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<int32&&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<const int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<const volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemovePointer<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemoveReference<int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveReference<int32*>::Type>::Value));
	always_check((TIsSame<int32, TRemoveReference<int32&>::Type>::Value));
	always_check((TIsSame<int32, TRemoveReference<int32&&>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveReference<const int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveReference<volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveReference<const volatile int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveReference<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemoveCVRef<int32>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveCVRef<int32*>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<int32&>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<int32&&>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<const int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<volatile int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<const volatile int32>::Type>::Value));
	always_check((TIsSame<int32, TRemoveCVRef<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TRemoveExtent<int32[1]>::Type>::Value));
	always_check(!(TIsSame<int32, TRemoveExtent<int32[1][2]>::Type>::Value));
	always_check((TIsSame<int32[2][3], TRemoveExtent<int32[1][2][3]>::Type>::Value));

	always_check((TIsSame<int32, TRemoveAllExtents<int32[1]>::Type>::Value));
	always_check((TIsSame<int32, TRemoveAllExtents<int32[1][2]>::Type>::Value));
	always_check((TIsSame<int32, TRemoveAllExtents<int32[1][2][3]>::Type>::Value));

	always_check((TIsSame<int32, TMakeSigned<int32>::Type>::Value));
	always_check((TIsSame<int32, TMakeSigned<uint32>::Type>::Value));

	always_check((TIsSame<uint32, TMakeUnsigned<int32>::Type>::Value));
	always_check((TIsSame<uint32, TMakeUnsigned<uint32>::Type>::Value));

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

	always_check((TIsSame<int32, TDecay<int32>::Type>::Value));
	always_check((TIsSame<int32*, TDecay<int32*>::Type>::Value));
	always_check((TIsSame<int32*, TDecay<int32[]>::Type>::Value));
	always_check((TIsSame<int32, TDecay<int32&>::Type>::Value));
	always_check((TIsSame<int32, TDecay<int32&&>::Type>::Value));
	always_check((TIsSame<int32, TDecay<const int32>::Type>::Value));
	always_check((TIsSame<int32, TDecay<volatile int32>::Type>::Value));
	always_check((TIsSame<int32, TDecay<const volatile int32>::Type>::Value));
	always_check((TIsSame<int32, TDecay<const volatile int32&>::Type>::Value));

	always_check((TIsSame<int32, TConditional<true, int32, int64>::Type>::Value));
	always_check((TIsSame<int64, TConditional<false, int32, int64>::Type>::Value));

	always_check((TIsSame<int, TUnderlyingType<ETestEnumClass>::Type>::Value));
	always_check((TIsSame<uint8, TUnderlyingType<ETestEnumClass8>::Type>::Value));
	always_check((TIsSame<uint32, TUnderlyingType<ETestEnumClass32>::Type>::Value));
	always_check((TIsSame<uint64, TUnderlyingType<ETestEnumClass64>::Type>::Value));

	always_check((TIsSame<int32, TInvokeResult<int32()>::Type>::Value));
	always_check((TIsSame<int32, TInvokeResult<int32(int32), int32>::Type>::Value));
//	always_check((TIsSame<char(&)[2], TInvokeResult<char(&())[2]>::Type>::Value));

	always_check((TIsSame<void, TVoid<int32>::Type>::Value));
	always_check((TIsSame<void, TVoid<int32, int64>::Type>::Value));

	// Common.h

	always_check((TIsSame<int32, TCommonType<int8, int32>::Type>::Value));
	always_check((TIsSame<int64, TCommonType<int8, int32, int64>::Type>::Value));
	always_check((TIsSame<double, TCommonType<float, double>::Type>::Value));

	always_check((TIsSame<int32, TCommonReference<int8, int32>::Type>::Value));
	always_check((TIsSame<int64, TCommonReference<int8, int32, int64>::Type>::Value));
	always_check((TIsSame<double, TCommonReference<float, double>::Type>::Value));

	// Swappable.h

	always_check(TIsSwappable<int32>::Value);
	always_check(TIsSwappable<FTestStructG>::Value);
	always_check(TIsSwappable<FTestStructN>::Value);
	always_check(!TIsSwappable<FSingleton>::Value);

	always_check((TIsSwappableWith<int32&, int32&>::Value));

	// CopyQualifiers.h

	always_check((TIsSame<               int32, TCopyConst<               int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyConst<const          int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyConst<const volatile int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyConst<               int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyConst<const          int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyConst<const volatile int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyConst<               int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyConst<const          int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyConst<const volatile int32, const volatile int32>::Type>::Value));
	
	always_check((TIsSame<               int32, TCopyVolatile<               int32,                int32>::Type>::Value));
	always_check((TIsSame<               int32, TCopyVolatile<const          int32,                int32>::Type>::Value));
	always_check((TIsSame<      volatile int32, TCopyVolatile<const volatile int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyVolatile<               int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyVolatile<const          int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyVolatile<const volatile int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyVolatile<               int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyVolatile<const          int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyVolatile<const volatile int32, const volatile int32>::Type>::Value));
	
	always_check((TIsSame<               int32, TCopyCV<               int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCV<const          int32,                int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCV<const volatile int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCV<               int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCV<const          int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCV<const volatile int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCV<               int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCV<const          int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCV<const volatile int32, const volatile int32>::Type>::Value));
	
	always_check((TIsSame<int32,   TCopyReference<int32,   int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyReference<int32,   int32& >::Type>::Value));
	always_check((TIsSame<int32&&, TCopyReference<int32,   int32&&>::Type>::Value));
	always_check((TIsSame<int32&,  TCopyReference<int32&,  int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyReference<int32&,  int32& >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyReference<int32&,  int32&&>::Type>::Value));
	always_check((TIsSame<int32&&, TCopyReference<int32&&, int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyReference<int32&&, int32& >::Type>::Value));
	always_check((TIsSame<int32&&, TCopyReference<int32&&, int32&&>::Type>::Value));

	always_check((TIsSame<               int32, TCopyCVRef<               int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<const          int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<               int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<const          int32, const          int32>::Type>::Value));
	always_check((TIsSame<      volatile int32, TCopyCVRef<      volatile int32,                int32>::Type>::Value));
	always_check((TIsSame<      volatile int32, TCopyCVRef<               int32,       volatile int32>::Type>::Value));
	always_check((TIsSame<      volatile int32, TCopyCVRef<      volatile int32,       volatile int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<const          int32,                int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCVRef<const volatile int32,                int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<               int32, const          int32>::Type>::Value));
	always_check((TIsSame<const          int32, TCopyCVRef<const          int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCVRef<const volatile int32, const          int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCVRef<               int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCVRef<const          int32, const volatile int32>::Type>::Value));
	always_check((TIsSame<const volatile int32, TCopyCVRef<const volatile int32, const volatile int32>::Type>::Value));
	
	always_check((TIsSame<int32,   TCopyCVRef<int32,   int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyCVRef<int32,   int32& >::Type>::Value));
	always_check((TIsSame<int32&&, TCopyCVRef<int32,   int32&&>::Type>::Value));
	always_check((TIsSame<int32&,  TCopyCVRef<int32&,  int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyCVRef<int32&,  int32& >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyCVRef<int32&,  int32&&>::Type>::Value));
	always_check((TIsSame<int32&&, TCopyCVRef<int32&&, int32  >::Type>::Value));
	always_check((TIsSame<int32&,  TCopyCVRef<int32&&, int32& >::Type>::Value));
	always_check((TIsSame<int32&&, TCopyCVRef<int32&&, int32&&>::Type>::Value));

	always_check((TIsSame<const           int32,   TCopyCVRef<const          int32,         int32  >::Type>::Value));
	always_check((TIsSame<const           int32&,  TCopyCVRef<               int32,   const int32& >::Type>::Value));
	always_check((TIsSame<const volatile  int32&&, TCopyCVRef<const volatile int32,   const int32&&>::Type>::Value));
	always_check((TIsSame<const           int32&,  TCopyCVRef<const          int32&,        int32  >::Type>::Value));
	always_check((TIsSame<const           int32&,  TCopyCVRef<const          int32&,  const int32& >::Type>::Value));
	always_check((TIsSame<const volatile  int32&,  TCopyCVRef<      volatile int32&,  const int32&&>::Type>::Value));
	always_check((TIsSame<const           int32&&, TCopyCVRef<const          int32&&,       int32  >::Type>::Value));
	always_check((TIsSame<const           int32&,  TCopyCVRef<const          int32&&, const int32& >::Type>::Value));
	always_check((TIsSame<const volatile  int32&&, TCopyCVRef<const volatile int32&&, const int32&&>::Type>::Value));

}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
