// tcc_stubs.h

#include <stdlib.h> // For NULL, size_t
#include <string.h> // For strcpy
#include <stdint.h> // For strcpy

#define TCC_IS_NATIVE
#include "libtcc.h" // for TCCState

// Define a simple realpath stub
// On an embedded system, this will likely always return the input path
// or a simplified version, as there's no real filesystem to resolve.
// It should allocate memory like realpath() usually does.
char *realpath(const char *path, char *resolved_path);

// Define a simple getcwd stub
// On an embedded system, there's no "current working directory".
// Return a fixed dummy path.
char *getcwd(char *buf, size_t size);

// // Dummy stub, doing nothing.
// void tcc_run_free(TCCState *s1);

// Dummy stub, doing nothing.
int mprotect(void *addr, size_t len, int prot);

// numworks_tcc_heap.h
#include <stddef.h> // For size_t

// The TCC heap buffer (defined in tcc_stubs.c file)
extern uint8_t tcc_heap_buffer[];
extern size_t tcc_heap_current_offset;
extern const size_t TCC_HEAP_SIZE; // Make this visible too

void tcc_numworks_heap_init() ;
void *numworks_tcc_malloc(size_t size) ;
void *numworks_tcc_realloc(void *ptr, size_t size) ;
void numworks_tcc_free(void *ptr) ;
