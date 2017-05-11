#ifndef dropboxClientCommandHandler_h
#define dropboxClientCommandHandler_h

#include "dropboxClient.h"
#include "dropboxUtil.h"
#include <libgen.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUNCTION_COUNT 5

bool command_download(char **args);
bool command_upload(char **args);
bool command_list(char **args);
bool command_get_sync_dir(char **args);
bool command_exit(char **args);
bool execute_command(char **args);

#endif
