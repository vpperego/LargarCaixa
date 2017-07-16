#include "../include/dropboxSynch.h"

 void print_file_list(struct list_head *file_list)
{
  file_t * iterator;
  printf("print_file_list START\n" );
  list_for_each_entry(iterator, file_list, file_list){
    printf("File name : %s\n",iterator->filename );
  }
  printf("print_file_list END\n" );
}

bool updated_existing_file(char *fullpath, struct dirent *ent, int synch_socket,
                           struct list_head *file_list) {
    file_t *current_file;
    struct stat file_stat;
    if ((current_file = file_list_search(file_list, ent->d_name)) != NULL) {
        stat(fullpath, &file_stat);
        if (difftime(file_stat.st_mtime, current_file->last_modified) > 0) {

            current_file->last_modified = file_stat.st_mtime;

        return true;
        }
    }
    return false;
}


bool check_deleted_file(struct thread_info *ti,struct list_head *file_list){
  file_t * current_file;
  bool deleted = false;
  if ((current_file = is_file_missing(ti->working_directory, file_list)) != NULL) {
	   deleted = true;
     list_del(&current_file->file_list);
    // printf("sending command DELETE_FILE \n");
    send_data(DELETE_FILE, ti->newsockfd,
              strlen(DELETE_FILE) * sizeof(char) +1,ti->ssl );
    send_data(current_file->filename, ti->newsockfd,
              sizeof(current_file->filename) * sizeof(char),ti->ssl);

  }
  return deleted;
}

void update_fullpath(char *fullpath, char *userid, char *filename) {
    strcpy(fullpath, userid);
    strcat(fullpath, "/");
    strcat(fullpath, filename);
}

char * check_valid_string(struct buffer *file)
{
  char *real_string;
  if(strlen(file->data) > file->size)
  {
      real_string = malloc((sizeof(char) * file->size));
      strncpy(real_string,file->data,file->size);
      real_string[file->size] = '\0';
    }else
      real_string = file->data ;
    return real_string;
}

void listen_changes(struct thread_info *ti,  struct list_head *file_list ,char * userid,char * fullpath){
  struct buffer *filename, *request;


  while (true) {

      request = read_data(ti->newsockfd,ti->ssl);

      request->data = check_valid_string(request);

  //  printf("REQUEST:%s\n", request->data);
     if (strcmp(CHECK_DONE, request->data) == 0) {
            break;
       }
     filename = read_data(ti->newsockfd,ti->ssl);
     filename->data = check_valid_string(filename);
     if(ti->isServer==false)
     {
       strcpy(fullpath, ti->working_directory);
       strcat(fullpath, filename->data);
     }
     else
       update_fullpath(fullpath, userid, filename->data);


    if (strcmp(DOWNLOAD_FILE, request->data) == 0) {
//      printf("DOWNLOAD_FILE %s\n",filename->data );
      send_file_from_path(ti->newsockfd, fullpath,ti->ssl);
      file_list_add(file_list,fullpath);
    }else if (strcmp(UPDATE_FILE, request->data)==0){
      //printf("UPDATE_FILE %s\n",filename->data );

      file_list_remove(file_list, filename->data);
      remove(fullpath);
      receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
      file_list_add(file_list,fullpath);
    }else if (strcmp(DELETE_FILE, request->data) == 0) {
      // printf("DELETE FILE %s\n", filename->data );
        file_list_remove(file_list, filename->data);
        remove(fullpath); // delete the file
    } else {
      //  printf("SAVING FILE %s IN %s\n",filename->data,fullpath );
        receive_file_and_save_to_path(ti->newsockfd, fullpath, ti->ssl);
        file_list_add(file_list,fullpath);
    }
    free(request->data);
  }
}

bool is_new_file(char * file,struct list_head *file_list){
  file_t * iterator;
  list_for_each_entry(iterator, file_list, file_list) {
    if(strcmp(file,iterator->filename)==0)
      return false;
  }
  return true;
}


/*
  check the changes made in the working_directory of synch thread and send to listen_changes
*/
void check_changes(struct thread_info *ti, struct list_head *file_list,char *fullpath){
  bool updated;

  DIR * dir = opendir(ti->working_directory);
  struct dirent *ent;
  // print_file_list(file_list);

  bool deleted = check_deleted_file (ti,file_list);
  while ((ent = readdir(dir)) != NULL) {
      if (!is_a_file(ent->d_name)) {
          continue;
      }
      updated = false;

      if(ti->isServer == true)
      {
        strcpy(fullpath, ti->working_directory);
        strcat(fullpath,"/");
        strcat(fullpath, ent->d_name);
      }else{
        strcpy(fullpath, ti->working_directory);
        strcat(fullpath, ent->d_name);
      }
      updated = updated_existing_file(fullpath, ent, ti->newsockfd, file_list);
      if(updated ==true){
        send_data(UPDATE_FILE, ti->newsockfd,
                  strlen(UPDATE_FILE) * sizeof(char) + 1,ti->ssl);
                  send_data(ent->d_name, ti->newsockfd,
                            strlen(ent->d_name) * sizeof(char),ti->ssl );
                  send_file_from_path(ti->newsockfd, fullpath, ti->ssl);

      }else if(deleted==false &&  is_new_file(ent->d_name,file_list)==true) {
      //     printf("SENDING_FILE %s\n", fullpath);
          file_list_add(file_list, fullpath);
          send_data(SENDING_FILE, ti->newsockfd,
                    strlen(SENDING_FILE) * sizeof(char) + 1,ti->ssl);
          send_data(ent->d_name, ti->newsockfd,
                    strlen(ent->d_name) * sizeof(char),ti->ssl  );
          send_file_from_path(ti->newsockfd, fullpath, ti->ssl);
      }
  }
  closedir(dir);
   send_data(CHECK_DONE, ti->newsockfd,
                  (strlen(CHECK_DONE) * sizeof(char)),ti->ssl );

}

/*
  Execute in the start of synch_listen to get all files from server
*/

void get_server_file_list(int synch_socket, struct list_head *file_list, SSL *ssl) {

    struct buffer *server_file;
    file_t *current_file;
    while (true) {
        server_file = read_data(synch_socket, ssl);

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

     if ((current_file = is_file_missing(ti->working_directory, file_list)) != NULL) {
        send_data(DELETE_FILE, ti->newsockfd, strlen(DELETE_FILE) * sizeof(char), ti->ssl);


       send_data(current_file->filename, ti->newsockfd,
                  sizeof(current_file->filename) * sizeof(char) + 1, ti->ssl);

        list_del(&current_file->file_list);
    }
}

/*
  Search for missing files in the file list and remove them from the list and also remove from the server

*/
bool rename_files(char *fullpath, struct dirent *ent, struct thread_info *ti,
                  struct list_head *file_list) {
    file_t *current_file;
    if ((current_file = is_file_missing(ti->userid, file_list)) != NULL) {

        send_data(DELETE_FILE, ti->newsockfd,
                  strlen(DELETE_FILE) * sizeof(char), ti->ssl );
        send_data(current_file->filename, ti->newsockfd,
                  sizeof(current_file->filename) * sizeof(char), ti->ssl);

        list_del(&current_file->file_list);
      //  file_list_add(file_list, fullpath);

      //  send_data(ent->d_name, ti->newsockfd, strlen(ent->d_name) * sizeof(char));
      //  send_file_from_path(ti->newsockfd, fullpath);
        return true;
    }
    return false;
}

 /*
  Main synch thread executed in client side
 */
void *synch_listen(void *thread_info) {

    struct thread_info *ti = (struct thread_info *)thread_info;

    char *fullpath = malloc(strlen(ti->userid) + MAXNAME + 1);

    struct list_head *file_list = malloc(sizeof(file_list));

       INIT_LIST_HEAD(file_list);
      get_server_file_list(ti->newsockfd, file_list, ti->ssl);
      download_missing_files(ti, file_list);

    do {
      check_changes(ti,file_list,fullpath);
      listen_changes(ti, file_list,ti->userid, fullpath);
      sleep(5);

     } while (true);
}

/*
  Creates the file_list of @userid for synch_server
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
  closedir(dir);
  return file_list;
}

/*
  Send the server file list from synch_server to synch_listen
*/
void send_server_file_list(struct list_head *file_list,int newsockfd, SSL *ssl){
  char *buffer;
  file_t *iterator;
  list_for_each_entry(iterator, file_list, file_list) {
      buffer = file_t_to_char(iterator);
      send_data(buffer,newsockfd, sizeof(file_t), ssl);
      free(buffer);
  }
  send_data(FILE_SEND_OVER, newsockfd,
            strlen(CREATE_SYNCH_THREAD) * sizeof(char), ssl);
}



/* From Assignment Specification
 * Synchronizes the directory named "synch_dir_<username>" with the clients
 * synch_dir.
 */
void *synch_server(void *thread_info) {
    struct thread_info *ti = (struct thread_info *)thread_info;

       char *userid;
      char fullpath[255];
      if(ti->isServer==false){
          send_data(ti->userid,ti->newsockfd,strlen(ti->userid),ti->ssl);
  //        printf("useird = %s\n",ti->userid );
          //strcpy(userid,ti->userid);
          userid = ti->userid;
      }else{
         userid = read_user_name(ti->newsockfd, ti->ssl);
         strcpy(ti->userid,userid);
      }
      ti->working_directory = ti->userid;

      struct list_head *file_list = create_server_file_list(userid);

      send_server_file_list(file_list,ti->newsockfd, ti->ssl);

      while (true) {

        listen_changes(ti, file_list,ti->userid, fullpath);
        check_changes(ti,file_list,fullpath);
        sleep(5);
      }
      return NULL;
}
