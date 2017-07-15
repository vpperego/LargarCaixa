#ifndef __dropboxClient__
#define __dropboxClient__

#include "dropboxClientUI.h"
#include "dropboxList.h"
#include "dropboxSharedSocket.h"
#include "dropboxUtil.h"
#include "dropboxSynch.h"
#include <dirent.h>
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
#include <openssl/ssl.h>
#include <openssl/err.h>

#define CLIENT_ARGUMENTS 4
#define MAXFILES 256
#define ARGUMENTS "Client IP Port"

extern char userid[MAXNAME];
extern int client_socket;

int connect_server(char *host, int port);
void sync_client();
void printSSLCert();
void startSSL();
void send_file(char *file);
void get_file(char *file);
void close_connection();
void check_sync_dir();
void get_server_file_list();
void synch_deleted();
bool updated_existing_file(char *fullpath, struct dirent *ent);
bool rename_files(char *fullpath, struct dirent *ent);

#endif
