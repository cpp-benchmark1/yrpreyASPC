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
    address.sin_port = htons(8096);
    
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
    
    printf("TCP Server listening on port 8096...\n");
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        return NULL;
    }
    
    // Read data
    int valread = read(new_socket, buffer, 1024);
    if (valread > 0) {
        buffer[valread] = '\0';
        received_data = malloc(valread + 1);
        if (received_data != NULL) {
            strcpy(received_data, buffer);
        }
    }
    
    close(new_socket);
    close(server_fd);
    
    return received_data;
}

// First intermediate function - process configuration ID
int* processConfigID(char* data) {
    if (data == NULL) {
        return NULL;
    }
    
    // Convert string to signed int
    signed int config_id = (signed int)atoi(data);
    
    // Allocate memory for pointer
    int* ptr = malloc(sizeof(int));
    if (ptr != NULL) {
        *ptr = config_id;
        printf("Configuration ID processed: %d\n", *ptr);
    }
    
    return ptr;
}

// Second intermediate function
void validateConfig(int* config_ptr) {
    if (config_ptr == NULL) {
        return;
    }
    
    // Some validation logic
    if (*config_ptr > 0) {
        printf("Configuration ID is positive: %d\n", *config_ptr);
    } else {
        printf("Configuration ID is not positive: %d\n", *config_ptr);
    }
    
    // Set pointer to NULL (this creates the vulnerability)
    if (*config_ptr % 2 == 0) {
        printf("Even configuration ID - setting pointer to NULL\n");
        config_ptr = NULL;
    }
    
    // SINK CWE 476
    printf("Final configuration value: %d\n", *config_ptr);
}

int main() {
    char* network_data = NULL;
    int* config_ptr = NULL;
    
    // Receive data from TCP network
    network_data = receiveDataFromTCP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process configuration ID through first intermediate function
        config_ptr = processConfigID(network_data);
        
        // Conditional flow based on configuration ID
        if (config_ptr != NULL && *config_ptr > 10) {
            printf("Configuration ID > 10 - proceeding with validation\n");
            // Call second intermediate function
            validateConfig(config_ptr);
        } else {
            printf("Configuration ID <= 10 - skipping validation\n");
        }
        
        free(config_ptr);
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}