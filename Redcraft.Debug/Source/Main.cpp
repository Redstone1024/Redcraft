#include "CoreMinimal.h"
#include "HAL/Memory.h"

#include <iostream>
#include <cassert>
#include <new>

NS_STD_USING
NS_REDCRAFT_USING

struct FTest
{
	FTest() { cout << "FTest()" << endl; }
	~FTest() { cout << "~FTest()" << endl; }
	FTest(int32) { cout << "FTest(int32)" << endl; }
	FTest(const FTest&) { cout << "FTest(const FTest&)" << endl; }
	FTest(FTest&&) { cout << "FTest(FTest&&)" << endl; }
	FTest& operator =(const FTest&) { cout << "FTest& operator =(const FTest&)" << endl; return *this; }
	FTest& operator =(FTest&&) { cout << "FTest& operator =(FTest&&)" << endl;  return *this; }
	friend bool operator ==(const FTest&, const FTest&) { cout << "bool operator ==(const FTest&, const FTest&)" << endl;  return true; }
};

int main()
{
	FTest* A = new FTest[2];
	FTest* B = new FTest[2];
	int32* C = new int32[2];
	int32* D = new int32[2];

	cout << " --- " << endl;

	Memory::DefaultConstructItems<FTest>(A, 2);
	Memory::DestructItems<FTest>(A, 2);
	Memory::ConstructItems<FTest>(A, C, 2);
	Memory::CopyAssignItems(B, A, 2);
	Memory::RelocateConstructItems<FTest>(A, C, 2);
	Memory::MoveConstructItems(B, A, 2);
	Memory::MoveAssignItems(B, A, 2);
	cout << (Memory::CompareItems(A, B, 2) ? "True" : "False") << endl;

	Memory::DefaultConstructItems<int32>(C, 2);
	Memory::DestructItems<int32>(C, 2);
	Memory::ConstructItems<int32>(C, D, 2);
	Memory::CopyAssignItems(D, C, 2);
	Memory::RelocateConstructItems<int32>(D, C, 2);
	Memory::MoveConstructItems(D, C, 2);
	Memory::MoveAssignItems(D, C, 2);
	cout << (Memory::CompareItems(C, D, 2) ? "True" : "False") << endl;

	cout << " --- " << endl;

	delete[] A;
	delete[] B;
	delete[] C;
	delete[] D;

	cout << "Done!" << endl;
	return 0;
}
