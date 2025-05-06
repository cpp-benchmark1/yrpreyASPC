#include "buffer_manager_cwe415.h"
#include <stdio.h>

// Global buffer tracking
Buffer *global_buffers = NULL;
int buffer_count = 0;

char* allocate_buffer(size_t size) {
    // Allocate memory for the buffer
    char *buffer = (char *)malloc(size);
    if (!buffer) {
        return NULL;
    }
    
    // Initialize the buffer
    memset(buffer, 0, size);
    
    // Track the buffer
    Buffer *new_buffers = realloc(global_buffers, (buffer_count + 1) * sizeof(Buffer));
    if (!new_buffers) {
        free(buffer);
        return NULL;
    }
    
    global_buffers = new_buffers;
    global_buffers[buffer_count].data = buffer;
    global_buffers[buffer_count].size = size;
    global_buffers[buffer_count].is_freed = 0;
    buffer_count++;
    
    return buffer;
}

void free_buffer(char *buffer) {
    if (!buffer) {
        return;
    }
    
    // Find the buffer in our tracking array
    for (int i = 0; i < buffer_count; i++) {
        if (global_buffers[i].data == buffer) {
            // This is where the vulnerability exists
            // We don't check if the buffer was already freed
            free(buffer);
            global_buffers[i].is_freed = 1;
            global_buffers[i].data = NULL;
            return;
        }
    }
    
    // If we get here, the buffer wasn't found in our tracking array
    // This is another potential issue - we might free memory we didn't allocate
    free(buffer);
} 