#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

REDCRAFTUTILITY_API void TestMemory();
REDCRAFTUTILITY_API void TestAlignment();
REDCRAFTUTILITY_API void TestMemoryBuffer();
REDCRAFTUTILITY_API void TestMemoryMalloc();
REDCRAFTUTILITY_API void TestMemoryOperator();
REDCRAFTUTILITY_API void TestPointerTraits();
REDCRAFTUTILITY_API void TestUniquePointer();
REDCRAFTUTILITY_API void TestSharedPointer();
REDCRAFTUTILITY_API void TestObserverPointer();

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
