#include "segvCatch.h"

void SegFaultFunc()
{
	int *p = NULL;
	*p = 123;
}
void Func3()
{
	SegFaultFunc();
}
void Func2()
{
	Func3();
}
void Func1()
{
	Func2();
}
int main()
{
	initSegvCatch();
	Func1();
	return 0;
}
