#ifndef __dropboxServer__
#define __dropboxServer__

#include "dropboxList.h"
#include "dropboxSemaphore.h"
#include "dropboxSharedSocket.h"
#include "dropboxUtil.h"
#include "dropboxSynch.h"

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
#define DEVICE_FREE -1



typedef struct client {
  struct list_head client_list;
  int devices[MAX_SESSIONS];
  char userid[MAXNAME];
  struct file_info files[MAXFILES];
  int logged_in;
} client_t;



void *sync_server();
void *client_thread(void *thread_info);
void receive_file(char *file);
void send_file(char *file);
char *read_user_name(int newsockfd);
void server_listen(int server_socket);

struct list_head client_list;
void client_list_init();
client_t *client__list_signup(char *userid);
client_t *client_list_search(char *userid);
bool client_open_session(client_t *client, int device_id);
bool client_close_session(client_t *client, int device_id);

#endif
