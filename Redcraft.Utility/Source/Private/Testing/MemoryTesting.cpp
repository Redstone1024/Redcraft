#include "Testing/MemoryTesting.h"

#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestMemory()
{
	TestAlignment();
	TestMemoryBuffer();
	TestMemoryMalloc();
	TestMemoryOperator();
}

void TestAlignment()
{
	int32 Unaligned = 0xAAAA;

	int32 Aligned8  = Memory::Align(Unaligned,  8);
	int32 Aligned16 = Memory::Align(Unaligned, 16);
	int32 Aligned32 = Memory::Align(Unaligned, 32);
	int32 Aligned64 = Memory::Align(Unaligned, 64);

	int32 AlignedDown8  = Memory::AlignDown(Unaligned,  8);
	int32 AlignedDown16 = Memory::AlignDown(Unaligned, 16);
	int32 AlignedDown32 = Memory::AlignDown(Unaligned, 32);
	int32 AlignedDown64 = Memory::AlignDown(Unaligned, 64);

	int32 AlignedArbitrary8  = Memory::AlignArbitrary(Unaligned,  8);
	int32 AlignedArbitrary16 = Memory::AlignArbitrary(Unaligned, 16);
	int32 AlignedArbitrary32 = Memory::AlignArbitrary(Unaligned, 32);
	int32 AlignedArbitrary64 = Memory::AlignArbitrary(Unaligned, 64);
	
	always_check((Memory::IsAligned(Aligned8,   8) && Aligned8  > Unaligned));
	always_check((Memory::IsAligned(Aligned16, 16) && Aligned16 > Unaligned));
	always_check((Memory::IsAligned(Aligned32, 32) && Aligned32 > Unaligned));
	always_check((Memory::IsAligned(Aligned64, 64) && Aligned64 > Unaligned));

	always_check((Memory::IsAligned(Aligned8,   8) && AlignedDown8  < Unaligned));
	always_check((Memory::IsAligned(Aligned16, 16) && AlignedDown16 < Unaligned));
	always_check((Memory::IsAligned(Aligned32, 32) && AlignedDown32 < Unaligned));
	always_check((Memory::IsAligned(Aligned64, 64) && AlignedDown64 < Unaligned));

	always_check((Memory::IsAligned(AlignedArbitrary8,   8)));
	always_check((Memory::IsAligned(AlignedArbitrary16, 16)));
	always_check((Memory::IsAligned(AlignedArbitrary32, 32)));
	always_check((Memory::IsAligned(AlignedArbitrary64, 64)));

}

void TestMemoryBuffer()
{
	int64 TempA;
	int64 TempB;
	int64 TempC;
	int64 TempD;
	uint8* PtrA = reinterpret_cast<uint8*>(&TempA);
	uint8* PtrB = reinterpret_cast<uint8*>(&TempB);
	uint8* PtrC = reinterpret_cast<uint8*>(&TempC);
	uint8* PtrD = reinterpret_cast<uint8*>(&TempD);
	
	TempA = 0x0123456789ABCDEF;
	TempB = 0x0123456789AB0000;
	Memory::Memmove(PtrA, PtrA + 2, 6);
	always_check((TempA << 16) == TempB);

	TempA = 1004;
	TempB = 1005;
	TempC = 1005;
	TempD = 1006;
	int32 ResultA = Memory::Memcmp(PtrA, PtrB, sizeof(int64));
	int32 ResultB = Memory::Memcmp(PtrB, PtrC, sizeof(int64));
	int32 ResultC = Memory::Memcmp(PtrC, PtrD, sizeof(int64));
	always_check((ResultA < 0) != (ResultB < 0));
	always_check(ResultB == 0);

	Memory::Memset(PtrA, 0x3F, sizeof(int64));
	always_check(TempA == 0x3F3F3F3F3F3F3F3F);
	Memory::Memset(TempB, 0x3F);
	always_check(TempB == 0x3F3F3F3F3F3F3F3F);

	Memory::Memzero(PtrA, sizeof(int64));
	always_check(TempA == 0);
	Memory::Memzero(TempB);
	always_check(TempB == 0);

	TempA = 0x0123456789ABCDEF;
	Memory::Memcpy(PtrC, PtrA, sizeof(int64));
	always_check(TempA == TempC);
	TempB = 0xDEDCBA9876543210;
	Memory::Memcpy(TempD, TempB);
	always_check(TempB == TempD);

}

void TestMemoryMalloc()
{
	int32* PtrA;
	int64* PtrB;

	PtrA = reinterpret_cast<int32*>(Memory::SystemMalloc(sizeof(int32)));
	*PtrA = 0x01234567;
	always_check(*PtrA == 0x01234567);
	PtrB = reinterpret_cast<int64*>(Memory::SystemRealloc(PtrA, sizeof(int64)));
	*PtrB = 0x0123456789ABCDEF;
	always_check(*PtrB == 0x0123456789ABCDEF);
	Memory::SystemFree(PtrB);

	PtrA = reinterpret_cast<int32*>(Memory::Malloc(sizeof(int32), 1024));
	always_check(Memory::IsAligned(PtrA, 1024));
	*PtrA = 0x01234567;
	always_check(*PtrA == 0x01234567);
	PtrB = reinterpret_cast<int64*>(Memory::Realloc(PtrA, sizeof(int64), 1024));
	always_check(Memory::IsAligned(PtrB, 1024));
	*PtrB = 0x0123456789ABCDEF;
	always_check(*PtrB == 0x0123456789ABCDEF);
	Memory::Free(PtrB);

	PtrA = new int32;
	PtrB = new int64;
	*PtrA = 0x01234567;
	always_check(*PtrA == 0x01234567);
	*PtrB = 0x0123456789ABCDEF;
	always_check(*PtrB == 0x0123456789ABCDEF);
	delete PtrA;
	delete PtrB;

	struct alignas(1024) FTest { int32 A; };
	FTest* PtrC = new FTest[4];
	always_check(Memory::IsAligned(PtrC, 1024));
	PtrC->A = 0x01234567;
	always_check(PtrC->A == 0x01234567);
	delete[] PtrC;

}

NAMESPACE_UNNAMED_BEGIN

struct FTracker
{
	static int32 Status;
	FTracker()                                               { always_check(Status == 0); Status = -1;               }
	FTracker(const FTracker&)                                { always_check(Status == 1); Status = -1;               }
	FTracker(FTracker&&)                                     { always_check(Status == 2); Status = -1;               }
	~FTracker()                                              { always_check(Status == 3); Status = -1;               }
	FTracker& operator=(const FTracker&)                     { always_check(Status == 4); Status = -1; return *this; }
	FTracker& operator=(FTracker&&)                          { always_check(Status == 5); Status = -1; return *this; }
};

int32 FTracker::Status = -1;

NAMESPACE_UNNAMED_END

void TestMemoryOperator()
{

	FTracker* PtrA = reinterpret_cast<FTracker*>(Memory::Malloc(sizeof(FTracker)));
	FTracker* PtrB = reinterpret_cast<FTracker*>(Memory::Malloc(sizeof(FTracker)));

	FTracker::Status = 0;
	Memory::DefaultConstruct<FTracker>(PtrA);
	always_check(FTracker::Status == -1);

	FTracker::Status = 1;
	Memory::Construct<FTracker>(PtrA, PtrB);
	always_check(FTracker::Status == -1);

	FTracker::Status = 1;
	Memory::CopyConstruct(PtrA, PtrB);
	always_check(FTracker::Status == -1);

	FTracker::Status = 2;
	Memory::MoveConstruct(PtrA, PtrB);
	always_check(FTracker::Status == -1);

	FTracker::Status = 3;
	Memory::Destruct(PtrA);
	always_check(FTracker::Status == -1);

	FTracker::Status = 4;
	Memory::CopyAssign(PtrA, PtrB);
	always_check(FTracker::Status == -1);

	FTracker::Status = 5;
	Memory::MoveAssign(PtrA, PtrB);
	always_check(FTracker::Status == -1);

	Memory::Free(PtrA);
	Memory::Free(PtrB);
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
