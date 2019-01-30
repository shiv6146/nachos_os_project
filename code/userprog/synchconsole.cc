#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { readAvail->V(); }

static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile) {
	readAvail = new Semaphore("read avail", 0);
	writeDone = new Semaphore("write done", 0);
	console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
	mutex = new Semaphore("mutex for synchconsole", 1);
}

SynchConsole::~SynchConsole() {
	delete console;
	delete writeDone;
	delete readAvail;
}

void SynchConsole::SynchPutChar(const char ch) {
	mutex->P();
	console->PutChar(ch);
	writeDone->P();
	mutex->V();
}


int SynchConsole::SynchGetChar() {
	readAvail->P();
	if (console->feof()) {
		return EOF;
	} else {
		return (int)console->GetChar();
	}
}

void SynchConsole::SynchPutString(const char s[]) {
	int i;
	for (i = 0; s[i] != '\0'; i++) {
		SynchPutChar(s[i]);
	}
}

void SynchConsole::SynchGetString(char *s, int n) {
	mutex->P();
	int i;
	for (i = 0; i < n - 1; i++) {
		char c = SynchGetChar();
		if (c == EOF) break;
		s[i] = c;
		if (c == '\n') {
			i++;
			break;
		}
	}
	s[i] = '\0'; // Terminate the string with null char
	mutex->V();
}

void SynchConsole::SynchPutInt(int n) {
	char buf[MAX_STRING_SIZE];
	snprintf(buf, MAX_STRING_SIZE, "%d", n); // Read buf upto n chars
	SynchPutString(buf); // print buf upto n chars to console
}

void SynchConsole::SynchGetInt(int *n) {
	char buf[MAX_STRING_SIZE];
	SynchGetString(buf, MAX_STRING_SIZE);
	*n = 0;
	ASSERT(sscanf(buf, "%d", n) != EOF); // fail if we receive EOF as input
}