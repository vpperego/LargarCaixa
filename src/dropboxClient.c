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

char * get_sync_dir(char *userid)
{
  struct passwd *pw = getpwuid(getuid());
  char *sync_dir_path = malloc(sizeof(char )*256);
  const char *homedir = pw->pw_dir;

  strcpy(sync_dir_path, homedir);
  strcat(sync_dir_path, "/sync_dir_");
  strcat(sync_dir_path, userid);
  strcat(sync_dir_path,"/");
  return sync_dir_path;
}

bool is_a_file(char *filename)
{
  if(strcmp(filename,".")!=0 && strcmp(filename,"..")!=0)
    return true;
  else
    return false;
}

file_t* file_list_search(char *filename) {
  file_t *iterator;
  list_for_each_entry(iterator, &file_list, file_list)
    if(strcmp(filename, iterator->filename) == 0)
      return iterator;
  return NULL;
}

file_t* file_list_add(char* filename) {
  file_t *new_file = malloc(sizeof(file_t));
  struct stat file_stat;
  strcpy(new_file->filename, filename);
  char *filepath = get_sync_dir(filename);
  strcat(filepath,filename);

  stat(filepath, &file_stat);

  new_file->last_modified = file_stat.st_mtime;
  list_add(&new_file->file_list, &file_list);
  return new_file;
}


void sync_thread()
{
  DIR *dir;
  struct dirent *ent;
  char * sync_dir_path = get_sync_dir(userid);
  char * fullpath=NULL;
  file_t * current_file;
  struct stat file_stat;

   //TODO - take the last change time from the server.
  if ((dir = opendir (sync_dir_path)) != NULL) {
    while((ent = readdir (dir)) != NULL){
      strcat(fullpath,sync_dir_path);
      strcat(fullpath,ent->d_name);
      //file is in the list, we need to compare the last modified time
      if(is_a_file(ent->d_name)==true
        && (current_file=file_list_search(ent->d_name))!=NULL){


          stat(fullpath, &file_stat);
          if(difftime(file_stat.st_mtime,current_file->last_modified) > 0)
          {
            current_file->last_modified = file_stat.st_mtime;
            send_file_from_path(client_socket, fullpath);
          }
        }else{// TODO - insert new file
            file_list_add(ent->d_name);
            send_file_from_path(client_socket,fullpath);
        }
      }
  }
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
