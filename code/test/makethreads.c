#include "syscall.h"

void thread(void *i){
	PutString("Thread with param: ");
	PutInt(*(int*)i);
	PutChar('\n');
	UserThreadExit();
}

int main(){
	int param[12];
	for (int i = 0; i < 12; i++) {
		param[i] = i + 1;
	}
	for (int i = 0; i < 12; i++) {
		UserThreadCreate(thread,(void *)(param[i]));
	}
	Exit(0);
}

