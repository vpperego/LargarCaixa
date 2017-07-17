#ifndef __dropboxSynch__
#define __dropboxSynch__

#include <stdio.h>
#include <stdlib.h>
#include "dropboxUtil.h"
#include "dropboxSharedSocket.h"
#include "dropboxReplicaManager.h"


#include <dirent.h>
void *synch_listen(void *thread_info);
void *synch_server(void *thread_info);

#endif
