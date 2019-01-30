#ifndef USERPROG_FRAMEPROVIDER_H_
#define USERPROG_FRAMEPROVIDER_H_

#include "bitmap.h"

class FrameProvider {
    public:
        FrameProvider(int numPages);    // Constructor
        ~FrameProvider();   // Destructor
        int GetEmptyFrame();    // to retrieve a free frame which is available
        void ReleaseFrame(int pageNum); // to release an allocated frame
        int NumAvailFrame(); // to get the num of available frames for allocation
    
    private:
        BitMap *bitMap;
        int size;
};

#endif /* USERPROG_FRAMEPROVIDER_H_ */