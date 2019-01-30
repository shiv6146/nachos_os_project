#include "frameprovider.h"
#include "sysdep.h"
#include "synch.h"

static Semaphore *pageSem = new Semaphore("Page Semaphore", 1);

FrameProvider::FrameProvider(int numPages) {
    bitMap = new BitMap(numPages);
    size = numPages;
}

FrameProvider::~FrameProvider() {
    delete bitMap;
}

// Returns the frame offset address if its able to find a free frame else returns NULL
int FrameProvider::GetEmptyFrame() {
    
    pageSem->P();
    if (bitMap->NumClear() <= 0) {
        pageSem->V();
        return -1;
    }

    int page = bitMap->Find();
    pageSem->V();
    return page;
}

// Unsets the particluar frame in the bitmap so that its available for other allocations
void FrameProvider::ReleaseFrame(int pageNum) {
    if (!bitMap->Test(pageNum)) {
        printf("[ERROR] Invalid Page number!\n");
        return;
    }
    bitMap->Clear(pageNum);
}

// Returns the num of available frames from bitmap
int FrameProvider::NumAvailFrame() {
    return bitMap->NumClear();
}