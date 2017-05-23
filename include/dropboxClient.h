#ifndef __dropboxClient__
#define __dropboxClient__

#include "dropboxClientUI.h"
#include "dropboxSharedSocket.h"
#include "dropboxUtil.h"
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#define CLIENT_ARGUMENTS 4
#define MAXNAME 256
#define MAXFILES 256
#define ARGUMENTS "Client IP Port"

extern char userid[MAXNAME];
extern int client_socket;

int connect_server(char *host, int port);
void sync_client();
void send_file(char *file);
void get_file(char *file);
void close_connection();
void check_sync_dir();

#endif
