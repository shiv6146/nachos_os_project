// addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include "synch.h"

#define UserStackSize	 8192	// increase this as necessary! (dependent on the PageSize! Need to think about increasing it more than the PageSize)
#define NumThreadPages 4
// This gives the number of thread stacks that can be allocated for a given page size
// Doing this to avoid putting a const hard limit on num of threads created by a userprog
#define MAX_USER_THREADS divRoundUp(UserStackSize, PageSize)

class AddrSpace
{
  public:
    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

    Semaphore *SemForJoins[MAX_USER_THREADS];
    bool IsStackFree();
    bool IsStackFull();
    int UserStackAllocate();
    void RevokeStack(int num);
    int GetStack(int val);
    void IsLastThread();
    bool Test(int bitVal);

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch 

  private:
      TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!
      unsigned int numPages;	// Number of pages in the virtual 
    // address space
      BitMap *stack;
      bool isEnd;
      bool isOverflow;
      Semaphore *blockThread;
      Semaphore *tsem;
      int numThreads;
};

#endif // ADDRSPACE_H
