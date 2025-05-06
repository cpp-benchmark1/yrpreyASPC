#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include "buffer_manager_cwe415.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    struct iovec iov[2];
    char *buffer1 = NULL;
    char *buffer2 = NULL;
    size_t total_received = 0;
    
    // Initialize buffers
    buffer1 = allocate_buffer(BUFFER_SIZE);
    buffer2 = allocate_buffer(BUFFER_SIZE);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Failed to allocate buffers\n");
        return;
    }
    
    // Set up iovec structures
    iov[0].iov_base = buffer1;
    iov[0].iov_len = BUFFER_SIZE;
    iov[1].iov_base = buffer2;
    iov[1].iov_len = BUFFER_SIZE;
    
    // Read data from client using readv
    ssize_t bytes_read = readv(client_socket, iov, 2);
    
    if (bytes_read > 0) {
        // Process the received data
        printf("Received %zd bytes\n", bytes_read);
        printf("Buffer1 first 10 bytes: %.10s\n", buffer1);
        printf("Buffer2 first 10 bytes: %.10s\n", buffer2);
        
        // If we didn't receive all data, try to receive the rest
        if (bytes_read < BUFFER_SIZE * 2) {
            printf("Didn't receive all data, trying to receive more...\n");
            char *remaining_buffer = (bytes_read <= BUFFER_SIZE) ? buffer2 : NULL;
            size_t remaining_size = (bytes_read <= BUFFER_SIZE) ? BUFFER_SIZE : 0;
            
            if (remaining_buffer) {
                ssize_t more_bytes = recv(client_socket, remaining_buffer, remaining_size, 0);
                if (more_bytes > 0) {
                    bytes_read += more_bytes;
                    printf("Received additional %zd bytes\n", more_bytes);
                    printf("Buffer2 first 10 bytes after additional read: %.10s\n", buffer2);
                }
            }
        }
        
        // Simulate some processing
        if (bytes_read > BUFFER_SIZE) {
            printf("Received more than BUFFER_SIZE bytes\n");
            // This is where the vulnerability can be triggered
            // We'll free buffer1 twice if certain conditions are met
            printf("Freeing buffer1 first time\n");
            free_buffer(buffer1);
            
            // Some complex processing that might lead to double free
            if (strstr(buffer2, "trigger")) {
                printf("Found trigger string, attempting second free\n");
                // This is the second free that causes the double free
                free_buffer(buffer1);
            } else {
                printf("No trigger string found in buffer2\n");
            }
        } else {
            printf("Received less than or equal to BUFFER_SIZE bytes\n");
        }
    } else {
        printf("No data received or error\n");
    }
    
    // Clean up
    printf("Cleaning up buffer2\n");
    free_buffer(buffer2);
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        handle_client(client_socket);
    }
    
    return 0;
} 