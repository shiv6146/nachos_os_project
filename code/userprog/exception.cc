// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "userthread.h"
#include "sysdep.h"
#include "forkexec.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}

// From MIPS machine to Linux mode
void copyStringFromMachine(int from, char *to, int size) {
	int i;
	for (i = 0; i < size; i++) {
		machine->ReadMem(from+i, 1, (int*)(to+i));

		if (*(to+i) == '\0') return; // if str end is reached then return
	}
	to[size - 1] = '\0'; // mark the end of str
}

// From Linux mode to MIPS machine
void copyStringToMachine(char *from, int to, int size) {
	int i;
	for (i = 0; i < size; i++) {
		machine->WriteMem(to+i, 1, (int)from[i]);
	}
}

//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler (ExceptionType which) {
	int type = machine->ReadRegister (2);

	if (which == SyscallException) {
		switch(type) {
			case SC_Exit:
			{
				currentThread->space->IsLastThread();
				MajNbProc(-1);
				delete currentThread->space;
				if (GetNbProc() >= 0) {
					currentThread->Finish();
				} else {
					interrupt->Halt();
				}
			}
			break;
			case SC_Halt:
			{
				DEBUG('a', "Shutdown, initiated by user program. \n");
				currentThread->space->IsLastThread();
				delete currentThread->space;
				interrupt->Halt();
			}
			break;
			case SC_PutChar:
			{
				DEBUG('a', "PutChar called by user program\n");
				synchconsole->SynchPutChar((char)machine->ReadRegister(4)); // Input args is read from register r4
			}
			break;
			case SC_GetChar:
			{
				DEBUG('a', "GetChar called by user program\n");
				machine->WriteRegister(2, (int)synchconsole->SynchGetChar()); // Output/return val is written to register r2
			}
			break;
			case SC_PutString:
			{
				DEBUG('a', "PutString called by user program\n");
				int from = machine->ReadRegister(4);
				char buffer[MAX_STRING_SIZE];
				copyStringFromMachine(from, buffer, MAX_STRING_SIZE); // From MIPS machine into Linux mode
				synchconsole->SynchPutString(buffer);
			}
			break;
			case SC_GetString:
			{
				DEBUG('a', "GetString called by user program\n");
				int from = machine->ReadRegister(4);
				int size = machine->ReadRegister(5); // Input arg2 is read from register r5
				char buffer[MAX_STRING_SIZE];
				synchconsole->SynchGetString(buffer, size);
				copyStringToMachine(buffer, from, size); // From Linux mode into MIPS machine
			}
			break;
			case SC_PutInt:
			{
				int n = machine->ReadRegister(4);
				synchconsole->SynchPutInt(n);
			}
			break;
			case SC_GetInt:
			{
				int n;
				synchconsole->SynchGetInt(&n);
				machine->WriteRegister(2, n);
			}
			break;
			case SC_UserThreadCreate:
			{
				DEBUG('a', "UserThreadCreate called by user program\n");
				// Read the syscall params
				int f = machine->ReadRegister(4);
				int arg = machine->ReadRegister(5);

				// Create, schedule and return thread id of the new thread
				do_UserThreadCreate(f, arg);
			}
			break;
			case SC_UserThreadExit:
			{
				DEBUG('a', "UserThreadExit called by user program\n");
				do_UserThreadExit();
			}
			break;
			case SC_UserThreadJoin:
			{
				DEBUG('a', "UserThreadJoin called by user program\n");
				int tid = machine->ReadRegister(4);
				UserThreadJoin(tid);
			}
			break;
			case SC_ForkExec:
			{
				DEBUG('a', "ForkExec called by user program\n");
				char *buffer = new char[MAX_STRING_SIZE];
				int s = machine->ReadRegister(4);
				copyStringFromMachine(s, buffer, MAX_STRING_SIZE);
				int res = ForkExec(buffer);
				machine->WriteRegister(2, res);
				delete buffer;
			}
			break;
			default:
				printf("Unexpected user mode exception %d %d\n", which, type);
				ASSERT(FALSE);
		}
	}

	UpdatePC();
}




