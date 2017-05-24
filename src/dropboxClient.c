#include "../include/dropboxClient.h"
#include <errno.h>

char userid[MAXNAME];
int client_socket;

/* From Assignment Specification
 * Connects the client to the server.
 * host - server address
 * port - server port
 */
int connect_server(char *host, int port) {
  struct hostent *server;
  struct sockaddr_in serv_addr;
  int sockfd;

  if ((server = gethostbyname(host)) == NULL) {
    perror("ERROR, no such host: ");
    exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ERROR opening socket: ");
    exit(0);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting\n");
    exit(0);
  }

  char buffer[256];
  strcpy(buffer, userid);

  // send userid to server
  send_data(userid, sockfd, (int)(strlen(userid) * sizeof(char)));

  return sockfd;
}

/* From Assignment Specification
 * Synchronizes the directory named "sync_dir_<username>" with the server.
 * The directory is located in the users home.
 */
void sync_client() {}

/* From Assignment Specification
 * Sends a file with the path "file" to the server.
 * Must be called in order to upload a file.
 * file - path/filename.ext
 */
void send_file(char *file) {
  char *command = "upload";
  send_data(command, client_socket, (int)(strlen(command) * sizeof(char)));
  send_data(file, client_socket, (int)(strlen(file) * sizeof(char)));
  send_file_from_path(client_socket, file);
}

/* From Assignment Specification
 * Fetch a file named "file" from the server.
 * Must be called in order to download a file.
 * file - filename.ext
 */
void get_file(char *file) {
  char *command = "download";
  send_data(command, client_socket, (int)(strlen(command) * sizeof(char)));
  send_data(file, client_socket, (int)(strlen(file) * sizeof(char)));
  receive_file_and_save_to_path(client_socket, file);
}

/* From Assignment Specification
 * Closes the connection to the server
 */
void close_connection() {
  char *command = "exit";
  send_data(command, client_socket, (int)(strlen(command) * sizeof(char)));
  close(client_socket);
}

/*
  Check if the client sync_dir is already created
*/
void check_sync_dir()
{
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;
  char sync_dir_path[256];
  struct buffer *filename;
  strcpy(sync_dir_path, homedir);
  strcat(sync_dir_path, "/sync_dir_");
  strcat(sync_dir_path, userid);
  DIR* dir = opendir(sync_dir_path);
  char fullname[256];

  if (dir) {
    // TODO - if the dir already exist, we need to check every file to see what needs to be synchronized

  } else if(ENOENT == errno){
    /* dir doesn't exist */
    mkdir(sync_dir_path, 0777);
    send_data(GET_ALL_FILES, client_socket, (int)(strlen(GET_ALL_FILES) * sizeof(char)));
    while(true){

      filename = read_data(client_socket);
      strcpy(fullname,sync_dir_path);
      strcat(fullname,"/");
      strcat(fullname,filename->data);
      if(strcmp(FILE_SEND_OVER,filename->data)==0)
        break;
  //      printf("vamos ler o filename %s!!\n",fullname);
      receive_file_and_save_to_path(client_socket,fullname);  /* code */
    }
  }

}

int main(int argc, char *argv[]) {
  if (argc < CLIENT_ARGUMENTS) {
    fprintf(stderr, "usage %s %s\n", argv[0], ARGUMENTS);
    exit(0);
  }
  // save user name
  strcpy(userid, argv[1]);
  client_socket = connect_server(argv[2], atoi(argv[3]));
  //TODO -refactor needed here.
  check_sync_dir();
  start_client_interface();
  return 0;
}
