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
    servaddr.sin_port = htons(8085);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8085...\n");
    
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

// Intermediate function 1 - Data sanitization
char* sanitizeData(char* data) {
    if (!data) {
        printf("Error: No data to sanitize\n");
        return NULL;
    }
    
    // Remove leading and trailing whitespace
    char* start = data;
    char* end = data + strlen(data) - 1;
    
    // Skip leading whitespace
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }
    
    // Skip trailing whitespace
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    // Null terminate
    *(end + 1) = '\0';
    
    // Check if data is still valid after sanitization
    if (strlen(start) == 0) {
        printf("Error: Data became empty after sanitization\n");
        return NULL;
    }
    
    printf("Data sanitization completed\n");
    return start;
}

// Intermediate function 2 - Batch size calculation
char* calculateBatchSize(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Calculating batch size from data: %s\n", data);
    
    // Convert to int for calculations
    int batch_size = atoi(data);
    
    // Check for reasonable batch sizes
    if (batch_size < 0) {
        printf("Warning: Negative batch size (%d) detected\n", batch_size);
    }
    
    if (batch_size == 0) {
        printf("Warning: Zero batch size, no elements will be processed\n");
    }
    
    if (batch_size > 100000) {
        printf("Warning: Very large batch size (%d) may cause performance issues\n", batch_size);
    }
    
    // Check if batch size is within memory constraints
    if (batch_size > 1000000) {
        printf("Warning: Extremely large batch size (%d) may exhaust memory\n", batch_size);
    }
    
    printf("Batch size calculation completed\n");
    return data;
}

// Intermediate function 3 - Processing preparation
char* prepareProcessing(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Preparing data processing parameters\n");
    
    // Final checks before processing
    int final_count = atoi(data);
    
    // Check system resource availability (simplified)
    if (final_count > 500000) {
        printf("Warning: High processing count (%d) may stress system resources\n", final_count);
    }
    
    // Check for potential integer overflow scenarios
    if (final_count < -1000000) {
        printf("Warning: Very negative count (%d) may cause unexpected behavior\n", final_count);
    }
    
    printf("Processing preparation completed\n");
    return data;
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromUDP();
    
    if (networkData) {
        printf("Received data from UDP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* sanitized = sanitizeData(networkData);
        if (sanitized) {
            char* batch_sized = calculateBatchSize(sanitized);
            if (batch_sized) {
                char* prepared = prepareProcessing(batch_sized);
                if (prepared) {
                    // Convert string to signed int
                    int element_count = atoi(prepared);
                    
                    printf("Processing %d elements in batch\n", element_count);
                    
                    int processed = 0;
                    // SINK CWE 606
                    while (element_count > 0) {
                        printf("Processing element %d\n", processed + 1);
                        
                        processed++;
                        element_count--;
                        
                        // Check for potential infinite loop (but don't break)
                        if (processed > 100000) {
                            printf("Warning: Very high processing count, potential DoS\n");
                        }
                    }
                    
                    printf("Batch processing completed: %d elements processed\n", processed);
                }
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}