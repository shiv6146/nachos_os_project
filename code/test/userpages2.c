#include "syscall.h"
#define THIS "xxx userprog1"
#define THAT "yyy userprog1"

 const int N  = 10;



 void f(void *ss) {
	void puts(char* s) {
		char* p;
		for(p=s; *p != '\0'; p++)
			PutChar(*p);
	}

 	int i;
	for(i=0; i<N; i++) {
		puts((char*) ss);
		PutChar('\n');
	}
}

 int main ()
{
	UserThreadCreate(f, (void *) THIS);
	f((void*) THAT);
	return 0;
}