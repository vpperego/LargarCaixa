#include "../include/dropboxServer.h"
#include "../include/dropboxUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>

//@TODO make multi-client
struct client client;

void sync_server()
{

}

void receive_file(char *file)
{

}

void send_file(char *file)
{

}
/*
  Starts (and returns) the main socket server (i.e., the listen socket)
*/
int start_server()
{
  int sockfd;

  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror ("ERROR opening socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    perror ("ERROR on binding");

  return sockfd;
}

void *client_thread(void * client_socket)
{
  while(true){
    //read command from client
    struct buffer *command = read_data(*((int *)client_socket));

    //@TODO refactor
    if(strcmp(command->data, "upload") == 0){
      //do not change this order!
      struct buffer *filename = read_data(*((int *)client_socket));
      struct buffer *data = read_data(*((int *)client_socket));
      FILE *fp;
      char file_path[1024];
      char *bname;

      strcpy(file_path, client.userid);
      strcat(file_path, "/");
      bname = basename(filename->data);
      strcat(file_path, bname);

      fp = fopen(file_path, "w+");
      if(fp == NULL)
      {
        perror ("ERROR - Failed to open file for writing\n");
        exit(1);
      }

      if(fwrite(data->data, sizeof(char), data->size, fp) != data->size)
      {
        perror ("ERROR - Failed to write bytes to file\n");
        exit(1);
      }
      fclose(fp);
    }
  }

	return NULL;
}

/*
 * See if the user is valid (i.e, is registered and has an available device slot)
*/
bool is_client_valid(void)
{
  //@TODO check for client folder and number of devices
  return true;
}

//read data from client given the protocol '[int size][data size bytes]'
struct buffer* read_data(int newsockfd){
  int buflen, n;
  //read data size
  n = read(newsockfd, (char*)&buflen, sizeof(buflen));
  if (n < 0) perror ("ERROR reading from socket");
  buflen = ntohl(buflen);
  char *buffer_data = malloc(sizeof(char)*buflen);
  int amount_read = 0;
  //keep reading data until size is reached
  while(amount_read < buflen){
    n = read(newsockfd, (void *)buffer_data, buflen);
    amount_read += n;
    if (n < 0)
    {
      perror ("ERROR reading from socket");
      close(newsockfd);
      exit(0);
    }
  }

  struct buffer *buffer = malloc(sizeof(struct buffer));
  buffer->data = buffer_data;
  buffer->size = sizeof(char)*buflen;
  return buffer;

}

void read_user_name(int newsockfd){
  struct buffer *buffer = read_data(newsockfd);

  //@TODO insert into list
  strcpy(client.userid, buffer->data);
  //make client folder
  mkdir(client.userid, 0777);
}

/*
 Executes the main socket listen. 
*/
void server_listen(int server_socket)
{
  int newsockfd;
  socklen_t clilen;
  struct sockaddr_in cli_addr;
  pthread_t th;

  listen(server_socket, 50);

  clilen = sizeof(struct sockaddr_in);
  //wait for new connections and create a new thread for each client
  while(true)
  {
    if ((newsockfd = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1)
      perror ("ERROR on accept");

    read_user_name(newsockfd);

    if(is_client_valid())
      pthread_create(&th,NULL,client_thread,&newsockfd);
  }
}

int main(int argc, char *argv[]) {

  int server_socket = start_server();
  server_listen(server_socket);
}
