#include "../include/dropboxReplicaManager.h"

char * main_wd;



SSL * ssl_service(int newsockfd);
struct thread_info * create_rm_thread_info(int newsockfd, SSL * new_ssl);
char * create_working_directory(int rm_id);
void * rm_synch(void *thread_info);
void rm_synch_listener(struct thread_info * ti);

/*
  Main function executed by secondary RM's.
  Listen for request from the primary rm (i.e., server)
*/
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
       pthread_create(&th, NULL, rm_synch, listener_thread_info);

     }

}

/*
  Creates the working directory for the secondary RM to store data
*/
char * create_working_directory(int rm_id){
  char * working_directory = malloc(sizeof(char)*1024);
  sprintf(working_directory,"RM_%d",rm_id);
  mkdir(working_directory, 0777) ;
  return working_directory;
}

/*
  Send the update to all replicas

*/
int updateReplicas(struct list_head * rm_list,char * command,char *fullpath,char *filename){
  rm_t *iterator;
  struct buffer * rm_answer;
  list_for_each_entry(iterator, rm_list,rm_list){

      send_data(command, iterator->newsockfd, strlen(command), iterator->ssl);

      send_data(filename, iterator->newsockfd, strlen(filename), iterator->ssl);

      if(strcmp(command,DELETE_FILE))
        send_file_from_path(iterator->newsockfd, fullpath, iterator->ssl);

      rm_answer = read_data(iterator->newsockfd,iterator->ssl);
  //    printf("Answer: %s\n",rm_answer->data );
      if(!strcmp(rm_answer->data,UPDATE_FAIL))
        return UPDATE_ERROR;
   }
   return UPDATE_OK;
}

struct thread_info * create_rm_thread_info(int newsockfd, SSL * new_ssl){
  struct buffer *listen_buffer;
  struct thread_info *thread_info = malloc(sizeof(struct thread_info));
  thread_info->ssl = new_ssl;
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
void *rm_synch(void *thread_info) {

   struct thread_info *ti = (struct thread_info *)thread_info;
   ti->working_directory = malloc(sizeof(char)*1024);
   strcpy(ti->working_directory,main_wd);
   strcat(ti->working_directory,"/");
   strcat(ti->working_directory,ti->userid);
  //  printf("WD: %s\n", );
   mkdir(ti->working_directory, 0777);

   struct list_head *file_list = malloc(sizeof(file_list));

   INIT_LIST_HEAD(file_list);
   get_file_list(ti->newsockfd, file_list, ti->ssl);
  // download_missing_files(ti, file_list);

    rm_synch_listener(ti);
    return NULL;
}


void  rm_synch_listener(struct thread_info * ti){
  struct buffer *filename, *request;

  char fullpath[255];
  while (true) {
  //  printf("waiting for new request...\n" );

    request = read_data(ti->newsockfd,ti->ssl);
    request->data = check_valid_string(request);

    filename = read_data(ti->newsockfd,ti->ssl);
    filename->data = check_valid_string(filename);

    strcpy(fullpath, ti->working_directory);
    strcat(fullpath, "/");
    strcat(fullpath, filename->data);

//    printf("fullpath:%s\n",fullpath );
//
    if (strcmp(DOWNLOAD_FILE, request->data) == 0) {
      send_file_from_path(ti->newsockfd, fullpath,ti->ssl);
    //  file_list_add(ti->file_list,fullpath);
    }else if (strcmp(UPDATE_FILE, request->data)==0){
    //  file_list_remove(ti->file_list, filename->data);
      remove(fullpath);
      receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
      //file_list_add(ti->file_list,fullpath);
    }else if (strcmp(DELETE_FILE, request->data) == 0) {
      //file_list_remove(ti->file_list, filename->data);
      remove(fullpath); // delete the file
    }else {
      receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
//      printf("Before file_list add Answer...\n" );

    //  file_list_add(ti->file_list,fullpath);
    //  printf("After file_list add Answer...\n" );

      }
//      printf("sending Answer...\n" );
    send_data(UPDATE_DONE,ti->newsockfd,strlen(UPDATE_DONE), ti->ssl);
  }
}
