#include "syscall.h"

int main() {
	PutString("Enter a string of characters from A-Z. Maximum length allowed is : 20 \n");
	char str[20];
	GetString(str,20);
	PutString("The string entered by the user is \n");
	PutString(str);
	PutChar('\n');
}
