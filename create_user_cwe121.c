#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 16  

void process_data(char *data) {
    char small_buffer[SMALL_BUFFER_SIZE];  
    printf("[DEBUG] small_buffer address: %p\n", (void*)small_buffer);
    
    strcpy(small_buffer, data);  
    
    printf("[DEBUG] Data copied to small_buffer: %s\n", small_buffer);
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
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        free(buffer);
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        is_server = 1;
        addr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Bind failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        if (listen(sock, 1) < 0) {
            printf("Listen failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        printf("Server listening on port %d...\n", PORT);
        printf("Buffer size is %d bytes. Try sending more than %d bytes to trigger overflow.\n", 
               SMALL_BUFFER_SIZE, SMALL_BUFFER_SIZE);
        
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("Accept failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        int bytes_received = read(client_sock, buffer, BUFFER_SIZE);
        if (bytes_received > 0) {
            printf("Received %d bytes\n", bytes_received);
            process_data(buffer);  
        }
        
        close(client_sock);
    } else {
        // Client mode
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Connection failed\n");
            free(buffer);
            close(sock);
            return 1;
        }
        
        printf("Connected to server\n");
        printf("Enter your payload (max %d characters): ", BUFFER_SIZE - 1);
        
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
            }
            
            if (write(sock, buffer, strlen(buffer) + 1) < 0) {
                printf("Send failed\n");
            } else {
                printf("Data sent successfully\n");
            }
        }
    }
    
    // Cleanup
    free(buffer);
    close(sock);
    
    return 0;
}

/*
To test it:

1. Compile with:
gcc -O0 -g -fno-stack-protector create_user_cwe121.c -o create_user_cwe121

2. Run server:
./create_user_cwe121 server

3. Run client in another terminal:
./create_user_cwe121

4. Enter a string longer than 16 characters to trigger the buffer overflow
*/
