#include "Testing/Testing.h"

#include "Numeric/Numeric.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

NAMESPACE_PRIVATE_BEGIN

void TestLiteral()
{
//	always_check((CSameAs<decltype(0i8),  int8 >));
	always_check((CSameAs<decltype(0i16), int16>));
	always_check((CSameAs<decltype(0i32), int32>));
	always_check((CSameAs<decltype(0i64), int64>));

	always_check((CSameAs<decltype(0u8),  uint8 >));
	always_check((CSameAs<decltype(0u16), uint16>));
	always_check((CSameAs<decltype(0u32), uint32>));
	always_check((CSameAs<decltype(0u64), uint64>));

	always_check((CSameAs<decltype(0imax),  intmax>));
	always_check((CSameAs<decltype(0umax), uintmax>));

	always_check((CSameAs<decltype(0.0f32), float32>));
	always_check((CSameAs<decltype(0.0f64), float64>));
}

void TestBit()
{
	always_check(Math::ByteSwap<uint8 >(0x00              ) == 0x00              );
	always_check(Math::ByteSwap<uint16>(0x0011            ) == 0x1100            );
	always_check(Math::ByteSwap<uint32>(0x00112233        ) == 0x33221100        );
	always_check(Math::ByteSwap<uint64>(0x0011223344556677) == 0x7766554433221100);

	always_check(Math::IsSingleBit(0b0000u) == false);
	always_check(Math::IsSingleBit(0b0001u) == true );
	always_check(Math::IsSingleBit(0b0010u) == true );
	always_check(Math::IsSingleBit(0b0011u) == false);
	always_check(Math::IsSingleBit(0b0100u) == true );
	always_check(Math::IsSingleBit(0b0101u) == false);
	always_check(Math::IsSingleBit(0b0110u) == false);
	always_check(Math::IsSingleBit(0b0111u) == false);
	always_check(Math::IsSingleBit(0b1000u) == true );
	always_check(Math::IsSingleBit(0b1001u) == false);

	always_check(Math::CountAllZero(0b00000000u8) == 8);
	always_check(Math::CountAllZero(0b11111111u8) == 0);
	always_check(Math::CountAllZero(0b00011101u8) == 4);

	always_check(Math::CountAllOne(0b00000000u8) == 0);
	always_check(Math::CountAllOne(0b11111111u8) == 8);
	always_check(Math::CountAllOne(0b00011101u8) == 4);

	always_check(Math::CountLeftZero(0b00000000u8) == 8);
	always_check(Math::CountLeftZero(0b11111111u8) == 0);
	always_check(Math::CountLeftZero(0b00011100u8) == 3);

	always_check(Math::CountLeftOne(0b00000000u8) == 0);
	always_check(Math::CountLeftOne(0b11111111u8) == 8);
	always_check(Math::CountLeftOne(0b11100011u8) == 3);

	always_check(Math::CountRightZero(0b00000000u8) == 8);
	always_check(Math::CountRightZero(0b11111111u8) == 0);
	always_check(Math::CountRightZero(0b00011100u8) == 2);

	always_check(Math::CountRightOne(0b00000000u8) == 0);
	always_check(Math::CountRightOne(0b11111111u8) == 8);
	always_check(Math::CountRightOne(0b11100011u8) == 2);

	always_check(Math::BitWidth(0b0000u) == 0);
	always_check(Math::BitWidth(0b0001u) == 1);
	always_check(Math::BitWidth(0b0010u) == 2);
	always_check(Math::BitWidth(0b0011u) == 2);
	always_check(Math::BitWidth(0b0100u) == 3);
	always_check(Math::BitWidth(0b0101u) == 3);
	always_check(Math::BitWidth(0b0110u) == 3);
	always_check(Math::BitWidth(0b0111u) == 3);

	always_check(Math::BitCeil(0b00000000u) == 0b00000001u);
	always_check(Math::BitCeil(0b00000001u) == 0b00000001u);
	always_check(Math::BitCeil(0b00000010u) == 0b00000010u);
	always_check(Math::BitCeil(0b00000011u) == 0b00000100u);
	always_check(Math::BitCeil(0b00000100u) == 0b00000100u);
	always_check(Math::BitCeil(0b00000101u) == 0b00001000u);
	always_check(Math::BitCeil(0b00000110u) == 0b00001000u);
	always_check(Math::BitCeil(0b00000111u) == 0b00001000u);
	always_check(Math::BitCeil(0b00001000u) == 0b00001000u);
	always_check(Math::BitCeil(0b00001001u) == 0b00010000u);

	always_check(Math::BitFloor(0b00000000u) == 0b00000000u);
	always_check(Math::BitFloor(0b00000001u) == 0b00000001u);
	always_check(Math::BitFloor(0b00000010u) == 0b00000010u);
	always_check(Math::BitFloor(0b00000011u) == 0b00000010u);
	always_check(Math::BitFloor(0b00000100u) == 0b00000100u);
	always_check(Math::BitFloor(0b00000101u) == 0b00000100u);
	always_check(Math::BitFloor(0b00000110u) == 0b00000100u);
	always_check(Math::BitFloor(0b00000111u) == 0b00000100u);
	always_check(Math::BitFloor(0b00001000u) == 0b00001000u);
	always_check(Math::BitFloor(0b00001001u) == 0b00001000u);

	always_check(Math::RotateLeft(0b00011101u8,  0) == 0b00011101u8);
	always_check(Math::RotateLeft(0b00011101u8,  1) == 0b00111010u8);
	always_check(Math::RotateLeft(0b00011101u8,  4) == 0b11010001u8);
	always_check(Math::RotateLeft(0b00011101u8,  9) == 0b00111010u8);
	always_check(Math::RotateLeft(0b00011101u8, -1) == 0b10001110u8);

	always_check(Math::RotateRight(0b00011101u8,  0) == 0b00011101u8);
	always_check(Math::RotateRight(0b00011101u8,  1) == 0b10001110u8);
	always_check(Math::RotateRight(0b00011101u8,  4) == 0b11010001u8);
	always_check(Math::RotateRight(0b00011101u8,  9) == 0b10001110u8);
	always_check(Math::RotateRight(0b00011101u8, -1) == 0b00111010u8);
}

void TestMath()
{
	always_check( Math::IsWithin(0, 0, 1));
	always_check(!Math::IsWithin(1, 0, 1));
	always_check(!Math::IsWithin(2, 0, 1));

	always_check( Math::IsWithinInclusive(0, 0, 1));
	always_check( Math::IsWithinInclusive(1, 0, 1));
	always_check(!Math::IsWithinInclusive(2, 0, 1));

	always_check(Math::IsNearlyEqual(Math::Trunc(2.00), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(2.25), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(2.75), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(3.00), 3.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Trunc(-2.00), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(-2.25), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(-2.75), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Trunc(-3.00), -3.0, 1e-8));

	always_check(Math::TruncTo<int>(2.00) == 2);
	always_check(Math::TruncTo<int>(2.25) == 2);
	always_check(Math::TruncTo<int>(2.75) == 2);
	always_check(Math::TruncTo<int>(3.00) == 3);

	always_check(Math::TruncTo<int>(-2.00) == -2);
	always_check(Math::TruncTo<int>(-2.25) == -2);
	always_check(Math::TruncTo<int>(-2.75) == -2);
	always_check(Math::TruncTo<int>(-3.00) == -3);

	always_check(Math::IsNearlyEqual(Math::Ceil(2.00), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(2.25), 3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(2.75), 3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(3.00), 3.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Ceil(-2.00), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(-2.25), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(-2.75), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Ceil(-3.00), -3.0, 1e-8));

	always_check(Math::CeilTo<int>(2.00) == 2);
	always_check(Math::CeilTo<int>(2.25) == 3);
	always_check(Math::CeilTo<int>(2.75) == 3);
	always_check(Math::CeilTo<int>(3.00) == 3);

	always_check(Math::CeilTo<int>(-2.00) == -2);
	always_check(Math::CeilTo<int>(-2.25) == -2);
	always_check(Math::CeilTo<int>(-2.75) == -2);
	always_check(Math::CeilTo<int>(-3.00) == -3);

	always_check(Math::IsNearlyEqual(Math::Floor(2.00), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(2.25), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(2.75), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(3.00), 3.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Floor(-2.00), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(-2.25), -3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(-2.75), -3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Floor(-3.00), -3.0, 1e-8));

	always_check(Math::FloorTo<int>(2.00) == 2);
	always_check(Math::FloorTo<int>(2.25) == 2);
	always_check(Math::FloorTo<int>(2.75) == 2);
	always_check(Math::FloorTo<int>(3.00) == 3);

	always_check(Math::FloorTo<int>(-2.00) == -2);
	always_check(Math::FloorTo<int>(-2.25) == -3);
	always_check(Math::FloorTo<int>(-2.75) == -3);
	always_check(Math::FloorTo<int>(-3.00) == -3);

	always_check(Math::IsNearlyEqual(Math::Round(2.00), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(2.25), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(2.75), 3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(3.00), 3.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Round(-2.00), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(-2.25), -2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(-2.75), -3.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Round(-3.00), -3.0, 1e-8));

	always_check(Math::RoundTo<int>(2.00) == 2);
	always_check(Math::RoundTo<int>(2.25) == 2);
	always_check(Math::RoundTo<int>(2.75) == 3);
	always_check(Math::RoundTo<int>(3.00) == 3);

	always_check(Math::RoundTo<int>(-2.00) == -2);
	always_check(Math::RoundTo<int>(-2.25) == -2);
	always_check(Math::RoundTo<int>(-2.75) == -3);
	always_check(Math::RoundTo<int>(-3.00) == -3);

	always_check(Math::Abs(-1) == 1);
	always_check(Math::Abs( 0) == 0);
	always_check(Math::Abs( 1) == 1);

	always_check(Math::Sign(-4) == -1);
	always_check(Math::Sign( 0) ==  0);
	always_check(Math::Sign( 4) ==  1);

	always_check(Math::Min(1, 2, 3, 4, 5) == 1);
	always_check(Math::Min(5, 4, 3, 2, 1) == 1);
	always_check(Math::Max(1, 2, 3, 4, 5) == 5);
	always_check(Math::Max(5, 4, 3, 2, 1) == 5);

	always_check(Math::MinIndex(1, 2, 3, 4, 5) == 0);
	always_check(Math::MinIndex(5, 4, 3, 2, 1) == 4);
	always_check(Math::MaxIndex(1, 2, 3, 4, 5) == 4);
	always_check(Math::MaxIndex(5, 4, 3, 2, 1) == 0);

	always_check(Math::Div( 5,  2).Quotient  ==  2);
	always_check(Math::Div( 5,  2).Remainder ==  1);
	always_check(Math::Div( 5, -2).Quotient  == -2);
	always_check(Math::Div( 5, -2).Remainder ==  1);
	always_check(Math::Div(-5,  2).Quotient  == -2);
	always_check(Math::Div(-5,  2).Remainder == -1);
	always_check(Math::Div(-5, -2).Quotient  ==  2);
	always_check(Math::Div(-5, -2).Remainder == -1);

	always_check(Math::DivAndCeil(4 + 0, 4) == 1);
	always_check(Math::DivAndCeil(4 + 1, 4) == 2);
	always_check(Math::DivAndCeil(4 + 3, 4) == 2);
	always_check(Math::DivAndCeil(4 + 4, 4) == 2);

	always_check(Math::DivAndCeil(-4 - 0, 4) == -1);
	always_check(Math::DivAndCeil(-4 - 1, 4) == -1);
	always_check(Math::DivAndCeil(-4 - 3, 4) == -1);
	always_check(Math::DivAndCeil(-4 - 4, 4) == -2);

	always_check(Math::DivAndFloor(4 + 0, 4) == 1);
	always_check(Math::DivAndFloor(4 + 1, 4) == 1);
	always_check(Math::DivAndFloor(4 + 3, 4) == 1);
	always_check(Math::DivAndFloor(4 + 4, 4) == 2);

	always_check(Math::DivAndFloor(-4 - 0, 4) == -1);
	always_check(Math::DivAndFloor(-4 - 1, 4) == -2);
	always_check(Math::DivAndFloor(-4 - 3, 4) == -2);
	always_check(Math::DivAndFloor(-4 - 4, 4) == -2);

	always_check(Math::DivAndRound(4 + 0, 4) == 1);
	always_check(Math::DivAndRound(4 + 1, 4) == 1);
	always_check(Math::DivAndRound(4 + 3, 4) == 2);
	always_check(Math::DivAndRound(4 + 4, 4) == 2);

	always_check(Math::DivAndRound(-4 - 0, 4) == -1);
	always_check(Math::DivAndRound(-4 - 1, 4) == -1);
	always_check(Math::DivAndRound(-4 - 3, 4) == -2);
	always_check(Math::DivAndRound(-4 - 4, 4) == -2);

	always_check(Math::IsNearlyEqual(4.0, 4.0));

	always_check(Math::IsNearlyZero(0.0));

	always_check(Math::IsInfinity( TNumericLimits<float32>::Infinity()));
	always_check(Math::IsInfinity(-TNumericLimits<float32>::Infinity()));

	always_check(Math::IsNaN( TNumericLimits<float32>::QuietNaN()));
	always_check(Math::IsNaN(-TNumericLimits<float32>::QuietNaN()));
	always_check(Math::IsNaN( TNumericLimits<float32>::SignalingNaN()));
	always_check(Math::IsNaN(-TNumericLimits<float32>::SignalingNaN()));

	always_check(Math::IsNaN(Math::NaN<float32>(4u)));

	always_check(Math::IsNormal(1.0e4));
	always_check(Math::IsNormal(1.0e8));

	always_check(!Math::IsNegative(+1.0));
	always_check(!Math::IsNegative(+0.0));
	always_check( Math::IsNegative(-0.0));
	always_check( Math::IsNegative(-1.0));

	always_check(Math::Exponent(1.0) == 0);
	always_check(Math::Exponent(2.0) == 1);
	always_check(Math::Exponent(4.0) == 2);

	always_check(Math::NaNPayload(Math::NaN<float32>(4u)) == 4u);

	enum class ETest : uint16 { A = 65535 };

	always_check(Math::NaNPayload<ETest>(Math::NaN<float32>(ETest::A)) == ETest::A);

	always_check(Math::IsNearlyEqual(Math::FMod(5.0, 2.0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::FMod(5.0, 2.5), 0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::FMod(5.0, 3.0), 2.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::FMod(-5.0, 2.0), -1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::FMod(-5.0, 2.5), -0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::FMod(-5.0, 3.0), -2.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Remainder(5.0, 2.0),  1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Remainder(5.0, 2.5),  0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Remainder(5.0, 3.0), -1.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Remainder(-5.0, 2.0), -1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Remainder(-5.0, 2.5), -0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Remainder(-5.0, 3.0),  1.0, 1e-8));

	always_check(Math::RemQuo(5.0, 2.0).Quotient == 2);
	always_check(Math::RemQuo(5.0, 2.5).Quotient == 2);
	always_check(Math::RemQuo(5.0, 3.0).Quotient == 2);

	always_check(Math::IsNearlyEqual(Math::RemQuo(5.0, 2.0).Remainder,  1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RemQuo(5.0, 2.5).Remainder,  0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RemQuo(5.0, 3.0).Remainder, -1.0, 1e-8));

	always_check(Math::RemQuo(-5.0, 2.0).Quotient == -2);
	always_check(Math::RemQuo(-5.0, 2.5).Quotient == -2);
	always_check(Math::RemQuo(-5.0, 3.0).Quotient == -2);

	always_check(Math::IsNearlyEqual(Math::RemQuo(-5.0, 2.0).Remainder, -1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RemQuo(-5.0, 2.5).Remainder, -0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RemQuo(-5.0, 3.0).Remainder,  1.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::ModF(123.456).IntegralPart,   123.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::ModF(123.456).FractionalPart, 0.456, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Exp(-1.5), 0.2231301601, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp(-1.0), 0.3678794412, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp( 0.0), 1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp( 1.0), 2.7182818284, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp( 1.5), 4.4816890703, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Exp2(-1.5), 0.3535533906, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp2(-1.0), 0.5000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp2( 0.0), 1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp2( 1.0), 2.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Exp2( 1.5), 2.8284271247, 1e-8));

	always_check(Math::IsNearlyEqual(Math::ExpMinus1(-1.5), -0.7768698398, 1e-8));
	always_check(Math::IsNearlyEqual(Math::ExpMinus1(-1.0), -0.6321205588, 1e-8));
	always_check(Math::IsNearlyEqual(Math::ExpMinus1( 0.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::ExpMinus1( 1.0),  1.7182818284, 1e-8));
	always_check(Math::IsNearlyEqual(Math::ExpMinus1( 1.5),  3.4816890703, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Log(0.5), -0.6931471806, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log(1.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log(1.5),  0.4054651081, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log(2.0),  0.6931471806, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log(2.5),  0.9162907319, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Log2(0.5), -1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log2(1.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log2(1.5),  0.5849625007, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log2(2.0),  1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log2(2.5),  1.3219280949, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Log10(0.5), -0.3010299957, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log10(1.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log10(1.5),  0.1760912591, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log10(2.0),  0.3010299957, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log10(2.5),  0.3979400087, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Log1Plus(0.5), 0.4054651081, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log1Plus(1.0), 0.6931471806, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log1Plus(1.5), 0.9162907319, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log1Plus(2.0), 1.0986122887, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Log1Plus(2.5), 1.2527629685, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Square(0.0), 0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Square(1.0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Square(2.0), 4.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Square(3.0), 9.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Cube(0.0),  0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cube(1.0),  1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cube(2.0),  8.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cube(3.0), 27.0, 1e-8));

	always_check(Math::Pow(2, 0) == 1);
	always_check(Math::Pow(2, 1) == 2);
	always_check(Math::Pow(2, 2) == 4);
	always_check(Math::Pow(2, 3) == 8);

	always_check(Math::IsNearlyEqual(Math::Pow(2.0, 0.0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Pow(2.0, 1.0), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Pow(2.0, 2.0), 4.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Pow(2.0, 3.0), 8.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Sqrt(0), 0, 1));
	always_check(Math::IsNearlyEqual(Math::Sqrt(1), 1, 1));
	always_check(Math::IsNearlyEqual(Math::Sqrt(4), 2, 1));
	always_check(Math::IsNearlyEqual(Math::Sqrt(8), 2, 1));

	always_check(Math::IsNearlyEqual(Math::Sqrt(0.0), 0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sqrt(1.0), 1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sqrt(2.0), 1.4142135624, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sqrt(3.0), 1.7320508076, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Cbrt(0), 0, 1));
	always_check(Math::IsNearlyEqual(Math::Cbrt(1), 1, 1));
	always_check(Math::IsNearlyEqual(Math::Cbrt(4), 1, 1));
	always_check(Math::IsNearlyEqual(Math::Cbrt(8), 2, 1));

	always_check(Math::IsNearlyEqual(Math::Cbrt(0.0), 0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cbrt(1.0), 1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cbrt(2.0), 1.2599210499, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cbrt(3.0), 1.4422495703, 1e-8));

	always_check(Math::Sum(1, 2, 3, 4, 5) == 15);

	always_check(Math::SquaredSum(1, 2, 3, 4, 5) == 55);

	always_check(Math::Avg(1, 2, 3, 4, 5) == 3);

	always_check(Math::IsNearlyEqual(Math::Hypot(1.0, 2.0, 3.0, 4.0, 5.0), 7.4161984871, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Sin(-9.0), -0.4121184852, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin(-6.0),  0.2794154982, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin(-2.0), -0.9092974268, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin(-1.0), -0.8414709848, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin( 0.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin( 1.0),  0.8414709848, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin( 2.0),  0.9092974268, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin( 6.0), -0.2794154982, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Sin( 9.0),  0.4121184852, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Cos(-9.0), -0.9111302619, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos(-6.0),  0.9601702866, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos(-2.0), -0.4161468365, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos(-1.0),  0.5403023059, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos( 0.0),  1.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos( 1.0),  0.5403023059, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos( 2.0), -0.4161468365, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos( 6.0),  0.9601702866, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Cos( 9.0), -0.9111302619, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Tan(-9.0),  0.4523156594, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan(-6.0),  0.2910061914, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan(-2.0),  2.1850398633, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan(-1.0), -1.5574077247, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan( 0.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan( 1.0),  1.5574077247, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan( 2.0), -2.1850398633, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan( 6.0), -0.2910061914, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Tan( 9.0), -0.4523156594, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Asin(-1.0), -1.5707963268, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Asin(-0.5), -0.5235987756, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Asin( 0.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Asin( 0.5),  0.5235987756, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Asin( 1.0),  1.5707963268, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Acos(-1.0), 3.1415926536, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Acos(-0.5), 2.0943951024, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Acos( 0.0), 1.5707963268, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Acos( 0.5), 1.0471975512, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Acos( 1.0), 0.0000000000, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Atan(-1.0), -0.7853981634, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan(-0.5), -0.4636476090, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan( 0.0),  0.0000000000, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan( 0.5),  0.4636476090, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan( 1.0),  0.7853981634, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Atan2(-1.0, -1.0), -2.3561944902, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan2(-0.5, -1.0), -2.6779450446, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan2( 0.0, -1.0),  3.1415926536, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan2( 0.5, -1.0),  2.6779450446, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Atan2( 1.0, -1.0),  2.3561944902, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Sinh(-9.0), -4051.5419020, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh(-6.0), -201.71315737, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh(-2.0), -3.6268604078, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh(-1.0), -1.1752011936, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh( 0.0),  0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh( 1.0),  1.1752011936, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh( 2.0),  3.6268604078, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh( 6.0),  201.71315737, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Sinh( 9.0),  4051.5419020, 1e-4));

	always_check(Math::IsNearlyEqual(Math::Cosh(-9.0), 4051.5420254, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh(-6.0), 201.71563612, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh(-2.0), 3.7621956911, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh(-1.0), 1.5430806348, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh( 0.0), 1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh( 1.0), 1.5430806348, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh( 2.0), 3.7621956911, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh( 6.0), 201.71563612, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Cosh( 9.0), 4051.5420254, 1e-4));

	always_check(Math::IsNearlyEqual(Math::Tanh(-9.0), -1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh(-6.0), -1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh(-2.0), -0.9640275801, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh(-1.0), -0.7615941559, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh( 0.0),  0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh( 1.0),  0.7615941559, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh( 2.0),  0.9640275801, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh( 6.0),  1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Tanh( 9.0),  1.0000000000, 1e-4));

	always_check(Math::IsNearlyEqual(Math::Asinh(-9.0), -2.8934439858, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh(-6.0), -2.4917798526, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh(-2.0), -1.4436354752, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh(-1.0), -0.8813735870, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh( 0.0),  0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh( 1.0),  0.8813735870, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh( 2.0),  1.4436354752, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh( 6.0),  2.4917798526, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Asinh( 9.0),  2.8934439858, 1e-4));

	always_check(Math::IsNaN(        Math::Acosh(-9.0)                    ));
	always_check(Math::IsNaN(        Math::Acosh(-6.0)                    ));
	always_check(Math::IsNaN(        Math::Acosh(-2.0)                    ));
	always_check(Math::IsNaN(        Math::Acosh(-1.0)                    ));
	always_check(Math::IsNaN(        Math::Acosh( 0.0)                    ));
	always_check(Math::IsNearlyEqual(Math::Acosh( 1.0), 0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Acosh( 2.0), 1.3169578969, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Acosh( 6.0), 2.4778887302, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Acosh( 9.0), 2.8872709503, 1e-4));

	always_check(Math::IsInfinity(   Math::Atanh(-1.0)                     ));
	always_check(Math::IsNearlyEqual(Math::Atanh(-0.5), -0.5493061443, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Atanh( 0.0),  0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Atanh( 0.5),  0.5493061443, 1e-4));
	always_check(Math::IsInfinity(   Math::Atanh( 1.0)                     ));

	always_check(Math::IsNearlyEqual(Math::Erf(-6.0), -1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf(-2.0), -0.9953222650, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf(-1.0), -0.8427007929, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf( 0.0),  0.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf( 1.0),  0.8427007929, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf( 2.0),  0.9953222650, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erf( 6.0),  1.0000000000, 1e-4));

	always_check(Math::IsNearlyEqual(Math::Erfc(-6.0), 2.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc(-2.0), 1.9953222650, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc(-1.0), 1.8427007929, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc( 0.0), 1.0000000000, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc( 1.0), 0.1572992070, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc( 2.0), 0.0046777349, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Erfc( 6.0), 0.0000000000, 1e-4));

	always_check(Math::IsNearlyEqual(Math::Gamma(-0.75), -4.8341465442, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Gamma(-0.50), -3.5449077018, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Gamma(-0.25), -4.9016668098, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Gamma( 0.25),  3.6256099082, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Gamma( 0.50),  1.7724538509, 1e-4));
	always_check(Math::IsNearlyEqual(Math::Gamma( 0.75),  1.2254167025, 1e-4));

	always_check(Math::IsNearlyEqual(Math::LogGamma(-0.75), 1.5757045971, 1e-4));
	always_check(Math::IsNearlyEqual(Math::LogGamma(-0.50), 1.2655121235, 1e-4));
	always_check(Math::IsNearlyEqual(Math::LogGamma(-0.25), 1.5895753125, 1e-4));
	always_check(Math::IsNearlyEqual(Math::LogGamma( 0.25), 1.2880225246, 1e-4));
	always_check(Math::IsNearlyEqual(Math::LogGamma( 0.50), 0.5723649429, 1e-4));
	always_check(Math::IsNearlyEqual(Math::LogGamma( 0.75), 0.2032809514, 1e-4));

	always_check(Math::IsNearlyEqual(Math::LdExp(1.0, 0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::LdExp(1.0, 1), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::LdExp(1.0, 2), 4.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::LdExp(1.0, 3), 8.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::RadiansToDegrees(0.0),                              0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RadiansToDegrees(Math::TNumbers<float64>::Pi),    180.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::RadiansToDegrees(Math::TNumbers<float64>::TwoPi), 360.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::DegreesToRadians(  0.0), 0.0,                            1e-8));
	always_check(Math::IsNearlyEqual(Math::DegreesToRadians(180.0), Math::TNumbers<float64>::Pi,    1e-8));
	always_check(Math::IsNearlyEqual(Math::DegreesToRadians(360.0), Math::TNumbers<float64>::TwoPi, 1e-8));

	always_check(Math::GCD(0, 0) == 0);
	always_check(Math::GCD(0, 1) == 1);
	always_check(Math::GCD(9, 6) == 3);

	always_check(Math::LCM(0, 0) ==  0);
	always_check(Math::LCM(0, 1) ==  0);
	always_check(Math::LCM(9, 6) == 18);

	always_check(Math::IsNearlyEqual(Math::Clamp(0.0, 1.0, 2.0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Clamp(1.0, 1.0, 2.0), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Clamp(2.0, 1.0, 2.0), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Clamp(3.0, 1.0, 2.0), 2.0, 1e-8));

	always_check(Math::IsNearlyEqual(Math::WrappingClamp(0.5, 0.0, 2.0), 0.5, 1e-8));
	always_check(Math::IsNearlyEqual(Math::WrappingClamp(1.5, 0.0, 2.0), 1.5, 1e-8));
	always_check(Math::IsNearlyEqual(Math::WrappingClamp(2.5, 0.0, 2.0), 0.5, 1e-8));
	always_check(Math::IsNearlyEqual(Math::WrappingClamp(3.5, 0.0, 2.0), 1.5, 1e-8));

	always_check(Math::IsNearlyEqual(Math::Lerp(0.0, 2.0, 0.0), 0.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Lerp(0.0, 2.0, 0.5), 1.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Lerp(0.0, 2.0, 1.0), 2.0, 1e-8));
	always_check(Math::IsNearlyEqual(Math::Lerp(0.0, 2.0, 1.5), 3.0, 1e-8));

	always_check(static_cast<uint8>(Math::LerpStable(0, 255, 0.0)) ==   0);
	always_check(static_cast<uint8>(Math::LerpStable(0, 255, 0.5)) == 127);
	always_check(static_cast<uint8>(Math::LerpStable(0, 255, 1.0)) == 255);
}

NAMESPACE_PRIVATE_END

void TestNumeric()
{
	NAMESPACE_PRIVATE::TestLiteral();
	NAMESPACE_PRIVATE::TestBit();
	NAMESPACE_PRIVATE::TestMath();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
