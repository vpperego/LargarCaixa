#include "../include/dropboxSynch.h"

void get_server_file_list(int synch_socket, struct list_head *file_list) {
    struct buffer *server_file;
    file_t *current_file;
    while (true) {
        server_file = read_data(synch_socket);

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
                  (int)(strlen(DOWNLOAD_FILE) * sizeof(char) + 1));
        send_data(missing_file->filename, ti->newsockfd,
                  strlen(missing_file->filename) * sizeof(char) + 1);

        strcpy(fullpath, sync_dir_path);
        strcat(fullpath, "/");
        strcat(fullpath, missing_file->filename);
        receive_file_and_save_to_path(ti->newsockfd, fullpath);
    }
}

/*
  check if a file from the list is missing in the sync_dir.
  if it's missing send a request to the server to delete the file
*/
void synch_deleted(struct thread_info *ti, struct list_head *file_list) {
    file_t *current_file;
    if ((current_file = is_file_missing(ti->userid, file_list)) != NULL) {
        send_data(DELETE_FILE, ti->newsockfd, strlen(DELETE_FILE) * sizeof(char));
        send_data(current_file->filename, ti->newsockfd,
                  sizeof(current_file->filename) * sizeof(char) + 1);

        list_del(&current_file->file_list);
    }
}

bool updated_existing_file(char *fullpath, struct dirent *ent, int synch_socket,
                           struct list_head *file_list) {
    file_t *current_file;
    struct stat file_stat;
    if ((current_file = file_list_search(file_list, ent->d_name)) != NULL) {
        stat(fullpath, &file_stat);
        if (difftime(file_stat.st_mtime, current_file->last_modified) > 0) {
            send_data(SENDING_FILE, synch_socket,
                      strlen(SENDING_FILE) * sizeof(char) + 1);

            current_file->last_modified = file_stat.st_mtime;
            //      char * file_buffer = file_t_to_char(current_file);
            send_data(current_file->filename, synch_socket,
                      strlen(current_file->filename) * sizeof(char) + 1);
            send_file_from_path(synch_socket, fullpath);
        }
        return true;
    }
    return false;
}


/*
  Search for missing files in the file list and remove them from the list and also remove from the server

*/
bool rename_files(char *fullpath, struct dirent *ent, struct thread_info *ti,
                  struct list_head *file_list) {
    file_t *current_file;
    if ((current_file = is_file_missing(ti->userid, file_list)) != NULL) {
      printf("DELETE_FILE %s\n", current_file->filename);
        send_data(DELETE_FILE, ti->newsockfd,
                  strlen(DELETE_FILE) * sizeof(char) );
        send_data(current_file->filename, ti->newsockfd,
                  sizeof(current_file->filename) * sizeof(char));

        list_del(&current_file->file_list);
      //  file_list_add(file_list, fullpath);

      //  send_data(ent->d_name, ti->newsockfd, strlen(ent->d_name) * sizeof(char));
      //  send_file_from_path(ti->newsockfd, fullpath);
        return true;
    }
    return false;
}
bool check_deleted_file(struct thread_info *ti,struct list_head *file_list){
  file_t * current_file;
  bool deleted = false;
  while ((current_file = is_file_missing(ti->userid, file_list)) != NULL) {
	deleted = true;
    send_data(DELETE_FILE, ti->newsockfd,
              strlen(DELETE_FILE) * sizeof(char) +1 );
    send_data(current_file->filename, ti->newsockfd,
              sizeof(current_file->filename) * sizeof(char));
    list_del(&current_file->file_list);
     
  }
  return deleted;
}
/*
 TODO - remove from the list what's already downloaded
 -don't create file SENDING_FILE
 */
void *synch_listen(void *thread_info) {
    struct thread_info *ti = (struct thread_info *)thread_info;

    DIR *dir;
    struct dirent *ent;
    char *fullpath = malloc(strlen(ti->userid) + MAXNAME + 1);
    bool updated = false;
    bool deleted = false;
  //  send_data("SEND_FILES", ti->newsockfd,
    //          strlen("SEND_FILES") * sizeof(char) + 1);

    struct list_head *file_list = malloc(sizeof(file_list));

    INIT_LIST_HEAD(file_list);
    get_server_file_list(ti->newsockfd, file_list);
    download_missing_files(ti, file_list);

    do {
        deleted = check_deleted_file (ti,file_list);
        dir = opendir(ti->working_directory);
        while ((ent = readdir(dir)) != NULL) {
            updated = false;

            strcpy(fullpath, ti->working_directory);
            strcat(fullpath, ent->d_name);

            if (!is_a_file(ent->d_name)) {
                continue;
            }
          //  synch_deleted(ti, file_list);
            printf("Arquivo %s\n", ent->d_name);
        //    renamed =  rename_files(fullpath, ent, ti, file_list);
            deleted = updated_existing_file(fullpath, ent, ti->newsockfd, file_list);
            if (!deleted && !updated) {
                file_list_add(file_list, fullpath);
                send_data(SENDING_FILE, ti->newsockfd,
                          strlen(SENDING_FILE) * sizeof(char) + 1);
                send_data(ent->d_name, ti->newsockfd,
                          strlen(ent->d_name) * sizeof(char) + 1);
                send_file_from_path(ti->newsockfd, fullpath);
            }
        }

  //      send_data("DONE", ti->newsockfd,
    //              strlen("DONE") * sizeof(char) + 1);
        closedir(dir);
        //free(file_list);
        sleep(5);
    } while (true);
}

/*
  Creates the file_list of userid for synch_server
*/
struct list_head *  create_server_file_list(char * userid){
  struct list_head *file_list = malloc(sizeof(file_list));
    DIR *dir;
    struct dirent *ent;
      char fullpath[MAXNAME];
  INIT_LIST_HEAD(file_list);
//  struct buffer *sendFiles = read_data(ti->newsockfd);
  if ((dir = opendir(userid)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir(dir)) != NULL) {
          if (is_a_file(ent->d_name) == true) {
              strcpy(fullpath, userid);
              strcat(fullpath, "/");
              strcat(fullpath, ent->d_name);
              file_list_add(file_list, fullpath);
          }
      }
  }
  return file_list;
}

/*
  Send the server file list from synch_server to synch_listen
*/
void send_server_file_list(struct list_head *file_list,int newsockfd){
  char *buffer;
  file_t *iterator;
  list_for_each_entry(iterator, file_list, file_list) {
      buffer = file_t_to_char(iterator);
      send_data(buffer,newsockfd, sizeof(file_t));
      free(buffer);
  }
  send_data(FILE_SEND_OVER, newsockfd,
            strlen(CREATE_SYNCH_THREAD) * sizeof(char));
}

void update_fullpath(char *fullpath, char *userid, char *filename) {
    strcpy(fullpath, userid);
    strcat(fullpath, "/");
    strcat(fullpath, filename);
}

/* From Assignment Specification
 * Synchronizes the directory named "synch_dir_<username>" with the clients
 * synch_dir.
 */
void *synch_server(void *thread_info) {
    struct thread_info *ti = (struct thread_info *)thread_info;

    char *userid = read_user_name(ti->newsockfd);
      char fullpath[MAXNAME]; // TODO - FIX THIS SIZE
      struct buffer *filename, *request;

//    if(strcmp("SEND_FILES", sendFiles->data) != 0) {
      struct list_head *file_list = create_server_file_list(userid);

      send_server_file_list(file_list,ti->newsockfd);

        while (true) {
             // TODO GET THE FILE INFO AND SET IT IN THE LIST
            request = read_data(ti->newsockfd);
        //    if (strcmp("DONE", request->data) == 0) {
      ///          break;
      //      }
          //  printf("REQUEST:%s\n", request->data);
             filename = read_data(ti->newsockfd);
            update_fullpath(fullpath, userid, filename->data);
            //  printf("New fullpath : %s para request %s\n",fullpath,request->data );
            if (strcmp(RENAME_FILE, request->data) == 0) {
        //      printf("RENAME_FILE %s\n",filename->data );
          //    printf("fullpath value = %s\n",fullpath );
              remove(fullpath);
                file_list_remove(file_list, filename->data);

               filename = read_data(ti->newsockfd); // get the filename
                receive_file_and_save_to_path(ti->newsockfd, fullpath);
            } else if (strcmp(DOWNLOAD_FILE, request->data) == 0) {
        //      printf("DOWNLOAD_FILE %s\n",filename->data );
                send_file_from_path(ti->newsockfd, fullpath);
            } else if (strcmp(DELETE_FILE, request->data) == 0) {
        //      printf("DELETE FILE %s\n", filename->data );
                file_list_remove(file_list, filename->data);
                remove(fullpath); // delete the file
            } else {
                receive_file_and_save_to_path(ti->newsockfd, fullpath);
            }
        }


    return NULL;
}
