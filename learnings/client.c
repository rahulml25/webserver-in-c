#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>

#define PORT 3000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // Starting wsa
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("Error starting WSA");
        exit(1);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Set up server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    char *ip = inet_ntoa(*(struct in_addr *)*gethostbyname("")->h_addr_list);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(ip); // INADDR_ANY;

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to server");
        exit(1);
    }
    printf("Connected to server\n");

    send(sockfd, "GET / HTTP/1.1\r\n\r\n", 19, 0);

    // Receive data
    ssize_t total_bytes_received = 0;
    ssize_t bytes_received;
    do
    {
        bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            perror("Error reading from socket");
            exit(1);
        }
        total_bytes_received += bytes_received;
        // Process or store received data as needed
        // Here, you can print or save the received data
        fwrite(buffer, 1, bytes_received, stdout);
        fwrite(buffer, 1, bytes_received, stderr);

    } while (bytes_received > 0);

    printf("\nTotal bytes received: %zd\n", total_bytes_received);

    // Close socket
    close(sockfd);

    return 0;
}
