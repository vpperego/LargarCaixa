#ifndef __dropboxUtil__
#define __dropboxUtil__

#include "dropboxList.h"
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

#define GET_ALL_FILES "GET_ALL_FILES"
#define NEW_CONNECTION "NEW_CONNECTION"
#define SEND_NAME "SEND_NAME"
#define CREATE_SYNCH_THREAD "CREATE_SYNCH_THREAD"
#define FILE_SEND_OVER "FILE_SEND_OVER"
#define EO_LIST "*END_OF_LIST_TAG*"

typedef enum { false, true } bool;

typedef struct file
{
  struct list_head file_list;
  char filename[256];
  time_t last_modified;
}file_t;


char *read_line(void);
char **split_args(char *command);
file_t* file_list_search(struct list_head *file_list, char *filename);
file_t* file_list_add(struct list_head *file_list ,char* filename,char *userid) ;
char * get_sync_dir(char *userid);
bool is_a_file(char *filename);

#endif
