#include "../include/dropboxClientUI.h"

void start_client_interface()
{
    bool finished = false;
    char *command;
    char **args;
    
    do {
        printf("> ");
        command = read_line();
        args = split_args(command);
        finished = execute_command(args);
        
        free(command);
        free(args);
        
    } while (!finished);
    
}
