// addrspace.cc 
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option 
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

#include <strings.h>		/* for bzero */

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader * noffH)
{
    noffH->noffMagic = WordToHost (noffH->noffMagic);
    noffH->code.size = WordToHost (noffH->code.size);
    noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
    noffH->initData.size = WordToHost (noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost (noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
	WordToHost (noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}

static void ReadAtVirtual(OpenFile *executable, int virtualaddr, int numBytes, int position, TranslationEntry *pageTable, unsigned numPages) {

    if ((numBytes <= 0) ||  (virtualaddr < 0) || ((unsigned)virtualaddr > numPages * PageSize)) {
		printf("[ERROR] ReadAtVirtual address invalid!\n");
		return;
	}

	int numPages_old = machine->pageTableSize;
	TranslationEntry *page_old = machine->pageTable;

	char *into = new char[numBytes+(numBytes%4)];
	executable->ReadAt(into,numBytes,position);
	int i;

	machine->pageTable = pageTable ;
	machine->pageTableSize = numPages ;
	
	for (i=0; i < numBytes; i+=4) {
		machine->WriteMem((int)(virtualaddr+i), 4, (int)(*(int *)(into+i)));
	}

	machine->pageTable = page_old ;
	machine->pageTableSize = numPages_old ;

	delete [] into;
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical 
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable)
{
    NoffHeader noffH;
    unsigned int i, size;
    int numPagesOld;
    TranslationEntry *oldPage;

    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
	(WordToHost (noffH.noffMagic) == NOFFMAGIC))
	SwapHeader (&noffH);
    ASSERT (noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp (size, PageSize);
    size = numPages * PageSize;

    ASSERT (numPages <= NumPhysPages);	// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG ('a', "Initializing address space, num pages %d, size %d\n",
	   numPages, size);

    // Check if a new frame can be allocated by the frameprovider
    isOverflow = (unsigned) frameProvider->NumAvailFrame() < numPages;
    if (isOverflow) return;

    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
      {
	  pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	  pageTable[i].physicalPage = frameProvider->GetEmptyFrame();
	  pageTable[i].valid = TRUE;
	  pageTable[i].use = FALSE;
	  pageTable[i].dirty = FALSE;
	  pageTable[i].readOnly = FALSE;	// if the code segment was entirely on 
	  // a separate page, we could set its 
	  // pages to be read-only
      }

    
    for (i = 0; i < MAX_USER_THREADS; i++) {
        this->SemForJoins[i] = new Semaphore("ThreadSemJoin", 1);
    }

    oldPage = machine->pageTable;
    numPagesOld = machine->pageTableSize;

    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

    // Zero out the addrspace
    for (i = 0; i < numPages * PageSize; i += 4) {
        machine->WriteMem(i, 4, 0);
    }

    machine->pageTable = oldPage;
    machine->pageTableSize = numPagesOld;

    stack = new BitMap(MAX_USER_THREADS);
    for (i = 0; i < NumThreadPages; i++) {
        stack->Mark(i);
    }

    isEnd = false;
    blockThread = new Semaphore("BlockMultipleThreads", 0);
    tsem = new Semaphore("ThreadSemaphore", 1);
    numThreads = 0;

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0)
      {
	  DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
		 noffH.code.virtualAddr, noffH.code.size);
	//   executable->ReadAt (&(machine->mainMemory[noffH.code.virtualAddr]),
	// 		      noffH.code.size, noffH.code.inFileAddr);
        ReadAtVirtual(executable, noffH.code.virtualAddr, noffH.code.size, noffH.code.inFileAddr, pageTable, numPages);
      }
    if (noffH.initData.size > 0)
      {
	  DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
		 noffH.initData.virtualAddr, noffH.initData.size);
	//   executable->ReadAt (&
	// 		      (machine->mainMemory
	// 		       [noffH.initData.virtualAddr]),
	// 		      noffH.initData.size, noffH.initData.inFileAddr);
        ReadAtVirtual(executable, noffH.initData.virtualAddr, noffH.initData.size, noffH.initData.inFileAddr, pageTable, numPages);
      }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace ()
{
  // LB: Missing [] for delete
  // delete pageTable;
  if (!isOverflow) {
      for (unsigned i = 0; i < MAX_USER_THREADS; i++) {
          delete this->SemForJoins[i];
      }
      for (unsigned i = 0; i < isOverflow; i++) {
          frameProvider->ReleaseFrame(pageTable[i].physicalPage);
      }
      delete stack;
      delete []pageTable;
  }
  // End of modification
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters ()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister (i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister (PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister (NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister (StackReg, numPages * PageSize - 16);
    DEBUG ('a', "Initializing stack register to %d\n",
	   numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void
AddrSpace::SaveState ()
{
    // SHIVA: Now we have more than 1 execution context (multiple threads) so we are saving the current execution context
    this->pageTable = machine->pageTable;
    this->numPages = machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState ()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

bool AddrSpace::IsStackFree() {
	return (stack->NumClear()) > 0;
}

bool AddrSpace::IsStackFull() {
	return isOverflow;
}

int AddrSpace::UserStackAllocate() {
	tsem->P();
	
    // By default we allocate 4 pages for each thread
	if ((stack->NumClear()) <= (NumThreadPages - 1)) {
		printf("Unable to allocate stack\n");
		return -1;
	}

	int tmp = stack->Find();
	for(int i=1; i < NumThreadPages; i++) {
		if(stack->Test(tmp + i)) {
			printf("Pages block(%d) is not available\n", NumThreadPages);
			return -1;
		}
		stack->Mark(tmp + i);
	}
	numThreads++;
	tsem->V();
	return tmp;
}

void AddrSpace::RevokeStack (int numStack) {
	tsem->P();

	for (int i=0; i < NumThreadPages; i++) {
		if (!stack->Test(numStack + i)) {
			printf("[ERROR] numStack %d\n", numStack);
			return;
		}
		stack->Clear(numStack + i);
	}

	numThreads--;
	if(isEnd && numThreads == 0) {
		blockThread->V();
	}

	tsem->V();
}


int AddrSpace::GetStack(int BitmapValue) {
	return PageSize * numPages - BitmapValue * PageSize;
}

// Block Halt() when there are other alive threads
void AddrSpace::IsLastThread() {
	if(numThreads != 0) {
		isEnd = true;
		blockThread->P();
		isEnd = false;
	}
}

bool AddrSpace::Test(int BitmapValue) {
	return this->stack->Test(BitmapValue);
}