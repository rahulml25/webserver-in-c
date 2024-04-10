// #define _DEFAULT_SOURCE

#include <stdio.h>
/*#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

sigjmp_buf jmpbuf;

void sigsegv_handler(int signal)
{
    // printf("Segmentation fault occurred.\n");
    siglongjmp(jmpbuf, 1); // Jump back to sigsetjmp point
}

int bellowCausedError()
{
    return sigsetjmp(jmpbuf, 1);
}

int main()
{
    // Install signal handler for segmentation fault
    struct sigaction action;
    action.sa_handler = sigsegv_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGSEGV, &action, NULL);

    // Set the jump point
    if (!bellowCausedError())
    {
        // Attempt to access invalid memory
        int *ptr = NULL;
        *ptr = 10; // This will cause a segmentation fault
    }
    else
    {
        printf("Continuing after SIGSEGV.\n");
    }

    printf("Program continues after handling SIGSEGV.\n");

    return 0;
}
*/

#include <windows.h>

#define __try if (1)
#define __except(x) if (0)
// #define __finally   } else

void my_function()
{
    __try
    {
        int ten = 10;
        int zero = 0;
        // Perform some operations that might cause an exception
        int result = ten / zero;        // Division by zero to trigger an exception
        printf("Result: %d\n", result); // This won't be executed if an exception occurs
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // Exception handler block
        printf("Exception caught!\n");
    }
    // __finally
    // {
    //     // Finally block
    //     printf("Finally block executed.\n");
    // }
}

int main()
{
    my_function();
    return 0;
}
