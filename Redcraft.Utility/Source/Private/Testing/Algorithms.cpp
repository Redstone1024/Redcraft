#include "Testing/Testing.h"

#include "Algorithms/Algorithms.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

NAMESPACE_PRIVATE_BEGIN

void TestBasic()
{
	TArray<int> Arr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	auto Iter = Arr.Begin();

	Algorithms::Advance(Iter, 5);

	always_check(*Iter == 5);

	always_check(Algorithms::Distance(Arr.Begin(), Iter) == 5);

	always_check(Algorithms::Distance(Arr) == 10);

	always_check(*Algorithms::Next(Iter, 2) == 7);
	always_check(*Algorithms::Prev(Iter, 2) == 3);
}

NAMESPACE_PRIVATE_END

void TestAlgorithms()
{
	NAMESPACE_PRIVATE::TestBasic();
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
