#include <stdio.h>
#include <pthread.h>

// Function for the first thread
void *thread1_function(void *arg)
{
    for (int i = 1; i <= 50; i++)
    {
        printf("Thread 1: %d\n", i);
    }
    pthread_exit(NULL);
}

// Function for the second thread
void *thread2_function(void *arg)
{
    for (int i = 1; i <= 50; i++)
    {
        printf("Thread 2: %d\n", i);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread1, thread2;

    // Create the first thread
    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    // Create the second thread
    if (pthread_create(&thread2, NULL, thread2_function, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
