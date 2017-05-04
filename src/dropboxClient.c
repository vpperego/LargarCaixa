#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *console_str[] = {
  "upload",
  "download",
  "list",
  "get_sync_dir",
  "exit"
};

bool (*console_func[]) (char **) = {
  &command_upload,
  &command_download,
  &command_list,
  &command_get_sync_dir,
  &command_exit
};

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
    fgets(buffer, 256, stdin);

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

bool command_upload(char **args)
{
  if (args[1] == NULL) {
      fprintf(stderr, "usage: upload <path/filename.ext>\n");
  }
  return false;
}

bool command_download(char **args)
{
  if (args[1] == NULL) {
      fprintf(stderr, "usage: download <filename.ext> \n");
  }
  return false;
}

bool command_list(char **args)
{
  return false;
}

bool command_get_sync_dir(char **args)
{
  return false;
}

bool command_exit(char **args)
{
  return true;
}

bool execute_command(char **args)
{
  if (args[0] == NULL) {
    return false;//no args, continue loop
  }

  int i;
  for (i = 0; i < FUNCTION_COUNT; i++) {
    if (strcmp(args[0], console_str[i]) == 0) {
      return (*console_func[i])(args);
    }
  }
  return false;
}

void start_client_interface()
{
  bool finished = false;
  char *command;
  char **args;

  do {
    printf("> ");
    command = read_line();
    args = split_args(command);
    finished = execute_command(args);

    free(command);
    free(args);

  } while (!finished);

}

int main(int argc, char *argv[]) {

  if (argc < CLIENT_ARGUMENTS) {
    fprintf(stderr,"usage %s %s\n", argv[0],ARGUMENTS);
    exit(0);
  }
  /*connect_server(argv[2],atoi(argv[3]));*/
  start_client_interface();
  return 0;
}
