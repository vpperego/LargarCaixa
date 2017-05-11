#ifndef __dropboxServer__
#define __dropboxServer__

#define MAXNAME 256
#include "dropboxSharedSocket.h"
#include "dropboxUtil.h"
#include <dirent.h>
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
#define MAXFILES 30
#define PORT 55000
#define MAX_SESSIONS 2

struct file_info {
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
};

struct client {
  int devices[MAX_SESSIONS];
  char userid[MAXNAME];
  struct file_info files[MAXFILES]; //;
  int logged_in;
};

void sync_server();
bool is_client_valid(void);
void *client_thread(void *client_socket);
void receive_file(char *file);
void send_file(char *file);
void read_user_name(int newsockfd);
void server_listen(int server_socket);
#endif
