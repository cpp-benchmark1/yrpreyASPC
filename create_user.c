    #include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char username[20];
    char mkdir_command[50];

    // Checks if the expected argument (username) has been passed
    if (argc != 2) {
        printf("Use: %s username\n", argv[0]);
        return 1;
    }

    // Copies the argument to the username variable
    strncpy(username, argv[1], sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';  // Garante que username seja terminado corretamente

    // Constructs the command to create the directory with the username
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir %s", username);

    // Executes the mkdir command to create the directory
    printf("Executing command: %s\n", mkdir_command);
    int result = system(mkdir_command);

    // Checks if the directory was created successfully
    if (result == 0) {
        printf("Directory '%s' created successfully.\n", username);
    } else {
        printf("Error creating directory '%s'.\n", username);
        return 1;  // Exits the program in case of an error during directory creation
    }

    // Here you can continue the flow of your program as needed

    return 0;
}
