#include "Testing/NumericTesting.h"

#include "Numeric/Numeric.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestNumeric()
{
	TestLiteral();
	TestBit();
}

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

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
