#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SMALL_BUFFER_SIZE 64
#define SOCKET_BUFFER_SIZE 1024

void process_data(char *data, int size) {
    char buffer[SMALL_BUFFER_SIZE];  // stack buffer

    // 💥 Buffer overflow if size > SMALL_BUFFER_SIZE
    // SINK
    memcpy(buffer, data, size);

    printf("Data copied to buffer: %.10s...\n", buffer);

    // Optional malloc to trigger runtime error
    void *new_ptr = malloc(128);
    (void)new_ptr;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in addr;
    char *recv_buffer = (char *)malloc(SOCKET_BUFFER_SIZE);
    int is_server = 0;

    if (!recv_buffer) {
        printf("Memory allocation failed\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        free(recv_buffer);
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        is_server = 1;
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Bind failed\n");
            free(recv_buffer);
            close(sock);
            return 1;
        }

        if (listen(sock, 1) < 0) {
            printf("Listen failed\n");
            free(recv_buffer);
            close(sock);
            return 1;
        }

        printf("Server listening on port %d...\n", PORT);

        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("Accept failed\n");
            free(recv_buffer);
            close(sock);
            return 1;
        }

        // SOURCE
        int bytes_received = read(client_sock, recv_buffer, SOCKET_BUFFER_SIZE);
        if (bytes_received > 0) {
            printf("Received %d bytes\n", bytes_received);
            process_data(recv_buffer, bytes_received);
        }

        close(client_sock);
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Connection failed\n");
            free(recv_buffer);
            close(sock);
            return 1;
        }

        printf("Connected to server\n");
        memset(recv_buffer, 'A', SOCKET_BUFFER_SIZE);

        if (write(sock, recv_buffer, SOCKET_BUFFER_SIZE) < 0) {
            printf("Send failed\n");
        } else {
            printf("Data sent successfully\n");
        }
    }

    free(recv_buffer);
    close(sock);
    return 0;
}


/*
To test it on Linux:

gcc -g stack_overflow_socket_cwe787.c -o stack_overflow_socket_cwe787
./stack_overflow_socket_cwe787 server


another terminal for the client:
./stack_overflow_socket_cwe787

On MacOS I recommend: 
clang -fsanitize=address -g stack_overflow_socket_cwe787.c -o stack_overflow_socket_cwe787
./stack_overflow_socket_cwe787 server

another terminal for the client:
./stack_overflow_socket_cwe787
*/