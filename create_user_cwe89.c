#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 100

// Simulated database structure
typedef struct {
    int id;
    char username[50];
    char email[50];
} User;

// Simulated database
User users[] = {
    {1, "admin", "admin@example.com"},
    {2, "user1", "user1@example.com"},
    {3, "test", "test@example.com"},
    {0, "", ""}  // Terminator
};

// Function to simulate database query
void execute_query(const char* query) {
    
    // Simulate query parsing and execution
    char username[50] = {0};
    char condition[100] = {0};
    
    // Try to extract the WHERE condition
    if (sscanf(query, "SELECT * FROM users WHERE %[^;]", condition) == 1) {
        
        // Check if the condition contains OR (SQL injection)
        if (strstr(condition, "OR") != NULL) {
            // In a real scenario, this would execute the full query and return all results
            printf("Query results:\n");
            for (int i = 0; users[i].id != 0; i++) {
                printf("ID: %d, Username: %s, Email: %s\n", 
                       users[i].id, users[i].username, users[i].email);
            }
        } else {
            // Normal single user query
            if (sscanf(condition, "username = '%[^']'", username) == 1) {
                for (int i = 0; users[i].id != 0; i++) {
                    if (strcmp(users[i].username, username) == 0) {
                        printf("Query results:\n");
                        printf("ID: %d, Username: %s, Email: %s\n", 
                               users[i].id, users[i].username, users[i].email);
                        return;
                    }
                }
                printf("No results found\n");
            }
        }
    } else {
        printf("Invalid query format\n");
    }
}

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char username[BUFFER_SIZE] = {0};
    char query[BUFFER_SIZE * 2];

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
    ssize_t n = read(client_fd, username, BUFFER_SIZE - 1);
    if (n < 0) exit(EXIT_FAILURE);
    username[n] = '\0';

    close(client_fd);
    close(server_fd);

    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", username);
    
    execute_query(query);

    return 0;
}

/*
To test:

gcc create_user_cwe89.c -o create_user_cwe89
./create_user_cwe89

In another terminal:
echo "admin' OR '1'='1" | nc 127.0.0.1 8080
*/