#ifndef __TinyLine_H
#define __TinyLine_H

typedef void(*TinyLine_InputHandler)(void);
void TinyLine_setup(void);
char* TinyLine_readLine(void);
void TinyLine_dispatch(const char* command, TinyLine_InputHandler handler);

// You can redefine this macro to use your
// implementation of any function that aborts
// the program.
#ifndef FATAL_ERROR
#define FATAL_ERROR(message) fatalError(message);
#endif

#endif
