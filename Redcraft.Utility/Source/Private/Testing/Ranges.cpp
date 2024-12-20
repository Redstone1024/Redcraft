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

		const TArray<int> Brr  = Range::View(List.Begin(), List.End()) | Range::To<TArray<int>>();
		const TList<int>  Mist = Range::View(Arr.Begin(),  Arr.End())  | Range::To<TList<int>>();

		always_check(Arr  == Brr);
		always_check(List == Mist);
	}

	{
		const TArray<int> Arr  = { 1, 2, 3, 4, 5 };
		const TList<int>  List = { 1, 2, 3, 4, 5 };

		const TArray<int> Brr  = Range::View(List.Begin(), List.End()) | Range::To<TArray>();
		const TList<int>  Mist = Range::View(Arr.Begin(),  Arr.End())  | Range::To<TList>();

		always_check(Arr  == Brr);
		always_check(List == Mist);
	}
}

void TestFactory()
{
	{
		const TArray<int> Arr = { };
		const TArray<int> Brr = Range::Empty<int> | Range::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		const TArray<int> Arr = { 1 };
		const TArray<int> Brr = Range::Single(1) | Range::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		const TArray<int> Arr = { 0, 1, 2, 3, 4 };
		const TArray<int> Brr = Range::Iota(0, 5) | Range::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		auto View = Range::Iota(0, 5);

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
		auto View = Range::Iota(0);

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
		const TArray<int> Brr = Range::Repeat(0, 5) | Range::To<TArray<int>>();

		always_check(Arr == Brr);
	}

	{
		auto View = Range::Repeat(0, 8);

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
		auto View = Range::Repeat(0);

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

	TArray<int> Brr = Range::All(Arr) | Range::To<TArray<int>>();

	always_check(Arr == Brr);

	auto View = Range::All(MoveTemp(Arr));

	Arr.Reset();

	TArray<int> Crr = View | Range::To<TArray<int>>();

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

		auto View = Arr | Range::Move();

		auto First = View.Begin();
		auto Last  = View.End();

		FTracker Temp(*First++);

		Temp = *First++;

		always_check(First == Last);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7 };

		auto View = Arr | Range::Move();

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
		auto View = Range::Iota(0) | Range::Move();

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
			| Range::Filter([](int Value) { return Value % 2 == 0; })
			| Range::To<TArray<int>>();

		always_check(Brr == Crr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 2, 1, 0 };
		TArray<int> Brr = { 0, 2, 4, 4, 2, 0 };

		TArray<int> Crr = Arr
			| Range::Transform([](int Value) { return Value * 2; })
			| Range::To<TArray<int>>();

		always_check(Brr == Crr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 3, 2, 1, 0 };
		TArray<int> Brr = { 0, 2, 4, 4, 2, 0 };

		TArray<int> Crr = Arr
			| Range::Filter   ([](int Value) { return Value < 3; })
			| Range::Transform([](int Value) { return Value * 2; })
			| Range::To<TArray<int>>();

		TArray<int> Drr = Arr
			| Range::Transform([](int Value) { return Value * 2; })
			| Range::Filter   ([](int Value) { return Value < 6; })
			| Range::To<TArray<int>>();

		always_check(Brr == Crr);
		always_check(Brr == Drr);
	}

	{
		TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7 };

		TArray<int> Brr = Range::Iota(0)
			| Range::Take(8)
			| Range::To<TArray<int>>();

		TArray<int> Crr = Range::Iota(0)
			| Range::TakeWhile([](int Value) { return Value < 8; })
			| Range::To<TArray<int>>();

		always_check(Arr == Brr);
		always_check(Arr == Crr);
	}

	{
		TArray<int> Arr = { 0, 4, 7, 8, 3, 1, 10 };
		TArray<int> Brr = { 0, 2, 4 };

		TArray<int> Crr = Arr
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::Take(3)
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::To<TArray<int>>();

		TArray<int> Drr = Arr
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::TakeWhile([](int Value) { return Value < 10;     })
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::To<TArray<int>>();

		TArray<int> Err = Arr
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::Take(3)
			| Range::To<TArray<int>>();

		TArray<int> Frr = Arr
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::TakeWhile([](int Value) { return Value < 5;      })
			| Range::To<TArray<int>>();

		TArray<int> Grr = Arr
			| Range::Take(6)
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::To<TArray<int>>();

		TArray<int> Hrr = Arr
			| Range::TakeWhile([](int Value) { return Value < 10;     })
			| Range::Filter   ([](int Value) { return Value % 2 == 0; })
			| Range::Transform([](int Value) { return Value / 2;      })
			| Range::To<TArray<int>>();

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
