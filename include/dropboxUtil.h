#ifndef __dropboxUtil__
#define __dropboxUtil__

#define NEW_CONNECTION "NEW_CONNECTION"
#define SEND_NAME "SEND_NAME"
#define EO_LIST "*END_OF_LIST_TAG*"

#include <stdint.h>

typedef enum { false, true } bool;

typedef uint32_t datasize_t;

char *read_line(void);
char **split_args(char *command);
#endif
