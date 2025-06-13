#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mbstring.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wchar.h>
#include <locale.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024
#define HEADER_SIZE 16
#define PAYLOAD_SIZE 32
#define MAX_PACKETS 5

typedef struct {
    char header[HEADER_SIZE];
    wchar_t payload[PAYLOAD_SIZE];
    int packet_type;
    int sequence;
} NetworkPacket;

typedef struct {
    NetworkPacket packets[MAX_PACKETS];
    int packet_count;
    char session_id[HEADER_SIZE];
} ProtocolHandler;

void init_handler(ProtocolHandler *handler) {
    memset(handler, 0, sizeof(ProtocolHandler));
    strcpy(handler->session_id, "SESSION_001");
}

void process_packet_header(ProtocolHandler *handler, char *data, size_t len) {
    char temp_buffer[HEADER_SIZE];
    printf("process_packet_header: Received length: %d, Buffer size: %d\n", len, HEADER_SIZE);
    _swab(data, temp_buffer, len);
    
    if (handler->packet_count < MAX_PACKETS) {
        strcpy(handler->packets[handler->packet_count].header, temp_buffer);
        printf("process_packet_header: Copied data: %s\n", handler->packets[handler->packet_count].header);
        handler->packet_count++;
    }
}

void process_packet_payload(ProtocolHandler *handler, char *data, size_t len) {
    wchar_t wide_buffer[PAYLOAD_SIZE];
    char temp_buffer[PAYLOAD_SIZE];
    
    printf("process_packet_payload: Received length: %d, Buffer size: %d\n", len, PAYLOAD_SIZE);
    _mbscat(temp_buffer, data);
    mbstowcs(wide_buffer, temp_buffer, PAYLOAD_SIZE);
    
    if (handler->packet_count < MAX_PACKETS) {
        wmemcpy(handler->packets[handler->packet_count].payload, wide_buffer, wcslen(wide_buffer));
        printf("process_packet_payload: Copied wide data length: %d\n", wcslen(wide_buffer));
        handler->packet_count++;
    }
}

void process_packet_sequence(ProtocolHandler *handler) {
    if (handler->packet_count < MAX_PACKETS) {
        wchar_t seq_input[PAYLOAD_SIZE];
        wscanf(L"%ls", seq_input);
        
        char seq_part[HEADER_SIZE];
        wcstombs(seq_part, seq_input, HEADER_SIZE);
        
        _mbscat(handler->packets[handler->packet_count].header, seq_part);
        handler->packet_count++;
    }
}

void load_session_config(ProtocolHandler *handler) {
    char config_buffer[HEADER_SIZE];
    FILE *config_file = fopen("session.cfg", "r");
    
    if (config_file) {
        fscanf(config_file, "%s", config_buffer);
        _mbscat(handler->session_id, config_buffer);
        fclose(config_file);
    }
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    struct sockaddr_in addr;
    setlocale(LC_ALL, "");
    ProtocolHandler handler;
    SOCKET sock;
    char *buffer = (char *)malloc(BUFFER_SIZE);
    WSADATA wsaData;
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        free(buffer);
        return 1;
    }
    
    init_handler(&handler);
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        free(buffer);
        WSACleanup();
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Bind failed\n");
            free(buffer);
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        if (listen(sock, 1) == SOCKET_ERROR) {
            printf("Listen failed\n");
            free(buffer);
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        printf("Server listening on port %d...\n", PORT);
        printf("Buffer sizes: HEADER=%d, PAYLOAD=%d bytes\n", 
               HEADER_SIZE, PAYLOAD_SIZE);

        SOCKET client_sock = accept(sock, NULL, NULL);
        if (client_sock == INVALID_SOCKET) {
            printf("Accept failed\n");
            free(buffer);
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("Received %d bytes\n", bytes_received);
            
            process_packet_header(&handler, buffer, bytes_received);
            process_packet_payload(&handler, buffer, bytes_received);
            load_session_config(&handler);
            
            for (int i = 0; i < handler.packet_count; i++) {
                printf("Processing packet %d\n", i);
                process_packet_sequence(&handler);
            }
        }

        closesocket(client_sock);
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Connection failed\n");
            free(buffer);
            closesocket(sock);
            WSACleanup();
            return 1;
        }
        
        printf("Connected to server\n");
        
        char value[100] = "HEADER_OVERFLOW_";  // 16 bytes
        strcat(value, "PAYLOAD_OVERFLOW_");    // + 32 bytes
        strcat(value, "EXTRA_OVERFLOW_DATA");  // + remaining bytes
        
        printf("Sending pattern:\n");
        printf("First 16 bytes: %.*s\n", 16, value);
        printf("Next 32 bytes: %.*s\n", 32, value + 16);
        printf("Remaining bytes: %s\n", value + 48);
        printf("Total length: %d bytes\n", strlen(value));
        
        if (send(sock, value, strlen(value) + 1, 0) == SOCKET_ERROR) {
            printf("Send failed\n");
        } else {
            printf("Data sent successfully\n");
        }
    }
    
    free(buffer);
    closesocket(sock);
    WSACleanup();
#endif
    return 0;
}
