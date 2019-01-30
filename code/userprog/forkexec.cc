#include "system.h"
#include "addrspace.h"

static void StartForkExec(int arg) {
    currentThread->space->InitRegisters();	// set the initial register values
    currentThread->space->RestoreState();	// load page table register
    machine->Run();		// jump to the user progam
}

int ForkExec(char *filename) {

    AddrSpace *space;
    OpenFile *executable = fileSystem->Open(filename);

    if (executable == NULL) {
        fprintf (stderr, "Unable to open file %s\n", filename);
        return -1;
    }

    Thread *t = new Thread("UserProcess");

    if (t == NULL) {
        printf("[ERROR] Failed to create a new user thread!\n");
        return -1;
    }

    MajNbProc(1);

    space = new AddrSpace (executable);
    t->space = space;

    delete executable;

    if (space->IsStackFull()) {
        printf("[ERROR] User stack is full!\n");
        delete space;
        delete t;
        MajNbProc(-1);
        return -1;
    }

    t->ForkExec(StartForkExec, 0);

    currentThread->Yield();

    return 0;
}