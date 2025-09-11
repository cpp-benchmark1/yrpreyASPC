#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Import the dangerous gets function
char* gets(char*);

int main() {
    char buffer[256];
    
    printf("Enter environment variable value: ");
    fflush(stdout);
    
    // SINK CWE 242
    char* result = gets(buffer);
    
    if (result == NULL) {
        printf("Error reading input or EOF encountered\n");
        return 1;
    }
    
    printf("Environment data received: %s\n", buffer);
    
    // Save data directly to environment variable without any validation
    const char* env_var_name = "USER_CONFIG";
    
    printf("Setting environment variable: %s\n", env_var_name);
    
    int setenv_result = setenv(env_var_name, buffer, 1);
    
    if (setenv_result == 0) {
        printf("Environment variable set successfully\n");
        
        // Verify the environment variable was set
        char* env_value = getenv(env_var_name);
        if (env_value) {
            printf("Verification: %s = %s\n", env_var_name, env_value);
        } else {
            printf("Warning: Environment variable not found after setting\n");
        }
    } else {
        printf("Failed to set environment variable\n");
        return 1;
    }
    
    return 0;
}