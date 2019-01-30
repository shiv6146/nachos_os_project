#include "syscall.h"


int main() {
	PutString("Please enter a character from A-Z \n");
	char c = GetChar();
	PutString("Your character is : ");
	PutChar(c);
	PutChar('\n');
}
