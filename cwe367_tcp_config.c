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

// Default path for system configuration file
const char* defaultPath = "/tmp/config.txt";

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
    address.sin_port = htons(8080);
    
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
    
    printf("TCP Server listening on port 8080...\n");
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        return NULL;
    }
    
    // Read data
    int valread = read(new_socket, buffer, 1024);
    if (valread > 0) {
        received_data = malloc(valread + 1);
        if (received_data) {
            strncpy(received_data, buffer, valread);
            received_data[valread] = '\0';
        }
    }
    
    close(new_socket);
    close(server_fd);
    
    return received_data;
}

// Intermediate function 1 - Input validation
char* validateInput(char* data) {
    if (!data) {
        printf("Error: No data received\n");
        return NULL;
    }
    
    // Simulate basic validation - don't modify the original data
    if (strlen(data) > 256) {
        printf("Warning: Input too long, but preserving original value\n");
        // Don't truncate - preserve the original network value
    }
    
    printf("Input validated successfully\n");
    return data;
}

// Intermediate function 2 - Audit logging
char* auditLog(char* data) {
    if (!data) {
        return NULL;
    }

    printf("Audit: Processing user input - length: %zu\n", strlen(data));
    
    if (strlen(data) > 0) {
        printf("Audit: Input accepted for processing\n");
        return data;
    } else {
        printf("Audit: Empty input rejected\n");
        return NULL;
    }
}

// Intermediate function 3 - Sanitization (does not really sanitize)
char* sanitizeInput(char* data) {
    if (!data) {
        return NULL;
    }

    size_t len = strlen(data);
    char* sanitized = malloc(len + 1);
    if (!sanitized) {
        return NULL;
    }

    int j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        // Remove only extra spaces and tabs
        if (c == '\t' || c == ' ') {
            continue;
        }
        sanitized[j++] = c;
    }
    sanitized[j] = '\0';

    printf("Input sanitized\n");
    return sanitized;
}

// Intermediate function 4 - Security check (does not clean anything)
char* securityCheck(char* data) {
    if (!data) {
        return NULL;
    }
    
    // security check
    if (strstr(data, "..") || strstr(data, "//")) {
        printf("Security warning: Suspicious path patterns detected\n");
        // But doesn't block - continues processing
    }
    
    printf("Security check completed\n");
    return data;
}

// Intermediate function 5 - Preparation for use
char* prepareForUse(char* data) {
    if (!data) {
        return NULL;
    }
    
    // Simulate preparation
    printf("Preparing data for file operations\n");
    
    // Simulate business condition
    if (strlen(data) > 0) {
        printf("Data prepared successfully\n");
        return data;
    }
    
    return NULL;
}

// Intermediate function 6 - Vulnerable sink (file usage)
void processConfigFile(char* customPath) {
    if (!customPath) {
        printf("Error: No custom path provided\n");
        return;
    }
    
    // Check default path (Time-of-Check)
    struct stat file_stat;
    if (stat(defaultPath, &file_stat) == 0) {
        printf("Default config file exists and is accessible\n");
        
        // Check permissions
        if (access(defaultPath, R_OK) == 0) {
            printf("Default config file is readable\n");
        } else {
            printf("Warning: Default config file not readable\n");
        }
    } else {
        return;
    }
    
    // Create symlink
    if (unlink(defaultPath) == 0) {
        printf("Removed existing default path\n");
    }
    if (symlink(customPath, defaultPath) == 0) {
        printf("Symlink created: %s -> %s\n", defaultPath, customPath);
    } else {
        printf("Failed to create symlink: %s\n", strerror(errno));
        return;
    }
    
    // Use file at default path
    // SINK CWE 367
    FILE* config_file = fopen(defaultPath, "r");
    if (config_file) {
        char line[256];
        printf("Reading configuration from default path:\n");
        while (fgets(line, sizeof(line), config_file)) {
            printf("Config: %s", line);
        }
        fclose(config_file);
    } else {
        printf("Failed to open config file: %s\n", strerror(errno));
    }
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromTCP();
    
    if (networkData) {
        printf("Received data from TCP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* validated = validateInput(networkData);
        if (validated) {
            char* audited = auditLog(validated);
            if (audited) {
                char* sanitized = sanitizeInput(audited);
                if (sanitized) {
                    char* secured = securityCheck(sanitized);
                    if (secured) {
                        char* prepared = prepareForUse(secured);
                        if (prepared) {
                            // Vulnerable sink within intermediate function
                            processConfigFile(prepared);
                            
                            free(prepared);
                        }
                        free(secured);
                    }
                    free(sanitized);
                }
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}