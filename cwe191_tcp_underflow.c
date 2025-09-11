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
    address.sin_port = htons(8088);
    
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
    
    printf("TCP Server listening on port 8088...\n");
    
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

// Intermediate function 1 - Configuration parsing
char* parseConfiguration(char* data) {
    if (!data) {
        printf("Error: No configuration data to parse\n");
        return NULL;
    }
    
    // Check if data contains valid numeric characters
    int has_digits = 0;
    int has_minus = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] >= '0' && data[i] <= '9') {
            has_digits = 1;
        } else if (data[i] == '-' && i == 0) {
            has_minus = 1;
        } else if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n') {
            printf("Warning: Invalid character '%c' in configuration data\n", data[i]);
        }
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found in configuration\n");
        return NULL;
    }
    
    if (has_minus) {
        printf("Info: Negative configuration value detected\n");
    }
    
    printf("Configuration parsing completed\n");
    return data;
}

// Intermediate function 2 - Lower limit validation
char* validateLowerLimit(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Validating lower limit from data: %s\n", data);
    
    // Convert to int for validation checks
    int lower_limit = atoi(data);
    
    // Check if lower limit is within reasonable bounds
    if (lower_limit < -1000) {
        printf("Warning: Very negative lower limit (%d) detected\n", lower_limit);
    }
    
    if (lower_limit == 0) {
        printf("Warning: Zero lower limit detected\n");
    }
    
    if (lower_limit > 1000) {
        printf("Warning: High positive lower limit (%d) detected\n", lower_limit);
    }
    
    printf("Lower limit validation completed\n");
    return data;
}

// Intermediate function 3 - Sink with integer underflow (vulnerable)
int calculateSystemBounds(char* limit_data) {
    if (!limit_data) {
        printf("Error: No limit data provided\n");
        return 0;
    }
    
    printf("Calculating system bounds from limit: %s\n", limit_data);
    
    // Convert string to signed int
    int lower_limit = atoi(limit_data);
    
    // System configuration values
    int base_memory_limit = 1000;
    int base_cpu_limit = 500;
    int base_network_limit = 200;
    
    printf("Base system limits - Memory: %d, CPU: %d, Network: %d\n",
           base_memory_limit, base_cpu_limit, base_network_limit);
    
    // SINK CWE 191
    int adjusted_memory = lower_limit - base_memory_limit;
    int adjusted_cpu = base_cpu_limit / 2;
    int adjusted_network = base_network_limit / 2;
    
    printf("Adjusted system limits - Memory: %d, CPU: %d, Network: %d\n",
           adjusted_memory, adjusted_cpu, adjusted_network);
    
    // Check for potential underflow (but don't prevent it)
    if (adjusted_memory < 0) {
        printf("Warning: Memory limit underflow detected\n");
    }
    
    if (adjusted_cpu < 0) {
        printf("Warning: CPU limit underflow detected\n");
    }
    
    if (adjusted_network < 0) {
        printf("Warning: Network limit underflow detected\n");
    }
    
    printf("System bounds calculation completed\n");
    return adjusted_memory;
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromTCP();
    
    if (networkData) {
        printf("Received configuration data from TCP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* parsed = parseConfiguration(networkData);
        if (parsed) {
            char* validated = validateLowerLimit(parsed);
            if (validated) {
                int result = calculateSystemBounds(validated);
                printf("Final system bounds result: %d\n", result);
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}