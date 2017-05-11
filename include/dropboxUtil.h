#ifndef __dropboxUtil__
#define __dropboxUtil__

#define NEW_CONNECTION "NEW_CONNECTION"
#define SEND_NAME "SEND_NAME"
#define EO_LIST "*END_OF_LIST_TAG*"

typedef enum { false, true } bool;

char *read_line(void);
char **split_args(char *command);
#endif
