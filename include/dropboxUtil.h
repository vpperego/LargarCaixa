#ifndef __dropboxUtil__
#define __dropboxUtil__


#define NEW_CONNECTION "NEW_CONNECTION"

typedef enum { false, true } bool;

char * read_line(void);
char **split_args(char *command);
#endif
