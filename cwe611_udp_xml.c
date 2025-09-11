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
#include <fcntl.h>

// UDP network function - Source
char* receiveDataFromUDP() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    char* received_data = NULL;
    socklen_t len;
    int n;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return NULL;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8099);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8099...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data != NULL) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// First intermediate function - parse data filename
char* parseDataFilename(char* data) {
    if (data == NULL) {
        return NULL;
    }
    
    // parsing logic
    printf("Parsing data filename: %s\n", data);
    
    // Check if filename has proper extension
    char* ext = strrchr(data, '.');
    if (ext != NULL && strcmp(ext, ".xml") == 0) {
        printf("Valid XML file extension found\n");
    } else {
        printf("Warning: No .xml extension found\n");
    }
    
    return data;
}

// intermediate function - prepare data processing
char* prepareDataProcessing(char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    
    // Some preparation logic
    printf("Preparing data processing for: %s\n", filename);
    
    // Check if file exists
    if (access(filename, F_OK) == 0) {
        printf("Data file exists and is accessible\n");
    } else {
        printf("Warning: Data file may not exist\n");
    }
    
    return filename;
}

int main() {
    char* network_data = NULL;
    char* parsed_filename = NULL;
    char* prepared_filename = NULL;
    
    // Initialize libxml2
    xmlInitParser();
    
    // Receive data from UDP network
    network_data = receiveDataFromUDP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process through first intermediate function
        parsed_filename = parseDataFilename(network_data);
        
        // Conditional flow based on filename
        if (parsed_filename != NULL && strlen(parsed_filename) < 50) {
            printf("Filename length < 50 - proceeding with preparation\n");
            // Call second intermediate function
            prepared_filename = prepareDataProcessing(parsed_filename);
        } else {
            printf("Filename too long - using original filename\n");
            prepared_filename = parsed_filename;
        }
        
        if (prepared_filename != NULL) {
            printf("Processing XML data file: %s\n", prepared_filename);
            
            // Open file descriptor
            int fd = open(prepared_filename, O_RDONLY);
            if (fd >= 0) {
                // SINK CWE 611
                xmlDocPtr doc = xmlReadFd(fd, prepared_filename, NULL, XML_PARSE_DTDLOAD | XML_PARSE_NOENT);
                
                if (doc == NULL) {
                    printf("Failed to parse XML data file: %s\n", prepared_filename);
                } else {
                    printf("Successfully parsed XML data file: %s\n", prepared_filename);
                    
                    // Process the XML document
                    xmlNodePtr root = xmlDocGetRootElement(doc);
                    if (root != NULL) {
                        printf("Root element: %s\n", root->name);
                    }
                    
                    xmlFreeDoc(doc);
                }
                
                close(fd);
            } else {
                printf("Failed to open data file: %s\n", prepared_filename);
            }
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    // Cleanup libxml2
    xmlCleanupParser();
    
    return 0;
}