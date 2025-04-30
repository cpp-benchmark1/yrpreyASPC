#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Funções vulneráveis que demonstram diferentes tipos de Format String vulnerabilities
void printfUnsafe(const char *userInput) {
    //SINK - Vulnerável a Format String
    printf(userInput);
}

void fprintfUnsafe(const char *userInput) {
    //SINK - Vulnerável a Format String
    fprintf(stdout, userInput);
}

void sprintfUnsafe(const char *userInput) {
    char buffer[100];
    //SINK - Vulnerável a Format String e Buffer Overflow
    sprintf(buffer, userInput);
}

int main() {
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = { 0 };

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                                &addrlen)) < 0) {
            perror("accept");
            continue;
        }

        // Read data from the socket
        valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0'; // Null-terminate the string
            printf("Received data: %s\n", buffer);
            
            // Demonstração das vulnerabilidades de Format String
            printf("\nTesting printf vulnerability:\n");
            printfUnsafe(buffer);
            
            printf("\n\nTesting fprintf vulnerability:\n");
            fprintfUnsafe(buffer);
            
            printf("\n\nTesting sprintf vulnerability:\n");
            sprintfUnsafe(buffer);
            printf("Buffer content: %s\n", buffer);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
