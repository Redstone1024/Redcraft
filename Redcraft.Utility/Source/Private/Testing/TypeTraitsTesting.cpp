#include "Testing/TypeTraitsTesting.h"
#include "Misc/AssertionMacros.h"
#include "TypeTraits/HelperClasses.h"
#include "TypeTraits/PrimaryType.h"
#include "TypeTraits/CompositeType.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// Warning: The test here is not a complete test, it is only used to determine whether the environment supports the traits

int32 TestObject;
void TestFunction() { }

struct FTestStruct { };

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

	always_check(TypeTraits::TIsMemberObjectPointer<int32(FTestStruct::*)>::Value);
	always_check(!TypeTraits::TIsMemberObjectPointer<int32(FTestStruct::*)()>::Value);

	always_check(!TypeTraits::TIsMemberFunctionPointer<int32(FTestStruct::*)>::Value);
	always_check(TypeTraits::TIsMemberFunctionPointer<int32(FTestStruct::*)()>::Value);

	always_check(!TypeTraits::TIsEnum<int32>::Value);
	always_check(!TypeTraits::TIsEnum<FTestStruct>::Value);
	always_check(TypeTraits::TIsEnum<ETestEnum>::Value);
	always_check(TypeTraits::TIsEnum<ETestEnumClass>::Value);

	always_check(!TypeTraits::TIsUnion<int32>::Value);
	always_check(!TypeTraits::TIsUnion<FTestStruct>::Value);
	always_check(TypeTraits::TIsUnion<FTestUnion>::Value);

	always_check(!TypeTraits::TIsUnion<int32>::Value);
	always_check(!TypeTraits::TIsUnion<FTestStruct>::Value);
	always_check(TypeTraits::TIsUnion<FTestUnion>::Value);

	always_check(!TypeTraits::TIsFunction<int32>::Value);
	always_check(!TypeTraits::TIsFunction<FTestStruct>::Value);
	always_check(!TypeTraits::TIsFunction<FTestUnion>::Value);
	always_check(TypeTraits::TIsFunction<int32(int32)>::Value);

	always_check(!TypeTraits::TIsEnumClass<ETestEnum>::Value);
	always_check(TypeTraits::TIsEnumClass<ETestEnumClass>::Value);

	// CompositeType.h

	always_check(!TypeTraits::TIsFundamental<FTestStruct>::Value);
	always_check(TypeTraits::TIsFundamental<int32>::Value);
	always_check(TypeTraits::TIsFundamental<float>::Value);
	always_check(!TypeTraits::TIsFundamental<int32*>::Value);
	always_check(TypeTraits::TIsFundamental<void>::Value);

	always_check(!TypeTraits::TIsArithmetic<FTestStruct>::Value);
	always_check(TypeTraits::TIsArithmetic<int32>::Value);
	always_check(TypeTraits::TIsArithmetic<float>::Value);
	always_check(!TypeTraits::TIsArithmetic<int32*>::Value);
	always_check(!TypeTraits::TIsArithmetic<void>::Value);

	always_check(!TypeTraits::TIsScalar<FTestStruct>::Value);
	always_check(TypeTraits::TIsScalar<int32>::Value);
	always_check(TypeTraits::TIsScalar<float>::Value);
	always_check(TypeTraits::TIsScalar<int32*>::Value);
	always_check(!TypeTraits::TIsScalar<void>::Value);

	always_check(TypeTraits::TIsObject<FTestStruct>::Value);
	always_check(!TypeTraits::TIsObject<FTestStruct&>::Value);
	always_check(TypeTraits::TIsObject<int32>::Value);
	always_check(TypeTraits::TIsObject<int32*>::Value);
	always_check(!TypeTraits::TIsObject<int32&>::Value);

	always_check(TypeTraits::TIsCompound<FTestStruct>::Value);
	always_check(TypeTraits::TIsCompound<FTestStruct&>::Value);
	always_check(!TypeTraits::TIsCompound<int32>::Value);
	always_check(TypeTraits::TIsCompound<int32*>::Value);
	always_check(TypeTraits::TIsCompound<int32&>::Value);

	always_check(!TypeTraits::TIsReference<int32>::Value);
	always_check(!TypeTraits::TIsReference<int32*>::Value);
	always_check(TypeTraits::TIsReference<int32&>::Value);
	always_check(TypeTraits::TIsReference<int32&&>::Value);

	always_check(!TypeTraits::TIsMemberPointer<FTestStruct>::Value);
	always_check(TypeTraits::TIsMemberPointer<int32(FTestStruct::*)>::Value);
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
