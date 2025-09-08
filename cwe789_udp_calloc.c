#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// UDP network function - Source
char* receiveDataFromUDP() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    char* received_data = NULL;
    socklen_t len;
    int n;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return NULL;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8083);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8083...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// Intermediate function 1 - Format verification
char* verifyFormat(char* data) {
    if (!data) {
        printf("Error: No data to verify\n");
        return NULL;
    }
    
    // Check if data is empty
    if (strlen(data) == 0) {
        printf("Error: Empty data received\n");
        return NULL;
    }
    
    // Check if data contains valid characters for batch size
    int has_digits = 0;
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] >= '0' && data[i] <= '9') {
            has_digits = 1;
        } else if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n') {
            printf("Warning: Non-numeric character found in batch size data\n");
        }
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found for batch size\n");
        return NULL;
    }
    
    printf("Format verification passed\n");
    return data;
}

// Intermediate function 2 - Data transformation
char* transformData(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Transforming network data: %s\n", data);
    
    // Remove leading whitespace
    while (*data == ' ' || *data == '\t' || *data == '\n') {
        data++;
    }
    
    // Check if data is still valid after trimming
    if (strlen(data) == 0) {
        printf("Error: Data became empty after trimming whitespace\n");
        return NULL;
    }
    
    // Check if the batch size is reasonable for processing
    long batch_size = atol(data);
    
    if (batch_size <= 0) {
        printf("Warning: Invalid batch size (%ld), using default\n", batch_size);
    }
    
    if (batch_size > 10000) {
        printf("Warning: Large batch size (%ld) may impact performance\n", batch_size);
    }
    
    printf("Data transformation completed\n");
    return data;
}

// Intermediate function 3 - Batch size calculation
char* calculateBatchSize(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Calculating batch processing size from data: %s\n", data);
    
    // Parse the batch size value
    long batch_size = atol(data);
    
    // Check for reasonable batch size limits
    if (batch_size < 1) {
        printf("Error: Batch size must be at least 1\n");
        return NULL;
    }
    
    if (batch_size > 1000000) {
        printf("Warning: Extremely large batch size (%ld) may cause memory issues\n", batch_size);
    }
    
    // Business logic: ensure batch size is within processing limits
    if (batch_size > 500000) {
        printf("Warning: Batch size exceeds recommended limit, may cause system stress\n");
    }
    
    printf("Batch size calculation completed\n");
    return data;
}

// Intermediate function 4 - Memory preparation
char* prepareMemory(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Preparing memory allocation parameters\n");
    
    // Final validation before memory allocation
    long final_size = atol(data);
    
    // Check if size is within system limits
    if (final_size > SIZE_MAX / sizeof(char)) {
        printf("Error: Size too large for system limits\n");
        return NULL;
    }
    
    // Check available memory (simplified check)
    if (final_size > 100000000) {  // 100MB
        printf("Warning: Large memory allocation requested (%ld bytes)\n", final_size);
    }
    
    // Business logic: ensure we have reasonable size for batch processing
    if (final_size < 1) {
        printf("Error: Invalid size for batch processing\n");
        return NULL;
    }
    
    printf("Memory preparation completed\n");
    return data;
}

int main() {
    size_t batch_sizes[2];
    // Receive data from network (Source)
    char* networkData = receiveDataFromUDP();
    
    if (networkData) {
        printf("Received data from UDP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* verified = verifyFormat(networkData);
        if (verified) {
            char* transformed = transformData(verified);
            if (transformed) {
                char* batch_sized = calculateBatchSize(transformed);
                if (batch_sized) {
                    char* prepared = prepareMemory(batch_sized);
                    if (prepared) {
                        // Convert string to size_t without validation - CWE-789 VULNERABILITY
                        batch_sizes[0] = (size_t)atol(prepared);
                        batch_sizes[1] = 1024;

                        // unsafe allocation
                        printf("Allocating batch processing buffer of size: %zu bytes\n", batch_sizes[0]);
                        // SINK CWE 789
                        void* batch_buffer = calloc(batch_sizes[0], sizeof(char));
                        if (batch_buffer == NULL) {
                            printf("Memory allocation failed\n");
                            free(networkData);
                            return 1;
                        }
                        printf("Batch buffer allocated successfully for data processing\n");
                        memset(batch_buffer, 0, batch_sizes[0]);
                        printf("Batch buffer initialized for processing operations\n");
                        free(batch_buffer);

                        // safe allocation
                        printf("Allocating another processing buffer of size: %zu bytes\n", batch_sizes[1]);
                        void* batch_buffer_2 = calloc(batch_sizes[1], sizeof(char));
                        if (batch_buffer_2 == NULL) {
                            printf("Memory allocation failed\n");
                            free(networkData);
                            return 1;
                        }
                        printf("Batch buffer allocated successfully for data processing\n");
                        memset(batch_buffer_2, 0, batch_sizes[1]);
                        printf("Batch buffer initialized for processing operations\n");
                        free(batch_buffer_2);
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