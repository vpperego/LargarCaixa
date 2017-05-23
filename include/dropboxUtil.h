#ifndef __dropboxUtil__
#define __dropboxUtil__

#define GET_ALL_FILES "GET_ALL_FILES"
#define NEW_CONNECTION "NEW_CONNECTION"
#define SEND_NAME "SEND_NAME"
#define FILE_SEND_OVER "FILE_SEND_OVER"
#define EO_LIST "*END_OF_LIST_TAG*"

typedef enum { false, true } bool;

char *read_line(void);
char **split_args(char *command);
#endif
