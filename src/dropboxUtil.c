#include "../include/dropboxUtil.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *read_line(void) {
  char *buffer;
  size_t bufsize = 1024;

  buffer = (char *)malloc(bufsize * sizeof(char));
  if (buffer == NULL) {
    perror("Unable to allocate buffer");
    exit(1);
  }

  getline(&buffer, &bufsize, stdin);
  // remove new line char
  buffer[strcspn(buffer, "\n")] = 0;
  return buffer;
}

char **split_args(char *command) {
  int bufsize = 1024, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(command, " ");
  while (token != NULL) {
    tokens[position] = token;
    position++;

    token = strtok(NULL, " ");
  }
  tokens[position] = NULL;
  return tokens;
}
