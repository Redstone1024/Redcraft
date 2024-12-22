#include "Testing/Testing.h"

#include "Algorithms/Algorithms.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "Ranges/Factory.h"
#include "Numerics/Math.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

NAMESPACE_PRIVATE_BEGIN

void TestBasic()
{
	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

		auto Iter = Arr.Begin();

		Algorithms::Advance(Iter, 5);

		always_check(*Iter == 5);

		always_check(Algorithms::Distance(Arr.Begin(), Iter) == 5);

		always_check(Algorithms::Distance(Arr) == 10);

		always_check(*Algorithms::Next(Iter)    == 6);
		always_check(*Algorithms::Next(Iter, 2) == 7);
		always_check(*Algorithms::Prev(Iter)    == 4);
		always_check(*Algorithms::Prev(Iter, 2) == 3);

		always_check(Algorithms::Next(Iter,     Arr.End()) == Arr.End());
		always_check(Algorithms::Next(Iter, 16, Arr.End()) == Arr.End());

		always_check(Algorithms::Prev(Iter, 16, Arr.Begin()) == Arr.Begin());

		Iter = Arr.Begin();

		Algorithms::Advance(Iter, Arr.End());

		always_check(Iter == Arr.End());

		Iter = Arr.Begin();

		always_check(Algorithms::Advance(Iter, 16, Arr.End()) == 6);

		always_check(Iter == Arr.End());
	}

	{
		TList<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

		auto Iter = Arr.Begin();

		Algorithms::Advance(Iter, 5);

		always_check(*Iter == 5);

		always_check(Algorithms::Distance(Arr.Begin(), Iter) == 5);

		always_check(Algorithms::Distance(Arr) == 10);

		always_check(*Algorithms::Next(Iter)    == 6);
		always_check(*Algorithms::Next(Iter, 2) == 7);
		always_check(*Algorithms::Prev(Iter)    == 4);
		always_check(*Algorithms::Prev(Iter, 2) == 3);

		always_check(Algorithms::Next(Iter,     Arr.End()) == Arr.End());
		always_check(Algorithms::Next(Iter, 16, Arr.End()) == Arr.End());

		always_check(Algorithms::Prev(Iter, 16, Arr.Begin()) == Arr.Begin());

		Iter = Arr.Begin();

		Algorithms::Advance(Iter, Arr.End());

		always_check(Iter == Arr.End());

		Iter = Arr.Begin();

		always_check(Algorithms::Advance(Iter, 16, Arr.End()) == 6);

		always_check(Iter == Arr.End());
	}

	{
		auto Arr = Ranges::Iota(0, 10);

		auto Iter = Arr.Begin();

		Algorithms::Advance(Iter, 5);

		always_check(*Iter == 5);

		always_check(Algorithms::Distance(Arr.Begin(), Iter) == 5);

		always_check(Algorithms::Distance(Arr) == 10);

		always_check(*Algorithms::Next(Iter)    == 6);
		always_check(*Algorithms::Next(Iter, 2) == 7);

		always_check(Algorithms::Next(Iter,     Arr.End()) == Arr.End());
		always_check(Algorithms::Next(Iter, 16, Arr.End()) == Arr.End());

		Iter = Arr.Begin();

		Algorithms::Advance(Iter, Arr.End());

		always_check(Iter == Arr.End());

		Iter = Arr.Begin();

		always_check(Algorithms::Advance(Iter, 16, Arr.End()) == 6);

		always_check(Iter == Arr.End());
	}
}

void TestSearch()
{
	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		TList<int>  Brr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		auto        Crr = Ranges::Iota(0, 10);

		always_check( Algorithms::AllOf(Arr, [](int A) { return A < 10; }));
		always_check( Algorithms::AllOf(Brr, [](int A) { return A < 10; }));
		always_check( Algorithms::AllOf(Crr, [](int A) { return A < 10; }));
		always_check(!Algorithms::AllOf(Arr, [](int A) { return A >  5; }));
		always_check(!Algorithms::AllOf(Brr, [](int A) { return A >  5; }));
		always_check(!Algorithms::AllOf(Crr, [](int A) { return A >  5; }));

		always_check( Algorithms::AllOf(Arr.Begin(), Arr.End(), [](int A) { return A < 10; }));
		always_check( Algorithms::AllOf(Brr.Begin(), Brr.End(), [](int A) { return A < 10; }));
		always_check( Algorithms::AllOf(Crr.Begin(), Crr.End(), [](int A) { return A < 10; }));
		always_check(!Algorithms::AllOf(Arr.Begin(), Arr.End(), [](int A) { return A >  5; }));
		always_check(!Algorithms::AllOf(Brr.Begin(), Brr.End(), [](int A) { return A >  5; }));
		always_check(!Algorithms::AllOf(Crr.Begin(), Crr.End(), [](int A) { return A >  5; }));

		always_check(Algorithms::AnyOf(Arr, [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Brr, [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Crr, [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Arr, [](int A) { return A >  5; }));
		always_check(Algorithms::AnyOf(Brr, [](int A) { return A >  5; }));
		always_check(Algorithms::AnyOf(Crr, [](int A) { return A >  5; }));

		always_check(Algorithms::AnyOf(Arr.Begin(), Arr.End(), [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Brr.Begin(), Brr.End(), [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Crr.Begin(), Crr.End(), [](int A) { return A < 10; }));
		always_check(Algorithms::AnyOf(Arr.Begin(), Arr.End(), [](int A) { return A >  5; }));
		always_check(Algorithms::AnyOf(Brr.Begin(), Brr.End(), [](int A) { return A >  5; }));
		always_check(Algorithms::AnyOf(Crr.Begin(), Crr.End(), [](int A) { return A >  5; }));

		always_check(!Algorithms::NoneOf(Arr, [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Brr, [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Crr, [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Arr, [](int A) { return A >  5; }));
		always_check(!Algorithms::NoneOf(Brr, [](int A) { return A >  5; }));
		always_check(!Algorithms::NoneOf(Crr, [](int A) { return A >  5; }));

		always_check(!Algorithms::NoneOf(Arr.Begin(), Arr.End(), [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Brr.Begin(), Brr.End(), [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Crr.Begin(), Crr.End(), [](int A) { return A < 10; }));
		always_check(!Algorithms::NoneOf(Arr.Begin(), Arr.End(), [](int A) { return A >  5; }));
		always_check(!Algorithms::NoneOf(Brr.Begin(), Brr.End(), [](int A) { return A >  5; }));
		always_check(!Algorithms::NoneOf(Crr.Begin(), Crr.End(), [](int A) { return A >  5; }));

		always_check( Algorithms::Contains(Arr,  5));
		always_check( Algorithms::Contains(Brr,  5));
		always_check( Algorithms::Contains(Crr,  5));
		always_check(!Algorithms::Contains(Arr, 10));
		always_check(!Algorithms::Contains(Brr, 10));
		always_check(!Algorithms::Contains(Crr, 10));

		always_check( Algorithms::Contains(Arr.Begin(), Arr.End(),  5));
		always_check( Algorithms::Contains(Brr.Begin(), Brr.End(),  5));
		always_check( Algorithms::Contains(Crr.Begin(), Crr.End(),  5));
		always_check(!Algorithms::Contains(Arr.Begin(), Arr.End(), 10));
		always_check(!Algorithms::Contains(Brr.Begin(), Brr.End(), 10));
		always_check(!Algorithms::Contains(Crr.Begin(), Crr.End(), 10));

		auto Projection = [](int A) { return A % 4; }; // Project to { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1 }

		always_check(Algorithms::Find(Arr, 2, { }, Projection) == Algorithms::Next(Arr.Begin(), 2));
		always_check(Algorithms::Find(Brr, 2, { }, Projection) == Algorithms::Next(Brr.Begin(), 2));
		always_check(Algorithms::Find(Crr, 2, { }, Projection) == Algorithms::Next(Crr.Begin(), 2));

		always_check(Algorithms::Find(Arr.Begin(), Arr.End(), 2, { }, Projection) == Algorithms::Next(Arr.Begin(), 2));
		always_check(Algorithms::Find(Brr.Begin(), Brr.End(), 2, { }, Projection) == Algorithms::Next(Brr.Begin(), 2));
		always_check(Algorithms::Find(Crr.Begin(), Crr.End(), 2, { }, Projection) == Algorithms::Next(Crr.Begin(), 2));

		always_check(Algorithms::Find(Arr, 10) == Arr.End());
		always_check(Algorithms::Find(Brr, 10) == Brr.End());
		always_check(Algorithms::Find(Crr, 10) == Crr.End());

		always_check(Algorithms::Find(Arr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Arr.Begin()));
		always_check(Algorithms::Find(Brr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Brr.Begin()));
		always_check(Algorithms::Find(Crr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Crr.Begin()));

		always_check(Algorithms::Find(Arr, Ranges::Iota(4, 16)).IsEmpty());
		always_check(Algorithms::Find(Brr, Ranges::Iota(4, 16)).IsEmpty());
		always_check(Algorithms::Find(Crr, Ranges::Iota(4, 16)).IsEmpty());

		always_check(Algorithms::FindIf(Arr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Arr.Begin(), 2));
		always_check(Algorithms::FindIf(Brr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Brr.Begin(), 2));
		always_check(Algorithms::FindIf(Crr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Crr.Begin(), 2));

		always_check(Algorithms::FindIf(Arr.Begin(), Arr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Arr.Begin(), 2));
		always_check(Algorithms::FindIf(Brr.Begin(), Brr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Brr.Begin(), 2));
		always_check(Algorithms::FindIf(Crr.Begin(), Crr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Crr.Begin(), 2));

		always_check(Algorithms::FindIf(Arr, [](int A) { return A == 10; }) == Arr.End());
		always_check(Algorithms::FindIf(Brr, [](int A) { return A == 10; }) == Brr.End());
		always_check(Algorithms::FindIf(Crr, [](int A) { return A == 10; }) == Crr.End());

		always_check(Algorithms::FindIfNot(Arr, [](int A) { return A > 0; }, Projection) == Arr.Begin());
		always_check(Algorithms::FindIfNot(Brr, [](int A) { return A > 0; }, Projection) == Brr.Begin());
		always_check(Algorithms::FindIfNot(Crr, [](int A) { return A > 0; }, Projection) == Crr.Begin());

		always_check(Algorithms::FindIfNot(Arr.Begin(), Arr.End(), [](int A) { return A > 0; }, Projection) == Arr.Begin());
		always_check(Algorithms::FindIfNot(Brr.Begin(), Brr.End(), [](int A) { return A > 0; }, Projection) == Brr.Begin());
		always_check(Algorithms::FindIfNot(Crr.Begin(), Crr.End(), [](int A) { return A > 0; }, Projection) == Crr.Begin());

		always_check(Algorithms::FindIfNot(Arr, [](int A) { return A < 8; }) == Algorithms::Next(Arr.Begin(), 8));
		always_check(Algorithms::FindIfNot(Brr, [](int A) { return A < 8; }) == Algorithms::Next(Brr.Begin(), 8));
		always_check(Algorithms::FindIfNot(Crr, [](int A) { return A < 8; }) == Algorithms::Next(Crr.Begin(), 8));

		always_check(Algorithms::FindLast(Arr, 2, { }, Projection) == Algorithms::Next(Arr.Begin(), 6));
		always_check(Algorithms::FindLast(Brr, 2, { }, Projection) == Algorithms::Next(Brr.Begin(), 6));
		always_check(Algorithms::FindLast(Crr, 2, { }, Projection) == Algorithms::Next(Crr.Begin(), 6));

		always_check(Algorithms::FindLast(Arr.Begin(), Arr.End(), 2, { }, Projection) == Algorithms::Next(Arr.Begin(), 6));
		always_check(Algorithms::FindLast(Brr.Begin(), Brr.End(), 2, { }, Projection) == Algorithms::Next(Brr.Begin(), 6));
		always_check(Algorithms::FindLast(Crr.Begin(), Crr.End(), 2, { }, Projection) == Algorithms::Next(Crr.Begin(), 6));

		always_check(Algorithms::FindLast(Arr, 10) == Arr.End());
		always_check(Algorithms::FindLast(Brr, 10) == Brr.End());
		always_check(Algorithms::FindLast(Crr, 10) == Crr.End());

		always_check(Algorithms::FindLast(Arr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Arr.Begin(), 5));
		always_check(Algorithms::FindLast(Brr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Brr.Begin(), 5));
		always_check(Algorithms::FindLast(Crr, Ranges::Iota(1, 4), { }, Projection).Begin() == Algorithms::Next(Crr.Begin(), 5));

		always_check(Algorithms::FindLast(Arr, Ranges::Iota(4, 16)).IsEmpty());
		always_check(Algorithms::FindLast(Brr, Ranges::Iota(4, 16)).IsEmpty());
		always_check(Algorithms::FindLast(Crr, Ranges::Iota(4, 16)).IsEmpty());

		always_check(Algorithms::FindLastIf(Arr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Arr.Begin(), 6));
		always_check(Algorithms::FindLastIf(Brr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Brr.Begin(), 6));
		always_check(Algorithms::FindLastIf(Crr, [](int A) { return A == 2; }, Projection) == Algorithms::Next(Crr.Begin(), 6));

		always_check(Algorithms::FindLastIf(Arr.Begin(), Arr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Arr.Begin(), 6));
		always_check(Algorithms::FindLastIf(Brr.Begin(), Brr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Brr.Begin(), 6));
		always_check(Algorithms::FindLastIf(Crr.Begin(), Crr.End(), [](int A) { return A == 2; }, Projection) == Algorithms::Next(Crr.Begin(), 6));

		always_check(Algorithms::FindLastIf(Arr, [](int A) { return A == 10; }) == Arr.End());
		always_check(Algorithms::FindLastIf(Brr, [](int A) { return A == 10; }) == Brr.End());
		always_check(Algorithms::FindLastIf(Crr, [](int A) { return A == 10; }) == Crr.End());

		always_check(Algorithms::FindLastIfNot(Arr, [](int A) { return A > 0; }, Projection) == Algorithms::Next(Arr.Begin(), 8));
		always_check(Algorithms::FindLastIfNot(Brr, [](int A) { return A > 0; }, Projection) == Algorithms::Next(Brr.Begin(), 8));
		always_check(Algorithms::FindLastIfNot(Crr, [](int A) { return A > 0; }, Projection) == Algorithms::Next(Crr.Begin(), 8));

		always_check(Algorithms::FindLastIfNot(Arr.Begin(), Arr.End(), [](int A) { return A > 0; }, Projection) == Algorithms::Next(Arr.Begin(), 8));
		always_check(Algorithms::FindLastIfNot(Brr.Begin(), Brr.End(), [](int A) { return A > 0; }, Projection) == Algorithms::Next(Brr.Begin(), 8));
		always_check(Algorithms::FindLastIfNot(Crr.Begin(), Crr.End(), [](int A) { return A > 0; }, Projection) == Algorithms::Next(Crr.Begin(), 8));

		always_check(Algorithms::FindLastIfNot(Arr, [](int A) { return A < 8; }) == Algorithms::Next(Arr.Begin(), 9));
		always_check(Algorithms::FindLastIfNot(Brr, [](int A) { return A < 8; }) == Algorithms::Next(Brr.Begin(), 9));
		always_check(Algorithms::FindLastIfNot(Crr, [](int A) { return A < 8; }) == Algorithms::Next(Crr.Begin(), 9));

		always_check(Algorithms::FindAdjacent(Arr, { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Arr.Begin()));
		always_check(Algorithms::FindAdjacent(Brr, { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Brr.Begin()));
		always_check(Algorithms::FindAdjacent(Crr, { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Crr.Begin()));

		always_check(Algorithms::FindAdjacent(Arr.Begin(), Arr.End(), { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Arr.Begin()));
		always_check(Algorithms::FindAdjacent(Brr.Begin(), Brr.End(), { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Brr.Begin()));
		always_check(Algorithms::FindAdjacent(Crr.Begin(), Crr.End(), { }, [](int A) { return Math::DivAndCeil(A, 2); }) == Algorithms::Next(Crr.Begin()));

		always_check(Algorithms::FindAdjacent(Arr) == Arr.End());
		always_check(Algorithms::FindAdjacent(Brr) == Brr.End());
		always_check(Algorithms::FindAdjacent(Crr) == Crr.End());

		always_check(Algorithms::Count(Arr, 2, { }, Projection) == 2);
		always_check(Algorithms::Count(Brr, 2, { }, Projection) == 2);
		always_check(Algorithms::Count(Crr, 2, { }, Projection) == 2);

		always_check(Algorithms::Count(Arr.Begin(), Arr.End(), 2, { }, Projection) == 2);
		always_check(Algorithms::Count(Brr.Begin(), Brr.End(), 2, { }, Projection) == 2);
		always_check(Algorithms::Count(Crr.Begin(), Crr.End(), 2, { }, Projection) == 2);

		always_check(Algorithms::Count(Arr, 10) == 0);
		always_check(Algorithms::Count(Brr, 10) == 0);
		always_check(Algorithms::Count(Crr, 10) == 0);

		always_check(Algorithms::CountIf(Arr, [](int A) { return A == 2; }, Projection) == 2);
		always_check(Algorithms::CountIf(Brr, [](int A) { return A == 2; }, Projection) == 2);
		always_check(Algorithms::CountIf(Crr, [](int A) { return A == 2; }, Projection) == 2);

		always_check(Algorithms::CountIf(Arr.Begin(), Arr.End(), [](int A) { return A == 2; }, Projection) == 2);
		always_check(Algorithms::CountIf(Brr.Begin(), Brr.End(), [](int A) { return A == 2; }, Projection) == 2);
		always_check(Algorithms::CountIf(Crr.Begin(), Crr.End(), [](int A) { return A == 2; }, Projection) == 2);

		always_check(Algorithms::CountIf(Arr, [](int A) { return A == 10; }) == 0);
		always_check(Algorithms::CountIf(Brr, [](int A) { return A == 10; }) == 0);
		always_check(Algorithms::CountIf(Crr, [](int A) { return A == 10; }) == 0);

		always_check(Algorithms::Mismatch(Arr, Arr, { }, Projection) == MakeTuple(Algorithms::Next(Arr.Begin(), 4), Algorithms::Next(Arr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Brr, Brr, { }, Projection) == MakeTuple(Algorithms::Next(Brr.Begin(), 4), Algorithms::Next(Brr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Crr, Crr, { }, Projection) == MakeTuple(Algorithms::Next(Crr.Begin(), 4), Algorithms::Next(Crr.Begin(), 4)));

		always_check(Algorithms::Mismatch(Arr.Begin(), Arr.End(), Brr.Begin(), Brr.End(), { }, Projection) == MakeTuple(Algorithms::Next(Arr.Begin(), 4), Algorithms::Next(Brr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Brr.Begin(), Brr.End(), Crr.Begin(), Crr.End(), { }, Projection) == MakeTuple(Algorithms::Next(Brr.Begin(), 4), Algorithms::Next(Crr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Crr.Begin(), Crr.End(), Arr.Begin(), Arr.End(), { }, Projection) == MakeTuple(Algorithms::Next(Crr.Begin(), 4), Algorithms::Next(Arr.Begin(), 4)));

		always_check(Algorithms::Mismatch(Arr, Brr, { }, Projection) == MakeTuple(Algorithms::Next(Arr.Begin(), 4), Algorithms::Next(Brr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Brr, Crr, { }, Projection) == MakeTuple(Algorithms::Next(Brr.Begin(), 4), Algorithms::Next(Crr.Begin(), 4)));
		always_check(Algorithms::Mismatch(Crr, Arr, { }, Projection) == MakeTuple(Algorithms::Next(Crr.Begin(), 4), Algorithms::Next(Arr.Begin(), 4)));

		always_check(Algorithms::Equal(Arr, Arr));
		always_check(Algorithms::Equal(Brr, Brr));
		always_check(Algorithms::Equal(Crr, Crr));

		always_check(Algorithms::Equal(Arr.Begin(), Arr.End(), Brr.Begin(), Brr.End()));
		always_check(Algorithms::Equal(Brr.Begin(), Brr.End(), Crr.Begin(), Crr.End()));
		always_check(Algorithms::Equal(Crr.Begin(), Crr.End(), Arr.Begin(), Arr.End()));

		always_check(Algorithms::Equal(Arr, Brr));
		always_check(Algorithms::Equal(Brr, Crr));
		always_check(Algorithms::Equal(Crr, Arr));

		always_check(Algorithms::StartsWith(Arr, Ranges::Iota(0, 8)));
		always_check(Algorithms::StartsWith(Brr, Ranges::Iota(0, 8)));
		always_check(Algorithms::StartsWith(Crr, Ranges::Iota(0, 8)));

		always_check(!Algorithms::StartsWith(Arr, Ranges::Iota(0, 8), { }, Projection));
		always_check(!Algorithms::StartsWith(Brr, Ranges::Iota(0, 8), { }, Projection));
		always_check(!Algorithms::StartsWith(Crr, Ranges::Iota(0, 8), { }, Projection));

		always_check(Algorithms::EndsWith(Arr, Ranges::Iota(8, 10)));
		always_check(Algorithms::EndsWith(Brr, Ranges::Iota(8, 10)));
		always_check(Algorithms::EndsWith(Crr, Ranges::Iota(8, 10)));

		always_check(Algorithms::EndsWith(Arr, Ranges::Iota(0, 2), { }, Projection));
		always_check(Algorithms::EndsWith(Brr, Ranges::Iota(0, 2), { }, Projection));
		always_check(Algorithms::EndsWith(Crr, Ranges::Iota(0, 2), { }, Projection));
	}
}

NAMESPACE_PRIVATE_END

void TestAlgorithms()
{
	NAMESPACE_PRIVATE::TestBasic();
	NAMESPACE_PRIVATE::TestSearch();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
