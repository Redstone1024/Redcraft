#pragma once

#include "CoreTypes.h"
#include "Numeric/Bit.h"
#include "Numeric/Limits.h"
#include "Templates/Tuple.h"
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

#define RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(Concept, Func)       \
	template <Concept T, Concept U> requires (CCommonType<T, U>) \
	FORCEINLINE constexpr auto Func(T A, U B)                    \
	{                                                            \
		return Math::Func<TCommonType<T, U>>(A, B);              \
	}

#define RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(Concept, Func)                     \
	template <Concept T, Concept U, Concept V> requires (CCommonType<T, U, V>) \
	FORCEINLINE constexpr auto Func(T A, U B, V C)                             \
	{                                                                          \
		return Math::Func<TCommonType<T, U, V>>(A, B, C);                      \
	}

template <CArithmetic T>
FORCEINLINE constexpr T IsWithin(T A, T MinValue, T MaxValue)
{
	return A >= MinValue && A < MaxValue;
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsWithin)

template <CArithmetic T>
FORCEINLINE constexpr T IsWithinInclusive(T A, T MinValue, T MaxValue)
{
	return A >= MinValue && A <= MaxValue;
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsWithinInclusive)

template <CArithmetic T>
FORCEINLINE constexpr T Trunc(T A)
{
	if constexpr (CIntegral<T>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::trunc(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T, CArithmetic U>
FORCEINLINE constexpr T TruncTo(U A)
{
	if constexpr (CIntegral<T> && CIntegral<U>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::trunc(static_cast<float>(A));
	}

	else if constexpr (CIntegral<T>)
	{
		return static_cast<T>(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T>
FORCEINLINE constexpr T Ceil(T A)
{
	if constexpr (CIntegral<T>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::ceil(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T, CArithmetic U>
FORCEINLINE constexpr T CeilTo(U A)
{
	if constexpr (CIntegral<T> && CIntegral<U>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::ceil(static_cast<float>(A));
	}

	else if constexpr (CIntegral<T>)
	{
		T I = Math::TruncTo<T>(A);

		I += static_cast<U>(I) < A;

		return I;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T>
FORCEINLINE constexpr T Floor(T A)
{
	if constexpr (CIntegral<T>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::floor(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T, CArithmetic U>
FORCEINLINE constexpr T FloorTo(U A)
{
	if constexpr (CIntegral<T> && CIntegral<U>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::floor(static_cast<float>(A));
	}

	else if constexpr (CIntegral<T>)
	{
		T I = Math::TruncTo<T>(A);

		I -= static_cast<U>(I) > A;

		return I;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T>
FORCEINLINE constexpr T Round(T A)
{
	if constexpr (CIntegral<T>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::round(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T, CArithmetic U>
FORCEINLINE constexpr T RoundTo(U A)
{
	if constexpr (CIntegral<T> && CIntegral<U>) return A;

	else if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::round(static_cast<float>(A));
	}

	else if constexpr (CIntegral<T>)
	{
		return Math::FloorTo<T>(A + static_cast<U>(0.5));
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CSigned T>
FORCEINLINE constexpr T Abs(T A)
{
	return A < 0 ? -A : A;
}

template <CUnsigned T>
FORCEINLINE constexpr T Abs(T A)
{
	return A;
}

template <CArithmetic T>
FORCEINLINE constexpr T Sign(T A)
{
	if (A == static_cast<T>(0)) return static_cast<T>( 0);
	if (A <  static_cast<T>(0)) return static_cast<T>(-1);

	return static_cast<T>(1);
}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr auto Min(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT B = Math::Min(InOther...);

		return A < B ? A : B;
	}
}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr auto Max(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT B = Math::Max(InOther...);

		return A > B ? A : B;
	}

}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr size_t MinIndex(T A, Ts... InOther)
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

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr size_t MaxIndex(T A, Ts... InOther)
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
FORCEINLINE constexpr auto Div(T A, T B)
{
	checkf(B != 0, TEXT("Illegal divisor. It must not be zero."));

	struct { T Quotient; T Remainder; } Result;

	Result.Quotient  = A / B;
	Result.Remainder = A % B;

	return Result;
}

template <CIntegral T>
FORCEINLINE constexpr T DivAndCeil(T A, T B)
{
	return (A + B - 1) / B;
}

template <CIntegral T>
FORCEINLINE constexpr T DivAndFloor(T A, T B)
{
	return A / B;
}

template <CIntegral T>
FORCEINLINE constexpr T DivAndRound(T A, T B)
{
	return A >= 0
		? (A + B / 2    ) / B
		: (A - B / 2 + 1) / B;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, Div)

template <CArithmetic T>
FORCEINLINE constexpr bool IsNearlyEqual(T A, T B, T Epsilon = TNumericLimits<T>::Epsilon())
{
	return Math::Abs<T>(A - B) <= Epsilon;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CArithmetic, IsNearlyEqual)
RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, IsNearlyEqual)

template <CArithmetic T>
FORCEINLINE constexpr bool IsNearlyZero(T A, T Epsilon = TNumericLimits<T>::Epsilon())
{
	return Math::Abs<T>(A) <= Epsilon;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CArithmetic, IsNearlyZero)

template <CFloatingPoint T>
FORCEINLINE constexpr T IsInfinity(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return (IntegralValue & Traits::ExponentMask) == Traits::ExponentMask && (IntegralValue & Traits::MantissaMask) == 0;
}

template <CFloatingPoint T>
FORCEINLINE constexpr T IsNaN(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return (IntegralValue & Traits::ExponentMask) == Traits::ExponentMask && (IntegralValue & Traits::MantissaMask) != 0;
}

template <CFloatingPoint T>
FORCEINLINE constexpr T IsNormal(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return (IntegralValue & Traits::ExponentMask) != 0 && (IntegralValue & Traits::ExponentMask) != Traits::ExponentMask;
}

template <CFloatingPoint T>
FORCEINLINE constexpr T IsDenorm(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return (IntegralValue & Traits::ExponentMask) == 0 && (IntegralValue & Traits::MantissaMask) != 0;
}

template <CFloatingPoint T>
FORCEINLINE constexpr bool IsNegative(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return (IntegralValue & Traits::SignMask) >> Traits::SignShift;
}

template <CFloatingPoint T>
FORCEINLINE constexpr uint Exponent(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return ((IntegralValue & Traits::ExponentMask) >> Traits::ExponentShift) - Traits::ExponentBias;
}

template <CFloatingPoint T, CUnsignedIntegral U>
FORCEINLINE constexpr T NaN(U Payload)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	checkf(Payload != 0, TEXT("Illegal payload. It must not be zero."));

	checkf(Payload < (static_cast<typename Traits::FIntegralT>(1) << Traits::MantissaBits), TEXT("Illegal payload. It must be less than 2^MantissaBits."));

	if (Payload == 0) return TNumericLimits<T>::QuietNaN();

	typename Traits::FIntegralT ValidPayload = Payload & Traits::MantissaMask;

	return Math::BitCast<T>(ValidPayload | Traits::ExponentMask);
}

template <CFloatingPoint T, CEnum U>
FORCEINLINE constexpr T NaN(U Payload)
{
	TUnderlyingType<U> IntegralValue = static_cast<TUnderlyingType<U>>(Payload);

	return Math::NaN<T>(IntegralValue);
}

template <CFloatingPoint T>
FORCEINLINE constexpr auto NaNPayload(T A)
{
	using Traits = NAMESPACE_PRIVATE::TFloatingTypeTraits<T>;

	auto IntegralValue = Math::BitCast<typename Traits::FIntegralT>(A);

	return IntegralValue & Traits::MantissaMask;
}

template <CEnum T, CFloatingPoint U>
FORCEINLINE constexpr auto NaNPayload(U A)
{
	return static_cast<T>(Math::NaNPayload(A));
}

template <CFloatingPoint T>
FORCEINLINE constexpr T FMod(T A, T B)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::fmod(A, B);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, FMod)

template <CFloatingPoint T>
FORCEINLINE constexpr T Remainder(T A, T B)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::remainder(A, B);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, Remainder)

template <CFloatingPoint T>
FORCEINLINE constexpr auto RemQuo(T A, T B)
{
	struct { int Quotient; T Remainder; } Result;

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
FORCEINLINE constexpr auto ModF(T A)
{
	struct { T IntegralPart; T FractionalPart; } Result;

	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		Result.FractionalPart = NAMESPACE_STD::modf(A, &Result.IntegralPart);

		return Result;
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return Result;
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Exp(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::exp(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Exp2(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::exp2(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T ExpMinus1(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::expm1(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Log(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::log(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Log2(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::log2(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Log10(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::log10(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Log1Plus(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::log1p(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T>
FORCEINLINE constexpr T Square(T A)
{
	return A * A;
}

template <CArithmetic T>
FORCEINLINE constexpr T Cube(T A)
{
	return A * A * A;
}

template <CIntegral T>
FORCEINLINE constexpr T Pow(T A, T B)
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

template <CFloatingPoint T>
FORCEINLINE constexpr T Pow(T A, T B)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::pow(A, B);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CFloatingPoint, Pow)

template <CIntegral T>
FORCEINLINE constexpr T Sqrt(T A)
{
	if (A < 0)
	{
		checkf(false, TEXT("Illegal argument. It must be greater than or equal to zero."));

		return TNumericLimits<T>::QuietNaN();
	}

	T X = A;

	while (true)
	{
		T Y = (X + A / X) / 2;

		if (Y >= X) return X;

		X = Y;
	}
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Sqrt(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::sqrt(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CIntegral T>
FORCEINLINE constexpr T Cbrt(T A)
{
	if (A < 0) return -Math::Cbrt(-A);

	T X = A;

	while (true)
	{
		T Y = (X + A / (X * X)) / 2;

		if (Y >= X) return X;

		X = Y;
	}
}

template <CFloatingPoint T>
FORCEINLINE constexpr T Cbrt(T A)
{
	if constexpr (CSameAs<T, float> || CSameAs<T, double>)
	{
		return NAMESPACE_STD::cbrt(A);
	}

	else static_assert(sizeof(T) == -1, "Unsupported floating point type.");

	return TNumericLimits<T>::QuietNaN();
}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr auto Sum(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT Sum = A + Math::Sum(InOther...);

		return Sum;
	}
}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr auto SquaredSum(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return Math::Square(A);

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT Sum = A + Math::SquaredSum(InOther...);

		return Sum;
	}
}

template <CArithmetic T, CArithmetic... Ts> requires (CCommonType<T, Ts...>)
FORCEINLINE constexpr auto Avg(T A, Ts... InOther)
{
	if constexpr (sizeof...(Ts) == 0) return A;

	else
	{
		using FCommonT = TCommonType<T, Ts...>;

		FCommonT Sum = A + Math::Sum(InOther...);

		return Sum / (sizeof...(Ts) + 1);
	}
}

template <CArithmetic T>
FORCEINLINE constexpr T Hypot(T A)
{
	return Math::Abs(A);
}

template <CArithmetic T, CArithmetic U>
FORCEINLINE constexpr auto Hypot(T A, U B)
{
	using FCommonT = TCommonType<T, U>;

	if constexpr (CIntegral<FCommonT>) return static_cast<FCommonT>(Math::Sqrt(Math::Square(A) + Math::Square(B)));

	else if constexpr (CSameAs<FCommonT, float> || CSameAs<FCommonT, double>)
	{
		return NAMESPACE_STD::hypot(static_cast<FCommonT>(A), static_cast<FCommonT>(B));
	}

	else static_assert(sizeof(FCommonT) == -1, "Unsupported floating point type.");

	return TNumericLimits<FCommonT>::QuietNaN();
}

template <CArithmetic T, CArithmetic U, CArithmetic V>
FORCEINLINE constexpr auto Hypot(T A, U B, V C)
{
	using FCommonT = TCommonType<T, U, V>;

	if constexpr (CIntegral<FCommonT>) return static_cast<FCommonT>(Math::Sqrt(Math::SquaredSum(A, B, C)));

	else if constexpr (CSameAs<FCommonT, float> || CSameAs<FCommonT, double>)
	{
		return NAMESPACE_STD::hypot(static_cast<FCommonT>(A), static_cast<FCommonT>(B), static_cast<FCommonT>(C));
	}

	else static_assert(sizeof(FCommonT) == -1, "Unsupported floating point type.");

	return TNumericLimits<FCommonT>::QuietNaN();
}

template <CArithmetic... Ts> requires (CCommonType<Ts...>)
FORCEINLINE constexpr auto Hypot(Ts... InOther)
{
	return Math::Sqrt(Math::SquaredSum(InOther...));
}

template <CArithmetic T>
FORCEINLINE constexpr T Clamp(T A, T MinValue, T MaxValue)
{
	return Math::Min(Math::Max(A, MinValue), MaxValue);
}

RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS(CArithmetic, Clamp)

template <CArithmetic T>
FORCEINLINE constexpr T WrappingClamp(T A, T MinValue, T MaxValue)
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

#undef RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS
#undef RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
