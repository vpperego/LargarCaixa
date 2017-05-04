#ifndef __dropboxUtil__
#define __dropboxUtil__

typedef enum { false, true } bool;

char * read_line(void);
char **split_args(char *command);
#endif
