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
	TestArrayTemplate<FDefaultAllocator,    0>();
	TestArrayTemplate<FHeapAllocator,       0>();
	TestArrayTemplate<TInlineAllocator<8>,  8>();
	TestArrayTemplate<TFixedAllocator<64>, 64>();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
