#include "preprocessor.h"

void remove_comments(const char *input, char *output, int *block)
{
    int i = 0, j = 0;
    int str = 0, ch = 0;

    while (input[i])
    {
        if (*block)
        {
            if (input[i] == '*' && input[i+1] == '/')
            {
                *block = 0;
                i += 2;
            }
            else
            {
                if (input[i] == '\n')
                    output[j++] = '\n';
                i++;
            }
        }
        else if (str || ch)
        {
            output[j++] = input[i];

            if (str && input[i] == '"')
                str = 0;
            else if (ch && input[i] == '\'')
                ch = 0;

            i++;
        }
        else if (input[i] == '"')
        {
            str = 1;
            output[j++] = input[i++];
        }
        else if (input[i] == '\'')
        {
            ch = 1;
            output[j++] = input[i++];
        }
        else if (input[i] == '/' && input[i+1] == '/')
        {
            while (input[i] && input[i] != '\n')
                i++;
        }
        else if (input[i] == '/' && input[i+1] == '*')
        {
            *block = 1;
            i += 2;
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';
}