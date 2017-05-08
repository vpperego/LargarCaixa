#ifndef __dropboxClient__
#define __dropboxClient__

#include "dropboxUtil.h"
#include "dropboxClientUI.h"
#include "dropboxSharedSocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>

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

#endif
