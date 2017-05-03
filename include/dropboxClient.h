#ifndef __dropboxClient__
#define __dropboxClient__

#define CLIENT_ARGUMENTS 4
#define ARGUMENTS "Client IP Port"


int connect_server(char *host, int port);
void sync_client();
void send_file(char *file);
void get_file(char *file);
void close_connection();

#endif
