#include <winsock2.h>
#include <unistd.h>
#include <pthread.h>
#include "helper.h"

#define PORT 3000

void handle_connection(int);

int main(int argc, char **argv)
{
    char *directory = ".";

    if (argc >= 3)
    {
        if (strcmp(argv[1], "--directory"))
            directory = argv[2];
    }
    printf("Setting up directory to '%s'\n", directory);

    if (chdir(directory) < 0)
    {
        printf("Failed to set current dir");
        return EXIT_FAILURE;
    }

    setbuf(stdout, NULL);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return EXIT_FAILURE;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == SOCKET_ERROR)
    {
        printf("Socket creation failed: %d...\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    char *ip = inet_ntoa(*(struct in_addr *)*gethostbyname("")->h_addr_list);
    struct sockaddr_in serv_add = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(ip),
        .sin_port = htons(PORT),
    };

    if (bind(server_fd, (struct sockaddr *)&serv_add, sizeof(serv_add)) == SOCKET_ERROR)
    {
        wprintf(L"bind failed with error %u\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return EXIT_FAILURE;
    }

    int connection_backlog = 5; // Maximum length of the queue of pending connections
    if (listen(server_fd, connection_backlog) != 0)
    {
        printf("Listen failed: %u \n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    printf("Listening on http://%s:%d\n", ip, PORT);

    while (1)
    {
        printf("Waiting for clients to connect...\n");

        struct sockaddr_in client_addr;            // Variable of type struct sockaddr_in to store the client address
        int client_addr_len = sizeof(client_addr); // Variable to store the length of the struct client_addr

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len); // Variable to represent the file descriptor (fd) of the client socket

        printf("client_fd: %d\n", client_fd);

        if (client_fd == SOCKET_ERROR)
        {
            printf("Failed to connect: %u \n", WSAGetLastError());
            return EXIT_FAILURE;
        }

        printf("Client connected\n");

        // If the current process is the child process
        // if (!fork())
        // {
        close(server_fd);
        handle_connection(client_fd);
        closesocket(client_fd);
        close(client_fd);
        // }
        // close(client_fd);
    }

    // closesocket(server_fd);
    // WSACleanup();

    return EXIT_SUCCESS;
}

void handle_connection(int client_fd)
{
    printf("Handle Connection\n");

    char readBuffer[1024] = {""};
    int bytesReceived = recv(client_fd, readBuffer, sizeof(readBuffer), 0);

    printf("\n\nData: %d\n", bytesReceived);
    // printf("\n\nData: \n'%s'\n\n", readBuffer);

    if (bytesReceived == SOCKET_ERROR)
    {
        printf("Receiving failed: %d \n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    const char *res = "HTTP/1.1 200 OK\r\n\r\n"; // HTTP response
    printf("Sending response: (%d) %s\n", strlen(res), res);
    send(client_fd, res, strlen(res), 0);
    return;

    char *method = strdup(readBuffer);  // "GET /some/path HTTP/1.1..."
    char *content = strdup(readBuffer); // "GET /some/path HTTP/1.1..."
    printf("Content: %s\n", content);
    method = strtok(method, " "); // GET POST PATCH and so on
    printf("Method: %s\n", method);

    // Extract the path -> "GET /some/path HTTP/1.1..."
    char *reqPath = strtok(readBuffer, " "); // -> "GET"
    printf("reqPath: %s", reqPath);
    reqPath = strtok(NULL, " "); // -> "/some/path"

    int bytesSent;

    /**
     * `send()` sends data on the client_fd socket.
     * If successful, returns 0 or greater indicating the number of bytes sent, otherwise
     * returns -1.
     */

    if (strcmp(reqPath, "/") == 0)
    {
        char *res = "HTTP/1.1 200 OK\r\n\r\n"; // HTTP response
        printf("Sending response: %s\n", res);
        bytesSent = send(client_fd, res, strlen(res), 0);
    }
    else if (strncmp(reqPath, "/echo/", 6) == 0)
    {
        // Parse the content
        reqPath = strtok(reqPath, "/"); // reqPath -> echo
        reqPath = strtok(NULL, "");     // reqPath -> foo/bar
        int contentLength = strlen(reqPath);

        char response[512];
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", contentLength, reqPath);
        printf("Sending response: %s\n", response);
        bytesSent = send(client_fd, response, strlen(response), 0);
    }
    else if (strcmp(reqPath, "/user-agent") == 0)
    {
        // Parse headers
        reqPath = strtok(NULL, "\r\n"); // reqPath -> HTTP/1.1
        reqPath = strtok(NULL, "\r\n"); // reqPath -> Host: 127.0.1:4221
        reqPath = strtok(NULL, "\r\n"); // reqPath -> User-Agent: curl/7.81.0

        // Parse the body
        char *body = strtok(reqPath, " "); // body -> User-Agent:
        body = strtok(NULL, " ");          // body -> curl/7.81.0
        int contentLength = strlen(body);

        char response[512];
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", contentLength, body);
        printf("Sending response: %s\n", response);
        bytesSent = send(client_fd, response, strlen(response), 0);
    }
    else if (strncmp(reqPath, "/files/", 7) == 0 && strcmp(method, "GET") == 0)
    {
        // Parse the file path
        char *filename = strtok(reqPath, "/");
        filename = strtok(NULL, "");

        // Open the file and check if the file exists
        FILE *fp = fopen(filename, "rb");
        if (!fp)
        {
            // If it doesn't exist, return 404
            printf("File not found");
            char *res = "HTTP/1.1 404 Not Found\r\n\r\n"; // HTTP response
            bytesSent = send(client_fd, res, strlen(res), 0);
        }
        else
        {
            printf("Opening file %s\n", filename);
        }

        // Read in binary and set the cursor at the end
        if (fseek(fp, 0, SEEK_END) < 0)
        {
            printf("Error reading the document\n");
        }

        // Get the size of the file
        size_t data_size = ftell(fp);

        // Rewind the cursor back
        rewind(fp);

        // Allocate enough memory for the contents
        void *data = malloc(data_size);

        // Fill in the content
        if (fread(data, 1, data_size, fp) != data_size)
        {
            printf("Error reading the document\n");
        }

        fclose(fp);

        // Return contents
        char response[1024];
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n%s", data_size, (char *)data);
        printf("Sending response: %s\n", response);
        bytesSent = send(client_fd, response, strlen(response), 0);
    }
    else if (strncmp(reqPath, "/files/", 7) == 0 && strcmp(method, "POST") == 0)
    {
        method = strtok(NULL, "\r\n"); // HTTP 1.1
        method = strtok(NULL, "\r\n"); // Content-Type
        method = strtok(NULL, "\r\n"); // User-Agent
        method = strtok(NULL, "\r\n"); // Accept: */*
        method = strtok(NULL, "\r\n"); // Content-Length: X

        char *contentLengthStr = strtok(method, " ");
        contentLengthStr = strtok(NULL, " ");

        int contentLength = atoi(contentLengthStr);

        // Parse the file path
        char *filename = strtok(reqPath, "/");
        filename = strtok(NULL, "");

        // Get the contents
        content = strtok(content, "\r\n"); // Content: POST /files/dumpty_yikes_dooby_237 HTTP/1.1
        printf("\n\n\nContent: %s\n\n\n", content);
        content = strtok(NULL, "\r\n"); // Host: localhost:4221
        content = strtok(NULL, "\r\n"); // User-Agent: curl/7.81.0
        content = strtok(NULL, "\r\n"); // Accept: */*
        content = strtok(NULL, "\r\n"); // Content-Length: 51
        printf("\n\n\nContent: %s\n\n\n", content);
        content = strtok(NULL, "\r\n"); // Content-Type: application/x-www-form-urlencoded
        printf("\n\n\nContent: %s\n\n\n", content);
        content = strtok(NULL, "\r\n"); // Content-Type: application/x-www-form-urlencoded
        printf("\n\n\nContent: %s\n\n\n", content);

        printf("\n---\nCreate a file %s with content length: %d\n\n %s\n---\n", filename, contentLength, content);

        // Open the file in write binary mode
        FILE *fp = fopen(filename, "wb");
        if (!fp)
        {
            // If the file could not be created/opened
            printf("File could not be opened");
            char *res = "HTTP/1.1 404 Not Found\r\n\r\n"; // HTTP response
            bytesSent = send(client_fd, res, strlen(res), 0);
        }
        else
        {
            printf("Opening file %s\n", filename);
        }

        // Write the contents
        if (fwrite(content, 1, contentLength, fp) != contentLength)
        {
            printf("Error writing the data");
        }

        fclose(fp);

        // Return contents
        // Return contents
        char response[1024];
        sprintf(response, "HTTP/1.1 201 Created\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n%s", contentLength, content);
        printf("Sending response: %s\n", response);
        bytesSent = send(client_fd, response, strlen(response), 0);
    }
    else
    {
        char *res = "HTTP/1.1 404 Not Found\r\n\r\n"; // HTTP response
        bytesSent = send(client_fd, res, strlen(res), 0);
    }

    if (bytesSent < 0)
    {
        printf("Send failed\n");
        exit(1);
    }
    else
    {
        return;
    }
}
