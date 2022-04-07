#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

REDCRAFTUTILITY_API void TestTemplates();
REDCRAFTUTILITY_API void TestInvoke();
REDCRAFTUTILITY_API void TestReferenceWrapper();
REDCRAFTUTILITY_API void TestOptional();
REDCRAFTUTILITY_API void TestVariant();
REDCRAFTUTILITY_API void TestAny();
REDCRAFTUTILITY_API void TestTuple();
REDCRAFTUTILITY_API void TestFunction();
REDCRAFTUTILITY_API void TestMiscTemplates();

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
