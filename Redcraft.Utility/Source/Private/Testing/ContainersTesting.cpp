#include "Testing/ContainersTesting.h"

#include "Containers/Containers.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestContainers()
{
	TestArray();
	TestStaticArray();
	TestArrayView();
	TestBitset();
	TestStaticBitset();
	TestList();
}

NAMESPACE_UNNAMED_BEGIN

template <typename Allocator, size_t Capacity>
void TestArrayTemplate()
{
	{
		TArray<int32, Allocator> ArrayA;
		TArray<int32, Allocator> ArrayB(4);
		TArray<int32, Allocator> ArrayC(4, 4);
		TArray<int32, Allocator> ArrayD(ArrayC);
		TArray<int32, Allocator> ArrayE(MoveTemp(ArrayB));
		TArray<int32, Allocator> ArrayF({ 0, 1, 2, 3 });

		TArray<int32, Allocator> ArrayG;
		TArray<int32, Allocator> ArrayH;
		TArray<int32, Allocator> ArrayI;

		ArrayG = ArrayD;
		ArrayH = MoveTemp(ArrayE);
		ArrayI = { 0, 1, 2, 3 };

		always_check((ArrayC == TArray<int32, Allocator>({ 4, 4, 4, 4 })));
		always_check((ArrayD == TArray<int32, Allocator>({ 4, 4, 4, 4 })));
		always_check((ArrayG == TArray<int32, Allocator>({ 4, 4, 4, 4 })));
		always_check((ArrayF == TArray<int32, Allocator>({ 0, 1, 2, 3 })));
		always_check((ArrayI == TArray<int32, Allocator>({ 0, 1, 2, 3 })));
	}

	{
		TArray<int32, Allocator> ArrayA = { 1, 2, 3 };
		TArray<int32, Allocator> ArrayB = { 7, 8, 9, 10 };
		TArray<int32, Allocator> ArrayC = { 1, 2, 3 };

		always_check((!(ArrayA == ArrayB)));
		always_check(( (ArrayA != ArrayB)));
		always_check(( (ArrayA <  ArrayB)));
		always_check(( (ArrayA <= ArrayB)));
		always_check((!(ArrayA >  ArrayB)));
		always_check((!(ArrayA >= ArrayB)));

		always_check(( (ArrayA == ArrayC)));
		always_check((!(ArrayA != ArrayC)));
		always_check((!(ArrayA <  ArrayC)));
		always_check(( (ArrayA <= ArrayC)));
		always_check((!(ArrayA >  ArrayC)));
		always_check(( (ArrayA >= ArrayC)));
	}

	{
		TArray<int32, Allocator> Array = { 1, 2, 3 };

		Array.Insert(Array.Begin() + 1, 2);
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 2, 3 })));

		Array.Insert(Array.End(), 2, 4);
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 2, 3, 4, 4 })));

		Array.Insert(Array.Begin(), { 1, 1, 4, 5, 1, 4 });
		always_check((Array == TArray<int32, Allocator>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4 })));

		Array.Emplace(Array.End(), 5);
		always_check((Array == TArray<int32, Allocator>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4, 5 })));

		Array.StableErase(Array.End() - 1);
		always_check((Array == TArray<int32, Allocator>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4 })));

		Array.StableErase(Array.End() - 2, Array.End());
		always_check((Array == TArray<int32, Allocator>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3 })));

		Array.Erase(Array.End() - 2);
		always_check((Array.Num() == 9));

		Array.Erase(Array.Begin(), Array.Begin() + 6);
		always_check((Array.Num() == 3));
	}

	{
		TArray<int32, Allocator> Array = { 1, 2, 3 };

		Array.PushBack(4);
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 3, 4 })));

		Array.EmplaceBack(5);
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 3, 4, 5 })));

		Array.EmplaceBack(5) = 6;
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 3, 4, 5, 6 })));

		Array.PopBack();
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 3, 4, 5 })));

		Array.SetNum(4);
		always_check((Array == TArray<int32, Allocator>({ 1, 2, 3, 4 })));

		Array.Reserve(64);
		always_check((Array.Num() ==  4));
		always_check((Array.Max() == 64 || Array.Max() == Capacity));

		Array.Shrink();
		always_check((Array.Num() == 4));
		always_check((Array.Max() == 4 || Array.Max() == Capacity));
	}
}

NAMESPACE_UNNAMED_END

void TestArray()
{
	TestArrayTemplate<FHeapAllocator,       0>();
	TestArrayTemplate<TInlineAllocator<8>,  8>();
	TestArrayTemplate<TFixedAllocator<64>, 64>();
}

void TestStaticArray()
{
	{
		TStaticArray<int32, 4> ArrayA = { 0, 0, 0, 0 };
		TStaticArray<int32, 4> ArrayB = { 0, 0, 0, 0 };
		TStaticArray<int32, 4> ArrayC = { 4, 4, 4, 4 };
		TStaticArray<int32, 4> ArrayD(ArrayC);
		TStaticArray<int32, 4> ArrayE(MoveTemp(ArrayB));
		TStaticArray<int32, 4> ArrayF = { 0, 1, 2, 3 };

		TStaticArray<int32, 4> ArrayG;
		TStaticArray<int32, 4> ArrayH;
		TStaticArray<int32, 4> ArrayI;

		ArrayG = ArrayD;
		ArrayH = MoveTemp(ArrayE);
		ArrayI = { 0, 1, 2, 3 };

		always_check((ArrayC == TStaticArray<int32, 4>({ 4, 4, 4, 4 })));
		always_check((ArrayD == TStaticArray<int32, 4>({ 4, 4, 4, 4 })));
		always_check((ArrayG == TStaticArray<int32, 4>({ 4, 4, 4, 4 })));
		always_check((ArrayF == TStaticArray<int32, 4>({ 0, 1, 2, 3 })));
		always_check((ArrayI == TStaticArray<int32, 4>({ 0, 1, 2, 3 })));
	}

	{
		TStaticArray ArrayA = { 1, 2, 3 };
		TStaticArray ArrayC = { 1, 2, 3 };

		always_check(( (ArrayA == ArrayC)));
		always_check((!(ArrayA != ArrayC)));
		always_check((!(ArrayA <  ArrayC)));
		always_check(( (ArrayA <= ArrayC)));
		always_check((!(ArrayA >  ArrayC)));
		always_check(( (ArrayA >= ArrayC)));
	}

	{
		int32 ArrayA[4] = { 1, 2, 3, 4 };
		TStaticArray<int32, 4> ArrayB = ToArray(ArrayA);
		auto [A, B, C, D] = ArrayB;

		always_check(A == 1);
		always_check(B == 2);
		always_check(C == 3);
		always_check(D == 4);
	}
}

void TestArrayView()
{
	{
		int32        ArrayA[] = { 0, 0, 0, 0 };
		TStaticArray ArrayB   = { 4, 4, 4, 4 };
		TArray       ArrayC   = { 0, 1, 2, 3 };

		TArrayView<int32, 0> ViewA;
		TArrayView<int32, 4> ViewB(ArrayA);
		TArrayView<int32, 4> ViewC(ArrayB);
		TArrayView<int32, 4> ViewD(ViewC);
		TArrayView<int32, 4> ViewE(MoveTemp(ViewB));
		TArrayView<int32, 4> ViewF(ArrayC);

		TArrayView<int32> ViewG;
		TArrayView<int32> ViewH;
		TArrayView<int32> ViewI;

		ViewG = ViewD;
		ViewH = MoveTemp(ViewE);
		ViewI = ArrayC;

		always_check(ViewC == ArrayB);
		always_check(ViewD == ArrayB);
		always_check(ViewG == ArrayB);
		always_check(ViewF == ArrayC);
		always_check(ViewI == ArrayC);
	}

	{
		int32 Array[] = { 0, 1, 2, 3 };

		TArrayView<int32, 4> View = Array;

		int32 First2[] = { 0, 1 };
		always_check(View.First<2>() == First2);
		always_check(View.First(2) == First2);

		int32 Last2[] = { 2, 3 };
		always_check(View.Last<2>() == Last2);
		always_check(View.Last(2) == Last2);

		int32 Subview2[] = { 1, 2 };
		always_check((View.Subview<1, 2>() == Subview2));
		always_check((View.Subview(1, 2) == Subview2));
	}

	{
		int32 Array[] = { 0, 1, 2, 3 };

		TArrayView<int32, 4> View = Array;

		always_check(View.Num() == 4);
		always_check(View.NumBytes() == 16);

		TArrayView ViewBytes = View.AsBytes();

		always_check(ViewBytes.Num() == 16);
		always_check(ViewBytes.NumBytes() == 16);
	}
}

void TestBitset()
{
	{
		FBitset BitsetA;
		FBitset BitsetB(16);
		FBitset BitsetC(16, 0b1010'0100'0100'0010);
		FBitset BitsetD(BitsetC);
		FBitset BitsetE(MoveTemp(BitsetB));
		FBitset BitsetF({ true, false, true, false });

		FBitset BitsetG;
		FBitset BitsetH;
		FBitset BitsetI;

		BitsetG = BitsetD;
		BitsetH = MoveTemp(BitsetE);
		BitsetI = { true, false, true, false };

		always_check((BitsetF == FBitset({ true, false, true, false })));
		always_check((BitsetI == FBitset({ true, false, true, false })));
	}

	{
		FBitset BitsetA(64, 0x0139'0239'0339'0439ull);
		uint64  IntA =      0x0139'0239'0339'0439ull;

		FBitset BitsetB(32, 0x017F'027Full);
		uint32  IntB =      0x017F'027Full;

		FBitset BitsetANDA = BitsetA; BitsetANDA &= BitsetB;
		FBitset BitsetANDB = BitsetB; BitsetANDB &= BitsetA;

		FBitset BitsetORA  = BitsetA; BitsetORA  &= BitsetB;
		FBitset BitsetORB  = BitsetB; BitsetORB  &= BitsetA;

		FBitset BitsetXORA = BitsetA; BitsetXORA &= BitsetB;
		FBitset BitsetXORB = BitsetB; BitsetXORB &= BitsetA;
		
		uint64 IntANDA = IntA; IntANDA &= IntB;
		uint32 IntANDB = IntB; IntANDB &= IntA;

		uint64 IntORA  = IntA; IntORA  &= IntB;
		uint32 IntORB  = IntB; IntORB  &= IntA;

		uint64 IntXORA = IntA; IntXORA &= IntB;
		uint32 IntXORB = IntB; IntXORB &= IntA;

		always_check((BitsetANDA.ToIntegral() == IntANDA));
		always_check((BitsetANDB.ToIntegral() == IntANDB));
		always_check((BitsetORA.ToIntegral()  == IntORA));
		always_check((BitsetORB.ToIntegral()  == IntORB));
		always_check((BitsetXORA.ToIntegral() == IntXORA));
		always_check((BitsetXORB.ToIntegral() == IntXORB));
	}
	
	{
		FBitset BitsetA(64, 0x0139'0239'0339'0439ull);
		uint64  IntA =      0x0139'0239'0339'0439ull;

		FBitset BitsetB(32, 0x017F'027Full);
		uint32  IntB =      0x017F'027Full;

		always_check(((BitsetA & BitsetB).ToIntegral() == (IntA & IntB)));
		always_check(((BitsetA | BitsetB).ToIntegral() == (IntA | IntB)));
		always_check(((BitsetA ^ BitsetB).ToIntegral() == (IntA ^ IntB)));
	}

	{
		FBitset Bitset(64, 0x0139'0239'0339'0439ull);
		uint64  Int =      0x0139'0239'0339'0439ull;

		always_check(((Bitset << 40).ToIntegral() == (Int << 40)));
		always_check(((Bitset >> 40).ToIntegral() == (Int >> 40)));
	}

	{
		FBitset BitsetA(4, 0b0000ull);
		FBitset BitsetB(4, 0b0101ull);
		FBitset BitsetC(4, 0b1111ull);

		always_check((!BitsetA.All() && !BitsetA.Any() &&  BitsetA.None()));
		always_check((!BitsetB.All() &&  BitsetB.Any() && !BitsetB.None()));
		always_check(( BitsetC.All() &&  BitsetC.Any() && !BitsetC.None()));
	}

	{
		FBitset Bitset(16);

		Bitset.Set();

		always_check((Bitset.Count() == 16));

		Bitset.Flip(8);

		always_check((Bitset.Count() == 15));

		Bitset.Flip(8);

		always_check((Bitset.Count() == 16));

		Bitset.Flip();

		always_check((Bitset.Count() == 0));

		Bitset.PushBack(true);

		always_check((Bitset.Num()   == 17));
		always_check((Bitset.Count() ==  1));

		Bitset.PopBack();

		always_check((Bitset.Num()   == 16));
		always_check((Bitset.Count() ==  0));

		Bitset.SetNum(32, true, true);

		always_check((Bitset.Num()   == 32));
		always_check((Bitset.Count() == 16));
	}

	{
		FBitset BitsetA(4);
		FBitset BitsetB(4);

		BitsetA[0] = true;
		BitsetA[1] = false;
		BitsetA[2] = true;
		BitsetA[3] = false;

		BitsetB[0] = true;
		BitsetB[1] = false;
		BitsetB[2] = true;
		BitsetB[3] = false;

		Swap(BitsetA, BitsetB);

		always_check((GetTypeHash(BitsetA) == GetTypeHash(BitsetB)));
	}
}

void TestStaticBitset()
{
	{
		TStaticBitset< 0> BitsetA;
		TStaticBitset<16> BitsetB;
		TStaticBitset<16> BitsetC(0b1010'0100'0100'0010);
		TStaticBitset<16> BitsetD(BitsetC);
		TStaticBitset<16> BitsetE(MoveTemp(BitsetB));
		TStaticBitset< 4> BitsetF(0b0101);

		TStaticBitset<16> BitsetG;
		TStaticBitset<16> BitsetH;
		TStaticBitset< 4> BitsetI;

		BitsetG = BitsetD;
		BitsetH = MoveTemp(BitsetE);
		BitsetI = 0b0101;

		always_check((BitsetF == TStaticBitset<4>(0b0101)));
		always_check((BitsetI == TStaticBitset<4>(0b0101)));
	}

	{
		TStaticBitset<32> BitsetA(0x0139'0239ull);
		uint32            IntA =  0x0139'0239ull;

		TStaticBitset<32> BitsetB(0x017F'027Full);
		uint32            IntB =  0x017F'027Full;

		TStaticBitset<32> BitsetANDA = BitsetA; BitsetANDA &= BitsetB;
		TStaticBitset<32> BitsetANDB = BitsetB; BitsetANDB &= BitsetA;

		TStaticBitset<32> BitsetORA  = BitsetA; BitsetORA  &= BitsetB;
		TStaticBitset<32> BitsetORB  = BitsetB; BitsetORB  &= BitsetA;

		TStaticBitset<32> BitsetXORA = BitsetA; BitsetXORA &= BitsetB;
		TStaticBitset<32> BitsetXORB = BitsetB; BitsetXORB &= BitsetA;
		
		uint32 IntANDA = IntA; IntANDA &= IntB;
		uint32 IntANDB = IntB; IntANDB &= IntA;

		uint32 IntORA  = IntA; IntORA  &= IntB;
		uint32 IntORB  = IntB; IntORB  &= IntA;

		uint32 IntXORA = IntA; IntXORA &= IntB;
		uint32 IntXORB = IntB; IntXORB &= IntA;

		always_check((BitsetANDA.ToIntegral() == IntANDA));
		always_check((BitsetANDB.ToIntegral() == IntANDB));
		always_check((BitsetORA.ToIntegral()  == IntORA));
		always_check((BitsetORB.ToIntegral()  == IntORB));
		always_check((BitsetXORA.ToIntegral() == IntXORA));
		always_check((BitsetXORB.ToIntegral() == IntXORB));
	}
	
	{
		TStaticBitset<32> BitsetA(0x0139'0239ull);
		uint32            IntA =  0x0139'0239ull;

		TStaticBitset<32> BitsetB(0x017F'027Full);
		uint32            IntB =  0x017F'027Full;

		always_check(((BitsetA & BitsetB).ToIntegral() == (IntA & IntB)));
		always_check(((BitsetA | BitsetB).ToIntegral() == (IntA | IntB)));
		always_check(((BitsetA ^ BitsetB).ToIntegral() == (IntA ^ IntB)));
	}

	{
		TStaticBitset<64> Bitset(0x0139'0239'0339'0439ull);
		uint64            Int =  0x0139'0239'0339'0439ull;

		always_check(((Bitset << 40).ToIntegral() == (Int << 40)));
		always_check(((Bitset >> 40).ToIntegral() == (Int >> 40)));
	}

	{
		TStaticBitset<4> BitsetA(0b0000ull);
		TStaticBitset<4> BitsetB(0b0101ull);
		TStaticBitset<4> BitsetC(0b1111ull);

		always_check((!BitsetA.All() && !BitsetA.Any() &&  BitsetA.None()));
		always_check((!BitsetB.All() &&  BitsetB.Any() && !BitsetB.None()));
		always_check(( BitsetC.All() &&  BitsetC.Any() && !BitsetC.None()));
	}

	{
		TStaticBitset<16> Bitset;

		Bitset.Set();

		always_check((Bitset.Count() == 16));

		Bitset.Flip(8);

		always_check((Bitset.Count() == 15));

		Bitset.Flip(8);

		always_check((Bitset.Count() == 16));

		Bitset.Flip();

		always_check((Bitset.Count() == 0));
	}

	{
		TStaticBitset<4> BitsetA;
		TStaticBitset<4> BitsetB;

		BitsetA[0] = true;
		BitsetA[1] = false;
		BitsetA[2] = true;
		BitsetA[3] = false;

		BitsetB[0] = true;
		BitsetB[1] = false;
		BitsetB[2] = true;
		BitsetB[3] = false;

		Swap(BitsetA, BitsetB);

		always_check((GetTypeHash(BitsetA) == GetTypeHash(BitsetB)));
	}
}

void TestList()
{
	{
		TList<int32> ListA;
		TList<int32> ListB(4);
		TList<int32> ListC(4, 4);
		TList<int32> ListD(ListC);
		TList<int32> ListE(MoveTemp(ListB));
		TList<int32> ListF({ 0, 1, 2, 3 });

		TList<int32> ListG;
		TList<int32> ListH;
		TList<int32> ListI;

		ListG = ListD;
		ListH = MoveTemp(ListE);
		ListI = { 0, 1, 2, 3 };

		always_check((ListC == TList<int32>({ 4, 4, 4, 4 })));
		always_check((ListD == TList<int32>({ 4, 4, 4, 4 })));
		always_check((ListG == TList<int32>({ 4, 4, 4, 4 })));
		always_check((ListF == TList<int32>({ 0, 1, 2, 3 })));
		always_check((ListI == TList<int32>({ 0, 1, 2, 3 })));
	}

	{
		TList<int32> ListA = { 1, 2, 3 };
		TList<int32> ListB = { 7, 8, 9, 10 };
		TList<int32> ListC = { 1, 2, 3 };

		always_check((!(ListA == ListB)));
		always_check(( (ListA != ListB)));
		always_check(( (ListA <  ListB)));
		always_check(( (ListA <= ListB)));
		always_check((!(ListA >  ListB)));
		always_check((!(ListA >= ListB)));

		always_check(( (ListA == ListC)));
		always_check((!(ListA != ListC)));
		always_check((!(ListA <  ListC)));
		always_check(( (ListA <= ListC)));
		always_check((!(ListA >  ListC)));
		always_check(( (ListA >= ListC)));
	}

	{
		TList<int32> List = { 1, 2, 3 };

		List.Insert(++List.Begin(), 2);
		always_check((List == TList<int32>({ 1, 2, 2, 3 })));

		List.Insert(List.End(), 2, 4);
		always_check((List == TList<int32>({ 1, 2, 2, 3, 4, 4 })));

		List.Insert(List.Begin(), { 1, 1, 4, 5, 1, 4 });
		always_check((List == TList<int32>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4 })));

		List.Emplace(List.End(), 5);
		always_check((List == TList<int32>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4, 5 })));

		List.Erase(--List.End());
		always_check((List == TList<int32>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3, 4, 4 })));

		List.Erase(----List.End(), List.End());
		always_check((List == TList<int32>({ 1, 1, 4, 5, 1, 4, 1, 2, 2, 3 })));
	}

	{
		TList<int32> List = { 1, 2, 3 };

		List.PushBack(4);
		always_check((List == TList<int32>({ 1, 2, 3, 4 })));

		List.EmplaceBack(5);
		always_check((List == TList<int32>({ 1, 2, 3, 4, 5 })));

		List.EmplaceBack(5) = 6;
		always_check((List == TList<int32>({ 1, 2, 3, 4, 5, 6 })));

		List.PopBack();
		always_check((List == TList<int32>({ 1, 2, 3, 4, 5 })));
		
		List.EmplaceFront(1) = 0;
		always_check((List == TList<int32>({ 0, 1, 2, 3, 4, 5 })));

		List.PopFront();
		always_check((List == TList<int32>({ 1, 2, 3, 4, 5 })));

		List.SetNum(4);
		always_check((List == TList<int32>({ 1, 2, 3, 4 })));
	}
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
