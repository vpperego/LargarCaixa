#ifndef __dropboxReplicaManager__
#define __dropboxReplicaManager__

#include "dropboxSynch.h"
#include "dropboxSharedSocket.h"

#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#define RM_PORT 60000
void main_replica_manager();

#endif
