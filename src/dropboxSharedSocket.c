#include "../include/dropboxSharedSocket.h"

void printSSLCert(SSL *ssl){
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

SSL * startCliSSL(){
  SSL * new_ssl;
	const SSL_METHOD *method;
  SSL_CTX *ctx;
	OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  SSL_library_init();
  method	=	TLSv1_client_method();
  ctx	=	SSL_CTX_new(method);
  if	(ctx	==	NULL){
        ERR_print_errors_fp(stderr);
        abort();
  }

  new_ssl	=	SSL_new(ctx);
  return new_ssl;
}

void ShutdownSSL(SSL *ssl)
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

SSL * startServerSSL() {
  SSL *new_ssl;
	const SSL_METHOD *method;
  SSL_CTX *ctx;
	OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  SSL_library_init();
  method	=	TLSv1_server_method();
  ctx	=	SSL_CTX_new(method);
  if	(ctx	==	NULL){
        ERR_print_errors_fp(stderr);
        abort();
  }
  int use_cert = SSL_CTX_use_certificate_file(ctx,	"CertFile.pem",	SSL_FILETYPE_PEM);
  int use_prv = SSL_CTX_use_PrivateKey_file(ctx,	"KeyFile.pem",	SSL_FILETYPE_PEM);
  new_ssl	=	SSL_new(ctx);
  return new_ssl;
}



/* From Assignment Specification
 * Connects the client to the server.
 * host - server address
 * port - server port
 * SSL - assigned ssl
 */
int connect_server(char *host, int port,SSL *ssl) {
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

  SSL_set_fd(ssl,	sockfd);
  if	(SSL_connect(ssl)	==	-1) {
    ERR_print_errors_fp(stderr);
  }
  else {

    printSSLCert(ssl);

  }

  return sockfd;
}


int start_server(int port) {
  int sockfd;

  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    perror("ERROR opening socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    perror("ERROR on binding");

  return sockfd;
}



void send_file_from_path(int socket, char *path, SSL * ssl) {
  char *source = NULL;
  FILE *fp = NULL;
  if ((fp = fopen(path, "r")) == NULL) {
    perror("ERROR FOPEN: ");
    return;
  }
  if (fp != NULL) {
    /* Go to the end of the file. */
    if (fseek(fp, 0L, SEEK_END) == 0) {
      /* Get the size of the file. */
      long bufsize = ftell(fp);
      if (bufsize == -1) { /* Error */
      }

      /* Allocate our buffer to that size. */
      source = malloc(sizeof(char) * (bufsize + 1));

      /* Go back to the start of the file. */
      if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */
      }

      /* Read the entire file into memory. */
      datasize_t newLen = fread(source, sizeof(char), bufsize, fp);
      if (ferror(fp) != 0) {
        fputs("Error reading file", stderr);
      }
      send_data(source, socket, (newLen * sizeof(char)), ssl);
    }
    fclose(fp);
  }
  free(source);
}

void receive_file_and_save_to_path(int socket, char *path, SSL * ssl) {
  struct buffer *data = read_data(socket, ssl);
  /*printf("Data size: %zi\n", data->size);*/
  /*printf("Data: %s\n", data->data);*/
  FILE *fp;

  fp = fopen(path, "w+");
  if (fp == NULL) {
    perror("ERROR - Failed to open file for writing\n");
    fclose(fp);
    exit(1);
  }

  if (fwrite(data->data, sizeof(char), data->size, fp) != data->size) {
    perror("ERROR - Failed to write bytes to file\n");
    fclose(fp);
    exit(1);
  }
  fclose(fp);
}

// send data with file size
void send_data(char *data, int sockfd, datasize_t datalen, SSL * ssl) {
  int n;
  datasize_t tmp = htonl(datalen);
  if ((n = (int)SSL_write(ssl, (void *)&tmp, sizeof(tmp))) < 0) {
    perror("ERROR writing to socket: ");
    close(sockfd);
    exit(0);
  }
  if ((n = (int)SSL_write(ssl, data, datalen)) < 0) {
    perror("ERROR writing to socket: ");
    close(sockfd);
    exit(0);
  }
}

// read data from client given the protocol '[datasize_t size][data size bytes]'
struct buffer *read_data(int newsockfd, SSL * ssl) {
  int n;
  datasize_t buflen;
  // read data size
  n = SSL_read(ssl, (char *)&buflen, sizeof(buflen));
  if (n < 0)
    perror("ERROR reading from socket");
  buflen = ntohl(buflen);
  char *buffer_data = malloc(sizeof(char) * buflen);
  datasize_t amount_read = 0;
  // keep reading data until size is reached
  while (amount_read < buflen) {
    n = (int)SSL_read(ssl, (void *)buffer_data + amount_read,
                  buflen - amount_read);
    amount_read += n;
    if (n < 0) {
      perror("ERROR reading from socket");
      close(newsockfd);
      exit(0);
    }
  }

  struct buffer *buffer = malloc(sizeof(struct buffer));
  buffer->data = buffer_data;
  buffer->size = sizeof(char) * buflen;
  return buffer;
}

char * check_valid_string(struct buffer *file)
{
  char *real_string;
  if(strlen(file->data) > file->size)
  {
      real_string = malloc((sizeof(char) * file->size));
      strncpy(real_string,file->data,file->size);
      real_string[file->size] = '\0';
    }else
      real_string = file->data ;
    return real_string;
}
