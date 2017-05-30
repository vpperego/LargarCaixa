#include "../include/dropboxClient.h"
#include <errno.h>

char userid[MAXNAME];
int client_socket,synch_socket;
struct list_head file_list;

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
  Download the files from the server that are missing in the client machine
*/
void download_missing_files()
{
  file_t * missing_file;
  char * sync_dir_path = get_sync_dir(userid);
  char  fullpath[MAXNAME];
  while(true){
    if((missing_file=is_file_missing(userid,&file_list))== NULL)
      break;
     send_data(DOWNLOAD_FILE, synch_socket, (int)(strlen(DOWNLOAD_FILE) * sizeof(char)+1));
    send_data(missing_file->filename,synch_socket,strlen(missing_file->filename)*sizeof(char)+1);

    strcpy(fullpath,sync_dir_path);
    strcat(fullpath,"/");
    strcat(fullpath,missing_file->filename);
    receive_file_and_save_to_path(synch_socket,fullpath);
  }

}


void * synch_thread()
{
  DIR *dir;
  struct dirent *ent;
  char * sync_dir_path = get_sync_dir(userid);
  char  fullpath[MAXNAME];//TODO - FIX THIS SIZE
  struct stat file_stat;
  file_t * current_file ;
  struct buffer * server_file;
  char * file_buffer;
  INIT_LIST_HEAD(&file_list);

  //tell the server to create a thread to synchronize with this one
  send_data(CREATE_SYNCH_THREAD, synch_socket, (int)(strlen(CREATE_SYNCH_THREAD) * sizeof(char)));
  //send the userid for the new server thread
  send_data(userid, synch_socket, (int)(strlen(userid) * sizeof(char)));

  while(true){
     server_file = read_data(synch_socket);

    if(strcmp(FILE_SEND_OVER,server_file->data)==0)
      break;
    current_file = char_to_file_t(server_file->data);

    list_add(&current_file->file_list, &file_list);
    //free(server_file->data);
  }

  download_missing_files();

   do{
      //CHECK IF A FILE WAS DELETED
     if((current_file=is_file_missing(userid,&file_list)) != NULL){
       send_data(DELETE_FILE,synch_socket,strlen(DELETE_FILE)*sizeof(char));
       send_data(current_file->filename,synch_socket,sizeof(current_file->filename)*sizeof(char)+1);

       list_del(&current_file->file_list);
     }

      dir = opendir (sync_dir_path);
      while((ent = readdir (dir)) != NULL){
        strcpy(fullpath,sync_dir_path);
        strcat(fullpath,ent->d_name);

         if(is_a_file(ent->d_name)==false)
          continue;

        if((current_file=file_list_search(&file_list,ent->d_name))!=NULL){
           stat(fullpath, &file_stat);
          if(difftime(file_stat.st_mtime,current_file->last_modified) > 0)
          {
            send_data(SENDING_FILE,synch_socket,strlen(SENDING_FILE)*sizeof(char)+1);

            current_file->last_modified = file_stat.st_mtime;
            file_buffer = file_t_to_char(current_file);
            send_data(current_file->filename,synch_socket,strlen(current_file->filename)*sizeof(char)+1);
            send_file_from_path(synch_socket, fullpath);
          }

        }else if((current_file=is_file_missing(userid,&file_list)) != NULL){
          send_data(RENAME_FILE,synch_socket,strlen(DELETE_FILE)*sizeof(char)+1);
          send_data(current_file->filename,synch_socket,sizeof(current_file->filename)*sizeof(char));

          list_del(&current_file->file_list);
          file_list_add(&file_list, fullpath);

          send_data(ent->d_name,synch_socket,strlen(ent->d_name)*sizeof(char));
          send_file_from_path(synch_socket, fullpath);
        }else{

            file_list_add(&file_list,fullpath);
            send_data(SENDING_FILE,synch_socket,strlen(SENDING_FILE)*sizeof(char)+1);
            send_data(ent->d_name,synch_socket,strlen(ent->d_name)*sizeof(char)+1);
          //  printf("ENVIOU O NOME\n" );

            send_file_from_path(synch_socket, fullpath);
          //  printf("ENVIOU O arq\n" );

        }

      }
        closedir(dir);
        sleep(5);
  }while(true);
}

void get_all_files(char * sync_dir_path)
{
  char fullpath[256];
  struct buffer *filename;

  send_data(GET_ALL_FILES, client_socket, (int)(strlen(GET_ALL_FILES) * sizeof(char)));
  while(true){
    filename = read_data(client_socket);
    strcpy(fullpath,sync_dir_path);
    strcat(fullpath,"/");
    strcat(fullpath,filename->data);
    if(strcmp(FILE_SEND_OVER,filename->data)==0)
      break;
//      printf("vamos ler o filename %s!!\n",fullname);
    receive_file_and_save_to_path(synch_socket,fullpath);  /* code */
  }
}

/*
  Create the sync_dir (if necessary) and starts the sync thread
*/
void start_sync_service(char *host, int port)
{
  char *sync_dir_path = get_sync_dir(userid);
  pthread_t th;

  synch_socket =  connect_server(host,port);

  DIR * sync_dir = opendir(sync_dir_path);
  if(ENOENT == errno)
  {
      mkdir(sync_dir_path, 0777);
      get_all_files(sync_dir_path);
  }
  closedir(sync_dir);
  pthread_create(&th, NULL, synch_thread,NULL);
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
  send_data(userid, client_socket, (int)(strlen(userid) * sizeof(char)));

  start_sync_service(argv[2], atoi(argv[3]));

  start_client_interface();
  pthread_exit(0);
}
