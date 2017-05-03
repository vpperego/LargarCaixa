#include "dropboxServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
void start_server()
{
  int sockfd, newsockfd, n;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    printf("ERROR on binding");

  listen(sockfd, 5);

  clilen = sizeof(struct sockaddr_in);
  if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
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
      close(sockfd);
      exit(0);
    }
    printf("Client say: %s\n", buffer);
        fgets(buffer, 256, stdin);
    /* write in the socket */
    n = write(newsockfd, buffer, 256);
    if (n < 0)
    {
      printf("ERROR writing to socket");
      close(newsockfd);
      close(sockfd);
      exit(0);
    }
  }
}

int main(int argc, char *argv[]) {

  start_server();

}
