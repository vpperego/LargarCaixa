#ifndef __dropboxServer__
#define __dropboxServer__

#define MAXNAME 256
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

void sync_server();
void receive_file(char *file);
void send_file(char *file);
#endif
