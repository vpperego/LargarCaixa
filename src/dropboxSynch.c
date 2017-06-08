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

bool rename_files(char *fullpath, struct dirent *ent, struct thread_info *ti,
                  struct list_head *file_list) {
    file_t *current_file;
    if ((current_file = is_file_missing(ti->userid, file_list)) != NULL) {
        send_data(RENAME_FILE, ti->newsockfd,
                  strlen(DELETE_FILE) * sizeof(char) + 1);
        send_data(current_file->filename, ti->newsockfd,
                  sizeof(current_file->filename) * sizeof(char));
        
        list_del(&current_file->file_list);
        file_list_add(file_list, fullpath);
        
        send_data(ent->d_name, ti->newsockfd, strlen(ent->d_name) * sizeof(char));
        send_file_from_path(ti->newsockfd, fullpath);
        return true;
    }
    return false;
}

/*
 TODO - remove from the list what's already downloaded
 -don't create file SENDING_FILE
 */
void *synch_listen(void *thread_info) {
    struct thread_info *ti = (struct thread_info *)thread_info;
    
    printf("Working directory: %s\n", ti->working_directory);
    printf("userid: %s\n", ti->userid);
    DIR *dir;
    struct dirent *ent;
    
    char *fullpath = malloc(strlen(ti->userid) + MAXNAME + 1);
    
    
    do {
        send_data("SEND_FILES", ti->newsockfd,
                  strlen("SEND_FILES") * sizeof(char) + 1);
        
        struct list_head *file_list = malloc(sizeof(file_list));
        
        INIT_LIST_HEAD(file_list);
        get_server_file_list(ti->newsockfd, file_list);
        download_missing_files(ti, file_list);
        synch_deleted(ti, file_list);
        dir = opendir(ti->working_directory);
        while ((ent = readdir(dir)) != NULL) {
            
            bool updated = false;
            bool renamed = false;
            strcpy(fullpath, ti->working_directory);
            strcat(fullpath, ent->d_name);
            
            if (!is_a_file(ent->d_name)) {
                continue;
            }
            
            updated = updated_existing_file(fullpath, ent, ti->newsockfd, file_list);
            renamed = rename_files(fullpath, ent, ti, file_list);
            if (!renamed && !updated) {
                file_list_add(file_list, fullpath);
                send_data(SENDING_FILE, ti->newsockfd,
                          strlen(SENDING_FILE) * sizeof(char) + 1);
                send_data(ent->d_name, ti->newsockfd,
                          strlen(ent->d_name) * sizeof(char) + 1);
                send_file_from_path(ti->newsockfd, fullpath);
            }
        }
        send_data("DONE", ti->newsockfd,
                  strlen("DONE") * sizeof(char) + 1);
        closedir(dir);
        free(file_list);
        sleep(10);
    } while (true);
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
    char *buffer;
    DIR *dir;
    struct dirent *ent;
    file_t *iterator;
    char fullpath[MAXNAME]; // TODO - FIX THIS SIZE
    
    while (true) {
        struct list_head *file_list = malloc(sizeof(file_list));
        INIT_LIST_HEAD(file_list);
        struct buffer *sendFiles = read_data(ti->newsockfd);
        if(strcmp("SEND_FILES", sendFiles->data) != 0) {
            continue;
        }
        
        // TODO - refactor here, create a function for this
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
        list_for_each_entry(iterator, file_list, file_list) {
            buffer = file_t_to_char(iterator);
            send_data(buffer, ti->newsockfd, sizeof(file_t));
            free(buffer);
        }
        send_data(FILE_SEND_OVER, ti->newsockfd,
                  strlen(CREATE_SYNCH_THREAD) * sizeof(char));
        
        struct buffer *filename, *request;
        while (true) {
            // TODO GET THE FILE INFO AND SET IT IN THE LIST
            request = read_data(ti->newsockfd);
            if (strcmp("DONE", request->data) == 0) {
                break;
            }
            filename = read_data(ti->newsockfd);
            update_fullpath(fullpath, userid, filename->data);
            //  printf("New fullpath : %s para request %s\n",fullpath,request->data );
            if (strcmp(RENAME_FILE, request->data) == 0) {
                file_list_remove(file_list, filename->data);
                remove(fullpath);                    // delete the file
                filename = read_data(ti->newsockfd); // get the filename
                receive_file_and_save_to_path(ti->newsockfd, fullpath);
            } else if (strcmp(DOWNLOAD_FILE, request->data) == 0) {
                send_file_from_path(ti->newsockfd, fullpath);
            } else if (strcmp(DELETE_FILE, request->data) == 0) {
                file_list_remove(file_list, filename->data);
                remove(fullpath); // delete the file
            } else {
                receive_file_and_save_to_path(ti->newsockfd, fullpath);
            }
        }
        free(file_list);
        closedir(dir);
    }
    
    return NULL;
}
