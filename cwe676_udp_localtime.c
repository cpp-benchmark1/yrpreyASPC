#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

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
    servaddr.sin_port = htons(8095);
    
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return NULL;
    }
    
    printf("UDP Server listening on port 8095...\n");
    
    len = sizeof(cliaddr);
    
    // Receive data
    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                 (struct sockaddr *) &cliaddr, &len);
    
    if (n > 0) {
        buffer[n] = '\0';
        received_data = malloc(n + 1);
        if (received_data != NULL) {
            strcpy(received_data, buffer);
        }
    }
    
    close(sockfd);
    return received_data;
}

// First intermediate function - parse schedule time
time_t parseScheduleTime(char* data) {
    if (data == NULL) {
        return 0;
    }
    
    // Convert string to time_t
    time_t schedule_time = (time_t)atol(data);
    
    // Some processing logic
    printf("Parsing schedule time: %ld\n", schedule_time);
    
    return schedule_time;
}

// Second intermediate function - validate schedule
time_t validateSchedule(time_t schedule_time) {
    // Some validation logic
    if (schedule_time > 0) {
        printf("Schedule time validated: %ld\n", schedule_time);
    } else {
        printf("Invalid schedule time\n");
    }
    
    return schedule_time;
}

int main() {
    char* network_data = NULL;
    time_t schedule_time = 0;
    time_t validated_time = 0;
    
    // Receive data from UDP network
    network_data = receiveDataFromUDP();
    
    if (network_data != NULL) {
        printf("Received network data: %s\n", network_data);
        
        // Process through first intermediate function
        schedule_time = parseScheduleTime(network_data);
        
        // Conditional flow based on schedule time
        if (schedule_time % 3 == 0) {
            // Call second intermediate function
            validated_time = validateSchedule(schedule_time);
        } else {
            printf("Schedule time not divisible by 3 - using original value\n");
            validated_time = schedule_time;
        }
        
        time_t t = validated_time;
        // SINK CWE 676
        struct tm* timeinfo = localtime(&t);
        
        if (timeinfo != NULL) {
            printf("Task scheduled for: %04d-%02d-%02d %02d:%02d:%02d\n", 
                   timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                   timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        } else {
            printf("Failed to schedule task\n");
        }
        
        free(network_data);
    } else {
        printf("Failed to receive network data\n");
    }
    
    return 0;
}