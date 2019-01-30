#include "syscall.h"

int main(){
	PutString("Enter a integer of your choice for Printing its Table:");
	int n = GetInt();
	for (int i = 1; i<=10; i++)
	{
		int j = n*i;
		PutInt(j);
		PutChar('\n');
	}
}