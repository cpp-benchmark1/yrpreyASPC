#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 27017
#define BUFFER_SIZE 1024

void mongoInsertUnsafe(const char* userInput) {
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    
    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017");
    collection = mongoc_client_get_collection(client, "test_db", "test_collection");
    
    bson_t* doc;
    doc = bson_new_from_json((const uint8_t*)userInput, -1, &error);
    
    if (!doc) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.message);
        return;
    }
    
    //SINK
    if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error)) {
        fprintf(stderr, "Insert failed: %s\n", error.message);
    } else {
        printf("Successfully inserted document: %s\n", userInput);
        
        // Print the inserted document in a readable format
        char* str = bson_as_canonical_extended_json(doc, NULL);
        printf("Inserted document details: %s\n", str);
        bson_free(str);
    }
    
    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
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
            
            // Vulnerable NoSQL injection point
            mongoInsertUnsafe(buffer);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
