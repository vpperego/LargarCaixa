#include "../include/dropboxReplicaManager.h"


SSL * ssl_service(int newsockfd);
struct thread_info * create_rm_thread_info(int newsockfd, SSL * new_ssl);

void * rm_synch_listener(void *thread_info);

void main_replica_manager(){

  pthread_t th;
  SSL * listener_ssl;
  struct thread_info * listener_thread_info;
  int newsockfd;
  struct sockaddr_in cli_addr;
  socklen_t clilen;
  int rm_socket = start_server(RM_PORT);

  clilen = sizeof(struct sockaddr_in);

  listen(rm_socket,5);
  printf("Iniciando listen....\n");
  while(true){
    if ((newsockfd = accept(rm_socket, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1)
        perror("ERROR ACCEPT: ");
      printf("Nova conexão....\n");

       listener_ssl = ssl_service(newsockfd);
       listener_thread_info = create_rm_thread_info(newsockfd,listener_ssl);

      pthread_create(&th, NULL, rm_synch_listener, listener_thread_info);


  }

}



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


void * rm_synch_listener(void *thread_info){
  struct thread_info *ti = (struct thread_info *)thread_info;
  printf("Iniciando thread de %s\n",ti->userid );

  return NULL;
}
