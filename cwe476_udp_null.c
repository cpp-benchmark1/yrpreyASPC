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
    servaddr.sin_port = htons(8097);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8097...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data != NULL) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// First intermediate function - parse processing index
int* parseProcessingIndex(char* data) {
    if (data == NULL) {
        return NULL;
    }
    
    // Convert string to signed int
    signed int processing_index = (signed int)atoi(data);
    
    // Allocate memory for pointer
    int* ptr = malloc(sizeof(int));
    if (ptr != NULL) {
        *ptr = processing_index;
        printf("Processing index parsed: %d\n", *ptr);
    }
    
    return ptr;
}

// Second intermediate function - prepare processing and set to NULL
int* prepareProcessing(int* processing_ptr) {
    if (processing_ptr == NULL) {
        return NULL;
    }
    
    // Some processing logic
    if (*processing_ptr >= 0) {
        printf("Processing index is valid: %d\n", *processing_ptr);
    } else {
        printf("Processing index is invalid: %d\n", *processing_ptr);
    }
    
    processing_ptr = NULL;
    
    return processing_ptr;
}

int main() {
    char* network_data = NULL;
    int* processing_ptr = NULL;
    int* final_ptr = NULL;
    
    // Receive data from UDP network
    network_data = receiveDataFromUDP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process through first intermediate function
        processing_ptr = parseProcessingIndex(network_data);
        
        // Conditional flow based on processing index
        if (processing_ptr != NULL && *processing_ptr < 100) {
            printf("Processing index < 100 - proceeding with preparation\n");
            // Call second intermediate function
            final_ptr = prepareProcessing(processing_ptr);
        } else {
            printf("Processing index >= 100 - using original pointer\n");
            final_ptr = processing_ptr;
        }
        
        // SINK CWE 476
        printf("Final processing value: %d\n", *final_ptr);
        
        free(processing_ptr);
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}