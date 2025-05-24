// tcc_stubs.c

#include <stdlib.h> // For NULL, size_t
#include <string.h> // For strcpy

#define PATH_MAX 128

// Define a simple realpath stub
// On an embedded system, this will likely always return the input path
// or a simplified version, as there's no real filesystem to resolve.
// It should allocate memory like realpath() usually does.
char *realpath(const char *path, char *resolved_path) {
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
char *getcwd(char *buf, size_t size) {
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
