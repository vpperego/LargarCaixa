#ifndef __dropboxServer__
#define __dropboxServer__

#include "dropboxList.h"
#include "dropboxSemaphore.h"
#include "dropboxSharedSocket.h"
#include "dropboxUtil.h"
#include "dropboxSynch.h"
#include "dropboxReplicaManager.h"

#include <dirent.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <libgen.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>


#define SERVER_PORT 55000
#define MAX_SESSIONS 2
#define DEVICE_FREE -1



typedef struct client {
  struct list_head client_list;
  int devices[MAX_SESSIONS];
  char userid[MAXNAME];
  struct list_head *file_list;
  int logged_in;
  dbsem_t *sem;
  struct list_head  *rm_list;
} client_t;



void *sync_server();
void *client_thread(void *thread_info);
void receive_file(char *file);
void send_file(char *file);
void ShutdownSSL();
void startSSL();
char *read_user_name(int newsockfd, SSL * ssl);
void server_listen(int server_socket);

struct list_head client_list;
void client_list_init();
client_t *client__list_signup(char *userid);
client_t *client_list_search(char *userid);
bool client_open_session(client_t *client, int device_id);
bool client_close_session(client_t *client, int device_id);
bool add_to_files_list(client_t* client);
dbsem_t *get_client_sem(char *userid);
struct list_head * get_client_rm(char *userid);
#endif
