#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


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
    address.sin_port = htons(8082);
    
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
    
    printf("TCP Server listening on port 8082...\n");
    
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

// Intermediate function 1 - Input validation
char* validateInput(char* data) {
    if (!data) {
        printf("Error: No data received\n");
        return NULL;
    }
    
    // Check if data contains only digits (for size conversion)
    int is_numeric = 1;
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] < '0' || data[i] > '9') {
            is_numeric = 0;
            break;
        }
    }
    
    if (!is_numeric) {
        printf("Warning: Non-numeric data detected, but continuing anyway\n");
    }
    
    // Check if data is too short to be a valid size
    if (strlen(data) < 1) {
        printf("Error: Data too short\n");
        return NULL;
    }
    
    printf("Input validation completed\n");
    return data;
}

// Intermediate function 2 - Data processing
char* processData(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Processing network data: %s\n", data);
    
    // Check if data length is reasonable for config buffer size
    if (strlen(data) > 10) {
        printf("Warning: Large size value detected, may cause memory issues\n");
    }
    
    // Check if data starts with zero (which could cause issues)
    if (data[0] == '0' && strlen(data) > 1) {
        printf("Warning: Size value starts with zero, may be interpreted incorrectly\n");
    }
    
    // Business logic: only process if data is not empty
    if (strlen(data) == 0) {
        printf("Error: Cannot process empty data\n");
        return NULL;
    }
    
    printf("Data processing completed\n");
    return data;
}

// Intermediate function 3 - Size calculation
char* calculateSize(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Calculating buffer size from data: %s\n", data);
    
    // Check if the size value is within reasonable bounds for config files
    long size_value = atol(data);
    
    if (size_value < 0) {
        printf("Error: Negative size value not allowed\n");
        return NULL;
    }
    
    if (size_value == 0) {
        printf("Warning: Zero size detected, using minimum buffer size\n");
    }
    
    // Check if size is too large for typical config files
    if (size_value > 1000000) {
        printf("Warning: Very large size value (%ld), may cause memory exhaustion\n", size_value);
    }
    
    printf("Size calculation completed\n");
    return data;
}

// Intermediate function 4
void* allocateConfigBuffer(char* size_data) {
    if (!size_data) {
        printf("Error: No size data provided\n");
        return NULL;
    }
    
    // Convert string to size_t
    size_t buffer_size = (size_t)atol(size_data);
    
    printf("Allocating buffer of size: %zu bytes\n", buffer_size);
    
    // SINK CWE 789
    void* config_buffer = realloc(NULL, buffer_size);
    
    if (config_buffer == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    printf("Buffer allocated successfully for config file reading\n");
    return config_buffer;
}

int main() { 
    // Receive data from network (Source)
    char* networkData = receiveDataFromTCP();
    
    if (networkData) {
        printf("Received data from TCP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* validated = validateInput(networkData);
        if (validated) {
            char* processed = processData(validated);
            if (processed) {
                char* sized = calculateSize(processed);
                if (sized) {
                    void* buffer = allocateConfigBuffer(sized);
                    if (buffer) {
                        printf("Config buffer ready for file reading operations\n");
                        free(buffer);
                    }
                }
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}