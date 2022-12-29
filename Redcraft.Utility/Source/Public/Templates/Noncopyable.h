#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

/** A class indicates that a derived class cannot be copied. */
struct FNoncopyable
{
	FNoncopyable() = default;
	FNoncopyable(const FNoncopyable&) = delete;
	FNoncopyable& operator=(const FNoncopyable&) = delete;
};

/** A class indicates that a derived class cannot be moved. */
struct FNonmovable
{
	FNonmovable() = default;
	FNonmovable(FNonmovable&&) = delete;
	FNonmovable& operator=(FNonmovable&&) = delete;
};

/** A class indicates that a derived class cannot be copied or moved. */
struct FSingleton // : FNoncopyable, FNonmovable
{
	// NOTE: Multiple inheritance is no longer used here, as that would break the EBO in MSVC

	FSingleton() = default;
	FSingleton(const FSingleton&) = delete;
	FSingleton(FSingleton&&) = delete;
	FSingleton& operator=(const FSingleton&) = delete;
	FSingleton& operator=(FSingleton&&) = delete;
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
