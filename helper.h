
#ifndef _helper_H
#define _helper_H

#ifndef _INC_STDIO
#include <stdio.h>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef __STDARG_H
#include <stdarg.h>
#endif
#ifndef _INC_STRING
#include <string.h>
#endif
#ifndef _MALLOC_H_
#include <malloc.h>
#endif
#ifndef _CTYPE_H
#include <ctype.h>
#endif

#if defined(OS_UNIX) && !not_networking
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

typedef struct strs
{
    char **data;
    size_t length;
} strs;

#if defined(OS_WINDOWS)
typedef unsigned long long int ull_int;
#endif

/// @brief Splites given `string`, on base of `delimiter`
/// @param string const char * @param delimiter const char *
/// @return `strs *`
strs *split(const char *string, const char *delimiter);

/// @brief Splites given `string` into two parts, on base of `delimiter`
/// @param string const char * @param delimiter const char *
/// @return `strs *`
strs *splitOnce(const char *string, const char *delimiter);

/// @brief Copies given `string` from `start` to `end`
/// @param string const char * @param start size_t @param end size_t
/// @return char *
char *copy_str(const char *string, size_t start, size_t end);
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#if defined(OS_WINDOWS)
char *strndup(const char *s, ull_int n);
#endif

#if defined(OS_UNIX) && !not_networking
char *getIP(const char *interface_name);
#endif

strs *splitOnce(const char *string, const char *delimiter)
{
    char *result = strstr(string, delimiter);
    strs *slices_result = (strs *)malloc(sizeof(strs));

    if (result == NULL)
    {
        slices_result->data = (char **)malloc(sizeof(char *) * 1),
        slices_result->data[0] = strdup(string),
        slices_result->length = 1;

        return slices_result;
    }

    size_t delimiter_pos = result - string, delimiterLen = strlen(delimiter);

    slices_result->data = (char **)malloc(sizeof(char *) * 2),
    slices_result->length = 2;

    slices_result->data[0] = strndup(string, delimiter_pos),
    slices_result->data[1] = copy_str(string, delimiter_pos + delimiterLen, strlen(string));

    return slices_result;
}

strs *split(const char *string, const char *delimiter)
{
    char **slices, *result = NULL;
    size_t stringLen = strlen(string), delimiterLen = strlen(delimiter);
    size_t slices_i = 0, last_pos = 0, curr_pos = 0;

    while (1)
    {
        (slices_i > 0)
            ? (slices = (char **)realloc(slices, sizeof(char *) * (slices_i + 1)))
            : (slices = (char **)malloc(sizeof(char *) * (slices_i + 1)));

        result = strstr(
            ((slices_i > 0) ? (string + curr_pos + delimiterLen) : string),
            delimiter);

        curr_pos = result - string;
        size_t start = (slices_i > 0) ? last_pos + delimiterLen : 0;

        if (result == NULL || last_pos == curr_pos)
        {
            // slices[slices_i] = (char *)malloc(sizeof(char) * stringLen - start + 1);
            slices[slices_i] = copy_str(string, start, stringLen);

            slices_i++;
            break;
        }

        // slices[slices_i] = (char *)malloc(sizeof(char) * (curr_pos - start + 1));
        slices[slices_i] = copy_str(string, start, curr_pos);

        last_pos = curr_pos;
        slices_i++;
    }

    strs *slices_result = (strs *)malloc(sizeof(strs));
    slices_result->data = slices,
    slices_result->length = slices_i;

    return slices_result;
}

char *copy_str(const char *string, size_t start, size_t end)
{
    size_t _ = strlen(string), __ = (end - start);
    const size_t len = (__ < _) ? __ : _;
    char *newStr = (char *)malloc(sizeof(char) * (len + 1));

    for (size_t i = 0; i < len; i++)
    {
        newStr[i] = string[start + i];
    }
    newStr[len] = '\0';

    return newStr;
}

char *fstring(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Determine the length of the formatted string
    int length = vsnprintf(NULL, 0, format, args);
    if (length < 0)
    {
        // Handle error
        va_end(args);
        return NULL;
    }

    // Allocate memory for the formatted string
    char *buffer = (char *)calloc(length + 1, sizeof(char)); // +1 for the null terminator
    if (buffer == NULL)
    {
        // Handle allocation failure
        va_end(args);
        return NULL;
    }

    // Format the string into the buffer
    va_start(args, format);
    vsnprintf(buffer, length + 1, format, args);
    va_end(args);

    return buffer;
}

#if defined(OS_WINDOWS)
char *strndup(const char *s, ull_int n)
{
    char *result;
    int len = strlen(s);

    if (n < len)
        len = n;

    result = (char *)malloc(sizeof(char) * (len + 1));
    if (!result)
        return 0;

    result[len] = '\0';
    return (char *)memcpy(result, s, len);
}
#endif

#if defined(OS_UNIX) && !not_networking
char *getIP(const char *interface_name)
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    // Get list of network interfaces
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Walk through the linked list of interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL || strcmp(ifa->ifa_name, interface_name) != 0)
        {
            continue;
        }

        // Only consider IPv4 interfaces
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0)
            {
                perror("getnameinfo");
                exit(EXIT_FAILURE);
            }

            freeifaddrs(ifaddr);
            return strdup(host);
        }
    }

    fprintf(stderr, "interface_name (%s): not found\n", interface_name);
    exit(EXIT_FAILURE);
}
#endif

#if !not_networking
void psockerr(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if defined(OS_WINDOWS)
    fprintf(stderr, "WSAError: %d\n", WSAGetLastError());
#elif defined(OS_UNIX)
    perror("recv");
#endif
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif

char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

// Function to trim trailing whitespace characters from a string
char *rtrim(char *s)
{
    char *end = s + strlen(s) - 1;
    while (end > s && isspace(*end))
        end--;

    *(end + 1) = '\0';
    return s;
}

// Function to trim leading and trailing whitespace characters from a string
inline char *trim(char *s)
{
    char *dummy_str = strdup(s);
    char *new_str = strdup(rtrim(ltrim(dummy_str)));
    free(dummy_str);
    return new_str;
}

#endif
