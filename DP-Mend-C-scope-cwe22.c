#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <archive.h>
#include <archive_entry.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Simulate configuration loading or path manipulation
const char* resolve_path(const char* input) {
    static char resolved_path[512];
    snprintf(resolved_path, sizeof(resolved_path), "%s", input);  // No sanitization
    return resolved_path;
}

// Indirect wrapper to open gz file
gzFile open_gzip_file(const char* path) {
    // SINK
    return gzopen(path, "rb");
}

// Vulnerable function using Zlib with more flow
void vulnerable_zlib_extraction(const char* filename) {
    gzFile file;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    const char* final_path = resolve_path(filename);
    file = open_gzip_file(final_path);
    if (file == NULL) {
        printf("Failed to open file: %s\n", final_path);
        return;
    }

    while ((bytes_read = gzread(file, buffer, BUFFER_SIZE)) > 0) {
        printf("Read %d bytes from compressed file\n", bytes_read);
    }

    gzclose(file);
}

// Simulate path transformation logic
char* prepare_archive_path(const char* input) {
    // SOURCE: user-supplied archive path from socket
    return strdup(input);  // No sanitization
}

// Simulate archive processing step
void extract_entry(struct archive* a, struct archive* ext, struct archive_entry* entry) {
    // SINK
    int r = archive_write_header(ext, entry);
    if (r != ARCHIVE_OK) {
        printf("Failed to write header: %s\n", archive_error_string(ext));
    } else {
        const void *buff;
        size_t size;
        la_int64_t offset;

        while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
            archive_write_data_block(ext, buff, size, offset);
        }
    }
}

// Vulnerable function using Libarchive
void vulnerable_libarchive_extraction(const char* filename) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;

    char* archive_path = prepare_archive_path(filename);

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
            ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    archive_write_disk_set_options(ext, flags);

    if (archive_read_open_filename(a, archive_path, 10240)) {
        printf("Failed to open archive: %s\n", archive_error_string(a));
        free(archive_path);
        return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        extract_entry(a, ext, entry);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    free(archive_path);
}

// Main function: reads gz/tar paths from socket
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    // Setup TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 1);

    printf("Waiting for connection on port %d...\n", PORT);
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

    read(new_socket, buffer, sizeof(buffer) - 1);  // SOURCE: user-controlled input
    printf("Received: %s\n", buffer);

    // Expected format: "<gz_path>|<tar_path>"
    char* gz_path = strtok(buffer, "|");
    char* tar_path = strtok(NULL, "|");

    if (!gz_path || !tar_path) {
        printf("Invalid input format. Use: <gz_path>|<tar_path>\n");
        close(new_socket);
        close(server_fd);
        return 1;
    }

    printf("Testing Zlib extraction...\n");
    vulnerable_zlib_extraction(gz_path);

    printf("\nTesting Libarchive extraction...\n");
    vulnerable_libarchive_extraction(tar_path);

    close(new_socket);
    close(server_fd);
    return 0;
}

/*
===========================
 How to Test This Program
===========================

1. Compile the code:
   gcc DP-Mend-C-scope-cwe22.c -o DP-Mend-C-scope-cwe22 -lz -larchive

2. Create the test files:

   # Create a gzip file from /etc/passwd (simulating a sensitive file)
   gzip -c /etc/passwd > /app/passwd.gz

   # Create a tar file with a malicious entry to simulate path traversal
   echo "malicious" > /tmp/payload.txt
   tar --transform='s|payload.txt|../../malicious.txt|' -cf /app/traversal.tar -C /tmp payload.txt

3. Start the server:

   ./DP-Mend-C-scope-cwe22

   The program will listen on TCP port 8080, waiting for a connection.

4. In another terminal, connect to the server and send the file paths:

   echo -n "/app/passwd.gz|/app/traversal.tar" | nc localhost 8080

   NOTE: The `-n` flag in echo is important to avoid sending a newline,
         which would break the file path parsing.

5. Observe the output:

   You should see:
   - "Read XXX bytes from compressed file" (Zlib extraction)
   - Libarchive extracting without error

6. Verify the exploit worked:

   # Check if the malicious file was written outside the intended path
   cat /malicious.txt

   If the content is "malicious", path traversal via libarchive was successful.

===============================
 CWE-22: Path Traversal Summary
===============================

- Zlib Function:
  - Receives the path via socket input
  - Passes it to gzopen() without validation
  - Allows arbitrary file read (e.g., /etc/passwd.gz)

- Libarchive Function:
  - Receives tar filename via socket
  - Extracts files without checking destination paths
  - Allows traversal using ../../ in tar entry names

This code intentionally demonstrates vulnerable behavior for educational and security testing purposes only.
Do not use in production.
*/