#define _DEFAULT_SOURCE

#if defined(_WIN32)
#define OS_WINDOWS
#elif defined(__linux__) || defined(__APPLE__)
#define OS_UNIX
#else
#define OS_UNKNOWN
#endif

#include <stdio.h>
#if defined(OS_WINDOWS)
#include <winsock2.h>
#elif defined(OS_UNIX)
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bits/types.h>
#include <sys/select.h>
#include <unistd.h>
#endif
#include <pthread.h>
#include "helper.h"
#include "request.h"

#if defined(OS_UNIX)
#define SOCKET_ERROR -1
#define closesocket close

typedef int SOCKET;
typedef __u_short u_short;
#elif defined(OS_WINDOWS)
typedef int socklen_t;
typedef unsigned char u_int8_t;
#endif

typedef struct
{
    char *ip;
    int port;
    int (*handler)(int client_fd, struct sockaddr_in client_addr);
    void (*onStart)(char *ip, int port);
} SocketThreadArgs;

#define PORT 3000

void exit_thread(int code);
void *socket_thread(void *arg);
int handle_connection(int client_fd, struct sockaddr_in client_addr);
char *handle_request(Request req);

static u_int8_t listening_count = 0;

void onStart(char *ip, int port)
{
    printf("Listening on http://%s:%d\n", ip, port);
    if (++listening_count == 2)
    {
        printf("\n");
    }
}

int main()
{
    pthread_t thread1, thread2;
    int returnCode = 0;

    {
        // #if defined(OS_WINDOWS)
        //         printf("This is Windows.\n");
        // #elif defined(__linux__)
        //         printf("This is Linux.\n");
        // #elif defined(__APPLE__)
        //         printf("This is macOS.\n");
        // #else
        //         fprintf(stderr, "Unknown operating system.\n");
        //         goto EXIT_POINT;
        // #endif

#if defined(OS_WINDOWS)
        printf("This is Windows.\n");
#elif defined(OS_UNIX)
        printf("This is UNIX.\n");
#else
        fprintf(stderr, "Platform is Not Supported.\n");
        goto EXIT_POINT;
#endif
    }

#if defined(OS_WINDOWS)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        returnCode = EXIT_FAILURE;
        goto EXIT_POINT;
    }
#endif

    SocketThreadArgs socket_args_1 = {
#if defined(OS_WINDOWS)
        .ip = inet_ntoa(*(struct in_addr *)*gethostbyname("")->h_addr_list),
#elif defined(OS_UNIX)
        .ip = getIP("eth0"),
#endif
        .port = PORT,
        .handler = handle_connection,
        .onStart = onStart,
    };
    SocketThreadArgs socket_args_2 = {
        .ip = "127.0.0.1",
        .port = socket_args_1.port,
        .handler = socket_args_1.handler,
        .onStart = socket_args_1.onStart,
    };

    // Create the first thread
    if (pthread_create(&thread1, NULL, socket_thread, (void *)&socket_args_1) != 0)
    {
        perror("pthread_create");
        returnCode = EXIT_FAILURE;
        goto EXIT_POINT;
    }

    // Create the second thread
    if (pthread_create(&thread2, NULL, socket_thread, (void *)&socket_args_2) != 0)
    {
        perror("pthread_create");
        returnCode = EXIT_FAILURE;
        goto EXIT_POINT;
    }

    // Wait for both threads to finish
    pthread_join(thread1, (void *)(&returnCode));
    pthread_join(thread2, (void *)(&returnCode));

EXIT_POINT:
#if defined(OS_WINDOWS)
    WSACleanup();
#endif
    return (returnCode != EXIT_SUCCESS) ? EXIT_FAILURE : EXIT_SUCCESS;
}

void exit_thread(int code)
{
    pthread_exit((void *)&code);
};

void *socket_thread(void *arg)
{
    SocketThreadArgs *args = (SocketThreadArgs *)arg;
    int returnCode = 0;

    struct sockaddr_in server_add = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(args->ip),
        .sin_port = htons(args->port),
    };

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == SOCKET_ERROR)
    {
        psockerr("Socket creation failed\n");
        exit_thread(EXIT_FAILURE);
    }

    // Re-using IP address & PORT number
    int optValue = 1;
#if defined(OS_UNIX)
    int optName = SO_REUSEADDR | SO_REUSEPORT;
#elif defined(OS_WINDOWS)
    int optName = SO_REUSEADDR;
#endif
    if (setsockopt(server_fd, SOL_SOCKET, optName, (const char *)&optValue, sizeof(optValue)))
    {
        perror("setsockopt");
        closesocket(server_fd);
        exit_thread(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&server_add, sizeof(server_add)) == SOCKET_ERROR)
    {
        psockerr("Failed to Bind with error\n");
        closesocket(server_fd);
        exit_thread(EXIT_FAILURE);
    }

    int connection_backlog = 10; // Maximum length of the queue of pending connections
    if (listen(server_fd, connection_backlog) != 0)
    {
        psockerr("Failed to Listen failed\n");
        closesocket(server_fd);
        exit_thread(EXIT_FAILURE);
    }

    args->onStart(inet_ntoa(server_add.sin_addr), ntohs(server_add.sin_port));

    while (1)
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        if (client_fd == SOCKET_ERROR)
        {
            psockerr("Failed to connect\n");
            exit_thread(EXIT_FAILURE);
        }

        // Receive Timeout set to - 5 seconds
        struct timeval recv_timeout = {
            recv_timeout.tv_sec = 5,
            recv_timeout.tv_usec = 0,
        };
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&recv_timeout, sizeof(recv_timeout)) < 0)
        {
            perror("setsockopt");
            closesocket(server_fd);
            exit_thread(EXIT_FAILURE);
        }

        // char host_url[1024] = {""};
        // sprintf(host_url, "http://%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // printf("\n>> %s\n", host_url);

        // Handling Request
        returnCode = args->handler(client_fd, client_addr);
        closesocket(client_fd);

        // printf("<< %s\n\n", host_url);

        if (returnCode != 0)
            break;
    }

    closesocket(server_fd);
    exit_thread(returnCode);
    return NULL;
}

int handle_connection(int client_fd, struct sockaddr_in client_addr)
{
#define BUFFER_SIZE 1024

    ssize_t bytesReceived = 0;
    size_t totalBytesReceived = 0;
    u_short recv_count = 1;

    char *readBuffer = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    char *requestBuffer = (char *)calloc(BUFFER_SIZE, sizeof(char));

    do
    {
        if (recv_count > 0)
            requestBuffer = (char *)realloc(requestBuffer, sizeof(char) * (BUFFER_SIZE * recv_count));

        bytesReceived = recv(client_fd, readBuffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            psockerr("Receiving failed: %d byte(s) recived.\n\n", totalBytesReceived);
            return EXIT_SUCCESS;
        }

        strcat(requestBuffer, readBuffer);
        totalBytesReceived += bytesReceived;
        recv_count++;

    } while (bytesReceived >= BUFFER_SIZE);

    // printf("\nReceived request: (%zu - bytes)\n'%s'\n\n", totalBytesReceived, requestBuffer);

    Request *request = parseRequest(client_addr, requestBuffer);
    char *responseBuffer;

    if (request == NULL)
    {
        responseBuffer = strdup("HTTP/1.1 400 BAD REQUEST\r\n\r\n");
        goto SEND_POINT;
    }
    printf("%s %s [received: %zu byte(s)]\n", getMethodStr(request->method), request->path, totalBytesReceived);

    responseBuffer = handle_request(*request);

SEND_POINT:
    printf("'%s'\n", responseBuffer);

    size_t responseLen = strlen(responseBuffer);
    ssize_t bytesSent = send(client_fd, responseBuffer, responseLen, 0);
    if (bytesSent == SOCKET_ERROR)
    {
        psockerr("Receiving failed\n");
        return EXIT_FAILURE;
    }
    printf("-> [sent: %zu byte(s)]\n\n", responseLen);

    free(readBuffer);
    free(requestBuffer);
    free(request);
    free(responseBuffer);
    return EXIT_SUCCESS;
}

char *handle_request(Request req)
{
    char *responseBuffer;

    printf("Request Method: %s\n", getMethodStr(req.method));

    if (strcmp(req.path, "/") == 0)
    {
        switch (req.method)
        {
        case GET_METHOD:
        {
            const char *data = "Hello World!\n";
            responseBuffer = fstring("HTTP/1.1 200 OK\r\n\r\n%s", data);
            break;
        }

        case OPTIONS_METHOD:
        {
            const char *data = "[\"GET\", \"OPTIONS\"]\n";
            const char *headers = "\r\ncontent-type: application/json";
            responseBuffer = fstring("HTTP/1.1 200 OK%s\r\n\r\n%s", headers, data);
            break;
        }

        default:
            responseBuffer = fstring("HTTP/1.1 405 METHOD NOT ALLOWED\r\n\r\n");
            break;
        }
    }
    else
    {
        responseBuffer = fstring("HTTP/1.1 404 NOT FOUND\r\n\r\n");
    }

    return responseBuffer;
}
