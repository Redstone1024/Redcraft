#pragma once

#include "CoreTypes.h"
#include "Numeric/Bit.h"
#include "Numeric/Math.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Math)

/** Seeds the random number generator. Return the previous seed. */
NODISCARD REDCRAFTUTILITY_API uint32 Seed(uint32 InSeed = 0);

/** @return The generated random number within the range of [0, 0x7FFFFFFF). */
NODISCARD REDCRAFTUTILITY_API uint32 Rand();

/** @return The generated random number within the range of [0, A). */
template <CIntegral T>
NODISCARD FORCEINLINE T Rand(T A)
{
	constexpr uint32 RandStateNum = 0x7FFFFFFF;

	if (A <= 0) return 0;

	if (A <= RandStateNum) return Rand() % A;

	constexpr uint32 BlockSize  = Math::BitFloor(RandStateNum);
	constexpr uint   BlockWidth = Math::CountRightZero(BlockSize);

	const T BlockNum = Math::DivAndCeil(A, BlockSize);

	T Result = 0;

	for (T I = 0; I < BlockNum; ++I)
	{
		Result ^= Rand();

		Result <<= BlockWidth;
	}

	return Math::Abs(Result) % A;
}

/** @return The generated random number within the range of [0, A). */
template <CFloatingPoint T>
NODISCARD FORCEINLINE T Rand(T A)
{
	constexpr uint32 RandStateNum = 0x7FFFFFFF;

	if (Math::IsNegative(A)) return TNumericLimits<T>::QuietNaN();

	constexpr size_t BlockNum = Math::DivAndCeil(sizeof(T), 4);

	T Multiplier = A;

	Multiplier /= BlockNum;
	Multiplier /= RandStateNum;

	T Result = 0;

	for (size_t I = 0; I < BlockNum; ++I)
	{
		Result += Rand() * Multiplier;
	}

	return Result;
}

/** @return The generated random number within the range of [A, B). */
template <CArithmetic T, CArithmetic U> requires (CCommonType<T, U>)
NODISCARD FORCEINLINE auto RandWithin(T A, U B)
{
	using FCommonT = TCommonType<T, U>;

	if (A == B) return static_cast<FCommonT>(A);

	if (A > B) return Math::RandWithin(B, A);

	return static_cast<FCommonT>(A + Math::Rand(B - A));
}

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
