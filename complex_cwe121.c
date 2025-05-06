#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 16
#define PASSWORD_SIZE 32
#define SESSION_ID_SIZE 64

typedef struct {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char session_id[SESSION_ID_SIZE];
    int is_authenticated;
} UserSession;

void init_session(UserSession *session) {
    memset(session, 0, sizeof(UserSession));
    session->is_authenticated = 0;
}

int process_credentials(UserSession *session, char *data, size_t len) {
    char temp_buffer[USERNAME_SIZE];
    
    memcpy(temp_buffer, data, len);
    memmove(session->username, temp_buffer, len);
    
    char *session_data = "SESSION_";
    memcpy(session->session_id, session_data, strlen(session_data));
    memcpy(session->session_id + strlen(session_data), temp_buffer, len);
    
    return 1;
}

int validate_session(UserSession *session) {
    char validation_buffer[SESSION_ID_SIZE];
    
    memset(validation_buffer, 0, SESSION_ID_SIZE);
    memcpy(validation_buffer, session->session_id, strlen(session->session_id));
    
    return 1;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in addr;
    char *buffer = (char *)malloc(BUFFER_SIZE);
    UserSession current_session;
    
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
        printf("Buffer sizes: USERNAME=%d, PASSWORD=%d, SESSION=%d bytes\n", 
               USERNAME_SIZE, PASSWORD_SIZE, SESSION_ID_SIZE);
        
        while (1) {
            int client_sock = accept(sock, NULL, NULL);
            if (client_sock < 0) {
                printf("Accept failed\n");
                continue;
            }
            
            init_session(&current_session);
            
            int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                printf("Received %d bytes\n", bytes_received);
                
                if (process_credentials(&current_session, buffer, bytes_received)) {
                    printf("Credentials processed\n");
                    printf("Username: %s\n", current_session.username);
                    printf("Session ID: %s\n", current_session.session_id);
                    
                    if (validate_session(&current_session)) {
                        printf("Session validated successfully\n");
                        current_session.is_authenticated = 1;
                    }
                }
            }
            
            close(client_sock);
        }
    } else {
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
            
            if (send(sock, buffer, strlen(buffer) + 1, 0) < 0) {
                printf("Send failed\n");
            } else {
                printf("Data sent successfully\n");
            }
        }
    }
    
    free(buffer);
    close(sock);
    
    return 0;
}

/*
To test it:

1. Compile with:
gcc -O0 -g -fno-stack-protector complex_cwe121.c -o complex_cwe121

2. Run server:
./complex_cwe121 server

3. Run client in another terminal:
./complex_cwe121

4. Enter a string longer than the smallest buffer size (16 bytes) to trigger the buffer overflow
*/ 