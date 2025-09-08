#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>


// TCP network function - Source
char* receiveDataFromTCP() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char* received_data = NULL;
    
    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return NULL;
    }
    
    // Configure socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8084);
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return NULL;
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }
    
    printf("TCP Server listening on port 8084...\n");
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        return NULL;
    }
    
    // Read data
    int valread = read(new_socket, buffer, 1024);
    if (valread > 0) {
        received_data = malloc(valread + 1);
        if (received_data) {
            strncpy(received_data, buffer, valread);
            received_data[valread] = '\0';
        }
    }
    
    close(new_socket);
    close(server_fd);
    
    return received_data;
}

// Intermediate function 1 - Input parsing
char* parseInput(char* data) {
    if (!data) {
        printf("Error: No data to parse\n");
        return NULL;
    }
    
    // Check if data contains only digits and optional minus sign
    int valid_format = 1;
    int has_digits = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] == '-' && i == 0) {
            // Allow minus sign only at the beginning
            continue;
        } else if (data[i] >= '0' && data[i] <= '9') {
            has_digits = 1;
        } else if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n') {
            valid_format = 0;
            break;
        }
    }
    
    if (!valid_format) {
        printf("Warning: Invalid format detected, but continuing anyway\n");
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found\n");
        return NULL;
    }
    
    printf("Input parsing completed\n");
    return data;
}

// Intermediate function 2 - Thread count validation
char* validateThreadCount(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Validating thread count from data: %s\n", data);
    
    // Convert to int for validation checks
    int thread_count = atoi(data);
    
    // Check for reasonable thread count ranges
    if (thread_count < 0) {
        printf("Warning: Negative thread count detected (%d)\n", thread_count);
    }
    
    if (thread_count == 0) {
        printf("Warning: Zero thread count, no threads will be created\n");
    }
    
    if (thread_count > 1000) {
        printf("Warning: High thread count (%d) may impact system performance\n", thread_count);
    }
    
    printf("Thread count validation completed\n");
    return data;
}

// Intermediate function 3
void executeThreadTasks(char* count_data) {
    if (!count_data) {
        printf("Error: No count data provided\n");
        return;
    }
    
    // Convert string to signed int 
    int task_count = atoi(count_data);
    
    printf("Executing %d thread tasks\n", task_count);
    
    // SINK CWE 606
    for (int i = 0; i < task_count; i++) {
        printf("Executing task %d of %d\n", i + 1, task_count);
        
        // Check for potential infinite loop (but don't break)
        if (i > 10000) {
            printf("Warning: Very high iteration count, potential DoS\n");
        }
    }
    
    printf("All thread tasks completed\n");
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromTCP();
    
    if (networkData) {
        printf("Received data from TCP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* parsed = parseInput(networkData);
        if (parsed) {
            char* validated = validateThreadCount(parsed);
            if (validated) {
                executeThreadTasks(validated);
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}