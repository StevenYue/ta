#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    size_t newLine = 0, words = 0, bytes = 0;
    void*   input;
    if ( 1 == argc )
    {
        input = stdin;
    }
    else
    {
        input = fopen(argv[1], "r");
        if ( !input )
        {
            printf("No such file");
            return -1;
        }
    }
    char it, prec=' ';
    while ( (it = getc(input)) != EOF )
    {
        ++bytes;
        if ( '\n' == it )
        {
            ++newLine;
        }
        if ( (' ' == it || '\n' == it) 
                && (' ' != prec && '\n' != prec) )
        {
            ++words;
        }
        prec = it;
    }
    printf("%ld %ld %ld %s\n", newLine, words, bytes, argv[1]);
    return 0;
}
