//
//  dropboxSharedSocket.h
//  LargarCaixa
//
//  Created by Pietro Degrazia on 5/8/17.
//  Copyright © 2017 Pietro Degrazia. All rights reserved.
//

#ifndef dropboxSharedSocket_h
#define dropboxSharedSocket_h

#define CONNECTION_OK "CONNECTION_OK"
#define CONNECTION_FAIL "CONNECTION_FAIL"
#include "dropboxUtil.h"
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct buffer {
  char *data;
  datasize_t size;
};


SSL * startCliSSL();
SSL * startServerSSL();
void ShutdownSSL(SSL *ssl);
int start_server(int port);
int connect_server(char *host, int port,SSL *ssl);
void send_file_from_path(int socket, char *path, SSL * ssl );
void receive_file_and_save_to_path(int socket, char *path, SSL * ssl);
void send_data(char *data, int sockfd, datasize_t datalen, SSL * ssl);
struct buffer *read_data(int newsockfd, SSL * ssl);
char * check_valid_string(struct buffer *file);

#endif /* dropboxSharedSocket_h */
