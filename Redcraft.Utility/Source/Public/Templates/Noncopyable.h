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
	FNonmovable(const FNonmovable&&) = delete;
	FNonmovable& operator=(const FNonmovable&&) = delete;
};

struct FSingleton : public FNoncopyable, public FNonmovable { };

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
