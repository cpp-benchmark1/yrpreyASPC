#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

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
    address.sin_port = htons(8094);
    
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
    
    printf("TCP Server listening on port 8094...\n");
    
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

// First intermediate function - process timestamp for log calculation
time_t processTimestampForLog(char* data) {
    if (data == NULL) {
        return 0;
    }
    
    // Convert string to time_t
    time_t timestamp = (time_t)atol(data);
    
    // Some processing logic
    if (timestamp > 0) {
        printf("Processing timestamp for log calculation: %ld\n", timestamp);
    }
    
    return timestamp;
}

// Second intermediate function
void calculateLogTime(time_t timestamp) {
    time_t t = timestamp;
    // SINK CWE 676
    struct tm* timeinfo = localtime(&t);
    
    if (timeinfo != NULL) {
        printf("Log time calculated: %02d:%02d:%02d\n", 
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    } else {
        printf("Failed to calculate log time\n");
    }
}

int main() {
    char* network_data = NULL;
    time_t timestamp = 0;
    
    // Receive data from TCP network
    network_data = receiveDataFromTCP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process timestamp through first intermediate function
        timestamp = processTimestampForLog(network_data);
        
        // Conditional flow based on timestamp value
        if (timestamp % 2 == 0) {
            printf("Even timestamp - proceeding with log calculation\n");
            // Call second intermediate function
            calculateLogTime(timestamp);
        } else {
            printf("Odd timestamp - skipping log calculation\n");
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}