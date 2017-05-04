#include "dropboxClient.h"
#include "dropboxUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int connect_server(char *host, int port)
{
  struct hostent *server;
  struct sockaddr_in serv_addr;
  int sockfd,n;
  char buffer[256];
  server = gethostbyname(host);

  if(server==NULL)
  {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
      printf("ERROR opening socket\n");
    exit(0);
  }

  serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
  {
  	printf("ERROR connecting\n");
		exit(0);
	}

  while(1)
  {
    printf("Enter the message: ");
    bzero(buffer, 256);
    strcpy(buffer,NEW_CONNECTION);

  /* write in the socket */
  n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
   {
    printf("ERROR writing to socket\n");
    close(sockfd);
    exit(0);
   }
    bzero(buffer,256);

  /* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0)
    {
    printf("ERROR reading from socket\n");
      close(sockfd);
    }
  }
  return 0;
}

void sync_client()
{

}

void send_file(char *file)
{

}

void get_file(char *file)
{

}

void close_connection()
{

}


int main(int argc, char *argv[]) {

  if (argc < CLIENT_ARGUMENTS) {
    fprintf(stderr,"usage %s %s\n", argv[0],ARGUMENTS);
    exit(0);
  }
  connect_server(argv[2],atoi(argv[3]));
  return 0;
}
