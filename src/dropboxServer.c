#include "../include/dropboxServer.h"
#include "../include/dropboxServerCommandHandler.h"

SSL * ssl;
dbsem_t list_access_mux;
dbsem_t file_list_access_mux;
dbsem_t open_session_mux;
sem_t * first_rm_sem;// (TODO - fix compatibility with apple API to use sem between process)semaphore for synchronization between server and first rm for reading shared memory
struct list_head *rm_list;

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


void send_all_files(char *userid, int sockfd) {
  DIR *dir;
  struct dirent *ent;
  char filepath[256];
  if ((dir = opendir(userid)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {

      if (is_a_file(ent->d_name)) {
        send_data(ent->d_name, sockfd,
                  strlen(ent->d_name) * sizeof(char), ssl); // send filename
        strcpy(filepath, userid);
        strcat(filepath, "/");
        strcat(filepath, ent->d_name);
        send_file_from_path(sockfd, filepath, ssl);
      }
    }
    send_data(FILE_SEND_OVER, sockfd, strlen(FILE_SEND_OVER) * sizeof(char), ssl);
    closedir(dir);
  } else
    printf("ERRO EM OPENDIR\n");
}

/*@TODO get actual file info and refactor eveything to use this list
bool add_to_files_list(client_t *client) {
  int i;
  dbsem_wait(&file_list_access_mux);


  for (i = 0; i < MAXFILES; i++) {
    if (client->files[i].size == 0) { // file is empty
      client->files[i].size = 1;      // temp
      break;
    }
  }

  dbsem_post(&file_list_access_mux);
  return i != MAXFILES;
}
*/
void *client_thread(void *thread_info) {

  printf("Starting client_thread\n" );
  struct thread_info *ti = (struct thread_info *)thread_info;
  struct buffer *command;
  client_t *client;

  dbsem_wait(&list_access_mux);
  if ((client = client_list_search(ti->userid)) == NULL) {
    printf("%s não registrado, registrar.\n", ti->userid);
    if ((client = client__list_signup(ti->userid)) == NULL) {
      printf("%s erro ao criar pasta!\n", ti->userid);
    }
  }
  dbsem_post(&list_access_mux);

  int session_id = ti->newsockfd;

  dbsem_wait(&open_session_mux);
  if (!client_open_session(client, session_id)) {
    dbsem_post(&open_session_mux);
    printf("%s já está usando todos os devices.\n", client->userid);
    send_data(CONNECTION_FAIL, ti->newsockfd, sizeof(CONNECTION_FAIL), ssl);
    close(ti->newsockfd);
    pthread_exit(NULL);
    return NULL;
  }
  dbsem_post(&open_session_mux);

  send_data(CONNECTION_OK, ti->newsockfd, sizeof(CONNECTION_OK), ssl);
  while (true) {

    command = read_data(((struct thread_info *)thread_info)->newsockfd, ssl);
//    printf("RECEBEU COMANDO\n");


    //@TODO refactor
    if (strcmp(command->data, GET_ALL_FILES) == 0) {
      send_all_files(client->userid,
                     ((struct thread_info *)thread_info)->newsockfd);
    }
    if (strcmp(command->data, "upload") == 0) {
  //    if (add_to_files_list(client))
        command_upload(((struct thread_info *)thread_info)->newsockfd, client, ssl);
    } else if (strcmp(command->data, "list") == 0) {
      command_list(((struct thread_info *)thread_info)->newsockfd, client, ssl);
    } else if (strcmp(command->data, "download") == 0) {
      command_download(((struct thread_info *)thread_info)->newsockfd, client, ssl);
    } else if (strcmp(command->data, "exit") == 0) {
      command_exit(((struct thread_info *)thread_info)->newsockfd, client);
    }
    // free(command);//is this right?
  }
  return NULL;
}

/*
  Get the proper info for thread_info and start the main synch service

*/
void * start_synch (void *thread_info){
  struct thread_info *ti = (struct thread_info *)thread_info;

  char *user;
     user = read_user_name(ti->newsockfd, ti->ssl);
   strcpy(ti->userid,user);

  client_t * client ;

  dbsem_wait(&list_access_mux);
  if ((client = client_list_search(ti->userid)) == NULL) {
    printf("%s não encontrado!\n", ti->userid);

    exit(0);
  }
  dbsem_post(&list_access_mux);

  ti->sem = client->sem ;
  ti->rm_list = client->rm_list ;
  ti->file_list = client->file_list ;
  printf("Creating synch_server\n");
  synch_server(ti);
  return NULL;
}

/*
 Executes the main socket listen.
 */
void server_listen(int server_socket) {
  int newsockfd;
  socklen_t clilen;
  struct sockaddr_in cli_addr;
  pthread_t th;
  char *userid ;

  // wait for new connections and create a new thread for each client
  printf("Dropbox Server Listening...");

  listen(server_socket, 5);

  clilen = sizeof(struct sockaddr_in);
  while (true) {
    if ((newsockfd = accept(server_socket, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1)
      perror("ERROR ACCEPT: ");
    ssl = startServerSSL();
    SSL_set_fd(ssl,	newsockfd);
    int ssl_err = SSL_accept(ssl);
    if(ssl_err <= 0)
    {
      ShutdownSSL(ssl);
    }
    printf("\nAceitou conexão de um socket.\n");
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));

    userid = read_user_name(newsockfd, ssl);

    thread_info->newsockfd = newsockfd;
    thread_info->ssl = ssl;
    strcpy(thread_info->userid, userid);
    if (strcmp(userid, CREATE_SYNCH_THREAD) == 0)
    {
      thread_info->isServer = true;

      pthread_create(&th, NULL, start_synch, thread_info);

    }
    else
      pthread_create(&th, NULL, client_thread, thread_info);
  }
}

void start_replica_manager(){
  pid_t first,second;

   rm_list = malloc(sizeof(rm_list));

  INIT_LIST_HEAD(rm_list);
  //create first replica manager
  first = fork();
  if(first == 0)
  {
    main_replica_manager(RM_PORT,2);

  }else{
    rm_t * new_rm = malloc(sizeof(rm_t));
    new_rm->ssl = startCliSSL();
    new_rm->port = RM_PORT;
    strcpy(new_rm->address,"localhost");
  //  new_rm->newsockfd = connect_server("localhost", RM_PORT, new_rm->ssl);
    list_add(&new_rm->rm_list, rm_list);
  }

  second = fork();
  if(second==0){
    main_replica_manager(RM_PORT+1,3);
  }else{
    rm_t * new_rm = malloc(sizeof(rm_t));
    new_rm->ssl = startCliSSL();
    strcpy(new_rm->address,"localhost");
    new_rm->port = RM_PORT + 1;
//    new_rm->newsockfd = connect_server("localhost", RM_PORT+1, new_rm->ssl);
    list_add(&new_rm->rm_list, rm_list);

  }

}

void client_list_init() {
  dbsem_init(&file_list_access_mux, 1);
  dbsem_init(&list_access_mux,1);
  dbsem_init(&open_session_mux,1);
  INIT_LIST_HEAD(&client_list);
}

rm_t * start_client_rm_connection(rm_t * rm){
  rm_t * new_rm = malloc(sizeof(rm_t));
  new_rm->ssl = startCliSSL();
  new_rm->newsockfd = connect_server(rm->address,rm->port, new_rm->ssl);

  return new_rm;
}

 struct list_head * create_client_rm_list(char *userid){
    rm_t *iterator;
    struct list_head *client_rm_list = malloc(sizeof(client_rm_list));
    INIT_LIST_HEAD(client_rm_list);

    list_for_each_entry(iterator,rm_list,rm_list){
      rm_t * new_rm  = start_client_rm_connection(iterator);
      send_data(userid,new_rm->newsockfd,strlen(userid),new_rm->ssl);  
      list_add(&new_rm->rm_list,client_rm_list);

    }
    return client_rm_list;
}

/*
  Creates the file_list of @userid
*/
struct list_head *  create_server_file_list(char * userid){
  struct list_head *file_list = malloc(sizeof(file_list));
  DIR *dir;
  struct dirent *ent;
    char fullpath[MAXNAME];
  INIT_LIST_HEAD(file_list);
//  struct buffer *sendFiles = read_data(ti->newsockfd);
  if ((dir = opendir(userid)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir(dir)) != NULL) {
          if (is_a_file(ent->d_name) == true) {
              strcpy(fullpath, userid);
              strcat(fullpath, "/");
              strcat(fullpath, ent->d_name);
              file_list_add(file_list, fullpath);
          }
      }
  }
  closedir(dir);
  return file_list;
}

/*
  Sign client to dropbox: create the user id, file list and rm list
*/
client_t *client__list_signup(char *userid) {
  client_t *client = malloc(sizeof(client_t));
  strcpy(client->userid, userid);
  client->logged_in = false;
  memset(client->devices, DEVICE_FREE, sizeof(client->devices));
  client->sem = malloc(sizeof(dbsem_t));
  dbsem_init(client->sem,1);
  client->rm_list = create_client_rm_list(client->userid);
  client->file_list =create_server_file_list(userid);
  /*memset(client->files, 0, sizeof(client->files));*/
  mkdir(client->userid, 0777);

  list_add(&client->client_list, &client_list);
  return client;
}

client_t *client_list_search(char *userid) {
  client_t *iterator;
  list_for_each_entry(iterator, &client_list,
                      client_list) if (strcmp(userid, iterator->userid) ==
                                       0) return iterator;
  return NULL;
}

bool client_open_session(client_t *client, int device_id) {
  int i;
  for (i = 0; i < MAX_SESSIONS; ++i) {
    if (client->devices[i] == DEVICE_FREE) {
      client->devices[i] = device_id;
      client->logged_in = true;
      printf("%s abriu sessão com device_id: %d\n", client->userid, device_id);
      return true;
    }
  }
  return false;
}

bool client_close_session(client_t *client, int device_id) {
  int i;
  for (i = 0; i < MAX_SESSIONS; ++i) {
    if (client->devices[i] == device_id) {
      client->devices[i] = DEVICE_FREE;
      if (i == MAX_SESSIONS - 1)
        client->logged_in = false;
      printf("%s fechou sessão com device_id: %d\n", client->userid, device_id);
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  int server_socket = start_server(SERVER_PORT);
  client_list_init();
  if(argc<2)
    start_replica_manager();
  server_listen(server_socket);
  return 0;
}
