#include "../include/dropboxUtil.h"
#include <dirent.h>

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

void file_list_remove(struct list_head *file_list, char *filename) {
  file_t *iterator;
  list_for_each_entry(iterator, file_list,
                      file_list) if (strcmp(iterator->filename, filename) ==
                                     0) {
    list_del(&iterator->file_list);
    return;
  }
}

/*
  Searches the sync_dir to see if a file was renamed (the filename is missing on
the file_list)

file_t * is_file_renamed(struct list_head *file_list,char *filename)
{
  file_t *iterator;

  list_for_each_entry(iterator,file_list,file_list)
    if(strcmp(filename, iterator->filename) == 0)
      return iterator;
  return NULL;
}*/

/*
  Searches the sync_dir to see if a file is missing (i.e., was deleted)
*/
file_t *is_file_missing(char *userid, struct list_head *file_list) {
  char *sync_dir_path = get_sync_dir(userid);
  DIR *dir;
  struct dirent *ent;
  file_t *iterator;
  bool found;
  dir = opendir(sync_dir_path);
  ;

  list_for_each_entry(iterator, file_list, file_list) {
    found = false;
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, iterator->filename) == 0) {
        found = true;
        break;
      }
    }
    if (found == false) {
      printf("MISSING %s\n", iterator->filename);
      return iterator;
    }
  }
  return NULL;
}

file_t *file_list_search(struct list_head *file_list, char *filename) {
  file_t *iterator;
  list_for_each_entry(iterator, file_list,
                      file_list) if (strcmp(filename, iterator->filename) ==
                                     0) return iterator;
  return NULL;
}

void remove_file_list(struct list_head *file_list, char *filename) {
  file_t *iterator;
  list_for_each_entry(iterator, file_list,
                      file_list) if (strcmp(filename, iterator->filename) ==
                                     0) {
    list_del(&iterator->file_list);
  }
}

file_t *file_list_add(struct list_head *file_list, char *fullpath) {
  file_t *new_file = malloc(sizeof(file_t));
  struct stat file_stat;
  char *filename = basename(fullpath);
  strcpy(new_file->filename, filename);

  if (stat(fullpath, &file_stat) != 0)
    printf("ERROR em stat!\n");

  new_file->last_modified = file_stat.st_mtime;

  list_add(&new_file->file_list, file_list);

  return new_file;
}

file_t *char_to_file_t(char *file) {
  int index = 0;
  file_t *copy_buffer = malloc(sizeof(file_t));

  memcpy(&copy_buffer->file_list, file, sizeof(copy_buffer->file_list));
  index += sizeof(copy_buffer->file_list);
  memcpy(copy_buffer->filename, file + index, sizeof(copy_buffer->filename));
  index += sizeof(copy_buffer->filename);
  memcpy(&copy_buffer->last_modified, file + index,
         sizeof(copy_buffer->last_modified));

  return copy_buffer;
}

char *file_t_to_char(file_t *file) {
  int index = 0;
  char *copy_buffer = malloc(sizeof(file_t));

  memcpy(copy_buffer, &file->file_list, sizeof(file->file_list));
  index += sizeof(file->file_list);
  memcpy(copy_buffer + index, file->filename, sizeof(file->filename));
  index += sizeof(file->filename);
  memcpy(copy_buffer + index, &file->last_modified,
         sizeof(file->last_modified));

  return copy_buffer;
}
/*
  Check if the given file name is not the own directory or the parent directory
*/
bool is_a_file(char *filename) {
  if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
    return true;
  else
    return false;
}

char *get_sync_dir(char *userid) {
  struct passwd *pw = getpwuid(getuid());
  char *sync_dir_path = malloc(sizeof(char) * 256);
  const char *homedir = pw->pw_dir;

  strcpy(sync_dir_path, homedir);
  strcat(sync_dir_path, "/sync_dir_");
  strcat(sync_dir_path, userid);
  strcat(sync_dir_path, "/");
  return sync_dir_path;
}
