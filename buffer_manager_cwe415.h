#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <stdlib.h>
#include <string.h>

// Custom buffer structure
typedef struct {
    char *data;
    size_t size;
    int is_freed;
} Buffer;

// Function declarations
char* allocate_buffer(size_t size);
void free_buffer(char *buffer);

// Global buffer tracking (this is part of the vulnerability)
extern Buffer *global_buffers;
extern int buffer_count;

#endif // BUFFER_MANAGER_H 