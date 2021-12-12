#include "Testing/TypeTraitsTesting.h"
#include "Misc/AssertionMacros.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/CompositeType.h"
#include "TypeTraits/TypeProperties.h"

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
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
