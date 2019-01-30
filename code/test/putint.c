#include "syscall.h"


int main(){
	PutString("Natural numbers from 1 to 10: \n");
	for(int i = 1;i<=10;i++)
	{
		PutInt(i);
		PutChar('\n');
	}
}