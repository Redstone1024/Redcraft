#include "Numerics/Random.h"

#include "Templates/Atomic.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Math)

NAMESPACE_UNNAMED_BEGIN

TAtomic<uint32> GRandState = 586103306;

NAMESPACE_UNNAMED_END

uint32 Seed(uint32 InSeed)
{
	uint32 OldSeed = GRandState.Load(EMemoryOrder::Relaxed);

	if (InSeed != 0) GRandState.Store(InSeed, EMemoryOrder::Relaxed);

	return OldSeed;
}

uint32 Rand()
{
	uint32 Result;

	GRandState.FetchFn(
		[&Result](uint32 Value)
		{
			Result = Value;

			Result ^= Result << 13;
			Result ^= Result >> 17;
			Result ^= Result << 5;

			return Result;
		},
		EMemoryOrder::Relaxed
	);

	return Result % 0x7FFFFFFF;
}

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
