#pragma once

#include "CoreTypes.h"
#include "Numerics/Bit.h"
#include "Numerics/Limits.h"
#include "Numerics/Numbers.h"
#include "Templates/Tuple.h"
#include "Templates/Utility.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

#include <cmath>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Math)

NAMESPACE_PRIVATE_BEGIN

template <CFloatingPoint T>
struct TFloatingTypeTraits
{
	static_assert(sizeof(T) == -1, "Unsupported floating point type.");
};

template <>
struct TFloatingTypeTraits<float>
{
	// IEEE-754 single precision floating point format.
	// SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM

	using FIntegralT = uint32;

	static constexpr int SignBits     = 1;
	static constexpr int ExponentBits = 8;
	static constexpr int MantissaBits = 23;

	static_assert(SignBits + ExponentBits + MantissaBits == sizeof(float) * 8);

	static constexpr int ExponentBias = 127;

	static constexpr int SignShift     = 31;
	static constexpr int ExponentShift = 23;
	static constexpr int MantissaShift = 0;

	static constexpr FIntegralT SignMask     = 0x80000000;
	static constexpr FIntegralT ExponentMask = 0x7F800000;
	static constexpr FIntegralT MantissaMask = 0x007FFFFF;
};

template <>
struct TFloatingTypeTraits<double>
{
	// IEEE-754 double precision floating point format.
	// SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

	using FIntegralT = uint64;

	static constexpr int SignBits     = 1;
	static constexpr int ExponentBits = 11;
	static constexpr int MantissaBits = 52;

	static_assert(SignBits + ExponentBits + MantissaBits == sizeof(double) * 8);

	static constexpr int ExponentBias = 1023;

	static constexpr int SignShift     = 63;
	static constexpr int ExponentShift = 52;
	static constexpr int MantissaShift = 0;

	static constexpr FIntegralT SignMask     = 0x8000000000000000;
	static constexpr FIntegralT ExponentMask = 0x7FF0000000000000;
	static constexpr FIntegralT MantissaMask = 0x000FFFFFFFFFFFFF;
};

NAMESPACE_PRIVATE_END

#define FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(Func)                            \
	{                                                                            \
		if constexpr (CSameAs<T, float> || CSameAs<T, double>)                   \
		{                                                                        \
			return NAMESPACE_STD::Func(A);                                       \
		}                                                                        \
		                                                                         \
		else static_assert(sizeof(T) == -1, "Unsupported floating point type."); \
		                                                                         \
		return TNumericLimits<T>::QuietNaN();                                    \
	}

#define FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(Func)                            \
	{                                                                            \
		if constexpr (CSameAs<T, float> || CSameAs<T, double>)                   \
		{                                                                        \
			return NAMESPACE_STD::Func(A, B);                                    \
		}                                                                        \
		                                                                         \
		else static_assert(sizeof(T) == -1, "Unsupported floating point type."); \
		                                                                         \
		return TNumericLimits<T>::QuietNaN();                                    \
	}

#define RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(Concept, Func)       \
	template <Concept T, Concept U> requires (CCommonType<T, U>) \
	NODISCARD FORCEINLINE constexpr auto Func(T A, U B)          \
	{                                                            \
		using FCommonT = TCommonType<T, U>;                      \
		                                                         \
		return Math::Func(                                       \
			static_cast<FCommonT>(A),                            \
			static_cast<FCommonT>(B)                             \
		);                                                       \
	}

#define RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(Concept, Func)                     \
	template <Concept T, Concept U, Concept V> requires (CCommonType<T, U, V>) \
	NODISCARD FORCEINLINE constexpr auto Func(T A, U B, V C)                   \
	{                                                                          \
		using FCommonT = TCommonType<T, U, V>;                                 \
		                                                                       \
		return Math::Func(                                                     \
			static_cast<FCommonT>(A),                                          \
			static_cast<FCommonT>(B),                                          \
			static_cast<FCommonT>(C)                                           \
		);                                                                     \
	}

/** @return true if the given value is within a range ['MinValue', 'MaxValue'), false otherwise. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T IsWithin(T A, T MinValue, T MaxValue)
{
	return A >= MinValue && A < MaxValue;
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsWithin)

/** @return true if the given value is within a range ['MinValue', 'MaxValue'], false otherwise. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T IsWithinInclusive(T A, T MinValue, T MaxValue)
{
	return A >= MinValue && A <= MaxValue;
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsWithinInclusive)

/** @return The nearest integer not greater in magnitude than the given value. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Trunc(T A)
{
	if constexpr (CIntegral<T>) return A;

	FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(trunc)
}

/** @return The nearest integer not greater in magnitude than the given value. */
template <CArithmetic T, CArithmetic U>
NODISCARD FORCEINLINE constexpr T TruncTo(U A)
{
	if constexpr (CIntegral<T>)
	{
		if constexpr (!CIntegral<U>)
		{
			return static_cast<T>(A);
		}
		else return A;
	}

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return Math::Trunc(static_cast<T>(A));
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

/** @return The nearest integer not less than the given value. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Ceil(T A)
{
	if constexpr (CIntegral<T>) return A;

	FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(ceil)
}

/** @return The nearest integer not less than the given value. */
template <CArithmetic T, CArithmetic U>
NODISCARD FORCEINLINE constexpr T CeilTo(U A)
{
	if constexpr (CIntegral<T>)
	{
		if constexpr (!CIntegral<U>)
		{
			T I = Math::TruncTo<T>(A);

			I += static_cast<U>(I) < A;

			return I;
		}
		else return A;
	}

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return Math::Ceil(static_cast<T>(A));
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

/** @return The nearest integer not greater than the given value. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Floor(T A)
{
	if constexpr (CIntegral<T>) return A;

	FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(floor)
}

/** @return The nearest integer not greater than the given value. */
template <CArithmetic T, CArithmetic U>
NODISCARD FORCEINLINE constexpr T FloorTo(U A)
{
	if constexpr (CIntegral<T>)
	{
		if constexpr (!CIntegral<U>)
		{
			T I = Math::TruncTo<T>(A);

			I -= static_cast<U>(I) > A;

			return I;
		}
		else return A;
	}

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return Math::Floor(static_cast<T>(A));
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

/** @return The nearest integer to the given value, rounding away from zero in halfway cases. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Round(T A)
{
	if constexpr (CIntegral<T>) return A;

	FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(round)
}

/** @return The nearest integer to the given value, rounding away from zero in halfway cases. */
template <CArithmetic T, CArithmetic U>
NODISCARD FORCEINLINE constexpr T RoundTo(U A)
{
	if constexpr (CIntegral<T>)
	{
		if constexpr (!CIntegral<U>)
		{
			return Math::FloorTo<T>(A + static_cast<U>(0.5));
		}
		else return A;
	}

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return Math::Round(static_cast<T>(A));
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

/** @return The absolute value of the given value. */
template <CSigned T>
NODISCARD FORCEINLINE constexpr T Abs(T A)
{
	return A < 0 ? -A : A;
}

/** @return The absolute value of the given value. */
template <CUnsigned T>
NODISCARD FORCEINLINE constexpr T Abs(T A)
{
	return A;
}

/** @return 0 if the given value is zero, -1 if it is negative, and 1 if it is positive. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Sign(T A)
{
	if (A == static_cast<T>(0)) return static_cast<T>( 0);
	if (A <  static_cast<T>(0)) return static_cast<T>(-1);

	return static_cast<T>(1);
}

/** @return The minimum value of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto Min(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT B = Math::Min(InOther...);

		return A < B ? A : B;
	}
}

/** @return The maximum value of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto Max(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT B = Math::Max(InOther...);

		return A > B ? A : B;
	}

}

/** @return The index of the minimum value of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr size_t MinIndex(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return 0;

	else
	{
		size_t Index = Math::MinIndex(InOther...);

		bool bFlag;

		ForwardAsTuple(InOther...).Visit([&bFlag, A](auto B) { bFlag = A < B; }, Index);

		return bFlag ? 0 : Index + 1;
	}
}

/** @return The index of the maximum value of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr size_t MaxIndex(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return 0;

	else
	{
		size_t Index = Math::MaxIndex(InOther...);

		bool bFlag;

		ForwardAsTuple(InOther...).Visit([&bFlag, A](auto B) { bFlag = A > B; }, Index);

		return bFlag ? 0 : Index + 1;
	}
}

template <CIntegral T>
struct TDiv { T Quotient; T Remainder; };

/** @return The quotient and remainder of the division of the given values. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr Math::TDiv<T> Div(T A, T B)
{
	checkf(B != 0, TEXT("Illegal divisor. It must not be zero."));

	Math::TDiv<T> Result;

	Result.Quotient  = A / B;
	Result.Remainder = A % B;

	return Result;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, Div)

/** @return The quotient of the division of the given values and rounds up. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T DivAndCeil(T A, T B)
{
	return A >= 0 ? (A + B - 1) / B : A / B;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, DivAndCeil)

/** @return The quotient of the division of the given values and rounds down. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T DivAndFloor(T A, T B)
{
	return A >= 0 ? A / B : (A - B + 1) / B;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, DivAndFloor)

/** @return The quotient of the division of the given values and rounds to nearest. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T DivAndRound(T A, T B)
{
	return A >= 0
		? (A + B / 2    ) / B
		: (A - B / 2 + 1) / B;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, DivAndRound)

/** @return true if the given values are nearly equal, false otherwise. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr bool IsNearlyEqual(T A, T B, T Epsilon = TNumericLimits<T>::Epsilon())
{
	return Math::Abs<T>(A - B) <= Epsilon;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CArithmetic, IsNearlyEqual)
RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsNearlyEqual)

/** @return true if the given value is nearly zero, false otherwise. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr bool IsNearlyZero(T A, T Epsilon = TNumericLimits<T>::Epsilon())
{
	return Math::Abs<T>(A) <= Epsilon;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CArithmetic, IsNearlyZero)

/** @return true if the given value is infinity, false otherwise. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr bool IsInfinity(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return (IntegralValue & FTraits::ExponentMask) == FTraits::ExponentMask && (IntegralValue & FTraits::MantissaMask) == 0;
}

/** @return true if the given value is NaN, false otherwise. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr bool IsNaN(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return (IntegralValue & FTraits::ExponentMask) == FTraits::ExponentMask && (IntegralValue & FTraits::MantissaMask) != 0;
}

/** @return true if the given value is normal, false otherwise. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr bool IsNormal(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return (IntegralValue & FTraits::ExponentMask) != 0 && (IntegralValue & FTraits::ExponentMask) != FTraits::ExponentMask;
}

/** @return true if the given value is subnormal, false otherwise. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr bool IsDenorm(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return (IntegralValue & FTraits::ExponentMask) == 0 && (IntegralValue & FTraits::MantissaMask) != 0;
}

/** @return true if the given value is negative, even -0.0, false otherwise. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr bool IsNegative(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return (IntegralValue & FTraits::SignMask) >> FTraits::SignShift;
}

/** @return The exponent of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr uint Exponent(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return ((IntegralValue & FTraits::ExponentMask) >> FTraits::ExponentShift) - FTraits::ExponentBias;
}

/** @return The NaN value with the given payload. */
template <CFloatingPoint T, CUnsignedIntegral U>
NODISCARD FORCEINLINE constexpr T NaN(U Payload)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	checkf(Payload != 0, TEXT("Illegal payload. It must not be zero."));

	checkf(Payload < (static_cast<typename FTraits::FIntegralT>(1) << FTraits::MantissaBits), TEXT("Illegal payload. It must be less than 2^MantissaBits."));

	if (Payload == 0) return TNumericLimits<T>::QuietNaN();

	typename FTraits::FIntegralT ValidPayload = Payload & FTraits::MantissaMask;

	return Math::BitCast<T>(ValidPayload | FTraits::ExponentMask);
}

/** @return The NaN value with the given payload. */
template <CFloatingPoint T, CEnum U>
NODISCARD FORCEINLINE constexpr T NaN(U Payload)
{
	TUnderlyingType<U> IntegralValue = static_cast<TUnderlyingType<U>>(Payload);

	return Math::NaN<T>(IntegralValue);
}

/** @return The NaN payload of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr auto NaNPayload(T A)
{
	using FTraits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename FTraits::FIntegralT>(A);

	return IntegralValue & FTraits::MantissaMask;
}

/** @return The NaN payload of the given value. */
template <CEnum T, CFloatingPoint U>
NODISCARD FORCEINLINE constexpr auto NaNPayload(U A)
{
	return static_cast<T>(Math::NaNPayload(A));
}

/** @return The remainder of the floating point division operation. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T FMod(T A, T B) FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(fmod)

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, FMod)

/** @return The signed remainder of the floating point division operation. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Remainder(T A, T B) FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(remainder)

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, Remainder)

template <CFloatingPoint T>
struct TRemQuo { int Quotient; T Remainder; };

/** @return The signed remainder and the three last bits of the division operation. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr Math::TRemQuo<T> RemQuo(T A, T B)
{
	Math::TRemQuo<T> Result;

	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		Result.Remainder = NAMESPACE_STD::remquo(A, B, &Result.Quotient);

		return Result;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return Result;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, RemQuo)

template <CFloatingPoint T>
struct TModF { T IntegralPart; T FractionalPart; };

/** @return The integral and fractional parts of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr Math::TModF<T> ModF(T A)
{
	Math::TModF<T> Result;

	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		Result.FractionalPart = NAMESPACE_STD::modf(A, &Result.IntegralPart);

		return Result;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return Result;
}

/** @return The e raised to the given power. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Exp(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(exp)

/** @return The 2 raised to the given power. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Exp2(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(exp2)

/** @return The e raised to the given power, minus one. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T ExpMinus1(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(expm1)

/** @return The natural logarithm of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Log(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(log)

/** @return The base-2 logarithm of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Log2(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(log2)

/** @return The base-10 logarithm of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Log10(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(log10)

/** @return The natural logarithm of one plus the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Log1Plus(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(log1p)

/** @return The square of the given values. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Square(T A)
{
	return A * A;
}

/** @return The cube of the given values. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Cube(T A)
{
	return A * A * A;
}

/** @return The 'A' raised to the power of 'B'. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T Pow(T A, T B)
{
	if (B < 0)
	{
		checkf(false, TEXT("Illegal exponent. It must be greater than or equal to zero for integral."));

		return TNumericLimits<T>::QuietNaN();
	}

	T Result = 1;

	while (B != 0)
	{
		if (B & 1) Result *= A;
		A *= A;
		B >>= 1;
	}

	return Result;
}

/** @return The 'A' raised to the power of 'B'. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Pow(T A, T B) FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(pow)

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CArithmetic, Pow)

/** @return The square root of the given value. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T Sqrt(T A)
{
	if (A < 0)
	{
		checkf(false, TEXT("Illegal argument. It must be greater than or equal to zero."));

		return TNumericLimits<T>::QuietNaN();
	}

	if (A == 0) return 0;

	T X = A;

	while (true)
	{
		T Y = (X + A / X) / 2;

		if (Y >= X) return X;

		X = Y;
	}
}

/** @return The square root of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Sqrt(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(sqrt)

/** @return The cube root of the given value. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T Cbrt(T A)
{
	if (A < 0) return -Math::Cbrt(-A);

	if (A == 0) return 0;

	T X = A;

	while (true)
	{
		T Y = (X + A / (X * X)) / 2;

		if (Y >= X) return X;

		X = Y;
	}
}

/** @return The cube root of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Cbrt(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(cbrt)

/** @return The sum of the given value. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto Sum(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT Sum = A + Math::Sum(InOther...);

		return Sum;
	}
}

/** @return The sum of the squared values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto SquaredSum(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return Math::Square(A);

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT Sum = Math::Square(A) + Math::SquaredSum(InOther...);

		return Sum;
	}
}

/** @return The average of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto Avg(T A, Ts... InOther)
{
	using FSize =
		TConditional<sizeof...(Ts) <= 0xFF,       uint8,
		TConditional<sizeof...(Ts) <= 0xFFFF,     uint16,
		TConditional<sizeof...(Ts) <= 0xFFFFFFFF, uint32, uint64>>>;

	using FCommonT = TCommonType<FSize, T, Ts...>;

	constexpr FSize Count = sizeof...(Ts) + 1;

	if constexpr (Count == 1) return static_cast<FCommonT>(A);

	else if constexpr (Count == 2)
	{
		FCommonT Array[] = { A, InOther... };

		if (Array[1] < Array[0]) Swap(Array[0], Array[1]);

		return static_cast<FCommonT>(Array[0] + (Array[1] - Array[0]) / 2);
	}

	else if constexpr (CIntegral<FCommonT>)
	{
		Math::TDiv<FCommonT> Temp[] =
		{
			Math::Div(static_cast<FCommonT>(A      ), static_cast<FCommonT>(Count)),
			Math::Div(static_cast<FCommonT>(InOther), static_cast<FCommonT>(Count))...
		};

		FCommonT Quotient  = 0;
		FCommonT Remainder = 0;

		for (FSize I = 0; I != Count; ++I)
		{
			Quotient  += Temp[I].Quotient;
			Remainder += Temp[I].Remainder;
		}

		Quotient += Remainder / Count;

		return Quotient;
	}

	else
	{
		FCommonT Sum = A + Math::Sum(InOther...);

		return static_cast<FCommonT>(Sum / Count);
	}
}

/** @return The square root of the sum of the squares of the given values. */
template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
NODISCARD FORCEINLINE constexpr auto Hypot(T A, Ts... InOther)
{
	using FCommonT = TCommonType<T, Ts...>;

	constexpr size_t Count = sizeof...(Ts) + 1;

	if constexpr (Count == 1) return Math::Abs(A);

	else if constexpr (Count == 2)
	{
		FCommonT Array[] = { A, InOther... };

		if constexpr (CSameAs<FCommonT, float> || CSameAs<FCommonT, double>)
		{
			return NAMESPACE_STD::hypot(static_cast<FCommonT>(Array[0]), static_cast<FCommonT>(Array[1]));
		}

		else return static_cast<FCommonT>(Math::Sqrt(Math::Square(Array[0]) + Math::Square(Array[1])));
	}

	else if constexpr (Count == 3)
	{
		FCommonT Array[] = { A, InOther... };

		if constexpr (CSameAs<FCommonT, float> || CSameAs<FCommonT, double>)
		{
			return NAMESPACE_STD::hypot(static_cast<FCommonT>(Array[0]), static_cast<FCommonT>(Array[1]), static_cast<FCommonT>(Array[2]));
		}

		else return Math::Sqrt(Math::SquaredSum(A, InOther...));
	}

	else return Math::Sqrt(Math::SquaredSum(A, InOther...));
}

/** @return The sine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Sin(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(sin)

/** @return The cosine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Cos(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(cos)

/** @return The tangent of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Tan(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(tan)

/** @return The arc sine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Asin(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(asin)

/** @return The arc cosine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Acos(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(acos)

/** @return The arc tangent of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Atan(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(atan)

/** @return The arc tangent of 'A' / 'B'. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Atan2(T A, T B) FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(atan2)

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, Atan2)

/** @return The hyperbolic sine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Sinh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(sinh)

/** @return The hyperbolic cosine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Cosh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(cosh)

/** @return The hyperbolic tangent of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Tanh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(tanh)

/** @return The hyperbolic arc sine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Asinh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(asinh)

/** @return The hyperbolic arc cosine of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Acosh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(acosh)

/** @return The hyperbolic arc tangent of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Atanh(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(atanh)

/** @return The error function of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Erf(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(erf)

/** @return The complementary error function of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Erfc(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(erfc)

/** @return The gamma function of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T Gamma(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(tgamma)

/** @return The natural logarithm of the gamma function of the given value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T LogGamma(T A) FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS(lgamma)

/** @return The value of 'A' is multiplied by 2 raised to the power of 'B'. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T LdExp(T A, int B) FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS(ldexp)

/** @return The degrees of the given radian value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T RadiansToDegrees(T A)
{
	return A * (static_cast<T>(180) / Math::TNumbers<T>::Pi);
}

/** @return The radians of the given degree value. */
template <CFloatingPoint T>
NODISCARD FORCEINLINE constexpr T DegreesToRadians(T A)
{
	return A * (Math::TNumbers<T>::Pi / static_cast<T>(180));
}

/** @return The greatest common divisor of the given values. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T GCD(T A, T B)
{
	using FUnsignedT = TMakeUnsigned<T>;

	FUnsignedT C = Math::Abs(A);
	FUnsignedT D = Math::Abs(B);

	if (C == 0) return D;
	if (D == 0) return C;

	uint Shift = Math::CountRightZero(C | D);

	C >>= Math::CountRightZero(C);

	do
	{
		D >>= Math::CountRightZero(D);

		if (C > D) Swap(C, D);

		D -= C;
	}
	while (D != 0);

	return C << Shift;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, GCD)

/** @return The least common multiple of the given values. */
template <CIntegral T>
NODISCARD FORCEINLINE constexpr T LCM(T A, T B)
{
	A = Math::Abs(A);
	B = Math::Abs(B);

	if (A == 0 || B == 0) return 0;

	return A / Math::GCD(A, B) * B;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, LCM)

/** @return The value of 'A' is clamped to the range ['MinValue', 'MaxValue']. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Clamp(T A, T MinValue, T MaxValue)
{
	return Math::Min(Math::Max(A, MinValue), MaxValue);
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, Clamp)

/** @return The value of 'A' is clamped to the range ['MinValue', 'MaxValue'], but it wraps around the range when exceeded. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T WrappingClamp(T A, T MinValue, T MaxValue)
{
	if (MinValue > MaxValue)
	{
		checkf(false, TEXT("Illegal range. MinValue must be less than or equal to MaxValue."));

		return TNumericLimits<T>::QuietNaN();
	}

	if (MinValue == MaxValue) return MinValue;

	if constexpr (CSameAs<T, bool>) return A;

	else if constexpr (CIntegral<T>)
	{
		using FUnsignedT = TMakeUnsigned<T>;

		FUnsignedT Range = MaxValue - MinValue;

		if (A < MinValue)
		{
			FUnsignedT Modulo = static_cast<FUnsignedT>(MinValue - A) % Range;

			return Modulo != 0 ? MaxValue - Modulo : MinValue;
		}

		if (A > MaxValue)
		{
			FUnsignedT Modulo = static_cast<FUnsignedT>(A - MaxValue) % Range;

			return Modulo != 0 ? MinValue + Modulo : MaxValue;
		}

		return A;
	}

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		T Range = MaxValue - MinValue;

		if (A < MinValue) return MaxValue - Math::FMod(MinValue - A, Range);
		if (A > MaxValue) return MinValue + Math::FMod(A - MaxValue, Range);

		return A;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, WrappingClamp)

/** @return The linear interpolation of the given values. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T Lerp(T A, T B, T Alpha)
{
	return A + Alpha * (B - A);
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, Lerp)

/** @return The stable linear interpolation of the given values. */
template <CArithmetic T>
NODISCARD FORCEINLINE constexpr T LerpStable(T A, T B, T Alpha)
{
	return A * (static_cast<T>(1) - Alpha) + B * Alpha;
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, LerpStable)

#undef FORWARD_FLOATING_POINT_IMPLEMENT_1_ARGS
#undef FORWARD_FLOATING_POINT_IMPLEMENT_2_ARGS

#undef RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS
#undef RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
