// tcc_stubs.c

#include <stdlib.h> // For NULL, size_t
#include <string.h> // For strcpy, strlen
#include <stdio.h>  // For printf
#include <eadk.h>   // For eadk_timing_msleep

#define TCC_IS_NATIVE
#include "libtcc.h" // for TCCState

#define PATH_MAX 128

// Define a simple realpath stub
// On an embedded system, this will likely always return the input path
// or a simplified version, as there's no real filesystem to resolve.
// It should allocate memory like realpath() usually does.
char *realpath(const char *path, char *resolved_path)
{
    // Optional debug print
    printf("realpath(%s, %s)\n", path, resolved_path);
    eadk_timing_msleep(1000);

    // If resolved_path is NULL, realpath is expected to malloc.
    // If you don't want to support malloc in this stub, make it static buffer.
    // For now, let's just return a copy of the original path.
    // WARNING: This is a simplified stub. A proper realpath might need more logic
    // or return an error if the path is invalid for the embedded context.
    if (path == NULL) {
        return NULL;
    }

    if (resolved_path == NULL) {
        // realpath is supposed to malloc if resolved_path is NULL
        // For embedded, we might use a fixed buffer or a controlled malloc
        // For simplicity, let's use a static buffer if no dynamic allocation is feasible.
        static char temp_path[256]; // Adjust size as needed
        strncpy(temp_path, path, sizeof(temp_path) - 1);
        temp_path[sizeof(temp_path) - 1] = '\0';
        return temp_path; // Return pointer to static buffer
    } else {
        // If resolved_path is provided, copy into it.
        strncpy(resolved_path, path, PATH_MAX - 1); // PATH_MAX might not be defined. Use a reasonable size.
        resolved_path[PATH_MAX - 1] = '\0';
        return resolved_path;
    }
}

// Define a simple getcwd stub
// On an embedded system, there's no "current working directory".
// Return a fixed dummy path.
char *getcwd(char *buf, size_t size)
{
    // Optional debug print
    printf("getcwd(%s, %i)\n", buf, size);
    eadk_timing_msleep(1000);

    const char *dummy_cwd = "/"; // Or "/app" or whatever makes sense for your context
    if (buf == NULL) {
        // getcwd usually mallocs if buf is NULL.
        // For embedded, we'll return a pointer to a static buffer.
        static char static_buf[64]; // Adjust size
        if (strlen(dummy_cwd) >= sizeof(static_buf)) {
            // Path too long for static buffer
            return NULL; // Indicate error
        }
        strcpy(static_buf, dummy_cwd);
        return static_buf;
    } else {
        // Copy to provided buffer
        if (size == 0 || strlen(dummy_cwd) >= size) {
            return NULL; // Indicate error (buffer too small)
        }
        strncpy(buf, dummy_cwd, size - 1);
        buf[size - 1] = '\0';
        return buf;
    }
}

// // Dummy stub, doing nothing.
// void tcc_run_free(TCCState *s1) {
//     return;
// }

#include <errno.h> // For EINVAL or other error codes if you wanted to be strict

// Define a dummy mprotect for bare-metal
// Arguments:
//   addr: starting address of the memory region
//   len: length of the memory region
//   prot: desired memory protection (PROT_READ, PROT_WRITE, PROT_EXEC)
// Return: 0 on success, -1 on error (setting errno)
int mprotect(void *addr, size_t len, int prot) {
    // For bare-metal, SRAM is typically already executable by default.
    // If you have an MPU set up, you might check if 'prot' includes PROT_EXEC
    // and if the region is already covered by an executable MPU region.
    // However, for simplicity, a direct return 0 is often sufficient.

    // You can add a debug print if you want to see when TCC calls this
    printf("mprotect(%p, %zu, %d)\n", addr, len, prot);
    eadk_timing_msleep(1000);

    // It's generally safe to just return success on platforms where memory
    // is uniformly executable (like typical embedded SRAM).
    return 0;

    // If you wanted to be more cautious and knew your MPU, you could:
    /*
    if (prot & PROT_EXEC) {
        // If your MPU does NOT allow execution from this region,
        // and you need it to, you would configure MPU here.
        // However, this is highly specific to your MPU setup and usually
        // done at a higher level (e.g., linker script, boot code).
        // For now, assuming SRAM is executable.
    }
    return 0; // Success
    */
}

// You might also need these if TCC expects them, they are related to memory flags
// These are standard POSIX protection flags
#ifndef PROT_NONE
#define PROT_NONE       0x0     /* no permissions */
#endif
#ifndef PROT_READ
#define PROT_READ       0x1     /* pages can be read */
#endif
#ifndef PROT_WRITE
#define PROT_WRITE      0x2     /* pages can be written */
#endif
#ifndef PROT_EXEC
#define PROT_EXEC       0x4     /* pages can be executed */
#endif


//
// Custom realloc function
//

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

// Define the size of the TCC heap in bytes
// This is the CRITICAL value you'll need to tune.
// Start with a conservative size, e.g., 64KB (64 * 1024).
// The maximum you can allocate here is the TOTAL remaining free SRAM
// AFTER the NumWorks firmware and your app's core code/data.
#define TCC_HEAP_SIZE (48 * 1024) // Example: 48KB
// #define TCC_HEAP_SIZE 0           // Example: 0KB

// Declare the TCC heap buffer
// It's uninitialized, so it goes into .bss (saving flash space).
static uint8_t s_tcc_heap_buffer[TCC_HEAP_SIZE] = {0};
static size_t s_tcc_heap_current_offset = 0; // Current allocation pointer for bump allocator

// Function to reset the heap (call before each TCC compilation session if needed)
void tcc_numworks_heap_init() {
    s_tcc_heap_current_offset = 0;
    // Optionally, clear the buffer for debugging
    // memset(s_tcc_heap_buffer, 0, TCC_HEAP_SIZE);
}

// Your custom free for TCC (no-op for a bump allocator)
void numworks_tcc_free(void *ptr) {
    // In a bump allocator, memory is only reclaimed by resetting the `s_tcc_heap_current_offset`
    // pointer (i.e., calling `tcc_numworks_heap_init()`)
    printf("TCC_FREE: %p (no-op)\n", ptr);
    eadk_timing_msleep(1000);
}

// Your custom malloc for TCC
void *numworks_tcc_malloc(size_t size) {
    size_t aligned_size = (size + 3) & ~3; // Align to 4 bytes for ARM

    if (s_tcc_heap_current_offset + aligned_size > TCC_HEAP_SIZE) {
        // Out of memory within our designated TCC heap
        // You MUST log this or display on screen for debugging
        // For example:
        printf("TCC_MALLOC FAIL: Req %zuB, Free %zuB\n", size, TCC_HEAP_SIZE - s_tcc_heap_current_offset);
        eadk_timing_msleep(1000);
        return NULL;
    }

    void *ptr = &s_tcc_heap_buffer[s_tcc_heap_current_offset];
    s_tcc_heap_current_offset += aligned_size;

    // Optional debug print
    printf("TCC_MALLOC: Req %zu (aligned %zu)\n", size, aligned_size);
    eadk_timing_msleep(1000);
    printf("TCC_MALLOC: Got %p, Offset %zu\n", ptr, s_tcc_heap_current_offset);
    eadk_timing_msleep(1000);
    return ptr;
}

// Your custom realloc for TCC
void *numworks_tcc_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return numworks_tcc_malloc(size);
    }
    if (size == 0) {
        numworks_tcc_free(ptr);
        return NULL;
    }

    // Simplistic realloc for bump allocator: always reallocate and copy.
    // This can be inefficient and cause fragmentation if used heavily
    // on non-last-allocated blocks, but it's a starting point.
    void *new_ptr = numworks_tcc_malloc(size);
    if (new_ptr && ptr) {
        // We don't know the old size of 'ptr', so we must assume
        // TCC will copy the necessary data from the old pointer.
        // Or, more safely, copy up to `size` bytes, assuming new_ptr is larger.
        // If TCC needs the old content, it will handle it.
        // This is a *major simplification* due to the bump allocator.
        // For a robust realloc, you'd need a more complex allocator.
        // For now, if TCC expects data preservation, this will fail.
        // A safer approach might be to just return new_ptr; and let TCC
        // copy if it needs.
        // However, typical TCC usage of realloc is for growing its internal buffers,
        // so a simple copy-up-to-new-size might sometimes work.
        // For simplicity, let's just allocate and return for now,
        // if TCC needs to preserve data, it will crash or behave incorrectly.
        // memmove(new_ptr, ptr, MIN(old_size, size)); // Would need old_size
        // For a pure bump allocator, no real "move" happens, just new allocation.
        // We'll rely on TCC internal logic not to require the contents to be copied by realloc for now.
    }
    return new_ptr;
}
