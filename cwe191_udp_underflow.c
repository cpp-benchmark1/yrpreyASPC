#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// CWE-191: Integer Underflow
// Example 2: UDP network data converted to signed int for underflow without validation

// UDP network function - Source
char* receiveDataFromUDP() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    char* received_data = NULL;
    socklen_t len;
    int n;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return NULL;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8089);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8089...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// Intermediate function 1 - Credit system validation
char* validateCreditData(char* data) {
    if (!data) {
        printf("Error: No credit data to validate\n");
        return NULL;
    }
    
    // Check if data contains valid numeric characters for credits
    int has_digits = 0;
    int has_minus = 0;
    int has_plus = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] >= '0' && data[i] <= '9') {
            has_digits = 1;
        } else if (data[i] == '-' && i == 0) {
            has_minus = 1;
        } else if (data[i] == '+' && i == 0) {
            has_plus = 1;
        } else if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n') {
            printf("Warning: Invalid character '%c' in credit data\n", data[i]);
        }
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found in credit information\n");
        return NULL;
    }
    
    if (has_minus) {
        printf("Info: Credit reduction value detected\n");
    }
    
    if (has_plus) {
        printf("Info: Credit addition value detected\n");
    }
    
    printf("Credit data validation completed\n");
    return data;
}

// Intermediate function 2 - Credit balance calculation
char* calculateCreditBalance(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Calculating credit balance from data: %s\n", data);
    
    // Convert to int for balance calculations
    int credit_change = atoi(data);
    
    // Check if credit change is within reasonable bounds
    if (credit_change < -500) {
        printf("Warning: Large credit reduction (%d) detected\n", credit_change);
    }
    
    if (credit_change == 0) {
        printf("Warning: Zero credit change detected\n");
    }
    
    if (credit_change > 1000) {
        printf("Warning: Large credit addition (%d) detected\n", credit_change);
    }
    
    // Check if credit change is a multiple of common values
    if (credit_change % 10 == 0) {
        printf("Info: Credit change is a multiple of 10\n");
    }
    
    // Check for common credit change values
    if (credit_change == -100 || credit_change == -50 || credit_change == -25) {
        printf("Info: Common credit reduction value detected\n");
    }
    
    printf("Credit balance calculation completed\n");
    return data;
}

// Intermediate function 3 - Transaction processing
char* processTransaction(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Processing transaction with data: %s\n", data);
    
    // Convert to int for transaction processing
    int transaction_amount = atoi(data);
    
    // Transaction processing parameters
    int transaction_fee = 5;
    int processing_cost = 10;
    int system_overhead = 3;
    
    printf("Transaction parameters - Fee: %d, Cost: %d, Overhead: %d\n",
           transaction_fee, processing_cost, system_overhead);
    
    // Check if transaction amount is reasonable
    if (transaction_amount < -1000) {
        printf("Warning: Very large transaction reduction (%d) detected\n", transaction_amount);
    }
    
    if (transaction_amount > 10000) {
        printf("Warning: Very large transaction addition (%d) detected\n", transaction_amount);
    }
    
    // Check if transaction amount is within daily limits
    if (abs(transaction_amount) > 5000) {
        printf("Warning: Transaction amount exceeds daily limit\n");
    }
    
    printf("Transaction processing completed\n");
    return data;
}

int main() {
    // Receive data from network (Source)
    char* networkData = receiveDataFromUDP();
    int reduction_values[2];
    
    if (networkData) {
        printf("Received credit data from UDP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* validated = validateCreditData(networkData);
        if (validated) {
            char* processed = calculateCreditBalance(validated);
            if (processed) {
                int credit_reduction = atoi(processed);
                
                // User account balances
                int user_credits = 1000;
                int user_points = 500;
                int user_tokens = 200;
                
                printf("User account balances - Credits: %d, Points: %d, Tokens: %d\n",
                        user_credits, user_points, user_tokens);

                reduction_values[0] = credit_reduction;
                reduction_values[1] = 10;
                
                int new_credits = user_credits - reduction_values[1];
                // SINK CWE 191
                int new_points = user_points - reduction_values[0];
                int new_tokens = user_tokens - reduction_values[1];
                
                printf("New account balances - Credits: %d, Points: %d, Tokens: %d\n",
                        new_credits, new_points, new_tokens);
                
                // Check for potential underflow (but don't prevent it)
                if (new_credits < 0) {
                    printf("Warning: Credits underflow detected\n");
                }
                
                if (new_points < 0) {
                    printf("Warning: Points underflow detected\n");
                }
                
                if (new_tokens < 0) {
                    printf("Warning: Tokens underflow detected\n");
                }
                
                printf("Credit reduction processing completed\n");
            }
            
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}