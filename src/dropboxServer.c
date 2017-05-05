#include "../include/dropboxServer.h"
#include "../include/dropboxUtil.h"
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

void *client_thread(void * client_socket)
{
//	printf("ENTRANDO NA THREAD DO CLIENTE");
  
    char buffer[256];
/*
 *Client main loop (read e write)
 */
    while(true)
    {
 	if(read((int)&client_socket,buffer,256)>=0)
	{
		/*
		 * TODO-interpretar comando do usuario
		 */
		return NULL;
	}else
	{
	
      	close((int)&client_socket);
	return NULL;
	}
    int n = write((int)&client_socket, buffer, 256);
    if (n < 0)
    {
      printf("ERROR writing to socket");
      close((int)&client_socket);
      exit(0);
    }
    }
	return NULL;
	/*exit(0) ;*/
}

/*
 * Verifica se o usuario é valido, analisando se eh cadastrado no server se possui slot de device disponivel.
*/
bool is_client_valid(int client_socket)
{
    char buffer[256];
    if( write(client_socket, SEND_NAME, strlen(SEND_NAME)) > 0 )
    {
	if(read(client_socket,buffer,256) >0)
	{
		//TODO - server recebeu o ID do cliente. Validar a partir daqui se o nome existe e se existe device disponivel.
		return true;
   	}
    } 
	 return false;   
}
/*
 Realiza o listen do server. 
*/
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
  while(true)
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

    if(strcmp(buffer, NEW_CONNECTION) == 0)
    {
	if(is_client_valid(newsockfd)==true)
	    pthread_create(&th,NULL,client_thread,&newsockfd);
    }else{
      printf("MESSAGE NOT RECOGNIZED\n");
    }
    /* write in the socket */
  }
}

int main(int argc, char *argv[]) {

  int server_socket = start_server();
  server_listen(server_socket);
}
