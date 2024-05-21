#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

void obter_input(char *buffer)
{
    int buf_ocup = 0;
    while (1)
    {
        buffer[buf_ocup] = getchar();
        if (buffer[buf_ocup] == '\n')
        {
            buffer[buf_ocup] = 0;
            return;
        }
        buf_ocup++;
    }
}

void copy_to_clipboard(const char* msg){
    // Use xclip to copy the message to the clipboard
    char command[50+strlen(msg)];
    snprintf(command, sizeof(command), "echo '%s' | xclip -selection clipboard", msg);
    system(command);   
}
#endif