#include "../include/dropboxServer.h"
#include "../include/dropboxServerCommandHandler.h"

//@TODO make multi-client
struct client client;

/* From Assignment Specification
 * Synchronizes the directory named "sync_dir_<username>" with the clients
 * sync_dir.
 */
void sync_server() {}

/* From Assignment Specification
 * Receive a file from the client.
 * file - path/filename.ext
 */
void receive_file(char *file) {}

/* From Assignment Specification
 * Send a file to the client.
 * file - filename.txt
 */
void send_file(char *file) {}

/*
 Starts (and returns) the main socket server (i.e., the listen socket)
 */
int start_server() {
  int sockfd;

  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    perror("ERROR opening socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    perror("ERROR on binding");

  return sockfd;
}

void *client_thread(void *client_socket) {
  while (true) {
    // read command from client
    printf("ESPERANDO COMANDO\n");
    struct buffer *command = read_data(*((int *)client_socket));
    printf("RECEBEU COMANDO\n");
    //@TODO refactor
    if (strcmp(command->data, "upload") == 0) {
      command_upload(*((int *)client_socket), (struct client *)&client);
    } else if (strcmp(command->data, "list") == 0) {
      command_list(*((int *)client_socket), (struct client *)&client);
    } else if (strcmp(command->data, "download") == 0) {
      command_download(*((int *)client_socket), (struct client *)&client);
    }
  }

  return NULL;
}

/*
 * See if the user is valid (i.e, is registered and has an available device
 * slot)
 */
bool is_client_valid(void) {
  //@TODO check for client folder and number of devices
  return true;
}

void read_user_name(int newsockfd) {
  printf("Vai ler username\n");
  struct buffer *buffer = read_data(newsockfd);
  printf("Username: %s\n", buffer->data);
  //@TODO insert into list
  strcpy(client.userid, buffer->data);
  // make client folder
  if (mkdir(client.userid, 0777) < 0) {
    perror("ERROR MKDIR: ");
  }
}

/*
 Executes the main socket listen.
 */
void server_listen(int server_socket) {
  int newsockfd;
  socklen_t clilen;
  struct sockaddr_in cli_addr;
  pthread_t th;

  listen(server_socket, 50);

  clilen = sizeof(struct sockaddr_in);
  // wait for new connections and create a new thread for each client
  printf("Dropbox Server Listening...");
  while (true) {
    if ((newsockfd = accept(server_socket, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1)
      perror("ERROR ACCEPT: ");
    printf("\nAceitou conexão de um socket.\n");
    read_user_name(newsockfd);

    if (is_client_valid())
      pthread_create(&th, NULL, client_thread, &newsockfd);
  }
}

int main(int argc, char *argv[]) {
  int server_socket = start_server();
  server_listen(server_socket);
}
