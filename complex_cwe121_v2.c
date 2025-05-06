#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SMALL_BUFFER 16
#define MEDIUM_BUFFER 32
#define LARGE_BUFFER 64
#define MAX_FILES 5

typedef struct {
    char filename[SMALL_BUFFER];
    char content[MEDIUM_BUFFER];
    char metadata[LARGE_BUFFER];
    int file_size;
} FileEntry;

typedef struct {
    FileEntry files[MAX_FILES];
    int file_count;
    char config_path[SMALL_BUFFER];
} FileSystem;

void init_filesystem(FileSystem *fs) {
    memset(fs, 0, sizeof(FileSystem));
    strcpy(fs->config_path, "config.txt");
}

void process_file_data(FileSystem *fs, char *data, size_t len) {
    char temp_buffer[SMALL_BUFFER];
    bcopy(data, temp_buffer, len);
    
    if (fs->file_count < MAX_FILES) {
        bcopy(temp_buffer, fs->files[fs->file_count].filename, len);
        fs->file_count++;
    }
}

void load_config(FileSystem *fs) {
    char config_buffer[SMALL_BUFFER];
    FILE *config_file = fopen(fs->config_path, "r");
    
    if (config_file) {
        fscanf(config_file, "%s", config_buffer);
        bcopy(config_buffer, fs->config_path, strlen(config_buffer) + 1);
        fclose(config_file);
    }
}

void process_file_content(FileSystem *fs, int file_index) {
    if (file_index >= 0 && file_index < fs->file_count) {
        char content_buffer[MEDIUM_BUFFER];
        scanf("%s", content_buffer);
        bcopy(content_buffer, fs->files[file_index].content, strlen(content_buffer) + 1);
    }
}

int main(int argc, char *argv[]) {
    FileSystem fs;
    int sock;
    struct sockaddr_in addr;
    struct iovec iov[3];
    char buffer1[SMALL_BUFFER];
    char buffer2[MEDIUM_BUFFER];
    char buffer3[LARGE_BUFFER];
    
    init_filesystem(&fs);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Bind failed\n");
            close(sock);
            return 1;
        }
        
        if (listen(sock, 1) < 0) {
            printf("Listen failed\n");
            close(sock);
            return 1;
        }
        
        printf("Server listening on port %d...\n", PORT);
        printf("Buffer sizes: SMALL=%d, MEDIUM=%d, LARGE=%d bytes\n", 
               SMALL_BUFFER, MEDIUM_BUFFER, LARGE_BUFFER);
        
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("Accept failed\n");
            close(sock);
            return 1;
        }
        
        iov[0].iov_base = buffer1;
        iov[0].iov_len = SMALL_BUFFER;
        iov[1].iov_base = buffer2;
        iov[1].iov_len = MEDIUM_BUFFER;
        iov[2].iov_base = buffer3;
        iov[2].iov_len = LARGE_BUFFER;
        
        ssize_t bytes_read = readv(client_sock, iov, 3);
        if (bytes_read > 0) {
            printf("Read %zd bytes\n", bytes_read);
            
            process_file_data(&fs, buffer1, bytes_read);
            load_config(&fs);
            
            for (int i = 0; i < fs.file_count; i++) {
                printf("Processing file %d\n", i);
                process_file_content(&fs, i);
            }
        }
        
        close(client_sock);
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Connection failed\n");
            close(sock);
            return 1;
        }
        
        printf("Connected to server\n");
        printf("Enter your payload (max %d characters): ", BUFFER_SIZE - 1);
        
        char *buffer = (char *)malloc(BUFFER_SIZE);
        if (!buffer) {
            printf("Memory allocation failed\n");
            close(sock);
            return 1;
        }
        
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
            }
            
            if (send(sock, buffer, strlen(buffer) + 1, 0) < 0) {
                printf("Send failed\n");
            } else {
                printf("Data sent successfully\n");
            }
        }
        
        free(buffer);
    }
    
    close(sock);
    return 0;
}

/*
Testing Instructions:

1. Compile the program:
   gcc -O0 -g -fno-stack-protector complex_cwe121_v2.c -o complex_cwe121_v2

2. Create a config.txt file with a long string (>16 bytes):
   echo "ThisIsAVeryLongConfigStringThatWillCauseOverflow" > config.txt

3. Run the server in one terminal:
   ./complex_cwe121_v2 server

4. Run the client in another terminal:
   ./complex_cwe121_v2

5. Test different payload sizes to trigger various vulnerabilities:

   a) Buffer Overflow in readv (first buffer):
      Enter a string > 16 bytes to overflow buffer1
      Example: "ThisIsAVeryLongStringThatWillOverflowTheFirstBuffer"

   b) Buffer Overflow in fscanf (config file):
      The config.txt file should contain a string > 16 bytes
      This will trigger overflow in load_config()

   c) Buffer Overflow in scanf:
      When prompted for file content, enter a string > 32 bytes
      This will trigger overflow in process_file_content()

Expected Results:
- Sending data > 16 bytes will cause a segmentation fault
- The crash occurs because:
  1. readv scatters the data across three buffers
  2. The first buffer (16 bytes) overflows
  3. This corrupts the stack
  4. Subsequent operations (bcopy, scanf) may also overflow
  5. Eventually leads to a crash

Note: The exact crash point may vary depending on the payload size and system architecture.
*/ 