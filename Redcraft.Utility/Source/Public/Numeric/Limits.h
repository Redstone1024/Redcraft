#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"

#include <climits>
#include <limits>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

static_assert(CHAR_BIT == 8, "CHAR_BIT must be 8");

enum class EFloatRoundMode : uint8
{
	TowardZero,
	ToNearest,
	Upward,
	Downward,
	Unknown,
};

enum class EFloatDenormMode : uint8
{
	Absent,
	Present,
	Unknown,
};

template <CArithmetic T>
struct TNumericLimits;

template <typename T> struct TNumericLimits<const          T> : public TNumericLimits<T> { };
template <typename T> struct TNumericLimits<      volatile T> : public TNumericLimits<T> { };
template <typename T> struct TNumericLimits<const volatile T> : public TNumericLimits<T> { };

template <CArithmetic T>
struct TNumericLimits
{
	static_assert(!CFloatingPoint<T> || NAMESPACE_STD::numeric_limits<T>::has_infinity,      "Floating point types must have infinity.");
	static_assert(!CFloatingPoint<T> || NAMESPACE_STD::numeric_limits<T>::has_quiet_NaN,     "Floating point types must have quiet NaN.");
	static_assert(!CFloatingPoint<T> || NAMESPACE_STD::numeric_limits<T>::has_signaling_NaN, "Floating point types must have signaling NaN.");

	static constexpr bool bIsExact = NAMESPACE_STD::numeric_limits<T>::is_exact;

	static constexpr EFloatRoundMode RoundMode =
		NAMESPACE_STD::numeric_limits<T>::round_style == NAMESPACE_STD::round_toward_zero         ? EFloatRoundMode::TowardZero :
		NAMESPACE_STD::numeric_limits<T>::round_style == NAMESPACE_STD::round_to_nearest          ? EFloatRoundMode::ToNearest  :
		NAMESPACE_STD::numeric_limits<T>::round_style == NAMESPACE_STD::round_toward_infinity     ? EFloatRoundMode::Upward     :
		NAMESPACE_STD::numeric_limits<T>::round_style == NAMESPACE_STD::round_toward_neg_infinity ? EFloatRoundMode::Downward   :
		EFloatRoundMode::Unknown;

	static constexpr EFloatDenormMode DenormMode =
		NAMESPACE_STD::numeric_limits<T>::has_denorm == NAMESPACE_STD::denorm_absent  ? EFloatDenormMode::Absent  :
		NAMESPACE_STD::numeric_limits<T>::has_denorm == NAMESPACE_STD::denorm_present ? EFloatDenormMode::Present :
		EFloatDenormMode::Unknown;

	static constexpr bool bHasDenormLoss = NAMESPACE_STD::numeric_limits<T>::has_denorm_loss;

	static constexpr bool bIsIEEE754 = NAMESPACE_STD::numeric_limits<T>::is_iec559 && sizeof(T) <= 8;

	static constexpr bool bIsModulo = NAMESPACE_STD::numeric_limits<T>::is_modulo;

	static constexpr int Radix = NAMESPACE_STD::numeric_limits<T>::radix;

	static constexpr int Digits   = NAMESPACE_STD::numeric_limits<T>::digits;
	static constexpr int Digits10 = NAMESPACE_STD::numeric_limits<T>::digits10;

	static constexpr int MaxExponent   = NAMESPACE_STD::numeric_limits<T>::max_exponent;
	static constexpr int MaxExponent10 = NAMESPACE_STD::numeric_limits<T>::max_exponent10;
	static constexpr int MinExponent   = NAMESPACE_STD::numeric_limits<T>::min_exponent;
	static constexpr int MinExponent10 = NAMESPACE_STD::numeric_limits<T>::min_exponent10;

	static constexpr bool bInterrupt = NAMESPACE_STD::numeric_limits<T>::traps;

	static constexpr T Min() { return NAMESPACE_STD::numeric_limits<T>::lowest(); }
	static constexpr T Max() { return NAMESPACE_STD::numeric_limits<T>::max();    }

	static constexpr T Epsilon() { return NAMESPACE_STD::numeric_limits<T>::epsilon(); }

	static constexpr T Infinity()     { return NAMESPACE_STD::numeric_limits<T>::infinity();      }
	static constexpr T QuietNaN()     { return NAMESPACE_STD::numeric_limits<T>::quiet_NaN();     }
	static constexpr T SignalingNaN() { return NAMESPACE_STD::numeric_limits<T>::signaling_NaN(); }

	static constexpr T MinNormal() { return NAMESPACE_STD::numeric_limits<T>::min();        }
	static constexpr T MinDenorm() { return NAMESPACE_STD::numeric_limits<T>::denorm_min(); }
};

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
