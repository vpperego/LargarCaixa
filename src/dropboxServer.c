<<<<<<< HEAD
#include "dropboxServer.h"
#include "dropboxUtil.h"
=======
#include "../include/dropboxServer.h"
#include "../include/dropboxUtil.h"
>>>>>>> a6160a71b71368c4414d0b42cab0c1048e2e4758
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

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
  inicializa o socket do server.
*/
int start_server()
{
  int sockfd;
  
  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    printf("ERROR on binding");

  return sockfd;
}

void *client_thread(int client_socket)
{
	printf("ENTRANDO NA THREAD DO CLIENTE");
}

void server_listen(int server_socket)
{
  int newsockfd,n;
  char buffer[256];
  socklen_t clilen;
  struct sockaddr_in cli_addr;
  pthread_t th;

  listen(server_socket, 5);

  clilen = sizeof(struct sockaddr_in);
  if ((newsockfd = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1)
    printf("ERROR on accept");
  while(1)
  {
    bzero(buffer, 256);

    /* read from the socket */
    n = read(newsockfd, buffer, 256);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      close(newsockfd);
      close(server_socket);
      exit(0);
    }
    
    if(strcmp(buffer,NEW_CONNECTION))
    {
      pthread_create(&th,NULL,server_listen,&newsockfd);
    }else{
	printf("MESSAGE NOT RECOGNIZED\n");
    }
   
    //printf("Client say: %s\n", buffer);
        fgets(buffer, 256, stdin);
    /* write in the socket */
    n = write(newsockfd, buffer, 256);
    if (n < 0)
    {
      printf("ERROR writing to socket");
      close(newsockfd);
      close(server_socket);
      exit(0);
    }
  }
}

int main(int argc, char *argv[]) {

  int server_socket = start_server();

}
