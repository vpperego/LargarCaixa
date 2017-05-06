#ifndef __dropboxServer__
#define __dropboxServer__

#define MAXNAME 256
#include "dropboxUtil.h"
#define MAXFILES 30
#define PORT 55000
#define MAX_SESSIONS 2

struct	file_info	{
    char name[MAXNAME];
    char extension[MAXNAME];
    char last_modified[MAXNAME];
    int size;
};

struct	client	{
    int devices[MAX_SESSIONS];
    char userid[MAXNAME];
    struct	file_info files[MAXFILES];//;
    int logged_in;
};

struct buffer {
    char *data;
    int size;
};

void sync_server();
bool is_client_valid(void);
void *client_thread(void * client_socket);
void receive_file(char *file);
void send_file(char *file);
struct buffer * read_data(int newsockfd);
void read_user_name(int newsockfd);
void server_listen(int server_socket);
#endif
