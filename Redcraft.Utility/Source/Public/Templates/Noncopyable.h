#pragma once

#include "CoreTypes.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FNoncopyable
{
	FNoncopyable() = default;
	FNoncopyable(const FNoncopyable&) = delete;
	FNoncopyable& operator=(const FNoncopyable&) = delete;
};

struct FNonmovable
{
	FNonmovable() = default;
	FNonmovable(FNonmovable&&) = delete;
	FNonmovable& operator=(FNonmovable&&) = delete;
};

// Multiple inheritance is no longer used here, as that would break the EBO in MSVC
struct FSingleton // : FNoncopyable, FNonmovable
{
	FSingleton() = default;
	FSingleton(const FSingleton&) = delete;
	FSingleton(FSingleton&&) = delete;
	FSingleton& operator=(const FSingleton&) = delete;
	FSingleton& operator=(FSingleton&&) = delete;
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
