#include "../include/dropboxClient.h"
#include <errno.h>

char userid[MAXNAME];
int client_socket, synch_socket;
SSL * ssl;

void printSSLCert(){
  X509 *cert;
  char *line;
  cert	=	SSL_get_peer_certificate(ssl);
  if	(cert	!=	NULL){
        line =	X509_NAME_oneline(
              X509_get_subject_name(cert),0,0);
        printf("Subject:	%s\n",	line);
        free(line);
        line	=	X509_NAME_oneline(
              X509_get_issuer_name(cert),0,0);
        printf("Issuer:	%s\n",	line);
  }
}

void startSSL(){
	const SSL_METHOD *method;
  SSL_CTX *ctx;
	OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  SSL_library_init();
  method	=	SSLv3_client_method();
  ctx	=	SSL_CTX_new(method);
  if	(ctx	==	NULL){
        ERR_print_errors_fp(stderr);
        abort();
  }

  ssl	=	SSL_new(ctx);
}

/* From Assignment Specification
 * Connects the client to the server.
 * host - server address
 * port - server port
 */
int connect_server(char *host, int port) {
  struct hostent *server;
  struct sockaddr_in serv_addr;
  int sockfd;

  startSSL();

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
  SSL_set_fd(ssl,	sockfd);
  if	(SSL_connect(ssl)	==	-1) {
    ERR_print_errors_fp(stderr);
  }
  else {
    printSSLCert();

    char buffer[256];
    strcpy(buffer, userid);

    // send userid to server
    //  send_data(userid, sockfd, (int)(strlen(userid) * sizeof(char)));
  }

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
  send_data(command, client_socket, (strlen(command) * sizeof(char)), ssl);
  send_data(file, client_socket, (strlen(file) * sizeof(char)), ssl);
  send_file_from_path(client_socket, file, ssl);
}

/* From Assignment Specification
 * Fetch a file named "file" from the server.
 * Must be called in order to download a file.
 * file - filename.ext
 */
void get_file(char *file) {
  char *command = "download";
  send_data(command, client_socket, (strlen(command) * sizeof(char)), ssl);
  send_data(file, client_socket, (strlen(file) * sizeof(char)), ssl);
  receive_file_and_save_to_path(client_socket, file, ssl);
}

/* From Assignment Specification
 * Closes the connection to the server
 */
void close_connection() {
  char *command = "exit";
  send_data(command, client_socket, (strlen(command) * sizeof(char)), ssl);
  close(client_socket);
}

void get_all_files(char *sync_dir_path) {
  char fullpath[256];
  struct buffer *filename;

  send_data(GET_ALL_FILES, client_socket, strlen(GET_ALL_FILES) * sizeof(char), ssl);
  while (true) {
    filename = read_data(client_socket, ssl);
    if (strcmp(FILE_SEND_OVER, filename->data) == 0)
      break;
    strcpy(fullpath, sync_dir_path);
    strcat(fullpath, "/");
    strcat(fullpath, filename->data);
    receive_file_and_save_to_path(synch_socket, fullpath, ssl); /* code */
  }
}

/*
  Create the sync_dir (if necessary) and starts the sync thread
*/
void start_sync_service(char *host, int port) {
  char *sync_dir_path = get_sync_dir(userid);
  pthread_t th;//,th2;

  synch_socket = connect_server(host, port);

  DIR *sync_dir = opendir(sync_dir_path);

  //creates the synch_dir if it's not created
  if (errno == ENOENT) {
    mkdir(sync_dir_path, 0777);
    sync_dir = opendir(sync_dir_path);
    get_all_files(sync_dir_path);
  }
  closedir(sync_dir);

//   tell the server to create a thread (synch_server) to synchronize with this one
  send_data(CREATE_SYNCH_THREAD, synch_socket,
            strlen(CREATE_SYNCH_THREAD) * sizeof(char), ssl);
  // send the userid for the new server thread

  send_data(userid, synch_socket, strlen(userid) * sizeof(char), ssl);

  struct thread_info *ti = malloc(sizeof(struct thread_info));
  strcpy(ti->userid, userid);
  ti->working_directory = sync_dir_path;
  ti->newsockfd = synch_socket;
  ti->isServer = false;
  ti->ssl = ssl;
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
  send_data(userid, client_socket, strlen(userid) * sizeof(char), ssl);
  struct buffer *server_response;
  server_response = read_data(client_socket, ssl);
  if (strcmp(server_response->data, CONNECTION_FAIL) == 0) {
    printf("Numero maximo de conexoes atingido\n");
    close(client_socket);
    exit(0);
  }

  start_sync_service(argv[2], atoi(argv[3]));

  start_client_interface(ssl);
  exit(0);
}
