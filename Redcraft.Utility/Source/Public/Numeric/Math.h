#pragma once

#include "CoreTypes.h"
#include "Numeric/Bit.h"
#include "Numeric/Limits.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/AssertionMacros.h"

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

template <CIntegral T>
FORCEINLINE constexpr auto Div(T LHS, T RHS)
{
	checkf(RHS != 0, TEXT("Illegal divisor. It must not be zero."));

	struct { T Quotient; T Remainder; } Result;

	Result.Quotient  = LHS / RHS;
	Result.Remainder = LHS % RHS;

	return Result;
}

RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS(CIntegral, Div)

template <CArithmetic T>
FORCEINLINE constexpr bool IsNearlyEqual(T LHS, T RHS, T Epsilon = TNumericLimits<T>::Epsilon())
{
	return Math::Abs<T>(LHS - RHS) <= Epsilon;
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

#undef RESOLVE_ARITHMETIC_AMBIGUITY_2_ARGS
#undef RESOLVE_ARITHMETIC_AMBIGUITY_3_ARGS

NAMESPACE_END(Math)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
