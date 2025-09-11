#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

// TCP network function - Source
char* receiveDataFromTCP() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char* received_data = NULL;
    
    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return NULL;
    }
    
    // Configure socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8098);
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return NULL;
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }
    
    printf("TCP Server listening on port 8098...\n");
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        return NULL;
    }
    
    // Read data
    int valread = read(new_socket, buffer, 1024);
    if (valread > 0) {
        buffer[valread] = '\0';
        received_data = malloc(valread + 1);
        if (received_data != NULL) {
            strcpy(received_data, buffer);
        }
    }
    
    close(new_socket);
    close(server_fd);
    
    return received_data;
}

// First intermediate function - does not really validate the path
char* validateConfigRequest(char* data) {
    if (data == NULL) {
        return NULL;
    }
    
    // validation logic
    if (strlen(data) == 0) {
        printf("Error: Empty configuration filename\n");
        return NULL;
    }
    
    // Check if filename contains only valid characters
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] < 32 || data[i] > 126) {
            printf("Error: Invalid characters in filename\n");
            return NULL;
        }
    }
    
    printf("Configuration request validated: %s\n", data);
    return data;
}

// Second intermediate function - process configuration file 
void processConfigFile(char* filename) {
    if (filename == NULL) {
        return;
    }
    
    printf("Processing configuration file: %s\n", filename);
    
    // Create XML parser context
    xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        printf("Failed to create XML parser context\n");
        return;
    }
    
    // SINK CWE 611
    xmlDocPtr doc = xmlCtxtReadFile(ctxt, filename, NULL, XML_PARSE_DTDLOAD | XML_PARSE_NOENT);
    
    if (doc == NULL) {
        printf("Failed to parse XML configuration file: %s\n", filename);
    } else {
        printf("Successfully parsed XML configuration file: %s\n", filename);
        
        // Process the XML document
        xmlNodePtr root = xmlDocGetRootElement(doc);
        if (root != NULL) {
            printf("Root element: %s\n", root->name);
        }
        
        xmlFreeDoc(doc);
    }
    
    xmlFreeParserCtxt(ctxt);
}

int main() {
    char* network_data = NULL;
    char* validated_filename = NULL;
    
    // Initialize libxml2
    xmlInitParser();
    
    // Receive data from TCP network
    network_data = receiveDataFromTCP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process
        validated_filename = validateConfigRequest(network_data);
        
        // Conditional flow based on filename
        if (validated_filename != NULL && strlen(validated_filename) > 3) {
            printf("Filename length > 3 - proceeding with processing\n");
            // Call second intermediate function (sink)
            processConfigFile(validated_filename);
        } else {
            printf("Filename too short - skipping processing\n");
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    // Cleanup libxml2
    xmlCleanupParser();
    
    return 0;
}