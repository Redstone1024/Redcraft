#include "Testing/TypeTraitsTesting.h"
#include "Misc/AssertionMacros.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/TypeProperties.h"
#include "TypeTraits/SupportedOperations.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Warning: The test here is not a complete test, it is only used to determine whether the environment supports the traits

int32 TestObject;
void TestFunction() { }

struct FTestStructA { };
struct FTestStructB { int32 Member; };
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

enum ETestEnum { };
enum class ETestEnumClass { };

union FTestUnion { };

void TestTypeTraits()
{
	// HelperClasses.h

	always_check(TypeTraits::TIntegralConstant<1>::Value == 1);
	always_check(static_cast<int32>(TypeTraits::TIntegralConstant<2>::Value) == 2);
	always_check(TypeTraits::TIntegralConstant<3>() == 3);

	always_check(!TypeTraits::FFalse::Value);
	always_check(TypeTraits::FTrue::Value);

	always_check(TypeTraits::TAnd<TypeTraits::FTrue>::Value);
	always_check(!TypeTraits::TAnd<TypeTraits::FFalse>::Value);
	always_check((TypeTraits::TAnd<TypeTraits::FTrue, TypeTraits::FTrue>::Value));
	always_check(!(TypeTraits::TAnd<TypeTraits::FFalse, TypeTraits::FTrue>::Value));
	always_check(!(TypeTraits::TAnd<TypeTraits::FTrue, TypeTraits::FFalse>::Value));
	always_check(!(TypeTraits::TAnd<TypeTraits::FFalse, TypeTraits::FFalse>::Value));

	always_check(TypeTraits::TOr<TypeTraits::FTrue>::Value);
	always_check(!TypeTraits::TOr<TypeTraits::FFalse>::Value);
	always_check((TypeTraits::TOr<TypeTraits::FTrue, TypeTraits::FTrue>::Value));
	always_check((TypeTraits::TOr<TypeTraits::FFalse, TypeTraits::FTrue>::Value));
	always_check((TypeTraits::TOr<TypeTraits::FTrue, TypeTraits::FFalse>::Value));
	always_check(!(TypeTraits::TOr<TypeTraits::FFalse, TypeTraits::FFalse>::Value));

	always_check(!TypeTraits::TNot<TypeTraits::FTrue>::Value);
	always_check(TypeTraits::TNot<TypeTraits::FFalse>::Value);

	// PrimaryType.h

	always_check(!TypeTraits::TIsVoid<int32>::Value);
	always_check(TypeTraits::TIsVoid<void>::Value);
	always_check(TypeTraits::TIsVoid<const void>::Value);
	always_check(TypeTraits::TIsVoid<const volatile void>::Value);
	always_check(TypeTraits::TIsVoid<volatile void>::Value);

	always_check(!TypeTraits::TIsNullPointer<int32>::Value);
	always_check(TypeTraits::TIsNullPointer<nullptr_t>::Value);

	always_check(TypeTraits::TIsIntegral<int32>::Value);
	always_check(!TypeTraits::TIsIntegral<float>::Value);

	always_check(!TypeTraits::TIsFloatingPoint<int32>::Value);
	always_check(TypeTraits::TIsFloatingPoint<float>::Value);

	always_check(!TypeTraits::TIsArray<int32>::Value);
	always_check(TypeTraits::TIsArray<int32[]>::Value);
	always_check(TypeTraits::TIsArray<int32[10]>::Value);

	always_check(!TypeTraits::TIsPointer<int32>::Value);
	always_check(TypeTraits::TIsPointer<int32*>::Value);

	always_check(!TypeTraits::TIsLValueReference<int32>::Value);
	always_check(TypeTraits::TIsLValueReference<int32&>::Value);
	always_check(!TypeTraits::TIsLValueReference<int32&&>::Value);
	
	always_check(!TypeTraits::TIsRValueReference<int32>::Value);
	always_check(!TypeTraits::TIsRValueReference<int32&>::Value);
	always_check(TypeTraits::TIsRValueReference<int32&&>::Value);

	always_check(TypeTraits::TIsMemberObjectPointer<int32(FTestStructA::*)>::Value);
	always_check(!TypeTraits::TIsMemberObjectPointer<int32(FTestStructA::*)()>::Value);

	always_check(!TypeTraits::TIsMemberFunctionPointer<int32(FTestStructA::*)>::Value);
	always_check(TypeTraits::TIsMemberFunctionPointer<int32(FTestStructA::*)()>::Value);

	always_check(!TypeTraits::TIsEnum<int32>::Value);
	always_check(!TypeTraits::TIsEnum<FTestStructA>::Value);
	always_check(TypeTraits::TIsEnum<ETestEnum>::Value);
	always_check(TypeTraits::TIsEnum<ETestEnumClass>::Value);

	always_check(!TypeTraits::TIsUnion<int32>::Value);
	always_check(!TypeTraits::TIsUnion<FTestStructA>::Value);
	always_check(TypeTraits::TIsUnion<FTestUnion>::Value);

	always_check(!TypeTraits::TIsUnion<int32>::Value);
	always_check(!TypeTraits::TIsUnion<FTestStructA>::Value);
	always_check(TypeTraits::TIsUnion<FTestUnion>::Value);

	always_check(!TypeTraits::TIsFunction<int32>::Value);
	always_check(!TypeTraits::TIsFunction<FTestStructA>::Value);
	always_check(!TypeTraits::TIsFunction<FTestUnion>::Value);
	always_check(TypeTraits::TIsFunction<int32(int32)>::Value);

	// CompositeType.h

	always_check(!TypeTraits::TIsFundamental<FTestStructA>::Value);
	always_check(TypeTraits::TIsFundamental<int32>::Value);
	always_check(TypeTraits::TIsFundamental<float>::Value);
	always_check(!TypeTraits::TIsFundamental<int32*>::Value);
	always_check(TypeTraits::TIsFundamental<void>::Value);

	always_check(!TypeTraits::TIsArithmetic<FTestStructA>::Value);
	always_check(TypeTraits::TIsArithmetic<int32>::Value);
	always_check(TypeTraits::TIsArithmetic<float>::Value);
	always_check(!TypeTraits::TIsArithmetic<int32*>::Value);
	always_check(!TypeTraits::TIsArithmetic<void>::Value);

	always_check(!TypeTraits::TIsScalar<FTestStructA>::Value);
	always_check(TypeTraits::TIsScalar<int32>::Value);
	always_check(TypeTraits::TIsScalar<float>::Value);
	always_check(TypeTraits::TIsScalar<int32*>::Value);
	always_check(!TypeTraits::TIsScalar<void>::Value);

	always_check(TypeTraits::TIsObject<FTestStructA>::Value);
	always_check(!TypeTraits::TIsObject<FTestStructA&>::Value);
	always_check(TypeTraits::TIsObject<int32>::Value);
	always_check(TypeTraits::TIsObject<int32*>::Value);
	always_check(!TypeTraits::TIsObject<int32&>::Value);

	always_check(TypeTraits::TIsCompound<FTestStructA>::Value);
	always_check(TypeTraits::TIsCompound<FTestStructA&>::Value);
	always_check(!TypeTraits::TIsCompound<int32>::Value);
	always_check(TypeTraits::TIsCompound<int32*>::Value);
	always_check(TypeTraits::TIsCompound<int32&>::Value);

	always_check(!TypeTraits::TIsReference<int32>::Value);
	always_check(!TypeTraits::TIsReference<int32*>::Value);
	always_check(TypeTraits::TIsReference<int32&>::Value);
	always_check(TypeTraits::TIsReference<int32&&>::Value);

	always_check(!TypeTraits::TIsMemberPointer<FTestStructA>::Value);
	always_check(TypeTraits::TIsMemberPointer<int32(FTestStructA::*)>::Value);

	// TypeProperties.h

	always_check(!TypeTraits::TIsConst<int32>::Value);
	always_check(TypeTraits::TIsConst<const int32>::Value);
	always_check(!TypeTraits::TIsConst<volatile int32>::Value);
	always_check(TypeTraits::TIsConst<const volatile int32>::Value);

	always_check(!TypeTraits::TIsVolatile<int32>::Value);
	always_check(!TypeTraits::TIsVolatile<const int32>::Value);
	always_check(TypeTraits::TIsVolatile<volatile int32>::Value);
	always_check(TypeTraits::TIsVolatile<const volatile int32>::Value);

	always_check(TypeTraits::TIsTrivial<FTestStructB>::Value);
	always_check(!TypeTraits::TIsTrivial<FTestStructC>::Value);

	always_check(TypeTraits::TIsTriviallyCopyable<FTestStructB>::Value);
	always_check(!TypeTraits::TIsTriviallyCopyable<FTestStructD>::Value);
	always_check(!TypeTraits::TIsTriviallyCopyable<FTestStructE>::Value);

	always_check(TypeTraits::TIsStandardLayout<FTestStructB>::Value);
	always_check(!TypeTraits::TIsStandardLayout<FTestStructE>::Value);
	always_check(!TypeTraits::TIsStandardLayout<FTestStructF>::Value);

	always_check(TypeTraits::THasUniqueObjectRepresentations<FTestStructF>::Value);
	always_check(!TypeTraits::THasUniqueObjectRepresentations<FTestStructG>::Value);

	always_check(TypeTraits::TIsEmpty<FTestStructA>::Value);
	always_check(!TypeTraits::TIsEmpty<FTestStructB>::Value);
	always_check(TypeTraits::TIsEmpty<FTestStructC>::Value);
	always_check(TypeTraits::TIsEmpty<FTestStructD>::Value);
	always_check(!TypeTraits::TIsEmpty<FTestStructE>::Value);
	always_check(!TypeTraits::TIsEmpty<FTestStructF>::Value);

	always_check(TypeTraits::TIsPolymorphic<FTestStructE>::Value);
	always_check(!TypeTraits::TIsPolymorphic<FTestStructF>::Value);

	always_check(TypeTraits::TIsAbstract<FTestStructE>::Value);
	always_check(!TypeTraits::TIsAbstract<FTestStructH>::Value);

	always_check(!TypeTraits::TIsFinal<FTestStructE>::Value);
	always_check(TypeTraits::TIsFinal<FTestStructH>::Value);

	always_check(!TypeTraits::TIsAggregate<int32>::Value);
	always_check(TypeTraits::TIsAggregate<int32[64]>::Value);
	always_check(TypeTraits::TIsAggregate<FTestStructB>::Value);
	always_check(!TypeTraits::TIsAggregate<FTestStructF>::Value);

	always_check(TypeTraits::TIsSigned<signed>::Value);
	always_check(!TypeTraits::TIsSigned<unsigned>::Value);

	always_check(!TypeTraits::TIsUnsigned<signed>::Value);
	always_check(TypeTraits::TIsUnsigned<unsigned>::Value);

	always_check(!TypeTraits::TIsBoundedArray<int32>::Value);
	always_check(!TypeTraits::TIsBoundedArray<int32[]>::Value);
	always_check(TypeTraits::TIsBoundedArray<int32[64]>::Value);

	always_check(!TypeTraits::TIsUnboundedArray<int32>::Value);
	always_check(TypeTraits::TIsUnboundedArray<int32[]>::Value);
	always_check(!TypeTraits::TIsUnboundedArray<int32[64]>::Value);

	always_check(!TypeTraits::TIsScopedEnum<ETestEnum>::Value);
	always_check(TypeTraits::TIsScopedEnum<ETestEnumClass>::Value);

	// SupportedOperations.h

	always_check(!TypeTraits::TIsDefaultConstructible<FTestStructI>::Value);
	always_check(TypeTraits::TIsDefaultConstructible<FTestStructJ>::Value);
	always_check(TypeTraits::TIsDefaultConstructible<FTestStructK>::Value);
	always_check(!TypeTraits::TIsDefaultConstructible<FTestStructL>::Value);

	always_check(!TypeTraits::TIsTriviallyDefaultConstructible<FTestStructI>::Value);
	always_check(!TypeTraits::TIsTriviallyDefaultConstructible<FTestStructJ>::Value);
	always_check(TypeTraits::TIsTriviallyDefaultConstructible<FTestStructK>::Value);
	always_check(!TypeTraits::TIsTriviallyDefaultConstructible<FTestStructL>::Value);

	always_check(!(TypeTraits::TIsConstructible<FTestStructI, int32>::Value));
	always_check((TypeTraits::TIsConstructible<FTestStructI, FTestStructI&>::Value));
	always_check((TypeTraits::TIsConstructible<FTestStructI, int32, double>::Value));

	always_check(!(TypeTraits::TIsTriviallyConstructible<FTestStructI, int32>::Value));
	always_check((TypeTraits::TIsTriviallyConstructible<FTestStructI, FTestStructI&>::Value));
	always_check(!(TypeTraits::TIsTriviallyConstructible<FTestStructI, int32, double>::Value));

	always_check(TypeTraits::TIsCopyConstructible<FTestStructM>::Value);
	always_check(TypeTraits::TIsCopyConstructible<FTestStructN>::Value);
	always_check(!TypeTraits::TIsCopyConstructible<FTestStructO>::Value);

	always_check(!TypeTraits::TIsTriviallyCopyConstructible<FTestStructM>::Value);
	always_check(TypeTraits::TIsTriviallyCopyConstructible<FTestStructN>::Value);
	always_check(!TypeTraits::TIsTriviallyCopyConstructible<FTestStructO>::Value);

	always_check(TypeTraits::TIsMoveConstructible<FTestStructP>::Value);
	always_check(TypeTraits::TIsMoveConstructible<FTestStructQ>::Value);
	always_check(!TypeTraits::TIsMoveConstructible<FTestStructR>::Value);

	always_check(!TypeTraits::TIsTriviallyMoveConstructible<FTestStructP>::Value);
	always_check(TypeTraits::TIsTriviallyMoveConstructible<FTestStructQ>::Value);
	always_check(!TypeTraits::TIsTriviallyMoveConstructible<FTestStructR>::Value);

	always_check(!(TypeTraits::TIsAssignable<FTestStructI, FTestStructH>::Value));
	always_check((TypeTraits::TIsAssignable<FTestStructI, FTestStructI&>::Value));
	always_check((TypeTraits::TIsAssignable<FTestStructI, int32>::Value));

	always_check(!(TypeTraits::TIsTriviallyAssignable<FTestStructI, FTestStructH>::Value));
	always_check((TypeTraits::TIsTriviallyAssignable<FTestStructI, FTestStructI&>::Value));
	always_check(!(TypeTraits::TIsTriviallyAssignable<FTestStructI, int32>::Value));

	always_check(TypeTraits::TIsCopyAssignable<FTestStructM>::Value);
	always_check(TypeTraits::TIsCopyAssignable<FTestStructN>::Value);
	always_check(!TypeTraits::TIsCopyAssignable<FTestStructO>::Value);

	always_check(!TypeTraits::TIsTriviallyCopyAssignable<FTestStructM>::Value);
	always_check(TypeTraits::TIsTriviallyCopyAssignable<FTestStructN>::Value);
	always_check(!TypeTraits::TIsTriviallyCopyAssignable<FTestStructO>::Value);

	always_check(TypeTraits::TIsMoveAssignable<FTestStructP>::Value);
	always_check(TypeTraits::TIsMoveAssignable<FTestStructQ>::Value);
	always_check(!TypeTraits::TIsMoveAssignable<FTestStructR>::Value);

	always_check(!TypeTraits::TIsTriviallyMoveAssignable<FTestStructP>::Value);
	always_check(TypeTraits::TIsTriviallyMoveAssignable<FTestStructQ>::Value);
	always_check(!TypeTraits::TIsTriviallyMoveAssignable<FTestStructR>::Value);

	always_check(TypeTraits::TIsDestructible<FTestStructS>::Value);
	always_check(TypeTraits::TIsDestructible<FTestStructT>::Value);
	always_check(!TypeTraits::TIsDestructible<FTestStructU>::Value);

	always_check(!TypeTraits::TIsTriviallyDestructible<FTestStructS>::Value);
	always_check(TypeTraits::TIsTriviallyDestructible<FTestStructT>::Value);
	always_check(!TypeTraits::TIsTriviallyDestructible<FTestStructU>::Value);

	always_check(!TypeTraits::THasVirtualDestructor<FTestStructT>::Value);
	always_check(TypeTraits::THasVirtualDestructor<FTestStructV>::Value);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
