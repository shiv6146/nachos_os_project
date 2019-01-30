#include "syscall.h"

void foobar(void *param) {
    int arg = *((int*) param);
    switch(arg) {
        case 1:
        {
            PutChar('S');
            PutChar('H');
            PutChar('I');
            PutChar('V');
            PutChar('A');
            break;
        }
        case 2:
        {
            PutChar('K');
            PutChar('A');
            PutChar('J');
            PutChar('A');
            PutChar('L');
            break;
        }
        case 3:
        {
            PutChar('V');
            PutChar('A');
            PutChar('R');
            PutChar('S');
            PutChar('H');
            PutChar('A');
            break;
        }
    }
}


int main() {
    int tmp = 1;

    int tid1 = UserThreadCreate(foobar, (void*) &tmp);
    if (tid1 < 0) PutString("Error creating new thread !\n");

    tmp = 2;
    int tid2 = UserThreadCreate(foobar, (void*) &tmp);
    if (tid2 < 0) PutString("Error creating new thread !\n");

    tmp = 3;
    int tid3 = UserThreadCreate(foobar, (void*) &tmp);
    if (tid3 < 0) PutString("Error creating new thread !\n");

    UserThreadJoin(tid1);
    UserThreadJoin(tid2);
    UserThreadJoin(tid3);

}
