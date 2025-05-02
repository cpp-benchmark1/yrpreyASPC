#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define USER_MAX 19

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char username[USER_MAX+1] = {0};
    char mkdir_cmd[50];

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) exit(EXIT_FAILURE);

    // Set socket options
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &(int){1}, sizeof(int));

    // Bind socket to port
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

    // Listen for a single connection
    listen(server_fd, 1);

    // Accept client connection
    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) exit(EXIT_FAILURE);

    ssize_t n = read(client_fd, username, USER_MAX);
    if (n < 0) exit(EXIT_FAILURE);
    username[n] = '\0';

    // Close connections
    close(client_fd);
    close(server_fd);

    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", username);
    printf("Executing: %s\n", mkdir_cmd);
    system(mkdir_cmd); 


    FILE *fp = fopen(username, "r");
    if (fp) {
        printf("Opened: %s\n", username);
        fclose(fp);
    }

    return 0;
}

/*
To test:

gcc create_user_cwe22.c -o create_user_cwe22
./create_user_cwe22

In another terminal:
echo "../../etc/passwd" | nc 127.0.0.1 8080

To test safely (e.g. in WSL):
echo "test" > /tmp/testfile.txt
echo "../../tmp/testfile.txt" | nc 127.0.0.1 8080
*/