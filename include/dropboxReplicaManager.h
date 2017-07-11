#ifndef __dropboxReplicaManager__
#define __dropboxReplicaManager__

#include "dropboxSynch.h"
#include "dropboxSemaphore.h"



#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

void main_replica_manager(void * server_shared_memory,sem_t * rm_sem);

#endif
