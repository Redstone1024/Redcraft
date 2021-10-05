#include "CoreMinimal.h"
#include "HAL/Memory.h"

#include <iostream>
#include <cassert>
#include <new>

NS_STD_USING
NS_REDCRAFT_USING

int main()
{
	int32* Ptr = new int32;
	*Ptr = 13;
	cout << *Ptr << endl;
	delete Ptr;
	return 0;
}
