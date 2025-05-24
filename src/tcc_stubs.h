// tcc_stubs.h

#include <stdlib.h> // For NULL, size_t
#include <string.h> // For strcpy
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

// Dummy stub, doing nothing.
void tcc_run_free(TCCState *s1);
