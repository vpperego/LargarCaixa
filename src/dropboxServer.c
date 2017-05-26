#include "../include/dropboxServer.h"
#include "../include/dropboxServerCommandHandler.h"

int readcount = 0, writecount = 0;
dbsem_t rmutex, wmutex, read_try, list_access;

/* From Assignment Specification
 * Synchronizes the directory named "synch_dir_<username>" with the clients
 * synch_dir.
 */
void * synch_server(void *thread_info)
{
  struct thread_info *ti = (struct thread_info *)thread_info;
  struct list_head * file_list = malloc(sizeof(file_list));
  printf("SYNC THREAD DO USER %s!!!\n",ti->userid);
  char *userid = read_user_name(ti->newsockfd);

  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir (userid)) != NULL) {
  /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      if(is_a_file(ent->d_name)==true)
      {
        file_list_add(file_list,ent->d_name, userid);
      }
    }
  }
  /*
     start file list and send to client
  */

  return NULL;
}

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

void send_all_files(char *userid,int sockfd)
{
//  printf("send_all_files\n" );
  DIR *dir;
  struct dirent *ent;
  char filepath[256];
  if ((dir = opendir (userid)) != NULL) {
  /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {

      if(is_a_file(ent->d_name)==true)
      {
        send_data(ent->d_name, sockfd, (int)(strlen(ent->d_name) * sizeof(char))); //send filename
        strcpy(filepath,userid);
        strcat(filepath,"/");
        strcat(filepath,ent->d_name);
        send_file_from_path(sockfd,filepath);
      }
    }
    send_data(FILE_SEND_OVER, sockfd, (int)(strlen(FILE_SEND_OVER) * sizeof(char)));
    closedir (dir);
  }
  else
    printf("ERRO EM OPENDIR\n" );

}
void *client_thread(void *thread_info) {

  struct thread_info *ti = (struct thread_info *)thread_info;
  struct buffer *command;
  client_t *client;

  dbsem_wait(&wmutex);
  writecount++;
  if(writecount == 1) dbsem_wait(&read_try);
  dbsem_post(&wmutex);

  dbsem_wait(&list_access);
  if((client = client_list_search(ti->userid)) == NULL) {
    printf("%s não registrado, registrar.\n", ti->userid);
    if((client = client__list_signup(ti->userid)) == NULL) {
      printf("%s erro ao criar pasta!\n", ti->userid);
      // error creating registering (mkdir error)
      // dbsem_post(&list_access); // since the server isnt loading old users at startup, these lines are commented
      // return NULL;              // currently we register all users again and the their folder may be already created
    }
  }

  dbsem_post(&list_access);

  dbsem_wait(&wmutex);
  writecount--;
  if(writecount == 0) dbsem_post(&read_try);
  dbsem_post(&wmutex);

  int session_id = ti->newsockfd;

  if(client_open_session(client, session_id) == false) {
    printf("%s já está usando todos os devices.\n", client->userid);
    // reached the max number of sessions
    // should we put it to wait? something like semaphore?
    // or just kick it off?
    close(ti->newsockfd);
    pthread_exit(NULL);
    return NULL;
  }

  while (true) {
    // read command from client
    printf("ESPERANDO COMANDO\n");
    command = read_data(((struct thread_info *)thread_info)->newsockfd);
    printf("RECEBEU COMANDO\n");

    //@TODO refactor
    if (strcmp(command->data, GET_ALL_FILES) == 0) {
      send_all_files(client->userid,((struct thread_info *)thread_info)->newsockfd);
    } if (strcmp(command->data, "upload") == 0) {
      command_upload(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "list") == 0) {
      command_list(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "download") == 0) {
      command_download(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "exit") == 0) {
      command_exit(((struct thread_info *)thread_info)->newsockfd, client);
    }
    //free(command);//is this right?
  }
  return NULL;
}

char *read_user_name(int newsockfd) {
  printf("Vai ler username\n");
  struct buffer *buffer = read_data(newsockfd);
  printf("Username: %s\n", buffer->data);
  return buffer->data;
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
    char *userid = malloc(sizeof(char) * MAXNAME);
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));
    userid = read_user_name(newsockfd);

    thread_info->newsockfd = newsockfd;
    strcpy(thread_info->userid, userid);
    //TODO - if userid == synch then create synch_thread
    if(strcmp(userid,CREATE_SYNCH_THREAD)==0)
      pthread_create(&th, NULL, synch_server, thread_info);

    else
      pthread_create(&th, NULL, client_thread, thread_info);
  }
}

int main(int argc, char *argv[]) {
  int server_socket = start_server();
  client_list_init();
  server_listen(server_socket);
}

void client_list_init() {

  dbsem_init(&wmutex, 1);
  dbsem_init(&rmutex, 1);
  dbsem_init(&read_try, 1);
  dbsem_init(&list_access, 1);

  // if(is_client_list_in_disk()) {
  //   client_list_fread();
  // } else {
    INIT_LIST_HEAD(&client_list);
  // }
}

client_t* client__list_signup(char* userid) {
  client_t *client = malloc(sizeof(client_t));
  strcpy(client->userid, userid);
  client->logged_in = false;
  memset(client->devices, DEVICE_FREE, sizeof(client->devices));
  memset(client->files, 0, sizeof(client->files));
  if (mkdir(client->userid, 0777) < 0) {
    // perror("ERROR MKDIR: ");
    // return NULL;
  }
  list_add(&client->client_list, &client_list);
  return client;
}

client_t* client_list_search(char *userid) {
  client_t *iterator;
  list_for_each_entry(iterator, &client_list, client_list)
    if(strcmp(userid, iterator->userid) == 0)
      return iterator;
  return NULL;
}

bool client_open_session(client_t *client, int device_id) {
  for (int i = 0; i < MAX_SESSIONS; ++i) {
    if(client->devices[i] == DEVICE_FREE) {
      client->devices[i] = device_id;
      client->logged_in = true;
      printf("%s abriu sessão com device_id: %d\n", client->userid, device_id);
      return true;
    }
  }
  return false;
}

bool client_close_session(client_t *client, int device_id) {
  for (int i = 0; i < MAX_SESSIONS; ++i) {
    if(client->devices[i] == device_id) {
      client->devices[i] = DEVICE_FREE;
      if(i == MAX_SESSIONS-1) client->logged_in = false;
      printf("%s fechou sessão com device_id: %d\n", client->userid, device_id);
      return true;
    }
  }
  return false;
}

// bool is_client_list_in_disk() {
//     if((FILE *dbp = fopen(CLIENTDB_PATH, "r")) == NULL) {
//       fclose(dbp); return false;
//     } else  {
//       fclose(dbp); return true;
//     }
// }

// void client_list_fread() {
//   return;
// }

// void client_list_fwrite() {
//   return;
// }
