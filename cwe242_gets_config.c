#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

char* gets(char*);

int main() {
    char buffer[128];
    
    printf("Enter configuration data: ");
    fflush(stdout);
    
    // SINK CWE 242
    char* result = gets(buffer);
    
    if (result == NULL) {
        printf("Error reading input or EOF encountered\n");
        return 1;
    }
    
    printf("Configuration data received: %s\n", buffer);
    
    // Save configuration data to file
    const char* config_file = "config_default.txt";
    
    printf("Saving configuration to file: %s\n", config_file);
    
    // Open file for writing
    int fd = open(config_file, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fd == -1) {
        printf("Failed to create config file: %s\n", strerror(errno));
        return 1;
    }
    
    // Write configuration data to file
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written == -1) {
        printf("Failed to write to config file: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    // Add newline to file
    write(fd, "\n", 1);
    
    close(fd);
    
    
    return 0;
}