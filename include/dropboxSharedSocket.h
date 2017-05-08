//
//  dropboxSharedSocket.h
//  LargarCaixa
//
//  Created by Pietro Degrazia on 5/8/17.
//  Copyright © 2017 Pietro Degrazia. All rights reserved.
//

#ifndef dropboxSharedSocket_h
#define dropboxSharedSocket_h

#include "dropboxUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>

void send_data(char *data, int sockfd, int datalen);

#endif /* dropboxSharedSocket_h */
