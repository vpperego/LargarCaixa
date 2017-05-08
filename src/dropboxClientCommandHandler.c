#include "../include/dropboxClientCommandHandler.h"

char *console_str[] = {
    "upload",
    "download",
    "list",
    "get_sync_dir",
    "exit"
};

bool (*console_func[]) (char **) = {
    &command_upload,
    &command_download,
    &command_list,
    &command_get_sync_dir,
    &command_exit
};

bool execute_command(char **args) {
    if (args[0] == NULL) {
        return false;//no args, continue loop
    }
    
    int i;
    for (i = 0; i < FUNCTION_COUNT; i++) {
        if (strcmp(args[0], console_str[i]) == 0) {
            return (*console_func[i])(args);
        }
    }
    return false;
}

bool command_upload(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: upload <path/filename.ext>\n");
        return false;
    }
    char *command = "upload";
    //send upload command
    send_data(command, client_socket, strlen(command) * sizeof(char));
    //send filename
    send_data(args[1], client_socket, strlen(args[1]) * sizeof(char));
    //senda file data
    send_file(args[1]);
    return false;
}

bool command_download(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: download <filename.ext> \n");
    }
    return false;
}

bool command_list(char **args)
{
    return false;
}

bool command_get_sync_dir(char **args)
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char sync_dir_path[256];
    
    strcpy(sync_dir_path, homedir);
    strcat(sync_dir_path, "/sync_dir_");
    strcat(sync_dir_path, userid);
    
    mkdir(sync_dir_path, 0777);
    return false;
}

bool command_exit(char **args)
{
    return true;
}
