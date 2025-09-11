#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// Log messages array 
char* log_messages[] = {
    "System startup completed",
    "User authentication successful",
    "Database connection established",
    "Configuration loaded successfully",
    "Backup process started",
    "Security scan completed",
    "Network interface configured",
    "Service monitoring active",
    "Error log cleared",
    "System maintenance scheduled"
};

#define LOG_MESSAGES_COUNT 10

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
    servaddr.sin_port = htons(8093);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8093...\n");
    
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

// First intermediate function - validate log request
int validateLogRequest(char* data) {
    if (data == NULL) {
        printf("Error: No log data received\n");
        return -1;
    }
    
    // Check if data is not empty
    if (strlen(data) == 0) {
        printf("Error: Empty log index\n");
        return -1;
    }
    
    // Check if data contains only digits and is reasonable length
    if (strlen(data) > 10) {
        printf("Error: Log index too long\n");
        return -1;
    }
    
    return 0;
}

// Second intermediate function - parse log index
int parseLogIndex(char* data) {
    if (validateLogRequest(data) != 0) {
        return -1;
    }
    
    // Additional validation for numeric content
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] < '0' || data[i] > '9') {
            printf("Error: Log index must be numeric\n");
            return -1;
        }
    }
    
    int index = atoi(data);
    
    // Check for negative values
    if (index < 0) {
        printf("Error: Log index cannot be negative\n");
        return -1;
    }
    
    return index;
}

// Third intermediate function - prepare log access
int prepareLogAccess(char* data) {
    int index = parseLogIndex(data);
    
    if (index < 0) {
        return -1;
    }
    
    return index;
}

// Function to retrieve log message
char* retrieveLogMessage(int index) {
    // SINK CWE 125
    printf("Retrieving log message at index: %d\n", index);
    return log_messages[index];
}

int main() {
    char* network_data = NULL;
    int log_index = -1;
    char* log_message = NULL;
    
    // Receive data from UDP network
    network_data = receiveDataFromUDP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process the network data through 3 intermediate functions
        log_index = prepareLogAccess(network_data);
        
        if (log_index >= 0) {
            printf("Processed log index: %d\n", log_index);
            
            // Access log message
            log_message = retrieveLogMessage(log_index);
            
            if (log_message != NULL) {
                printf("Log message: %s\n", log_message);
            } else {
                printf("Failed to retrieve log message\n");
            }
        } else {
            printf("Failed to process log index\n");
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}