#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"

class SynchConsole {
    public:
        // initialize the hardware console device
        SynchConsole(char *readFile, char *writeFile);

        // clean up console emulation
        ~SynchConsole();
        
        // Unix putchar(3S)
        void SynchPutChar(const char ch);

        // Unix getchar(3S)
        int SynchGetChar();
        
        // Unix puts(3S)
        void SynchPutString(const char *s);

        // Unix gets(3S)
        void SynchGetString(char *s, int n);

        void SynchPutInt(int i);
        void SynchGetInt(int *i);

    private:
        Console *console;
        Semaphore *mutex;
};

#endif // SYNCHCONSOLE_H