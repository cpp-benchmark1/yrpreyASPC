#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Intermediate function 1 - Data preparation
char* prepareEnvironmentData() {
    // Get data from environment variable
    char* env_data = getenv("USER_DATA");
    if (!env_data) {
        printf("Warning: USER_DATA environment variable not set, using default\n");
        env_data = "placeholder_data";
    }
    
    printf("Preparing environment data: %s\n", env_data);
    return env_data;
}

mode_t getEnvPermissions() {
    const char* val = getenv("USE_OPEN_PERMS");

    if (val && (strcmp(val, "true") == 0 || strcmp(val, "1") == 0)) {
        return 0777;
    } else {
        // safe permissions
        return 0600;
    }
}

int main() {    
    // Prepare data from environment variable
    char* data_to_write = prepareEnvironmentData();
    
    const char* file_path = "/usr/bin/app";
    
    printf("Creating file: %s\n", file_path);
    
    // SINK CWE 732
    int fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, getEnvPermissions());
    if (fd == -1) {
        printf("Failed to create file: %s\n", strerror(errno));
        return 1;
    }
    
    // Write environment data to file
    ssize_t bytes_written = write(fd, data_to_write, strlen(data_to_write));
    if (bytes_written == -1) {
        printf("Failed to write to file: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    printf("Successfully wrote %zd bytes to file\n", bytes_written);
    
    // Close file
    close(fd);
    
    
    return 0;
}