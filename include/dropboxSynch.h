#include <stdio.h>
#include <stdlib.h>
#include "dropboxUtil.h"
#include "dropboxSharedSocket.h"
#include <dirent.h>
void *synch_listen(void *thread_info);
void *synch_server(void *thread_info);
