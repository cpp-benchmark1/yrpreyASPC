#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 27018
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

/*
INSTRUCTIONS FOR TESTING USING DOCKER:

1. Create and enter the container:
   docker run -it --name ubuntu_projects -v "$PWD":/workspace -w /workspace ubuntu bash

2. Install dependencies:
   apt-get update
   apt-get install -y wget gnupg
   wget -qO - https://www.mongodb.org/static/pgp/server-7.0.asc | apt-key add -
   echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/7.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-7.0.list
   apt-get update
   apt-get install -y mongodb-org pkg-config libmongoc-dev libbson-dev netcat-traditional

3. Start MongoDB:
   mkdir -p /data/db
   mongod --dbpath /data/db --fork --logpath /var/log/mongodb/mongod.log

4. Compile the program:
   gcc create_user_cwe943.c -o create_user_cwe943 $(pkg-config --cflags --libs libmongoc-1.0)

5. Run the program:
   ./create_user_cwe943

6. In another terminal (using docker exec):
   docker exec -it ubuntu_projects bash

7. Test with payloads:
   # Normal test
   echo '{"username": "test", "password": "test123"}' | nc localhost 27018
   
   # NoSQL injection tests
   echo '{"$where": "function() { return true; }"}' | nc localhost 27018


*/
