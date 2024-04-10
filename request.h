#include "helper.h"
#include "map.h"

#if defined(OS_WINDOWS)
#include <winsock2.h>
#elif defined(OS_UNIX)
#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif
#endif

typedef enum Method
{
    INVALID_METHOD,
    GET_METHOD = 100,
    HEAD_METHOD,
    POST_METHOD,
    PUT_METHOD,
    DELETE_METHOD,
    CONNECT_METHOD,
    OPTIONS_METHOD,
    TRACE_METHOD,
    PATCH_METHOD,
} Method;

typedef struct
{
    char *ip;
    int port;
    char *path;
    Method method;
    Map headers;
    char *body;
} Request;

Method getMethod(const char *method)
{
    if (strcmp(method, "GET") == 0)
        return GET_METHOD;
    else if (strcmp(method, "HEAD") == 0)
        return HEAD_METHOD;
    else if (strcmp(method, "POST") == 0)
        return POST_METHOD;
    else if (strcmp(method, "PUT") == 0)
        return PUT_METHOD;
    else if (strcmp(method, "DELETE") == 0)
        return DELETE;
    else if (strcmp(method, "CONNECT") == 0)
        return CONNECT_METHOD;
    else if (strcmp(method, "OPTIONS") == 0)
        return OPTIONS_METHOD;
    else if (strcmp(method, "TRACE") == 0)
        return TRACE_METHOD;
    else if (strcmp(method, "PATCH") == 0)
        return PATCH_METHOD;
    else
        return INVALID_METHOD;
}

const char *getMethodStr(Method method)
{
    switch (method)
    {
    case GET_METHOD:
        return "GET";
    case HEAD_METHOD:
        return "HEAD";
    case POST_METHOD:
        return "POST";
    case PUT_METHOD:
        return "PUT";
    case DELETE:
        return "DELETE";
    case CONNECT_METHOD:
        return "CONNECT";
    case OPTIONS_METHOD:
        return "OPTIONS";
    case TRACE_METHOD:
        return "TRACE";
    case PATCH_METHOD:
        return "PATCH";
    default:
        return NULL;
    }
}

Request *parseRequest(struct sockaddr_in client_addr, const char *raw_request)
{
    char *requestBuffer = strdup(raw_request);
    strs *slices = splitOnce((raw_request), "\r\n\r\n");
    char *statusLine_N_Headers = strdup(slices->data[0]);

    if (slices->length != 2)
    {
        free(slices);
        return NULL;
    }

    char *body = (strlen(trim(slices->data[1])) != 0) ? strdup(slices->data[1]) : NULL;
    free(slices);

    slices = splitOnce(statusLine_N_Headers, "\r\n");
    char *statusLine = strdup(slices->data[0]);
    char *headers = (slices->length == 2) ? strdup(slices->data[1]) : NULL;
    free(slices);

    slices = split(statusLine, " ");
    if (slices->length != 3)
    {
        free(slices);
        return NULL;
    }

    Request *newRequest = (Request *)malloc(sizeof(Request));

    newRequest->ip = strdup(inet_ntoa(client_addr.sin_addr));
    newRequest->port = ntohs(client_addr.sin_port);

    newRequest->method = getMethod(slices->data[0]);
    newRequest->path = strdup(slices->data[1]);
    free(slices);

    if (newRequest->method == INVALID_METHOD)
    {
        free(newRequest);
        return NULL;
    }

    if (headers != NULL)
    {
        strs *headerSlices;
        newRequest->headers = initMap();
        slices = split(headers, "\r\n");

        for (size_t i = 0; i < slices->length; i++)
        {
            if (strlen(slices->data[i]) == 0)
                continue;

            headerSlices = splitOnce(slices->data[i], ":");
            if (headerSlices->length == 2 && !(strlen(headerSlices->data[0]) == 0 || strlen(headerSlices->data[1]) == 0))
                newRequest->headers.set(&newRequest->headers, trim(headerSlices->data[0]), trim(headerSlices->data[1]));
            free(headerSlices);
        }
    }

    newRequest->body = body;

    return newRequest;
}
