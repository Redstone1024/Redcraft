#include "Testing/MemoryTesting.h"

#include "Memory/Memory.h"
#include "Memory/Alignment.h"
#include "Memory/PointerTraits.h"
#include "Memory/UniquePointer.h"
#include "Memory/SharedPointer.h"
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
	TestPointerTraits();
	TestUniquePointer();
	TestSharedPointer();
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

	TempA = 0x0123456789ABCDEF;
	Memory::Memmove(TempB, TempA);
	always_check(TempB == TempA);

	TempA = 1004;
	TempB = 1005;
	TempC = 1005;
	TempD = 1006;
	int32 ResultA = Memory::Memcmp(PtrA, PtrB, sizeof(int64));
	int32 ResultB = Memory::Memcmp(PtrB, PtrC, sizeof(int64));
	int32 ResultC = Memory::Memcmp(PtrC, PtrD, sizeof(int64));
	always_check((ResultA < 0) == (ResultC < 0));
	always_check(ResultB == 0);
	int32 ResultD = Memory::Memcmp(TempA, TempB);
	int32 ResultE = Memory::Memcmp(TempB, TempC);
	int32 ResultF = Memory::Memcmp(TempC, TempD);
	always_check((ResultD < 0) == (ResultF < 0));
	always_check(ResultE == 0);

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
	delete [] PtrC;

	Memory::Free(Memory::Realloc(Memory::Malloc(0), 0));
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

void TestPointerTraits()
{
	always_check(!TPointerTraits<int64>::bIsPointer);

	always_check(TPointerTraits<int64*>::bIsPointer);
	always_check((CSameAs<TPointerTraits<int64*>::PointerType, int64*>));
	always_check((CSameAs<TPointerTraits<int64*>::ElementType, int64>));
	always_check(TPointerTraits<int64*>::ToAddress(nullptr) == nullptr);

	always_check(TPointerTraits<int64(*)[]>::bIsPointer);
	always_check((CSameAs<TPointerTraits<int64(*)[]>::PointerType, int64(*)[]>));
	always_check((CSameAs<TPointerTraits<int64(*)[]>::ElementType, int64>));
	always_check(TPointerTraits<int64*>::ToAddress(nullptr) == nullptr);

	always_check(TPointerTraits<TSharedPtr<int64>>::bIsPointer);
	always_check((CSameAs<TPointerTraits<TSharedPtr<int64>>::PointerType, TSharedPtr<int64>>));
	always_check((CSameAs<TPointerTraits<TSharedPtr<int64>>::ElementType, int64>));
	always_check(TPointerTraits<TSharedPtr<int64>>::ToAddress(nullptr) == nullptr);

	always_check(TPointerTraits<TSharedPtr<int64[]>>::bIsPointer);
	always_check((CSameAs<TPointerTraits<TSharedPtr<int64[]>>::PointerType, TSharedPtr<int64[]>>));
	always_check((CSameAs<TPointerTraits<TSharedPtr<int64[]>>::ElementType, int64>));
	always_check(TPointerTraits<TSharedPtr<int64[]>>::ToAddress(nullptr) == nullptr);

}

NAMESPACE_UNNAMED_BEGIN

struct FCounter
{
	static int32 Num;
	FCounter() { ++Num; }
	~FCounter() { --Num; }
};

int32 FCounter::Num = 0;

struct FDeleter
{
	static int32 Num;
	void operator()(FCounter* Ptr) { delete Ptr; ++Num; }
};

int32 FDeleter::Num = 0;

struct FArrayDeleter
{
	static int32 Num;
	void operator()(FCounter* Ptr) { delete [] Ptr; ++Num; }
};

int32 FArrayDeleter::Num = 0;

NAMESPACE_UNNAMED_END

void TestUniquePointer()
{
	{
		TUniqueRef<int32> Temp(new int32);
		*Temp = 15;
		always_check(*Temp.Get() = 15);
	}

	FCounter::Num = 0;
	FDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter;
		FCounter* PtrB = new FCounter;
		FCounter* PtrC = new FCounter;

		TUniqueRef<FCounter> TempA(PtrA);
		TUniqueRef<FCounter, FDeleter> TempB(PtrB);
		TUniqueRef<FCounter, FDeleter> TempC(PtrC, FDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter);
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		FCounter* PtrX = TempB.ReleaseAndReset(new FCounter);
		always_check(FCounter::Num == TempNum + 1);
		delete PtrX;
		
		TempNum = FCounter::Num;
		FCounter* PtrY = TempB.ReleaseAndReset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum + 1);
		delete PtrY;

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempC.GetDeleter().Num == 2);
	}

	always_check(FCounter::Num == 0);
	always_check(FDeleter::Num == 4);

	{
		TUniqueRef<int32[]> Temp(new int32[4]);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}
	
	     FCounter::Num = 0;
	FArrayDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter[4];
		FCounter* PtrB = new FCounter[4];
		FCounter* PtrC = new FCounter[4];

		TUniqueRef<FCounter[]> TempA(PtrA);
		TUniqueRef<FCounter[], FArrayDeleter> TempB(PtrB);
		TUniqueRef<FCounter[], FArrayDeleter> TempC(PtrC, FArrayDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4]);
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		FCounter* PtrX = TempB.ReleaseAndReset(new FCounter[4]);
		always_check(FCounter::Num == TempNum + 4);
		delete [] PtrX;
		
		TempNum = FCounter::Num;
		FCounter* PtrY = TempB.ReleaseAndReset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum + 4);
		delete [] PtrY;

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempC.GetDeleter().Num == 2);
	}

	always_check(     FCounter::Num == 0);
	always_check(FArrayDeleter::Num == 4);
	
	{
		TUniquePtr<int32> Temp = MakeUnique<int32>(NoInit);
		*Temp = 15;
		always_check(*Temp.Get() = 15);
	}

	{
		TUniquePtr<int32> Temp = MakeUnique<int32>();
		*Temp = 15;
		always_check(*Temp.Get() = 15);
	}

	FCounter::Num = 0;
	FDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter;
		FCounter* PtrB = new FCounter;
		FCounter* PtrC = new FCounter;

		TUniquePtr<FCounter> TempA(PtrA);
		TUniquePtr<FCounter, FDeleter> TempB(PtrB);
		TUniquePtr<FCounter, FDeleter> TempC(PtrC, FDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter);
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		FCounter* PtrX = TempB.ReleaseAndReset(new FCounter);
		always_check(FCounter::Num == TempNum + 1);
		delete PtrX;
		
		TempNum = FCounter::Num;
		FCounter* PtrY = TempB.ReleaseAndReset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum + 1);
		delete PtrY;

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempC.GetDeleter().Num == 2);

		TUniquePtr<FCounter, FDeleter> TempD(MoveTemp(TempB));

		TUniquePtr<FCounter, FDeleter> TempE;
		TempE = MoveTemp(TempC);
		TempE = nullptr;

		TempB.Reset(new FCounter);
		always_check(!!TempB);
		always_check(TempB.IsValid());
		delete TempB.Release();

	}

	always_check(FCounter::Num == 0);
	always_check(FDeleter::Num == 4);

	{
		TUniquePtr<int32[]> Temp = MakeUnique<int32[]>(4, NoInit);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	{
		TUniquePtr<int32[]> Temp = MakeUnique<int32[]>(4);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	     FCounter::Num = 0;
	FArrayDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter[4];
		FCounter* PtrB = new FCounter[4];
		FCounter* PtrC = new FCounter[4];

		TUniquePtr<FCounter[]> TempA(PtrA);
		TUniquePtr<FCounter[], FArrayDeleter> TempB(PtrB);
		TUniquePtr<FCounter[], FArrayDeleter> TempC(PtrC, FArrayDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4]);
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		FCounter* PtrX = TempB.ReleaseAndReset(new FCounter[4]);
		always_check(FCounter::Num == TempNum + 4);
		delete [] PtrX;
		
		TempNum = FCounter::Num;
		FCounter* PtrY = TempB.ReleaseAndReset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum + 4);
		delete [] PtrY;

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempC.GetDeleter().Num == 2);

		TUniquePtr<FCounter[], FArrayDeleter> TempD(MoveTemp(TempB));

		TUniquePtr<FCounter[], FArrayDeleter> TempE;
		TempE = MoveTemp(TempC);
		TempE = nullptr;

		TempB.Reset(new FCounter[4]);
		always_check(!!TempB);
		always_check(TempB.IsValid());
		delete [] TempB.Release();

	}

	always_check(     FCounter::Num == 0);
	always_check(FArrayDeleter::Num == 4);

	{
		TUniquePtr<int32> TempA;
		TUniquePtr<const int32> TempB = MoveTemp(TempA);
		TUniquePtr<const int32> TempC;
		TempC = MoveTemp(TempA);
	}

	{
		TUniquePtr<int32[]> TempA;
		TUniquePtr<const int32[]> TempB = MoveTemp(TempA);
		TUniquePtr<const int32[]> TempC;
		TempC = MoveTemp(TempA);
	}

}

void TestSharedPointer()
{

	FCounter::Num = 0;
	FDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter;
		FCounter* PtrB = new FCounter;
		FCounter* PtrC = new FCounter;

		TSharedRef<FCounter> TempA(PtrA);
		TSharedRef<FCounter> TempB(PtrB, FDeleter());
		TSharedRef<FCounter> TempC(PtrC, FDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);
		
		TempNum = FCounter::Num;
		TempC.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempA.GetDeleter<FDeleter>() == nullptr);
		always_check(TempC.GetDeleter<FDeleter>() != nullptr);
		always_check(TempC.GetDeleter<FDeleter>()->Num == 2);
		
		TSharedRef<FCounter> TempD(MoveTemp(TempB));
	}

	always_check(FCounter::Num == 0);
	always_check(FDeleter::Num == 4);

	{
		TSharedRef<int32[]> Temp = MakeShared<int32[]>(4, NoInit);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	{
		TSharedRef<int32[]> Temp = MakeShared<int32[]>(4);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	     FCounter::Num = 0;
	FArrayDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter[4];
		FCounter* PtrB = new FCounter[4];
		FCounter* PtrC = new FCounter[4];

		TSharedRef<FCounter[]> TempA(PtrA);
		TSharedRef<FCounter[]> TempB(PtrB, FArrayDeleter());
		TSharedRef<FCounter[]> TempC(PtrC, FArrayDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempA.GetDeleter<FArrayDeleter>() == nullptr);
		always_check(TempC.GetDeleter<FArrayDeleter>() != nullptr);
		always_check(TempC.GetDeleter<FArrayDeleter>()->Num == 2);

		TSharedRef<FCounter[]> TempD(MoveTemp(TempB));
	}

	always_check(     FCounter::Num == 0);
	always_check(FArrayDeleter::Num == 4);

	FCounter::Num = 0;
	FDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter;
		FCounter* PtrB = new FCounter;
		FCounter* PtrC = new FCounter;

		TSharedPtr<FCounter> TempA(PtrA);
		TSharedPtr<FCounter> TempB(PtrB, FDeleter());
		TSharedPtr<FCounter> TempC(PtrC, FDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter, FDeleter());
		always_check(FCounter::Num == TempNum);

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempA.GetDeleter<FDeleter>() == nullptr);
		always_check(TempC.GetDeleter<FDeleter>() != nullptr);
		always_check(TempC.GetDeleter<FDeleter>()->Num == 2);

		TSharedPtr<FCounter> TempD(MoveTemp(TempB));

		TSharedPtr<FCounter> TempE;
		TempE = MoveTemp(TempC);
		TempE = nullptr;

		TempB.Reset(new FCounter, FDeleter());
		always_check(!!TempB);
		always_check(TempB.IsValid());

	}

	always_check(FCounter::Num == 0);
	always_check(FDeleter::Num == 5);

	{
		TSharedPtr<int32[]> Temp = MakeShared<int32[]>(4, NoInit);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	{
		TSharedPtr<int32[]> Temp = MakeShared<int32[]>(4);
		Temp[0] = 15;
		always_check(Temp.Get()[0] = 15);
	}

	     FCounter::Num = 0;
	FArrayDeleter::Num = 0;

	{
		FCounter* PtrA = new FCounter[4];
		FCounter* PtrB = new FCounter[4];
		FCounter* PtrC = new FCounter[4];

		TSharedPtr<FCounter[]> TempA(PtrA);
		TSharedPtr<FCounter[]> TempB(PtrB, FArrayDeleter());
		TSharedPtr<FCounter[]> TempC(PtrC, FArrayDeleter());

		always_check(TempA == PtrA);
		always_check(TempC != TempB);
		always_check((TempA <=> PtrA) == strong_ordering::equal);
		always_check((TempC <=> TempB) != strong_ordering::equal);
		
		int32 TempNum;

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		TempNum = FCounter::Num;
		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(FCounter::Num == TempNum);

		always_check(GetTypeHash(TempB) == GetTypeHash(TempB.Get()));

		Swap(TempB, TempC);

		always_check(TempA.GetDeleter<FArrayDeleter>() == nullptr);
		always_check(TempC.GetDeleter<FArrayDeleter>() != nullptr);
		always_check(TempC.GetDeleter<FArrayDeleter>()->Num == 2);

		TSharedPtr<FCounter[]> TempD(MoveTemp(TempB));

		TSharedPtr<FCounter[]> TempE;
		TempE = MoveTemp(TempC);
		TempE = nullptr;

		TempB.Reset(new FCounter[4], FArrayDeleter());
		always_check(!!TempB);
		always_check(TempB.IsValid());

	}

	always_check(     FCounter::Num == 0);
	always_check(FArrayDeleter::Num == 5);

	{
		TSharedPtr<bool> Temp;
		always_check(!Temp.IsValid());
		if (Temp.Get() == nullptr) { }
	}

	{
		TSharedPtr<int32> Temp(new int32(123));

		always_check(Temp.IsValid());
		always_check(Temp.IsUnique());

		const int32 DeferenceTest = *Temp;

		Temp.Reset();

		always_check(Temp.GetSharedReferenceCount() == 0);
	}

	{
		TSharedPtr<bool> TempA(new bool(false));
		TSharedPtr<bool> TempB(TempA);
	}

	{
		TSharedPtr<bool> TempA(new bool(false));
		TSharedPtr<bool> TempB;
		TempB = TempA;
	}

	{
		struct FSharedTest { bool bTest; };

		TSharedPtr<FSharedTest> TempA(new FSharedTest());

		TempA->bTest = true;

		(*TempA).bTest = false;

		TSharedPtr<FSharedTest> TempB(TempA);

		TempA.Reset();
	}

	{
		class FBase { bool bTest; };

		class FDerived : public FBase { };

		{
			TSharedPtr<FBase> TempA(new FDerived());
			TSharedPtr<FDerived> TempB(StaticCast<FDerived>(TempA));
		}

		{
			TSharedPtr<FDerived> TempA(new FDerived());
			TSharedPtr<FBase> TempB(TempA);
		}

		{
			TSharedPtr<FDerived> TempA(new FDerived());
			TSharedPtr<FBase> TempB;
			TempB = TempA;
		}
	}

	{
		bool* Ptr = nullptr;
		TSharedPtr<bool> Temp(Ptr);
		always_check(!Temp.IsValid());
	}

	{
		TSharedPtr<bool> Temp(new bool(true));
		always_check(Temp.IsValid());
	}

	{
		TWeakPtr<bool> Temp;
		always_check(!Temp.Lock().IsValid());
	}

	{
		TSharedPtr<int32> TempShared(new int32(64));
		TWeakPtr<int32> TempWeak(TempShared);
		always_check(TempWeak.Lock().IsValid());
	}

	{
		TSharedPtr<int32> TempShared(new int32(64));
		TWeakPtr<int32> TempWeak;
		TempWeak = TempShared;

		always_check(TempWeak.Lock().IsValid());

		TempWeak.Reset();
		always_check(!TempWeak.Lock().IsValid());
	}

	{
		TSharedPtr<int32> TempShared(new int32(64));
		TWeakPtr<int32> TempWeak = TempShared;
		TempShared.Reset();
		always_check(!TempWeak.Lock().IsValid());
	}

	{
		TSharedPtr<int32> TempA(new int32(64));
		TSharedPtr<int32> TempB(new int32(21));
		TSharedPtr<int32> TempC(TempB);

		always_check(!(TempA == TempB));
		always_check(TempA != TempB);
		always_check(TempB == TempC);
	}

	{
		TSharedPtr<int32> TempA(new int32(64));
		TSharedPtr<int32> TempB(new int32(21));

		TWeakPtr<int32> WeakA(TempA);
		TWeakPtr<int32> WeakB(TempB);
		TWeakPtr<int32> WeakC(TempB);

		always_check(!(WeakA.Lock() == WeakB.Lock()));
		always_check(WeakA.Lock() != WeakB.Lock());
		always_check(WeakB.Lock() == WeakC.Lock());
	}

	{
		TSharedPtr<const int32> TempA(new int32(10));
		TSharedPtr<const float32> TempB(new float32(1.0f));
		TSharedPtr<const float32> TempC(new float32(2.0f));

		if (TempB == TempC) { }

		TempB = TempC;

		TSharedPtr<float32> TempD(new float32(123.0f));

		TempB = TempD;

		TWeakPtr<const float32> TempE = TempB;
		TWeakPtr<float32> TempF;

		TempF = ConstCast<float32>(TempC);
		*TempF.Lock() = 20.0f;
	}

	{
		TSharedPtr<struct FTest> Temp;
		struct FTest { int32 Value; };
		Temp = TSharedPtr<FTest>(new FTest());
		Temp->Value = 20;
	}

	{
		TSharedPtr<bool> TempA(nullptr);
		TSharedPtr<float32> TempB = nullptr;

		TWeakPtr<bool> TempD(nullptr);
		TWeakPtr<float32> TempE = nullptr;

		TempB = TSharedPtr<float32>(new float32(0.1f));
		TempB = nullptr;

		TempB = MakeShared<float32>(30.0f);
		TSharedPtr<float64> TempC(MakeShared<float64>(2.0));

		struct FTest
		{
			TSharedPtr<float32> Value;

			TSharedPtr<float32> FuncA() { return Value; }

			TSharedPtr<float32> FuncB() { return MakeShared<float32>(123.0f); }
		};
	}

	{
		TSharedRef<float32> Temp(new float32(123.0f));
	}

	{
		TSharedRef<float32> Temp(new float32(123.0f));
		const float& RefA = *Temp;
		const float& RefB = *Temp.Get();
	}

	{
		TSharedRef<float32> Temp = MakeShared<float32>(123.0f);
	}

	{
		TSharedRef<int32> TempA(new int32(1));
		TSharedPtr<int32> TempB(TempA);
	}

	{
		TSharedPtr<int32> TempA(new int32(1));
		TSharedRef<int32> TempB(TempA.ToSharedRef());
	}

	{
		TSharedRef<int32> Temp(new int32(10));
		Temp = TSharedRef<int32>(new int32(20));
	}

	{
		TSharedRef<int32> TempA(new int32(99));
		TWeakPtr<int32> TempB = TempA;
		always_check(TempB.Lock());
	}

	{
		TSharedRef<int32> IntRef1(new int32(99));
		TSharedRef<int32> IntRef2(new int32(21));
		always_check(!(IntRef1 == IntRef2));
		always_check(IntRef1 != IntRef2);
	}

	{
		TSharedRef<int32> TempA(new int32(21));
		TSharedPtr<int32> TempB(TempA);
		TSharedPtr<int32> TempC;

		always_check(TempA == TempB && TempB == TempA);
		always_check(!(TempA != TempB || TempB != TempA));
		always_check(!(TempA == TempC) && (TempA != TempC));
	}

	{
		struct FTest : public TSharedFromThis<FTest>
		{
			TSharedRef<FTest> FuncTest() { return AsShared(); }
		};

		TSharedPtr<FTest> TempA(new FTest());

		{
			FTest* Ptr = TempA.Get();
			TSharedRef<FTest> TempB(Ptr->FuncTest());
		}
	}

	{
		TSharedRef<int32> TempA = MakeShared<int32>();
		TSharedRef<const int32> TempB = TempA;
		TSharedRef<const int32> TempC = MakeShared<int32>();
		TempC = TempA;
	}

	{
		TSharedRef<int32[]> TempA = MakeShared<int32[]>(4);
		TSharedRef<const int32[]> TempB = TempA;
		TSharedRef<const int32[]> TempC = MakeShared<int32[]>(4);
		TempC = TempA;
	}

	{
		TSharedPtr<int32> TempA;
		TSharedPtr<const int32> TempB = TempA;
		TSharedPtr<const int32> TempC;
		TempC = TempA;
	}

	{
		TSharedPtr<int32[]> TempA;
		TSharedPtr<const int32[]> TempB = TempA;
		TSharedPtr<const int32[]> TempC;
		TempC = TempA;
	}

	{
		TWeakPtr<int32> TempA;
		TWeakPtr<const int32> TempB = TempA;
		TWeakPtr<const int32> TempC;
		TempC = TempA;
	}

	{
		TWeakPtr<int32[]> TempA;
		TWeakPtr<const int32[]> TempB = TempA;
		TWeakPtr<const int32[]> TempC;
		TempC = TempA;
	}

}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
