#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// TCP network function - Source
int receiveDataFromTCP() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    int received_value = 0;
    
    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 0;
    }
    
    // Configure socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return 0;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8086);
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 0;
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return 0;
    }
    
    printf("TCP Server listening on port 8086...\n");
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        return 0;
    }
    
    // Read data
    int valread = read(new_socket, buffer, 1024);
    if (valread > 0) {
        buffer[valread] = '\0';
        received_value = atoi(buffer);
    }
    
    close(new_socket);
    close(server_fd);
    
    return received_value;
}

// Intermediate function 1 - Configuration validation
int validateConfig(int config_value) {
    printf("Validating system configuration value: %d\n", config_value);
    
    // Check if configuration is within reasonable system limits
    if (config_value < 0) {
        printf("Warning: Negative configuration value detected\n");
    }
    
    if (config_value == 0) {
        printf("Warning: Zero configuration value detected\n");
    }
    
    if (config_value > 1000) {
        printf("Warning: Large configuration value (%d) may impact system performance\n", config_value);
    }
    
    // Check for common system configuration values
    if (config_value == 1 || config_value == 2 || config_value == 4 || config_value == 8) {
        printf("Info: Common system configuration value detected\n");
    }
    
    printf("Configuration validation completed\n");
    return config_value;
}

// Intermediate function 2 - System resource calculation
int calculateResources(int config_value) {
    printf("Calculating system resources based on configuration: %d\n", config_value);
    
    // Calculate memory allocation based on configuration
    int memory_blocks = config_value * 64;  // 64 bytes per block
    
    // Calculate CPU time slices based on configuration
    int time_slices = config_value * 10;    // 10ms per slice
    
    // Calculate network buffer size based on configuration
    int buffer_size = config_value * 1024;  // 1KB per unit
    
    printf("Calculated resources - Memory: %d blocks, Time: %d slices, Buffer: %d bytes\n", 
           memory_blocks, time_slices, buffer_size);
    
    // Check if calculated resources are reasonable
    if (memory_blocks > 100000) {
        printf("Warning: High memory allocation requested\n");
    }
    
    if (time_slices > 10000) {
        printf("Warning: High CPU time allocation requested\n");
    }
    
    printf("Resource calculation completed\n");
    return config_value;
}

// Intermediate function 3 - Sink with divide by zero (vulnerable)
int processSystemConfig(int config_value) {
    printf("Processing system configuration: %d\n", config_value);
    
    int total_system_load = 1000;
    int max_processes = 500;
    int network_connections = 200;
    
    printf("System metrics - Load: %d, Max Processes: %d, Connections: %d\n", 
           total_system_load, max_processes, network_connections);
    
    int load_per_config = total_system_load / 2;
    // SINK CWE 369
    int processes_per_config = max_processes / config_value;
    int connections_per_config = network_connections / 2;
    
    printf("Calculated ratios - Load per config: %d, Processes per config: %d, Connections per config: %d\n",
           load_per_config, processes_per_config, connections_per_config);
    
    // Save division results to environment variables
    char env_buffer[64];
    
    sprintf(env_buffer, "%d", load_per_config);
    setenv("SYSTEM_LOAD_PER_CONFIG", env_buffer, 1);
    
    sprintf(env_buffer, "%d", processes_per_config);
    setenv("SYSTEM_PROCESSES_PER_CONFIG", env_buffer, 1);
    
    sprintf(env_buffer, "%d", connections_per_config);
    setenv("SYSTEM_CONNECTIONS_PER_CONFIG", env_buffer, 1);
    
    printf("Division results saved to environment variables\n");
    
    return processes_per_config;
}

int main() {
    
    // Receive data from network (Source)
    int networkData = receiveDataFromTCP();
    
    printf("Received configuration value from TCP: %d\n", networkData);
    
    // Propagation flow through intermediate functions
    int validated = validateConfig(networkData);
    int calculated = calculateResources(validated);
    
    int processed = processSystemConfig(calculated);
    
    printf("Final configuration value: %d\n", processed);
    
    return 0;
}