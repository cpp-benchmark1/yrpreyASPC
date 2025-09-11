#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int main() {
    // Fixed data to write to the file
    const char* hostname_data = "localhost\n";
    
    printf("Preparing to write hostname data: %s", hostname_data);
    
    const char* file_path = "/etc/hostname";
    
    printf("Creating file: %s\n", file_path);
    
    int perms[] = { 0666, 0600 };
    // SINK CWE 732
    int fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, perms[0]);
    if (fd == -1) {
        printf("Failed to create file: %s\n", strerror(errno));
        return 1;
    }
    
    // Write fixed data to file
    ssize_t bytes_written = write(fd, hostname_data, strlen(hostname_data));
    if (bytes_written == -1) {
        printf("Failed to write to file: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    printf("Successfully wrote %zd bytes to file\n", bytes_written);
    
    // Close file
    close(fd);

    const char* log_path = "/var/log/logs.log";
    int fdd = open(log_path, O_CREAT | O_WRONLY | O_TRUNC, perms[1]);
    if (fdd == -1) {
        printf("Failed to create file: %s\n", strerror(errno));
        return 1;
    }
    // Adding content to the log file
    ssize_t bytes_written_log = write(fdd, hostname_data, strlen(hostname_data));
    if (bytes_written_log == -1) {
        printf("Failed to write to file: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    // Close file
    close(fdd);

    return 0;
}