#ifndef USERTHREAD_H
#define USERTHREAD_H

#include "thread.h"

typedef struct threadFun_t {
    int f;
    int arg;
} threadFun;

extern int do_UserThreadCreate(int f, int arg);
extern int do_UserThreadExit();
extern void StartUserThread(int f);
extern int UserThreadJoin(int tid);

#endif