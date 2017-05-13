#ifndef dropboxServerCommandHandler_h
#define dropboxServerCommandHandler_h

#include "dropboxServer.h"

void command_exit(int client_socket, struct client *client);
void command_upload(int client_socket, struct client *client);
void command_download(int client_socket, struct client *client);
void command_list(int client_socket, struct client *client);

#endif
