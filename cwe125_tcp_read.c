#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// Configuration items array 
char* config_items[] = {
    "debug_mode=off",
    "max_connections=100",
    "timeout=30",
    "log_level=info",
    "backup_enabled=true",
    "ssl_enabled=false",
    "cache_size=1024",
    "auto_restart=yes"
};

#define CONFIG_ITEMS_COUNT 8

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
    address.sin_port = htons(8092);
    
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
    
    printf("TCP Server listening on port 8092...\n");
    
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

// Function to validate configuration request
int validateConfigRequest(char* data) {
    if (data == NULL) {
        printf("Error: No data received\n");
        return -1;
    }
    
    // Check if data contains only digits
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] < '0' || data[i] > '9') {
            printf("Error: Invalid configuration index format\n");
            return -1;
        }
    }
    
    // Check if data is not empty
    if (strlen(data) == 0) {
        printf("Error: Empty configuration index\n");
        return -1;
    }
    
    return 0;
}

// Function to process configuration index
int processConfigIndex(char* data) {
    if (validateConfigRequest(data) != 0) {
        return -1;
    }
    
    // Additional validation for reasonable range
    int index = atoi(data);
    if (index < 0) {
        printf("Error: Configuration index cannot be negative\n");
        return -1;
    }
    
    return index;
}

// Function to get configuration item
char* getConfigItem(int index) {
    printf("Accessing configuration item at index: %d\n", index);
    // SINK CWE 125
    return config_items[index]; 
}

int main() {
    char* network_data = NULL;
    int config_index = -1;
    char* config_value = NULL;
    
    // Receive data from TCP network
    network_data = receiveDataFromTCP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process the network data through intermediate functions
        config_index = processConfigIndex(network_data);
        
        if (config_index >= 0) {
            printf("Processed configuration index: %d\n", config_index);
            
            // Access configuration item (sink)
            config_value = getConfigItem(config_index);
            
            if (config_value != NULL) {
                printf("Configuration item: %s\n", config_value);
            } else {
                printf("Failed to retrieve configuration item\n");
            }
        } else {
            printf("Failed to process configuration index\n");
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}