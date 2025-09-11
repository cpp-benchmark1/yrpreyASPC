#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// UDP network function - Source
int receiveDataFromUDP() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int received_value = 0;
    socklen_t len;
    int n;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return 0;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8087);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return 0;
    }
    
    printf("UDP Server listening on port 8087...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_value = atoi(buffer);
    }
    
    close(sockfd);
    return received_value;
}

// Intermediate function 1 - Batch size validation
int validateBatchSize(int batch_size) {
    printf("Validating batch size: %d\n", batch_size);
    
    // Check if batch size is within processing limits
    if (batch_size < 0) {
        printf("Warning: Negative batch size detected\n");
        return 1;
    }
    
    if (batch_size == 0) {
        printf("Warning: Zero batch size detected\n");
    }
    
    if (batch_size > 10000) {
        printf("Warning: Large batch size (%d) may impact processing performance\n", batch_size);
    }
    
    // Check for common batch processing sizes
    if (batch_size == 32 || batch_size == 64 || batch_size == 128 || batch_size == 256) {
        printf("Info: Common batch processing size detected\n");
    }
    
    printf("Batch size validation completed\n");
    return batch_size;
}

// Intermediate function 2 - Processing optimization
int optimizeProcessing(int batch_size) {
    printf("Optimizing processing for batch size: %d\n", batch_size);
    
    // Calculate optimal number of worker threads
    int optimal_threads = batch_size / 4;  // 4 items per thread
    
    // Calculate memory allocation per batch
    int memory_per_batch = batch_size / 16;
    
    // Calculate processing time estimation
    int estimated_time = batch_size / 2;
    
    printf("Optimization results - Threads: %d, Memory: %d bytes, Time: %d ms\n",
           optimal_threads, memory_per_batch, estimated_time);
    
    // Check if optimization parameters are reasonable
    if (optimal_threads > 100) {
        printf("Warning: High number of threads requested\n");
    }
    
    if (memory_per_batch > 1000000) {
        printf("Warning: High memory allocation requested\n");
    }
    
    if (estimated_time > 60000) {
        printf("Warning: Long processing time estimated\n");
    }
    
    printf("Processing optimization completed\n");
    return batch_size;
}

// Intermediate function 3 - Resource allocation
int allocateResources(int batch_size) {
    printf("Allocating resources for batch size: %d\n", batch_size);
    
    // Calculate resource requirements
    int cpu_cores_needed = batch_size / 8;
    int memory_blocks = batch_size / 2;
    int network_buffers = batch_size / 16;
    
    printf("Resource requirements - CPU cores: %d, Memory blocks: %d, Network buffers: %d\n",
           cpu_cores_needed, memory_blocks, network_buffers);
    
    // Check system capacity
    int available_cores = 8;
    int available_memory = 1000;
    int available_buffers = 100;
    
    printf("Available resources - CPU cores: %d, Memory blocks: %d, Network buffers: %d\n",
           available_cores, available_memory, available_buffers);
    
    // Calculate resource utilization percentages
    int cpu_utilization = (cpu_cores_needed * 1) / available_cores;
    int memory_utilization = (memory_blocks * 1) / available_memory;
    int buffer_utilization = (network_buffers * 1) / available_buffers;
    
    printf("Resource utilization - CPU: %d%%, Memory: %d%%, Buffers: %d%%\n",
           cpu_utilization, memory_utilization, buffer_utilization);
    
    printf("Resource allocation completed\n");
    return batch_size;
}

int main() {
    // Receive data from network (Source)
    int networkData = receiveDataFromUDP();
    int allocated_sizes[2];
    
    printf("Received batch size from UDP: %d\n", networkData);
    
    // Propagation flow through intermediate functions
    int validated = validateBatchSize(networkData);
    int optimized = optimizeProcessing(validated);
    int allocated = allocateResources(optimized);
    allocated_sizes[0] = allocated;
    allocated_sizes[1] = 1024;

    int total_items = 1000;
    int processing_time = 5000;  // 5 seconds
    int memory_usage = 2000;     // 2MB
    
    printf("Processing metrics - Total items: %d, Time: %d ms, Memory: %d KB\n",
           total_items, processing_time, memory_usage);
    
    int items_per_batch = total_items / 2;
    // safe division
    int time_per_batch = processing_time / allocated_sizes[1];
    // SINK CWE 369
    int memory_per_batch = memory_usage % allocated_sizes[0];
    
    printf("Calculated batch metrics - Items per batch: %d, Time per batch: %d ms, Memory per batch: %d KB\n",
           items_per_batch, time_per_batch, memory_per_batch);

    
    printf("Batch processing completed\n");
    
    return 0;
}