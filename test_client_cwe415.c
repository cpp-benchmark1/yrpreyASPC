#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *buffer1, *buffer2;
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    printf("Connected to server\n");
    
    // Prepare the data to trigger the vulnerability
    buffer1 = (char *)malloc(BUFFER_SIZE);
    buffer2 = (char *)malloc(BUFFER_SIZE);
    
    if (!buffer1 || !buffer2) {
        perror("Memory allocation failed");
        return -1;
    }
    
    // Fill first buffer with some data
    memset(buffer1, 'A', BUFFER_SIZE);
    
    // Fill second buffer with the trigger string
    memset(buffer2, 0, BUFFER_SIZE);
    strcpy(buffer2, "trigger");
    
    // Combine both buffers into one send
    char *combined_buffer = (char *)malloc(BUFFER_SIZE * 2);
    if (!combined_buffer) {
        perror("Memory allocation failed");
        return -1;
    }
    
    memcpy(combined_buffer, buffer1, BUFFER_SIZE);
    memcpy(combined_buffer + BUFFER_SIZE, buffer2, BUFFER_SIZE);
    
    printf("Sending combined buffer (%d bytes)\n", BUFFER_SIZE * 2);
    ssize_t sent = send(sock, combined_buffer, BUFFER_SIZE * 2, 0);
    if (sent < 0) {
        perror("Send failed");
        return -1;
    }
    printf("Sent %zd bytes\n", sent);
    
    // Clean up
    free(buffer1);
    free(buffer2);
    free(combined_buffer);
    close(sock);
    
    printf("Test client finished\n");
    return 0;
} 