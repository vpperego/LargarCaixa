#include "../include/dropboxSynch.h"

void print_file_list(struct list_head *file_list) {
  file_t *iterator;
  printf("print_file_list START\n");
  list_for_each_entry(iterator, file_list, file_list) {
    printf("File name : %s\n", iterator->filename);
  }
  printf("print_file_list END\n");
}

time_t getTimeServer(int synch_socket, SSL *ssl) {
  send_data(GET_TIME, synch_socket, strlen(GET_TIME) * sizeof(char) + 1, ssl);

  time_t now = time(0);
  char time_buffer[MAXNAME];
  sprintf(time_buffer, "%lld", (long long)now);
  send_data(time_buffer, strlen(time_buffer) * sizeof(char) + 1, synch_socket,
            ssl);

  struct buffer *response;
  response = read_data(synch_socket, ssl);
  time_t after_response = time(0);

  char *next;
  time_t server_time = (long long)strtol(response->data, &next, 10);
  time_t client_time = server_time + ((after_response - now) / 2);
  /*printf("final %lld\n", client_time);*/
  return client_time;
}

bool updated_existing_file(char *fullpath, struct dirent *ent, int synch_socket,
                           struct list_head *file_list, SSL *ssl) {
  file_t *current_file;
  struct stat file_stat;
  if ((current_file = file_list_search(file_list, ent->d_name)) != NULL) {
    stat(fullpath, &file_stat);
    time_t new_client_time = getTimeServer(synch_socket, ssl);
    long long diff1 = file_stat.st_mtime - time(0);
    long long diff2 = current_file->last_modified - (new_client_time * 100);
    if (diff1 > diff2) {

      current_file->last_modified = file_stat.st_mtime;

      return true;
    }
  }
  return false;
}

bool check_deleted_file(struct thread_info *ti, struct list_head *file_list) {
  file_t *current_file;
  bool deleted = false;
  if ((current_file = is_file_missing(ti->working_directory, file_list)) !=
      NULL) {
    deleted = true;
    list_del(&current_file->file_list);
    // printf("sending command DELETE_FILE \n");
    send_data(DELETE_FILE, ti->newsockfd,
              strlen(DELETE_FILE) * sizeof(char) + 1, ti->ssl);
    send_data(current_file->filename, ti->newsockfd,
              sizeof(current_file->filename) * sizeof(char), ti->ssl);
  }
  return deleted;
}

void update_fullpath(char *fullpath, char *userid, char *filename) {
  strcpy(fullpath, userid);
  strcat(fullpath, "/");
  strcat(fullpath, filename);
}

void listen_changes(struct thread_info *ti, struct list_head *file_list,
                    char *userid, char *fullpath) {
  struct buffer *filename, *request;

  while (true) {

    request = read_data(ti->newsockfd, ti->ssl);

    request->data = check_valid_string(request);

    if (strcmp(CHECK_DONE, request->data) == 0) {
      break;
    }
    filename = read_data(ti->newsockfd, ti->ssl);
    filename->data = check_valid_string(filename);
    // printf("REQUEST: %s\n", request->data);
    // printf("FILENAME: %s\n", request->data);

    if (ti->isServer == false) {
      strcpy(fullpath, ti->working_directory);
      strcat(fullpath, filename->data);
    } else
      update_fullpath(fullpath, userid, filename->data);

    if (strcmp(DOWNLOAD_FILE, request->data) == 0) {
      //      printf("DOWNLOAD_FILE %s\n",filename->data );
      send_file_from_path(ti->newsockfd, fullpath, ti->ssl);
      file_list_add(file_list, fullpath);
    } else if (strcmp(UPDATE_FILE, request->data) == 0) {
      // printf("UPDATE_FILE %s\n",filename->data );

      file_list_remove(file_list, filename->data);
      remove(fullpath);
      receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
      file_list_add(file_list, fullpath);
    } else if (strcmp(GET_TIME, request->data) == 0) {
      time_t now = time(0);
      char time_buffer[MAXNAME];
      sprintf(time_buffer, "%lld", (long long)now);
      send_data(time_buffer, strlen(time_buffer) * sizeof(char) + 1,
                ti->newsockfd, ti->ssl);
    } else if (strcmp(DELETE_FILE, request->data) == 0) {
      // printf("DELETE FILE %s\n", filename->data );
      file_list_remove(file_list, filename->data);
      remove(fullpath); // delete the file
    } else {
      //  printf("SAVING FILE %s IN %s\n",filename->data,fullpath );
      receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
      file_list_add(file_list, fullpath);
    }
    if (ti->isServer == true) {

      dbsem_wait(ti->sem);

      updateReplicas(ti->rm_list, request->data, fullpath, filename->data);
      dbsem_post(ti->sem);
    }
    free(request->data);
  }
}

bool is_new_file(char *file, struct list_head *file_list) {
  file_t *iterator;
  list_for_each_entry(iterator, file_list, file_list) {
    if (strcmp(file, iterator->filename) == 0)
      return false;
  }
  return true;
}

/*
  check the changes made in the working_directory of synch thread and send to
  listen_changes
*/
void check_changes(struct thread_info *ti, struct list_head *file_list,
                   char *fullpath) {
  bool updated;

  DIR *dir = opendir(ti->working_directory);
  struct dirent *ent;
  // print_file_list(file_list);

  bool deleted = check_deleted_file(ti, file_list);
  while ((ent = readdir(dir)) != NULL) {
    if (!is_a_file(ent->d_name)) {
      continue;
    }
    updated = false;

    if (ti->isServer == true) {
      strcpy(fullpath, ti->working_directory);
      strcat(fullpath, "/");
      strcat(fullpath, ent->d_name);
    } else {
      strcpy(fullpath, ti->working_directory);
      strcat(fullpath, ent->d_name);
    }
    updated =
        updated_existing_file(fullpath, ent, ti->newsockfd, file_list, ti->ssl);
    if (updated == true) {
      send_data(UPDATE_FILE, ti->newsockfd,
                strlen(UPDATE_FILE) * sizeof(char) + 1, ti->ssl);
      send_data(ent->d_name, ti->newsockfd, strlen(ent->d_name) * sizeof(char),
                ti->ssl);
      send_file_from_path(ti->newsockfd, fullpath, ti->ssl);

    } else if (deleted == false &&
               is_new_file(ent->d_name, file_list) == true) {
      //     printf("SENDING_FILE %s\n", fullpath);
      file_list_add(file_list, fullpath);
      send_data(SENDING_FILE, ti->newsockfd,
                strlen(SENDING_FILE) * sizeof(char) + 1, ti->ssl);
      send_data(ent->d_name, ti->newsockfd, strlen(ent->d_name) * sizeof(char),
                ti->ssl);
      send_file_from_path(ti->newsockfd, fullpath, ti->ssl);
    }
  }
  closedir(dir);
  send_data(CHECK_DONE, ti->newsockfd, (strlen(CHECK_DONE) * sizeof(char)),
            ti->ssl);
}

/*
  Execute in the start of synch_listen to get all files from server
*/

void get_file_list(int synch_socket, struct list_head *file_list, SSL *ssl) {

  struct buffer *server_file;
  file_t *current_file;
  while (true) {
    server_file = read_data(synch_socket, ssl);
    server_file->data = check_valid_string(server_file);

    if (strcmp(FILE_SEND_OVER, server_file->data) == 0)
      break;
    current_file = char_to_file_t(server_file->data);
    list_add(&current_file->file_list, file_list);
  }
}

/*
 Download the files from the server that are missing in the client machine
 */
void download_missing_files(struct thread_info *ti,
                            struct list_head *file_list) {
  file_t *missing_file;
  char *sync_dir_path = get_sync_dir(ti->userid);
  char fullpath[MAXNAME];
  while (true) {
    if ((missing_file = is_file_missing(ti->userid, file_list)) == NULL)
      break;

    send_data(DOWNLOAD_FILE, ti->newsockfd,
              (int)(strlen(DOWNLOAD_FILE) * sizeof(char) + 1), ti->ssl);

    send_data(missing_file->filename, ti->newsockfd,
              strlen(missing_file->filename) * sizeof(char) + 1, ti->ssl);

    strcpy(fullpath, sync_dir_path);
    strcat(fullpath, "/");
    strcat(fullpath, missing_file->filename);
    receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
  }
}

/*
  check if a file from the list is missing in the sync_dir.
  if it's missing send a request to the server to delete the file
*/
void synch_deleted(struct thread_info *ti, struct list_head *file_list) {
  file_t *current_file;

  if ((current_file = is_file_missing(ti->working_directory, file_list)) !=
      NULL) {
    send_data(DELETE_FILE, ti->newsockfd, strlen(DELETE_FILE) * sizeof(char),
              ti->ssl);

    send_data(current_file->filename, ti->newsockfd,
              sizeof(current_file->filename) * sizeof(char) + 1, ti->ssl);

    list_del(&current_file->file_list);
  }
}

/*
 Main synch thread executed in client side
*/
void *synch_listen(void *thread_info) {

  struct thread_info *ti = (struct thread_info *)thread_info;

  char *fullpath = malloc(strlen(ti->userid) + MAXNAME + 1);

  struct list_head *file_list = malloc(sizeof(file_list));

  INIT_LIST_HEAD(file_list);

  get_file_list(ti->newsockfd, file_list, ti->ssl);
  //  download_missing_files(ti, file_list);

  do {
    check_changes(ti, file_list, fullpath);
    listen_changes(ti, file_list, ti->userid, fullpath);
    sleep(5);

  } while (true);
}

/*
  Send the server file list from synch_server to synch_listen
*/
void send_file_list(struct list_head *file_list, int newsockfd, SSL *ssl) {
  char *buffer;
  file_t *iterator;
  list_for_each_entry(iterator, file_list, file_list) {
    buffer = file_t_to_char(iterator);
    send_data(buffer, newsockfd, sizeof(file_t), ssl);
    free(buffer);
  }
  send_data(FILE_SEND_OVER, newsockfd, strlen(FILE_SEND_OVER) * sizeof(char),
            ssl);
}

void synch_replica_info(struct thread_info *ti) {
  rm_t *iterator;
  struct list_head *rm_list = ti->rm_list;
  list_for_each_entry(iterator, rm_list, rm_list) {

    send_file_list(ti->file_list, iterator->newsockfd, iterator->ssl);
  }
}

/* From Assignment Specification
 * Synchronizes the directory named "synch_dir_<username>" with the clients
 * synch_dir.
 */
void *synch_server(void *thread_info) {
  struct thread_info *ti = (struct thread_info *)thread_info;

  char fullpath[255];
  ti->working_directory = ti->userid;

  //   struct list_head *file_list = create_server_file_list(ti->userid);
  //   dbsem_wait(ti->sem);
  synch_replica_info(ti);
  //   dbsem_post(ti->sem);

  send_file_list(ti->file_list, ti->newsockfd, ti->ssl);
  while (true) {

    listen_changes(ti, ti->file_list, ti->userid, fullpath);
    check_changes(ti, ti->file_list, fullpath);
    sleep(5);
  }
  return NULL;
}
