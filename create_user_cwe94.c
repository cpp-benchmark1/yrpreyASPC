#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef void (*function_ptr)();

// Função vulnerável que permite injeção de código
void dlsymUnsafe(const char *userInput) {
    void *handle;
    function_ptr func;
    
    // Carrega a biblioteca dinamicamente usando o input do usuário
    handle = dlopen(userInput, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }

    //SINK - Ponto de injeção de código
    func = (function_ptr)dlsym(handle, "some_function");
    
    if (func) {
        printf("Executing function pointer from dlsym...\n");
        func(); // Executa o código potencialmente malicioso
    }
    
    dlclose(handle);
}

// Função segura com biblioteca fixa
void dlsymSafe() {
    void *handle;
    function_ptr func;
    const char *safe_lib = "./libtest.so";
    
    handle = dlopen(safe_lib, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }

    //SAFE_SINK - Usa uma biblioteca conhecida e segura
    func = (function_ptr)dlsym(handle, "some_function");
    
    if (func) {
        printf("Executing function pointer from dlsym...\n");
        func();
    }
    
    dlclose(handle);
}

int main() {
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = { 0 };

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                                &addrlen)) < 0) {
            perror("accept");
            continue;
        }

        // Read data from the socket
        valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0'; // Null-terminate the string
            printf("Received data: %s\n", buffer);
            
            // Demonstração da vulnerabilidade de injeção de código
            printf("\nTesting code injection vulnerability:\n");
            dlsymUnsafe(buffer);
            
            // Demonstração do uso seguro
            printf("\nTesting safe function:\n");
            dlsymSafe();
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
