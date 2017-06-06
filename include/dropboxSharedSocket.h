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

void send_file_from_path(int socket, char *path);
void receive_file_and_save_to_path(int socket, char *path);
void send_data(char *data, int sockfd, datasize_t datalen);
struct buffer *read_data(int newsockfd);

#endif /* dropboxSharedSocket_h */
