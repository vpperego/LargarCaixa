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
time_t getTimeServer(int synch_socket, SSL * ssl);

void get_file_list(int synch_socket, struct list_head *file_list, SSL *ssl);
void download_missing_files(struct thread_info *ti,struct list_head *file_list);
void listen_changes(struct thread_info *ti,  struct list_head *file_list ,char * userid,char * fullpath);
#endif
