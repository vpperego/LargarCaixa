#include "../include/dropboxServerCommandHandler.h"

void command_exit(int client_socket, struct client *client) {
  client_close_session(client, client_socket);
  close(client_socket);
  pthread_exit(NULL);
}

void command_download(int client_socket, struct client *client, SSL *ssl) {
  printf("Server recebeu comando DOWNLOAD\n");

  struct buffer *filename = read_data(client_socket, ssl);
  printf("Filename: %s\n", filename->data);

  char file_path[1024];
  char *bname;
  strcpy(file_path, client->userid);
  strcat(file_path, "/");
  bname = basename(filename->data);
  strcat(file_path, bname);

  send_file_from_path(client_socket, file_path, ssl);
}

void command_upload(int client_socket, struct client *client, SSL *ssl) {
  printf("Server recebeu comando UPLOAD\n");
  struct buffer *filename = read_data(client_socket, ssl);
  printf("Filename: %s\n", filename->data);

  char file_path[1024];
  char *bname;
  strcpy(file_path, client->userid);
  strcat(file_path, "/");
  bname = basename(filename->data);
  strcat(file_path, bname);
  // RECEIVE FILE AND SAVE TO PATH
  receive_file_and_save_to_path(client_socket, file_path, ssl);
}

void command_list(int client_socket, struct client *client, SSL *ssl) {
  printf("Server recebeu comando LIST\n");
  DIR *dir;
  struct dirent *entry;
  char dirPath[1024];
  strcpy(dirPath, client->userid);

  if ((dir = opendir(dirPath)) == NULL) {
    perror("ERROR opendir: ");
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      send_data(entry->d_name, client_socket, strlen(entry->d_name) + 1, ssl);
    }
  }
  send_data(EO_LIST, client_socket, strlen(EO_LIST), ssl);

  if (closedir(dir) < 0) {
    perror("ERROR closedir: ");
  }
}
