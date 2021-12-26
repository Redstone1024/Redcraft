#include "Testing/TypeTraitsTesting.h"
#include "Misc/AssertionMacros.h"
#include "TypeTraits/TypeTraits.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Warning: The test here is not a complete test, it is only used to determine whether the environment supports the traits

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

void TestTypeTraits()
{
	// HelperClasses.h

	always_check((TypeTraits::TConstant<int32, 1>::Value == 1));
	always_check(static_cast<int32>(TypeTraits::TConstant<int32, 2>::Value) == 2);
	always_check((TypeTraits::TConstant<int32, 3>() == 3));

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

	// Miscellaneous.h

	always_check(TypeTraits::TRank<int32[1][2][3]>::Value == 3);
	always_check(TypeTraits::TRank<int32[1][2][3][4]>::Value == 4);
	always_check(TypeTraits::TRank<int32>::Value == 0);

	always_check(TypeTraits::TExtent<int32[1][2][3]>::Value == 1);
	always_check((TypeTraits::TExtent<int32[1][2][3][4], 1>::Value == 2));
	always_check(TypeTraits::TExtent<int32[]>::Value == 0);

	always_check(!(TypeTraits::TIsSame<int32, int64>::Value));
	always_check((TypeTraits::TIsSame<int32, int32>::Value));

	always_check(!(TypeTraits::TIsBaseOf<FTestStructH, FTestStructD>::Value));
	always_check(!(TypeTraits::TIsBaseOf<FTestStructH, FTestStructE>::Value));
	always_check((TypeTraits::TIsBaseOf<FTestStructE, FTestStructH>::Value));

	always_check((TypeTraits::TIsConvertible<int32, uint32>::Value));
	always_check(!(TypeTraits::TIsConvertible<FTestStructH*, FTestStructD*>::Value));
	always_check((TypeTraits::TIsConvertible<FTestStructH*, FTestStructE*>::Value));
	always_check(!(TypeTraits::TIsConvertible<FTestStructE*, FTestStructH*>::Value));
	always_check((TypeTraits::TIsConvertible<FTestStructW, FTestStructV>::Value));

	always_check((TypeTraits::TIsInvocable<void, int32()>::Value));
	always_check((TypeTraits::TIsInvocable<int32, int32()>::Value));
	always_check((TypeTraits::TIsInvocable<int32, int32(int32), int32>::Value));
	always_check(!(TypeTraits::TIsInvocable<int32, int32(int32), FTestStructA>::Value));
	always_check(!(TypeTraits::TIsInvocable<FTestStructA, int32(int32), int32>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<int32*>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<int32&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<int32&&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<const int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<const volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveConst<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<int32*>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<int32&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<int32&&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<const int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<const volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveVolatile<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<int32*>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<int32&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<int32&&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<const int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<volatile int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<const volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveCV<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<int32*>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<int32&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<int32&&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<const int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<const volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemovePointer<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<int32*>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<int32&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<int32&&>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<const int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<const volatile int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveReference<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<int32>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<int32*>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<int32&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<int32&&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<const int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<volatile int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<const volatile int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveCVRef<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveExtent<int32[1]>::Type>::Value));
	always_check(!(TypeTraits::TIsSame<int32, TypeTraits::TRemoveExtent<int32[1][2]>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32[2][3], TypeTraits::TRemoveExtent<int32[1][2][3]>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveAllExtents<int32[1]>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveAllExtents<int32[1][2]>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TRemoveAllExtents<int32[1][2][3]>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TMakeSigned<int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TMakeSigned<uint32>::Type>::Value));

	always_check((TypeTraits::TIsSame<uint32, TypeTraits::TMakeUnsigned<int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<uint32, TypeTraits::TMakeUnsigned<uint32>::Type>::Value));

	TypeTraits::TAlignedStorage<32, 4>::Type Aligned4;
	TypeTraits::TAlignedStorage<32, 8>::Type Aligned8;
	TypeTraits::TAlignedStorage<32, 16>::Type Aligned16;
	TypeTraits::TAlignedStorage<32, 32>::Type Aligned32;
	always_check((int64)(&Aligned4) % 4 == 0);
	always_check((int64)(&Aligned8) % 8 == 0);
	always_check((int64)(&Aligned16) % 16 == 0);
	always_check((int64)(&Aligned32) % 32 == 0);

	always_check(sizeof(TypeTraits::TAlignedUnion<8, int32, int32>::Type) == 8);
	always_check(sizeof(TypeTraits::TAlignedUnion<0, int8, int32>::Type) == 4);
	always_check(sizeof(TypeTraits::TAlignedUnion<0, int32, int64>::Type) == 8);
	always_check(sizeof(TypeTraits::TAlignedUnion<0, int32, double>::Type) == 8);

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32*, TypeTraits::TDecay<int32*>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32*, TypeTraits::TDecay<int32[]>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<int32&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<int32&&>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<const int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<volatile int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<const volatile int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TDecay<const volatile int32&>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TConditional<true, int32, int64>::Type>::Value));
	always_check((TypeTraits::TIsSame<int64, TypeTraits::TConditional<false, int32, int64>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TCommonType<int8, int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<int64, TypeTraits::TCommonType<int8, int32, int64>::Type>::Value));
	always_check((TypeTraits::TIsSame<double, TypeTraits::TCommonType<float, double>::Type>::Value));

	always_check((TypeTraits::TIsSame<int, TypeTraits::TUnderlyingType<ETestEnumClass>::Type>::Value));
	always_check((TypeTraits::TIsSame<uint8, TypeTraits::TUnderlyingType<ETestEnumClass8>::Type>::Value));
	always_check((TypeTraits::TIsSame<uint32, TypeTraits::TUnderlyingType<ETestEnumClass32>::Type>::Value));
	always_check((TypeTraits::TIsSame<uint64, TypeTraits::TUnderlyingType<ETestEnumClass64>::Type>::Value));

	always_check((TypeTraits::TIsSame<int32, TypeTraits::TInvokeResult<int32()>::Type>::Value));
	always_check((TypeTraits::TIsSame<int32, TypeTraits::TInvokeResult<int32(int32), int32>::Type>::Value));
//	always_check((TypeTraits::TIsSame<char(&)[2], TypeTraits::TInvokeResult<char(&())[2]>::Type>::Value));

	always_check((TypeTraits::TIsSame<void, TypeTraits::TVoid<int32>::Type>::Value));
	always_check((TypeTraits::TIsSame<void, TypeTraits::TVoid<int32, int64>::Type>::Value));

}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
