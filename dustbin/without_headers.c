// #include <winsock.h>

#define PORT 3000
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Predefined
int printf(const char *format, ...);
// int scanf(const char *format, ...);
int system(const char *command);
int chdir(const char *);

// User Defined
long long unsigned int strlen(const char *str);
int strcmp(const char *, const char *);

int main(int argc, char **argv)
{
    char *directory = ".";

    if (argc >= 3)
    {
        if (strcmp(argv[1], "--directory"))
            directory = argv[2];
    }
    printf("Setting up directory to %s\n", directory);

    if (chdir(directory) < 0)
    {
        printf("Failed to set current dir");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

long long unsigned int strlen(const char *str)
{
    unsigned int length = 0;

    // Loop until the null-terminator is encountered
    while (*str != '\0')
    {
        length++;
        str++;
    }

    return length;
}

int strcmp(const char *str1, const char *str2)
{
    int matched = 1;

    if (strlen(str1) != strlen(str2))
        return matched = 0;

    for (int i = 0, s = strlen(str1); i < s; i++)
    {
        if (str1[i] != str2[i])
        {
            matched = 0;
            break;
        }
    }

    return matched;
}
