#ifndef __dropboxUtil__
#define __dropboxUtil__

#include "dropboxList.h"
#include <libgen.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SHARED_MEMORY_SIZE 1024
#define MAXNAME 256
#define DELETE_FILE "DELETE_FILE"
#define UPDATE_FILE "UPDATE_FILE"
#define DOWNLOAD_FILE "DOWNLOAD_FILE"
#define GET_ALL_FILES "GET_ALL_FILES"
#define NEW_CONNECTION "NEW_CONNECTION"
#define SEND_NAME "SEND_NAME"
#define CREATE_SYNCH_THREAD "CREATE_SYNCH_THREAD"
#define CREATE_SYNCH_LISTEN "CREATE_SYNCH_LISTEN"
#define FILE_SEND_OVER "FILE_SEND_OVER"
#define EO_LIST "*END_OF_LIST_TAG*"
#define CHECK_DONE "CHECK_DONE"

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t datasize_t;
#define SENDING_FILE "SENDING_FILE"

struct file_info {
  char filename[MAXNAME];
  struct list_head file_list;
  char extension[MAXNAME];
  time_t last_modified;
  int size;
};

typedef struct file_info file_t;

struct thread_info {
  int newsockfd;
  char userid[MAXNAME];
  char *working_directory;
  bool isServer;
  SSL * ssl;
};

char *read_line(void);
char **split_args(char *command);
file_t *file_list_search(struct list_head *file_list, char *filename);
file_t *file_list_add(struct list_head *file_list, char *fullpath);
char *get_sync_dir(char *userid);
bool is_a_file(char *filename);
char *file_t_to_char(file_t *file);
file_t *char_to_file_t(char *file);
void file_list_remove(struct list_head *file_list, char *filename);
file_t *is_file_missing(char *working_directory, struct list_head *file_list);
char *read_user_name(int newsockfd, SSL * ssl) ;
#endif
