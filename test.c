#include <stdio.h>
#include "helper.h"

int main()
{
    char string[] = "Hello, World, Guys!";

    strs *slices = splitOnce(string, ", ");

    for (size_t i = 0; i < slices->length; i++)
    {
        printf("splited string: %s\n", slices->data[i]);
    }

    return 0;
}
