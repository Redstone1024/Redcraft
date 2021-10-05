#include "CoreMinimal.h"
#include "HAL/Memory.h"

#include <iostream>
#include <cassert>
#include <new>

NS_STD_USING
NS_REDCRAFT_USING

int main()
{
	check_no_entry();
	cout << "Done!" << endl;
	return 0;
}
