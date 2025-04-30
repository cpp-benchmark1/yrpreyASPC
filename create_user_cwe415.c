#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void process_data(char *data, int size) {
    char *buffer = (char *)malloc(size);
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    memcpy(buffer, data, size);
    free(buffer);
    buffer = NULL;
    
    sleep(1);
    
    if (buffer != NULL) {
        free(buffer);
    }
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in addr;
    char *buffer = (char *)malloc(BUFFER_SIZE);
    int is_server = 0;
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        free(buffer);
        return 1;
    }
    
    // Set up address
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        // Server mode
        is_server = 1;
        addr.sin_addr.s_addr = INADDR_ANY;
        
        // Bind socket
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Bind failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        // Listen for connections
        if (listen(sock, 1) < 0) {
            printf("Listen failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        printf("Server listening on port %d...\n", PORT);
        
        // Accept client connection
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("Accept failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        // Receive data from client
        int bytes_received = read(client_sock, buffer, BUFFER_SIZE);
        if (bytes_received > 0) {
            printf("Received %d bytes\n", bytes_received);
            process_data(buffer, bytes_received);
        }
        
        close(client_sock);
    } else {
        // Client mode
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        // Connect to server
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Connection failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        printf("Connected to server\n");
        
        // Fill buffer with data
        memset(buffer, 'A', BUFFER_SIZE);
        
        // Send data
        if (write(sock, buffer, BUFFER_SIZE) < 0) {
            printf("Send failed\n");
        } else {
            printf("Data sent successfully\n");
        }
    }
    
    // Cleanup
    free(buffer);
    close(sock);
    
    return 0;
}

/*
To test it:

gcc create_user_cwe415.c -o create_user_cwe415
./create_user_cwe415 server

another terminal for the client:
./create_user_cwe415

*/