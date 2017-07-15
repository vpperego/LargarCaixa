#include "../include/dropboxSharedSocket.h"

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
