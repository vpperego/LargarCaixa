#ifndef __dropboxClient__
#define __dropboxClient__

#include "dropboxUtil.h"
#define CLIENT_ARGUMENTS 4
#define FUNCTION_COUNT 5
#define MAXNAME 256
#define ARGUMENTS "Client IP Port"

int connect_server(char *host, int port);
void sync_client();
void send_file(char *file);
void get_file(char *file);
void close_connection();

bool command_download(char **args);
bool command_upload(char **args);
bool command_list(char **args);
bool command_get_sync_dir(char **args);
bool command_exit(char **args);
void send_data(char *data, int sockfd, int datalen);
#endif
