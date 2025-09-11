#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

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
    servaddr.sin_port = htons(8091);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8091...\n");
    
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

// Intermediate function 1 - Ranking system validation
char* validateRankingData(char* data) {
    if (!data) {
        printf("Error: No ranking data to validate\n");
        return NULL;
    }
    
    // Check if data contains valid numeric characters for points
    int has_digits = 0;
    int has_minus = 0;
    
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] >= '0' && data[i] <= '9') {
            has_digits = 1;
        } else if (data[i] == '-' && i == 0) {
            has_minus = 1;
        } else if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n') {
            printf("Warning: Invalid character '%c' in ranking data\n", data[i]);
        }
    }
    
    if (!has_digits) {
        printf("Error: No numeric data found in ranking information\n");
        return NULL;
    }
    
    if (has_minus) {
        printf("Info: Negative ranking points detected\n");
    }
    
    // Check if ranking points are within reasonable bounds
    int ranking_points = atoi(data);
    if (ranking_points < -100000) {
        printf("Warning: Very negative ranking points (%d) detected\n", ranking_points);
    }
    
    if (ranking_points > 100000) {
        printf("Warning: Large positive ranking points (%d) detected\n", ranking_points);
    }
    
    // Check if ranking points are multiples of common values
    if (ranking_points % 10 == 0) {
        printf("Info: Ranking points are a multiple of 10\n");
    }
    
    printf("Ranking data validation completed\n");
    return data;
}

// Intermediate function 2 - Points calculation
char* calculatePoints(char* data) {
    if (!data) {
        return NULL;
    }
    
    printf("Calculating points from ranking data: %s\n", data);
    
    // Convert to int for points calculations
    int ranking_points = atoi(data);
    
    // Base point values for different categories
    int base_bonus_points = 100;
    int base_achievement_points = 50;
    int base_level_points = 25;
    
    printf("Base point values - Bonus: %d, Achievement: %d, Level: %d\n",
           base_bonus_points, base_achievement_points, base_level_points);
    
    // Check if ranking points are within processing limits
    if (ranking_points < -50000) {
        printf("Warning: Very negative ranking points (%d) may cause calculation issues\n", ranking_points);
    }
    
    if (ranking_points > 50000) {
        printf("Warning: Very large ranking points (%d) may cause calculation issues\n", ranking_points);
    }
    
    printf("Points calculation completed\n");
    return data;
}

int main() {    
    // Receive data from network (Source)
    char* networkData = receiveDataFromUDP();
    int points_values[2];
    
    if (networkData) {
        printf("Received ranking data from UDP: %s\n", networkData);
        
        // Propagation flow through intermediate functions
        char* validated = validateRankingData(networkData);
        if (validated) {
            char* calculated = calculatePoints(validated);
            if (calculated) {
                
                // Convert string to signed int
                int ranking_points = atoi(calculated);
                points_values[0] = ranking_points;
                points_values[1] = 1;
                
                // User's current ranking data
                int current_score = 1000;
                int current_level = 10;
                int current_achievements = 5;
                
                printf("Current user ranking - Score: %d, Level: %d, Achievements: %d\n",
                       current_score, current_level, current_achievements);
                
                // SINK CWE 190
                int total_score = points_values[0] * current_score;
                // safe multiplication
                int total_level = current_level * points_values[1];
                int total_achievements = current_achievements * points_values[1];
                
                printf("Calculated totals - Score: %d, Level: %d, Achievements: %d\n",
                       total_score, total_level, total_achievements);
                       
                // Check for potential overflow (but don't prevent it)
                if (total_score < 0) {
                    printf("Warning: Score calculation overflow detected\n");
                }
                
                if (total_level < 0) {
                    printf("Warning: Level calculation overflow detected\n");
                }
                
                if (total_achievements < 0) {
                    printf("Warning: Achievements calculation overflow detected\n");
                }
                
                printf("Ranking system processing completed\n");
            }
        }
        
        free(networkData);
    } else {
        printf("No data received from network\n");
    }
    
    return 0;
}