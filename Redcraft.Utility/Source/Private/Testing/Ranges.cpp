#include "Testing/Testing.h"

#include "Ranges/Ranges.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

NAMESPACE_PRIVATE_BEGIN

void TestConversion()
{
	{
		const TArray<int> Arr  = { 1, 2, 3, 4, 5 };
		const TList<int>  List = { 1, 2, 3, 4, 5 };

		const TArray<int> Brr  = Ranges::View(List.Begin(), List.End()) | Ranges::To<TArray<int>>();
		const TList<int>  Mist = Ranges::View(Arr.Begin(),  Arr.End())  | Ranges::To<TList<int>>();

		always_check(Arr  == Brr);
		always_check(List == Mist);
	}

	{
		const TArray<int> Arr  = { 1, 2, 3, 4, 5 };
		const TList<int>  List = { 1, 2, 3, 4, 5 };

		const TArray<int> Brr  = Ranges::View(List.Begin(), List.End()) | Ranges::To<TArray>();
		const TList<int>  Mist = Ranges::View(Arr.Begin(),  Arr.End())  | Ranges::To<TList>();

		always_check(Arr  == Brr);
		always_check(List == Mist);
	}
}

void TestFactory()
{
	{
		const TArray<int> Arr = { };
		const TArray<int> Brr = Ranges::Empty<int> | Ranges::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		const TArray<int> Arr = { 1 };
		const TArray<int> Brr = Ranges::Single(1) | Ranges::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		const TArray<int> Arr = { 0, 1, 2, 3, 4 };
		const TArray<int> Brr = Ranges::Iota(0, 5) | Ranges::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		auto View = Ranges::Iota(0, 5);

		always_check(View.Num() == 5);
		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);
		always_check(Last  == ConstLast );

		ConstFirst = First;
		ConstLast  = Last;

		auto Iter = ConstFirst;
		auto Jter = ConstLast;

		++Iter;

		always_check(*Iter++ == 1);
	}

	{
		auto View = Ranges::Iota(0);

		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);

		ConstFirst = First;
		ConstLast  = Last;

		auto Iter = ConstFirst;
		auto Jter = ConstLast;

		++Iter;

		always_check(*Iter++ == 1);
	}

	{
		const TArray<int> Arr = { 0, 0, 0, 0, 0 };
		const TArray<int> Brr = Ranges::Repeat(0, 5) | Ranges::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		auto View = Ranges::Repeat(0, 8);

		always_check(View.Num() == 8);
		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);
		always_check(View.Back()  == 0);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);
		always_check(Last  == ConstLast );

		always_check(ConstLast - First == 8);

		ConstFirst = First;
		ConstLast  = Last;

		auto Iter = ConstFirst;
		auto Jter = ConstLast;

		++Iter;
		--Jter;

		always_check(*Iter++ == 0);
		always_check(*Jter-- == 0);

		Iter += 2;
		Jter -= 2;

		always_check(Iter[-1] == 0);
		always_check(Jter[ 1] == 0);

		Iter = Iter - 2;
		Jter = Jter + 2;

		always_check(*Iter == 0);
		always_check(*Jter == 0);

		Iter = 2 + Iter;
		Jter = Jter - 2;

		always_check(Iter - Jter == 0);
	}

	{
		auto View = Ranges::Repeat(0);

		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);

		ConstFirst = First;
		ConstLast  = Last;

		auto Iter = ConstFirst;
		auto Jter = ConstFirst + 8;

		++Iter;
		--Jter;

		always_check(*Iter++ == 0);
		always_check(*Jter-- == 0);

		Iter += 2;
		Jter -= 2;

		always_check(Iter[-1] == 0);
		always_check(Jter[ 1] == 0);

		Iter = Iter - 2;
		Jter = Jter + 2;

		always_check(*Iter == 0);
		always_check(*Jter == 0);

		Iter = 2 + Iter;
		Jter = Jter - 2;

		always_check(Iter - Jter == 0);
	}
}

void TestAllView()
{
	TArray<int> Arr = { 0, 1, 2, 3, 4 };

	TArray<int> Brr = Ranges::All(Arr) | Ranges::To<TArray<int>>();

	always_check(Arr == Brr);

	auto View = Ranges::All(MoveTemp(Arr));

	Arr.Reset();

	TArray<int> Crr = View | Ranges::To<TArray<int>>();

	always_check(Brr == Crr);
}

void TestMoveView()
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

		auto View = Arr | Ranges::Move();

		auto First = View.Begin();
		auto Last  = View.End();

		FTracker Temp(*First++);

		Temp = *First++;

		always_check(First == Last);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7 };

		auto View = Arr | Ranges::Move();

		always_check(View.Num() == 8);
		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);
		always_check(View.Back()  == 7);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);
		always_check(Last  == ConstLast );

		always_check(ConstLast - First == 8);

		ConstFirst = First;
		ConstLast  = Last;

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

	{
		auto View = Ranges::Iota(0) | Ranges::Move();

		always_check(!View.IsEmpty());
		always_check(!!View);

		always_check(View.Front() == 0);

		auto First = View.Begin();
		auto Last  = View.End();

		auto ConstFirst = AsConst(View).Begin();
		auto ConstLast  = AsConst(View).End();

		always_check(First == ConstFirst);

		ConstFirst = First;
		ConstLast  = Last;

		auto Iter = ConstFirst;
		auto Jter = ConstLast;

		++Iter;

		always_check(*Iter++ == 1);
	}
}

void TestMiscView()
{
	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7 };
		TArray<int> Brr = { 0, 2, 4, 6 };

		TArray<int> Crr = Arr
			| Ranges::Filter([](int Value) { return Value % 2 == 0; })
			| Ranges::To<TArray<int>>();

		always_check(Brr == Crr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 2, 1, 0 };
		TArray<int> Brr = { 0, 2, 4, 4, 2, 0 };

		TArray<int> Crr = Arr
			| Ranges::Transform([](int Value) { return Value * 2; })
			| Ranges::To<TArray<int>>();

		always_check(Brr == Crr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 3, 2, 1, 0 };
		TArray<int> Brr = { 0, 2, 4, 4, 2, 0 };

		TArray<int> Crr = Arr
			| Ranges::Filter   ([](int Value) { return Value < 3; })
			| Ranges::Transform([](int Value) { return Value * 2; })
			| Ranges::To<TArray<int>>();

		TArray<int> Drr = Arr
			| Ranges::Transform([](int Value) { return Value * 2; })
			| Ranges::Filter   ([](int Value) { return Value < 6; })
			| Ranges::To<TArray<int>>();

		always_check(Brr == Crr);
		always_check(Brr == Drr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7 };

		TArray<int> Brr = Ranges::Iota(0)
			| Ranges::Take(8)
			| Ranges::To<TArray<int>>();

		TArray<int> Crr = Ranges::Iota(0)
			| Ranges::TakeWhile([](int Value) { return Value < 8; })
			| Ranges::To<TArray<int>>();

		always_check(Arr == Brr);
		always_check(Arr == Crr);
	}

	{
		TArray<int> Arr = { 0, 4, 7, 8, 3, 1, 10 };
		TArray<int> Brr = { 0, 2, 4 };

		TArray<int> Crr = Arr
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::Take(3)
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::To<TArray<int>>();

		TArray<int> Drr = Arr
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::TakeWhile([](int Value) { return Value < 10;     })
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::To<TArray<int>>();

		TArray<int> Err = Arr
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::Take(3)
			| Ranges::To<TArray<int>>();

		TArray<int> Frr = Arr
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::TakeWhile([](int Value) { return Value < 5;      })
			| Ranges::To<TArray<int>>();

		TArray<int> Grr = Arr
			| Ranges::Take(6)
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::To<TArray<int>>();

		TArray<int> Hrr = Arr
			| Ranges::TakeWhile([](int Value) { return Value < 10;     })
			| Ranges::Filter   ([](int Value) { return Value % 2 == 0; })
			| Ranges::Transform([](int Value) { return Value / 2;      })
			| Ranges::To<TArray<int>>();

		always_check(Brr == Crr);
		always_check(Brr == Drr);
		always_check(Brr == Err);
		always_check(Brr == Frr);
		always_check(Brr == Grr);
		always_check(Brr == Hrr);
	}
}

NAMESPACE_PRIVATE_END

void TestRange()
{
	NAMESPACE_PRIVATE::TestConversion();
	NAMESPACE_PRIVATE::TestFactory();
	NAMESPACE_PRIVATE::TestAllView();
	NAMESPACE_PRIVATE::TestMoveView();
	NAMESPACE_PRIVATE::TestMiscView();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
