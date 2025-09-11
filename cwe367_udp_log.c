#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

// Default path for log/temporary data file
const char* defaultPath = "/tmp/processing.log";

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
    servaddr.sin_port = htons(8081);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8081...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// Intermediate function 1 - Format verification
char* checkFormat(char* data) {
    if (!data) {
        printf("Error: No data to check\n");
        return NULL;
    }

    if (strlen(data) == 0) {
        printf("Error: Empty data received\n");
        return NULL;
    }
    
    // Check if contains valid characters
    int valid_chars = 0;
    for (int i = 0; data[i]; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            valid_chars++;
        }
    }
    
    if (valid_chars < strlen(data) * 0.8) {
        printf("Warning: Data contains many non-printable characters\n");
    }
    
    printf("Format check passed\n");
    return data;
}

// Intermediate function 2 - Metadata processing
char* processMetadata(char* data) {
    if (!data) {
        return NULL;
    }
    
    // metadata processing
    time_t now = time(0);
    printf("Processing metadata at timestamp: %ld\n", now);
    
    if (strlen(data) > 10) {
        printf("Metadata: Large data packet detected\n");
        return data;
    } else {
        printf("Metadata: Small data packet, applying compression flag\n");
        return data;
    }
}

// Intermediate function 3 - Validation and preparation (does not really validate)
char* validateAndPrepare(char* data) {
    if (!data) return NULL;

    size_t len = strlen(data);
    char* out = malloc(len + 1);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = data[i];

        // Remove only extra spaces and tabs, not really validating
        if (c == ' ' || c == '\t' || c == '\n') {
            continue;
        }

        out[j++] = c;
    }

    out[j] = '\0';
    printf("Input validated and prepared\n");
    return out;
}


int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromUDP();
    
    if (networkData) {
        printf("Received data from UDP: %s\n", networkData);
        
        // Propagation flow - preserve original network value
        char* formatted = checkFormat(networkData);
        if (formatted) {
            char* metadata = processMetadata(formatted);
            if (metadata) {
                char* prepared = validateAndPrepare(metadata);
                if (prepared) {
                            // Check default path (Time-of-Check)
                            struct stat file_stat;
                            if (stat(defaultPath, &file_stat) == 0) {
                                printf("Default log file exists and is accessible\n");
                                
                                // Check permissions
                                if (access(defaultPath, W_OK) == 0) {
                                    printf("Default log file is writable\n");
                                } else {
                                    printf("Warning: Default log file not writable\n");
                                }
                            } else {
                                return 1;
                            }
                            
                            if (unlink(defaultPath) == 0) {
                                printf("Removed existing default path\n");
                            }
                            
                            if (symlink(prepared, defaultPath) == 0) {
                                printf("Symlink created: %s -> %s\n", defaultPath, prepared);
                            } else {
                                printf("Failed to create symlink: %s\n", strerror(errno));
                                free(prepared);
                                free(networkData);
                                return 1;
                            }
                            
                            // Use file at default path (now points to prepared)
                            // SINK CWE 367
                            FILE* log_file = fopen(defaultPath, "a");
                            if (log_file) {
                                printf("Writing to log file:\n");
                                fprintf(log_file, "%s", prepared);
                                fclose(log_file);
                                printf("Log entry written successfully\n");
                            } else {
                                printf("Failed to open log file: %s\n", strerror(errno));
                            }
                            
                            free(prepared);
                        }
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}