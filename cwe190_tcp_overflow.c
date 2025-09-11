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
    address.sin_port = htons(8090);
    
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
    
    printf("TCP Server listening on port 8090...\n");
    
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

// Intermediate function 1 - Configuration field validation
char* validateConfigField(char* data) {
    if (!data) {
        printf("Error: No configuration field data to validate\n");
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
            printf("Warning: Invalid character '%c' in configuration field\n", data[i]);
        }
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found in configuration field\n");
        return NULL;
    }
    
    if (has_minus) {
        printf("Info: Negative configuration field value detected\n");
    }
    
    // Check if configuration field is within reasonable bounds
    int config_value = atoi(data);
    if (config_value < -10000) {
        printf("Warning: Very negative configuration field (%d) detected\n", config_value);
    }
    
    if (config_value > 10000) {
        printf("Warning: Large positive configuration field (%d) detected\n", config_value);
    }
    
    printf("Configuration field validation completed\n");
    return data;
}

// Intermediate function 2 - Sink with integer overflow (vulnerable)
int calculateSystemMetrics(char* config_data) {
    if (!config_data) {
        printf("Error: No configuration data provided\n");
        return 0;
    }
    
    printf("Calculating system metrics from configuration: %s\n", config_data);
    
    int config_value = atoi(config_data);
    
    // System base values for calculations
    int base_memory_usage = 1000;
    int base_cpu_usage = 500;
    int base_network_usage = 200;
    
    printf("Base system usage - Memory: %d, CPU: %d, Network: %d\n",
           base_memory_usage, base_cpu_usage, base_network_usage);
    
    // SINK CWE 190
    int total_memory = base_memory_usage * config_value;
    int total_cpu = base_cpu_usage * 2;
    int total_network = base_network_usage * 2;
    
    printf("Calculated totals - Memory: %d, CPU: %d, Network: %d\n",
           total_memory, total_cpu, total_network);
    
    // Check for potential overflow (but don't prevent it)
    if (total_memory < 0) {
        printf("Warning: Memory calculation overflow detected\n");
    }
    
    if (total_cpu < 0) {
        printf("Warning: CPU calculation overflow detected\n");
    }
    
    if (total_network < 0) {
        printf("Warning: Network calculation overflow detected\n");
    }
    
    printf("System metrics calculation completed\n");
    return total_memory;
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromTCP();
    
    if (networkData) {
        printf("Received configuration data from TCP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* validated = validateConfigField(networkData);
        if (validated) {
            int result = calculateSystemMetrics(validated);
            printf("Final system metrics result: %d\n", result);
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}