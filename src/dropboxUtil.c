#include "../include/dropboxUtil.h"

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

file_t* file_list_search(struct list_head *file_list, char *filename) {
  file_t *iterator;
  list_for_each_entry(iterator, file_list, file_list)
    if(strcmp(filename, iterator->filename) == 0)
      return iterator;
  return NULL;
}

file_t* file_list_add(struct list_head *file_list ,char* filename,char *userid) {
  file_t *new_file = malloc(sizeof(file_t));
  struct stat file_stat;
  strcpy(new_file->filename, filename);
  char *filepath = get_sync_dir(userid);
  strcat(filepath,filename);

  stat(filepath, &file_stat);

  new_file->last_modified = file_stat.st_mtime;
  list_add(&new_file->file_list, file_list);
  return new_file;
}

/*
  Check if the given file name is not the own directory or the parent directory
*/
bool is_a_file(char *filename)
{
  if(strcmp(filename,".")!=0 && strcmp(filename,"..")!=0)
    return true;
  else
    return false;
}

char * get_sync_dir(char *userid)
{
  struct passwd *pw = getpwuid(getuid());
  char *sync_dir_path = malloc(sizeof(char )*256);
  const char *homedir = pw->pw_dir;

  strcpy(sync_dir_path, homedir);
  strcat(sync_dir_path, "/sync_dir_");
  strcat(sync_dir_path, userid);
  strcat(sync_dir_path,"/");
  return sync_dir_path;
}
