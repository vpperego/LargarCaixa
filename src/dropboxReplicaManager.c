#include "../include/dropboxReplicaManager.h"

char * main_wd;



SSL * ssl_service(int newsockfd);
struct thread_info * create_rm_thread_info(int newsockfd, SSL * new_ssl);
char * create_working_directory(int rm_id);
void * rm_synch_listener(void *thread_info);

void main_replica_manager(int port,int rm_id){

  pthread_t th;
  SSL * listener_ssl;
  struct thread_info * listener_thread_info;
  int newsockfd;
  struct sockaddr_in cli_addr;
  socklen_t clilen;

  clilen = sizeof(struct sockaddr_in);
  int rm_socket = start_server(port);

  main_wd = create_working_directory(rm_id);

  listen(rm_socket,5);

  while(true){
    if ((newsockfd = accept(rm_socket, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1)
        perror("ERROR ACCEPT: ");
       listener_ssl = ssl_service(newsockfd);
       listener_thread_info = create_rm_thread_info(newsockfd,listener_ssl);
       pthread_create(&th, NULL, rm_synch_listener, listener_thread_info);

     }

}

char * create_working_directory(int rm_id){
  char * working_directory = malloc(sizeof(char)*1024);


  sprintf(working_directory,"RM_%d",rm_id);
  if (mkdir(working_directory, 0777) < 0) {
    // perror("ERROR MKDIR: ");
    // return NULL;
  }
    return working_directory;
}
/*
int updateReplicas(list_head * rm_list,char * command,char *userid,char *file){
  rm_t *iterator;
//  list_for_each_entry(iterator, file_list,
  //                    file_list)
  list_for_each_entry(iterator,rm_list,rm_list){
    send_data(command,iterator->newsockfd,strlen(command));
    send_data(userid, iterator->newsockfd,strlen(command));
  }

}
*/
struct thread_info * create_rm_thread_info(int newsockfd, SSL * new_ssl){
  struct buffer *listen_buffer;
  struct thread_info *thread_info = malloc(sizeof(struct thread_info));
  thread_info->newsockfd = newsockfd;
  listen_buffer = read_data(thread_info->newsockfd,new_ssl);
  listen_buffer->data = check_valid_string(listen_buffer);
  strcpy(thread_info->userid,listen_buffer->data);
  return thread_info;
}


SSL * ssl_service(int newsockfd){
  SSL * ssl = startServerSSL();
  SSL_set_fd(ssl,	newsockfd);
  int ssl_err = SSL_accept(ssl);
  if(ssl_err <= 0)
  {
    ShutdownSSL(ssl);
  }
  return ssl;
}


/*
 Main synch thread executed in client side
*/
void *rm_synch_listener(void *thread_info) {

   struct thread_info *ti = (struct thread_info *)thread_info;
   printf("rm_synch_listener de %s\n", ti->userid);
   ti->working_directory = malloc(sizeof(char)*1024);
   strcpy(ti->working_directory,main_wd);
   strcat(ti->working_directory,"/");
   strcat(ti->working_directory,ti->userid);
  //  printf("WD: %s\n", );
   if (mkdir(ti->working_directory, 0777) < 0) {
       perror("ERROR MKDIR: ");
       return NULL;
   }

   char *fullpath = malloc(strlen(ti->userid) + MAXNAME + 1);

   struct list_head *file_list = malloc(sizeof(file_list));

   INIT_LIST_HEAD(file_list);
   get_file_list(ti->newsockfd, file_list, ti->ssl);
   download_missing_files(ti, file_list);

   do {
     listen_changes(ti, file_list,ti->userid, fullpath);
     sleep(5);

    } while (true);
}

/*
void * rm_synch_listener(void *thread_info){
  struct thread_info *ti = (struct thread_info *)thread_info;
  ti->working_directory = malloc(sizeof(char)*1024);
  strcpy(ti->working_directory,main_wd);
  strcat(ti->working_directory,"/");

  struct  buffer * request;
  while(true){
      request = read_data(ti->newsockfd,ti->ssl);

  }
  return NULL;
}
*/
