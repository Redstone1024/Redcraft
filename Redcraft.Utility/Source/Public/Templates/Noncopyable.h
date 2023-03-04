#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

// WARNING: Using these helper classes as base classes may raise potential EBO issues and is not recommended for objects with strict size constraints.

/** A class indicates that a derived class cannot be copied. */
struct FNoncopyable
{
	FNoncopyable()                               = default;
	FNoncopyable(const FNoncopyable&)            = delete;
	FNoncopyable(FNoncopyable&&)                 = default;
	FNoncopyable& operator=(const FNoncopyable&) = delete;
	FNoncopyable& operator=(FNoncopyable&&)      = default;
};

/** A class indicates that a derived class cannot be moved. */
struct FNonmovable
{
	FNonmovable()                              = default;
	FNonmovable(const FNonmovable&)            = default;
	FNonmovable(FNonmovable&&)                 = delete;
	FNonmovable& operator=(const FNonmovable&) = default;
	FNonmovable& operator=(FNonmovable&&)      = delete;
};

/** A class indicates that a derived class cannot be copied or moved. */
struct FSingleton // : FNoncopyable, FNonmovable
{
	// NOTE: Multiple inheritance is no longer used here, as that would break the EBO in MSVC

	FSingleton()                             = default;
	FSingleton(const FSingleton&)            = delete;
	FSingleton(FSingleton&&)                 = delete;
	FSingleton& operator=(const FSingleton&) = delete;
	FSingleton& operator=(FSingleton&&)      = delete;
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
