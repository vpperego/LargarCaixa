#include "../include/dropboxReplicaManager.h"
#include <semaphore.h>
#include <fcntl.h>

void main_replica_manager(void * server_shared_memory, sem_t *rm_sem){
  //DIR * rm_dir = opendir("RM_1");
  pthread_t th;

  struct thread_info *synch_server_info = malloc(sizeof(struct thread_info));
//  if (errno == ENOENT) {
//    mkdir("RM_1", 0777);
//  }
  while(true){
    printf("ANTES DO sem_wait\n");
    sem_wait(rm_sem);
//    printf("DEPOIS DO sem_wait\n");

    memcpy(&synch_server_info->newsockfd, server_shared_memory,sizeof(synch_server_info->newsockfd));
    printf("Recebeu do server socket %d\n",synch_server_info->newsockfd );

//    printf("REPLICA MANAGER userid %s\n",synch_server_info->userid );
    pthread_create(&th, NULL, synch_server, synch_server_info);

  }

}
