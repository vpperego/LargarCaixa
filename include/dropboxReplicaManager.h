#ifndef __dropboxReplicaManager__
#define __dropboxReplicaManager__

#include "dropboxSynch.h"
#include "dropboxSharedSocket.h"
#include "dropboxList.h"

#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#define RM_PORT 60000


struct rm_info {
  pid_t pid;
  int newsockfd;
  SSL *ssl;
  char address[256];
  int port;
  struct list_head  rm_list;
};

typedef struct rm_info rm_t;



void main_replica_manager(int port,int rm_id);

#endif
