#include "../include/dropboxClient.h"
#include <errno.h>

char userid[MAXNAME];
int client_socket, synch_socket;

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
  //  send_data(userid, sockfd, (int)(strlen(userid) * sizeof(char)));

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
  send_data(command, client_socket, (strlen(command) * sizeof(char)));
  send_data(file, client_socket, (strlen(file) * sizeof(char)));
  send_file_from_path(client_socket, file);
}

/* From Assignment Specification
 * Fetch a file named "file" from the server.
 * Must be called in order to download a file.
 * file - filename.ext
 */
void get_file(char *file) {
  char *command = "download";
  send_data(command, client_socket, (strlen(command) * sizeof(char)));
  send_data(file, client_socket, (strlen(file) * sizeof(char)));
  receive_file_and_save_to_path(client_socket, file);
}

/* From Assignment Specification
 * Closes the connection to the server
 */
void close_connection() {
  char *command = "exit";
  send_data(command, client_socket, (strlen(command) * sizeof(char)));
  close(client_socket);
}

void get_all_files(char *sync_dir_path) {
  char fullpath[256];
  struct buffer *filename;

  send_data(GET_ALL_FILES, client_socket, strlen(GET_ALL_FILES) * sizeof(char));
  while (true) {
    filename = read_data(client_socket);
    if (strcmp(FILE_SEND_OVER, filename->data) == 0)
      break;
    strcpy(fullpath, sync_dir_path);
    strcat(fullpath, "/");
    strcat(fullpath, filename->data);
    receive_file_and_save_to_path(synch_socket, fullpath); /* code */
  }
}

/*
  Create the sync_dir (if necessary) and starts the sync thread
*/
void start_sync_service(char *host, int port) {
  char *sync_dir_path = get_sync_dir(userid);
  pthread_t th;

  synch_socket = connect_server(host, port);

  DIR *sync_dir = opendir(sync_dir_path);
  if (errno == ENOENT) {
    mkdir(sync_dir_path, 0777);
    sync_dir = opendir(sync_dir_path);
    get_all_files(sync_dir_path);
  }
  closedir(sync_dir);

  // tell the server to create a thread to synchronize with this one
  send_data(CREATE_SYNCH_THREAD, synch_socket,
            strlen(CREATE_SYNCH_THREAD) * sizeof(char));
  // send the userid for the new server thread
  send_data(userid, synch_socket, strlen(userid) * sizeof(char));
  struct thread_info *ti = malloc(sizeof(struct thread_info));
  strcpy(ti->userid, userid);
  ti->working_directory = sync_dir_path;
  ti->newsockfd = synch_socket;
  pthread_create(&th, NULL, synch_listen, ti);
}

int main(int argc, char *argv[]) {
  if (argc < CLIENT_ARGUMENTS) {
    fprintf(stderr, "usage %s %s\n", argv[0], ARGUMENTS);
    exit(0);
  }
  // save user name
  strcpy(userid, argv[1]);
  client_socket = connect_server(argv[2], atoi(argv[3]));
  // send userid to server
  send_data(userid, client_socket, strlen(userid) * sizeof(char));
  struct buffer *server_response;
  server_response = read_data(client_socket);
  if (strcmp(server_response->data, CONNECTION_FAIL) == 0) {
    printf("Numero maximo de conexoes atingido\n");
    close(client_socket);
    exit(0);
  }

  start_sync_service(argv[2], atoi(argv[3]));

  start_client_interface();
  exit(0);
}
