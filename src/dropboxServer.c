#include "../include/dropboxServer.h"
#include "../include/dropboxServerCommandHandler.h"
#include <semaphore.h>

dbsem_t list_access_mux;
dbsem_t file_list_access_mux;
dbsem_t open_session_mux;
sem_t * first_rm_sem;// (TODO - fix compatibility with apple API to use sem between process)semaphore for synchronization between server and first rm for reading shared memory
void * rm_shared_memory;

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

void send_all_files(char *userid, int sockfd) {
  DIR *dir;
  struct dirent *ent;
  char filepath[256];
  if ((dir = opendir(userid)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {

      if (is_a_file(ent->d_name)) {
        send_data(ent->d_name, sockfd,
                  strlen(ent->d_name) * sizeof(char)); // send filename
        strcpy(filepath, userid);
        strcat(filepath, "/");
        strcat(filepath, ent->d_name);
        send_file_from_path(sockfd, filepath);
      }
    }
    send_data(FILE_SEND_OVER, sockfd, strlen(FILE_SEND_OVER) * sizeof(char));
    closedir(dir);
  } else
    printf("ERRO EM OPENDIR\n");
}

//@TODO get actual file info and refactor eveything to use this list
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
    send_data(CONNECTION_FAIL, ti->newsockfd, sizeof(CONNECTION_FAIL));
    close(ti->newsockfd);
    pthread_exit(NULL);
    return NULL;
  }
  dbsem_post(&open_session_mux);

  send_data(CONNECTION_OK, ti->newsockfd, sizeof(CONNECTION_OK));
  while (true) {
    // read command from client
//    printf("ESPERANDO COMANDO\n");
    command = read_data(((struct thread_info *)thread_info)->newsockfd);
//    printf("RECEBEU COMANDO\n");

    //@TODO refactor
    if (strcmp(command->data, GET_ALL_FILES) == 0) {
      send_all_files(client->userid,
                     ((struct thread_info *)thread_info)->newsockfd);
    }
    if (strcmp(command->data, "upload") == 0) {
      if (add_to_files_list(client))
        command_upload(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "list") == 0) {
      command_list(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "download") == 0) {
      command_download(((struct thread_info *)thread_info)->newsockfd, client);
    } else if (strcmp(command->data, "exit") == 0) {
      command_exit(((struct thread_info *)thread_info)->newsockfd, client);
    }
    // free(command);//is this right?
  }
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

  listen(server_socket, 50);

  clilen = sizeof(struct sockaddr_in);

  // wait for new connections and create a new thread for each client
  printf("Dropbox Server Listening...");
  while (true) {
    if ((newsockfd = accept(server_socket, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1)
      perror("ERROR ACCEPT: ");
    printf("\nAceitou conexão de um socket.\n");
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));
     
    userid = read_user_name(newsockfd);

    thread_info->newsockfd = newsockfd;
    strcpy(thread_info->userid, userid);
    if (strcmp(userid, CREATE_SYNCH_THREAD) == 0)
    {

  //    printf("Enviando para RM socket %d\n",thread_info->newsockfd );
      //memcpy(rm_shared_memory,&thread_info->newsockfd,sizeof(thread_info->newsockfd));
      //sem_post(first_rm_sem);

       pthread_create(&th, NULL, synch_server, thread_info);

    }
    else
      pthread_create(&th, NULL, client_thread, thread_info);
  }
}

void start_replica_manager(){
  pid_t first;

  // Our memory buffer will be readable and writable:
 int protection = PROT_READ | PROT_WRITE;

 // The buffer will be shared (meaning other processes can access it), but
 // anonymous (meaning third-party processes cannot obtain an address for it),
 // so only this process and its children will be able to use it:
 int visibility = MAP_ANONYMOUS | MAP_SHARED;

 rm_shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, protection, visibility, 0, 0);

 first_rm_sem = mmap (NULL,sizeof(dbsem_t), protection, visibility, 0, 0);// starts the semaphore for synchronization between rm and server
 sem_init(first_rm_sem,1,0);

 memcpy(rm_shared_memory,first_rm_sem,sizeof(dbsem_t));
 //check if the shared memory creation worked
 if (rm_shared_memory == MAP_FAILED)
      perror("ERROR ON CREATING SHARED MEMORY");

  //create first replica manager
  first = fork();
  if(first == 0)
    main_replica_manager(rm_shared_memory, first_rm_sem);

  //TODO - create other rms
}

void client_list_init() {
  dbsem_init(&file_list_access_mux, 1);
  dbsem_init(&list_access_mux,1);
  dbsem_init(&open_session_mux,1);
  INIT_LIST_HEAD(&client_list);
}

client_t *client__list_signup(char *userid) {
  client_t *client = malloc(sizeof(client_t));
  strcpy(client->userid, userid);
  client->logged_in = false;
  memset(client->devices, DEVICE_FREE, sizeof(client->devices));
  /*memset(client->files, 0, sizeof(client->files));*/
  if (mkdir(client->userid, 0777) < 0) {
    // perror("ERROR MKDIR: ");
    // return NULL;
  }
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
  int server_socket = start_server();
  client_list_init();
  start_replica_manager();
  server_listen(server_socket);
  return 0;
}
