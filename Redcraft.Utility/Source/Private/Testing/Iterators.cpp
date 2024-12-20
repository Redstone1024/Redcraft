#include "Testing/Testing.h"

#include "Iterators/Iterators.h"
#include "Containers/List.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

NAMESPACE_PRIVATE_BEGIN

void TestMoveIterator()
{
	{
		struct FTracker
		{
			FTracker()                           = default;
			FTracker(const FTracker&)            { always_check_no_entry(); }
			FTracker(FTracker&&)                 = default;
			~FTracker()                          = default;
			FTracker& operator=(const FTracker&) { always_check_no_entry(); }
			FTracker& operator=(FTracker&&)      = default;
		};

		FTracker Arr[2];

		auto First = MakeMoveIterator(&Arr[0]);
		auto Last  = MakeMoveIterator(&Arr[2]);

		FTracker Temp(*First++);

		Temp = *First++;

		always_check(First == Last);
	}

	{
		int Arr[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

		auto First = MakeMoveIterator(&Arr[0]);
		auto Last  = MakeMoveIterator(&Arr[8]);

		auto ConstFirst = MakeMoveIterator(&AsConst(Arr)[0]);
		auto ConstLast  = MakeMoveIterator(&AsConst(Arr)[8]);

		always_check(First == ConstFirst);
		always_check(Last  == ConstLast );

		always_check(ConstLast - First == 8);

		auto Iter = ConstFirst;
		auto Jter = ConstLast;

		++Iter;
		--Jter;

		always_check(*Iter++ == 1);
		always_check(*Jter-- == 7);

		Iter += 2;
		Jter -= 2;

		always_check(Iter[-1] == 3);
		always_check(Jter[ 1] == 5);

		Iter = Iter - 2;
		Jter = Jter + 2;

		always_check(*Iter == 2);
		always_check(*Jter == 6);

		Iter = 2 + Iter;
		Jter = Jter - 2;

		always_check(Iter - Jter == 0);
	}
}

void TestReverseIterator()
{
	int Arr[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };

	auto First = MakeReverseIterator(&Arr[8]);
	auto Last  = MakeReverseIterator(&Arr[0]);

	auto ConstFirst = MakeReverseIterator(&AsConst(Arr)[8]);
	auto ConstLast  = MakeReverseIterator(&AsConst(Arr)[0]);

	always_check(First == ConstFirst);
	always_check(Last  == ConstLast );

	always_check(ConstLast - First == 8);

	auto Iter = ConstFirst;
	auto Jter = ConstLast;

	++Iter;
	--Jter;

	always_check(*Iter++ == 1);
	always_check(*Jter-- == 7);

	Iter += 2;
	Jter -= 2;

	always_check(Iter[-1] == 3);
	always_check(Jter[ 1] == 5);

	Iter = Iter - 2;
	Jter = Jter + 2;

	always_check(*Iter == 2);
	always_check(*Jter == 6);

	Iter = 2 + Iter;
	Jter = Jter - 2;

	always_check(Iter - Jter == 0);
}

void TestCountedIterator()
{
	int Arr[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	auto First = MakeCountedIterator(&Arr[0], 8);
	auto Last  = First + 8;

	auto ConstFirst = MakeCountedIterator(&AsConst(Arr)[0], 8);
	auto ConstLast  = ConstFirst + 8;

	always_check(First == ConstFirst);
	always_check(Last  == ConstLast );

	always_check(ConstLast - First == 8);

	always_check(Last == DefaultSentinel);
	always_check(DefaultSentinel == Last);

	always_check(DefaultSentinel - First ==  8);
	always_check(First - DefaultSentinel == -8);

	always_check(First == ConstFirst);
	always_check(Last  == ConstLast );

	always_check(Last - First == 8);

	auto Iter = ConstFirst;
	auto Jter = ConstLast;

	++Iter;
	--Jter;

	always_check(*Iter++ == 1);
	always_check(*Jter-- == 7);

	Iter += 2;
	Jter -= 2;

	always_check(Iter[-1] == 3);
	always_check(Jter[ 1] == 5);

	Iter = Iter - 2;
	Jter = Jter + 2;

	always_check(*Iter == 2);
	always_check(*Jter == 6);

	Iter = 2 + Iter;
	Jter = Jter - 2;

	always_check(Iter - Jter == 0);
}

void TestInsertIterator()
{
	{
		TList<int> List = { 1, 2, 3 };

		auto Iter = MakeFrontInserter(List);

		*Iter++ = 1;
		*Iter++ = 2;
		*Iter++ = 3;

		always_check(List == TList<int>({ 3, 2, 1, 1, 2, 3 }));
	}

	{
		TList<int> List = { 1, 2, 3 };

		auto Iter = MakeBackInserter(List);

		*Iter++ = 1;
		*Iter++ = 2;
		*Iter++ = 3;

		always_check(List == TList<int>({ 1, 2, 3, 1, 2, 3 }));
	}

	{
		TList<int> List = { 1, 2, 3 };

		auto Iter = MakeInserter(List, ++++List.Begin());

		*Iter++ = 1;
		*Iter++ = 2;
		*Iter++ = 3;

		always_check(List == TList<int>({ 1, 2, 1, 2, 3, 3 }));
	}
}

NAMESPACE_PRIVATE_END

void TestIterator()
{
	NAMESPACE_PRIVATE::TestMoveIterator();
	NAMESPACE_PRIVATE::TestReverseIterator();
	NAMESPACE_PRIVATE::TestCountedIterator();
	NAMESPACE_PRIVATE::TestInsertIterator();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
