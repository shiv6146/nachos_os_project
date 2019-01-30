#include "userthread.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

static Semaphore *wait = new Semaphore("Thread wait", 0);
static Semaphore *isThreadExist = new Semaphore("ThreadExisting", 1);

// Init registers similar to Machine::InitRegisters and then run Machine::Run
// Also save and restore current state before starting to run new context
void StartUserThread(int fun) {
    threadFun *tfun = (threadFun*) fun;

    currentThread->space->SaveState();
    currentThread->space->InitRegisters();

    machine->WriteRegister(PCReg, tfun->f);
    machine->WriteRegister(NextPCReg, (tfun->f) + 4);
    machine->WriteRegister(4, tfun->arg);

    int allocStack = currentThread->space->UserStackAllocate();

    machine->WriteRegister(StackReg, currentThread->space->GetStack(allocStack));

    currentThread->initStackReg = allocStack;

    // Increment the wait threads semaphore as new thread has been created
    wait->V();
    
    machine->Run();
}

int do_UserThreadCreate(int f, int arg) {
    if (!currentThread->space->IsStackFree()) {
        printf("[ERROR] UserThread stack FULL!\n");
        return -1;
    }

    Thread *newThread = new Thread("User Thread");

    if (newThread == NULL) {
        printf("[ERROR] Failed to create UserThread!\n");
        return -1;
    }

    isThreadExist->P();

    threadFun *fn = new threadFun;

    fn->f = f;
    fn->arg = arg;

    newThread->Fork(StartUserThread, (int)fn);

    wait->P();

    currentThread->space->SemForJoins[newThread->initStackReg]->P();

    machine->WriteRegister(2, newThread->initStackReg);

    isThreadExist->V();

    currentThread->Yield();

    return 0;
}

// Thread will call the finish method to exit
int do_UserThreadExit() {

    if (currentThread->initStackReg == 0) return 0;

    currentThread->space->SemForJoins[currentThread->initStackReg]->V();

    // Release any dependent thread to the current thread
    if (currentThread->dependentTID != -1) {
        currentThread->space->SemForJoins[currentThread->dependentTID]->V();
    }

    // De-allocate the stack which has been allocated for the current thread
    currentThread->space->RevokeStack(currentThread->initStackReg);
    currentThread->Finish();

    return 0;
}

int UserThreadJoin(int tid) {

    // Current thread is already waiting on another thread
    // We prevent multiple calls to join by the same thread
    if (currentThread->dependentTID != -1) {
        printf("[ERROR] UserThread is dependent already on another thread!\n");
        return -1;
    }

    // We prevent thread calling join to itself
    if (currentThread->initStackReg == tid || tid == 0) {
        printf("[ERROR] UserThread trying to call JOIN on an invalid thread!\n");
        return -1;
    }

    // Check if the passed tid already has an allocated stack
    isThreadExist->P();

    if (!currentThread->space->Test(tid)) {
        printf("[ERROR] UserThread calling JOIN on a non existing thread!\n");
        isThreadExist->V();
        return -1;
    }

    isThreadExist->V();

    // Set the passed tid as current thread's dependent
    currentThread->dependentTID = tid;
    // Wait for the completion of passed tid
    currentThread->space->SemForJoins[tid]->P();
    return 0;
}