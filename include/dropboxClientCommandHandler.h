#ifndef dropboxClientCommandHandler_h
#define dropboxClientCommandHandler_h

#include "dropboxUtil.h"
#include "dropboxClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>

#define FUNCTION_COUNT 5

bool command_download(char **args);
bool command_upload(char **args);
bool command_list(char **args);
bool command_get_sync_dir(char **args);
bool command_exit(char **args);
bool execute_command(char **args);

#endif
