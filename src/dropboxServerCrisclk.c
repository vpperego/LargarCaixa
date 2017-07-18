#define dprint1(x)                                                             \
  do {                                                                         \
    if (CRISCLK_DEBUG >= 1)                                                    \
      fprintf x;                                                               \
  } while (0)
#define dprint2(x)                                                             \
  do {                                                                         \
    if (CRISCLK_DEBUG >= 2)                                                    \
      fprintf x;                                                               \
  } while (0)

#define CRISCLK_REQUEST "CRISCLK_REQUEST"
#define CRISCLK_DISCONNECT "CRISCLK_DISCONNECT"
#define CRISCLK_DISCONNECT_CODE 0
#define CRISCLK_TMEOUT_CODE 1
#define CRISCLK_TMEOUT_THRD_SLEEP 2
#define CRISCLK_INACTIV_TIMEOUT 10
#define CRISCLK_GARBAGE_TME 5

#include "dropboxServerCrisclk.h"

// SERVER

// TODO: timeout value reponse service.

void cl_list_print(crisclk_sv_t *clk) {
  crisclk_cl_t *listhead = clk->clist;
  crisclk_cl_t *iterator;
  dprint1((stderr, "Printing client connection list (%p):\n", listhead));
  for (iterator = listhead; iterator; iterator = iterator->next)
    dprint1((stderr, "[Cl %d] %p\n", iterator->sockfd, iterator));
  dprint1((stderr, "List ending.\n"));
}

void cl_list_add(crisclk_sv_t *clk, int clskt) {

  crisclk_cl_t *listhead = clk->clist;

  if (!listhead) {
    clk->clist = malloc(sizeof(crisclk_cl_t));
    listhead = clk->clist;
    listhead->thread = 0;
    listhead->sockfd = clskt;
    listhead->garbag = false;
    listhead->remove = false;
    listhead->tmeout = time(NULL);
    listhead->next = NULL;
    dprint1((stderr, "[Cl %d] Empty list, creating it: %p\n", clskt, listhead));
  } else {
    crisclk_cl_t *iterator;
    for (iterator = listhead; iterator->next; iterator = iterator->next)
      ;
    iterator->next = malloc(sizeof(crisclk_cl_t));
    iterator->next->thread = 0;
    iterator->next->sockfd = clskt;
    iterator->next->garbag = false;
    iterator->next->remove = false;
    iterator->next->tmeout = time(NULL);
    iterator->next->next = NULL;
    dprint1((stderr, "[Cl %d] Appending connection to list: %p\n", clskt,
             iterator->next));
  }

  if (CRISCLK_DEBUG)
    cl_list_print(clk);

  return;
}

void cl_list_remove_one(crisclk_sv_t *clk, crisclk_cl_t *clconn) {

  if (!clk)
    return;

  crisclk_cl_t *listhead = clk->clist;

  crisclk_cl_t *curr = NULL;
  crisclk_cl_t *prev = NULL;

  for (curr = listhead; curr; curr = curr->next) {
    if (curr == clconn) {
      if (!prev) {
        listhead = listhead->next;
        dprint1((stderr, "[Cl %d] Removing head from connection list: %p.\n",
                 clk->clist->sockfd, clk->clist));
        free(clk->clist);
        clk->clist = listhead;
      } else {
        prev->next = curr->next ? curr->next : NULL;
        dprint1((stderr, "[Cl %d] Removing %s from list: %p.\n", curr->sockfd,
                 curr->next ? "body" : "tail", curr));
        free(curr);
      }

      return;
    }
    prev = curr;
  }
  fprintf(stderr, "Error: cl_list_remove_one() haven't found the client "
                  "connection in list.\n");
}

void sv_close_connection(crisclk_sv_t *clk, crisclk_cl_t *clconn, time_t code) {

  dprint1(
      (stderr, "[Cl %d] Sending connection to garbage...\n", clconn->sockfd));
  time(&clconn->tmeout);
  char *data = (char *)&code;
  send_data(data, clconn->sockfd, sizeof(time_t));
  shutdown(clconn->sockfd, SHUT_RD);
  clconn->garbag = true;
}

void sv_close_connection_all(crisclk_sv_t *clk) {

  dprint1((stderr, "Sending all connections to garbage.\n"));
  crisclk_cl_t *iterator;

  pthread_mutex_lock(&clk->access);
  for (iterator = clk->clist; iterator; iterator = iterator->next)
    sv_close_connection(clk, iterator, CRISCLK_DISCONNECT_CODE);
  pthread_mutex_unlock(&clk->access);

  dprint1((stderr, "Done.\n"));
}

crisclk_cl_t *cl_list_tail(crisclk_sv_t *clk) {

  crisclk_cl_t *iterator = NULL;
  for (iterator = clk->clist; iterator->next; iterator = iterator->next)
    ;
  dprint2(
      (stderr, "[Cl %d] List tail found: %p\n", iterator->sockfd, iterator));

  return iterator;
}

void *crisclk_sv_client_thread(void *arg) {

  crisclk_sv_t *clk = (crisclk_sv_t *)arg;
  pthread_mutex_lock(&clk->access);
  crisclk_cl_t *clconn = cl_list_tail(clk);
  pthread_mutex_unlock(&clk->access);

  dprint1((stderr, "[Cl %d] Client thread started running.\n", clconn->sockfd));

  time_t ts;
  struct buffer *b = malloc(sizeof(struct buffer));
  char *data = (char *)&ts;

  while (true) {
    dprint2((stderr, "[Cl %d] Wainting client request.\n", clconn->sockfd));
    b = read_data(clconn->sockfd);
    dprint1((stderr, "[Cl %d] Read client socket. Data: %s\n", clconn->sockfd,
             b->data));
    if (!strcmp(b->data, CRISCLK_REQUEST)) {
      time(&ts);
      send_data(data, clconn->sockfd, sizeof(time_t));
      clconn->tmeout = ts;
    } else if (!strcmp(b->data, CRISCLK_DISCONNECT)) {
      dprint1((stderr, "[Cl %d] Client disconnected.\n", clconn->sockfd));
      close(clconn->sockfd);
      pthread_mutex_lock(&clk->access);
      cl_list_remove_one(clk, clconn);
      pthread_mutex_unlock(&clk->access);
      break;
    } else
      break;
  }

  dprint1((stderr, "[Cl %d] Client thread closing...\n", clconn->sockfd));

  return NULL;
}

void *crisclk_sv_deamon(void *arg) {

  crisclk_sv_t *clk = (crisclk_sv_t *)arg;

  dprint1((stderr, "Deamon started running: %d\n", clk->sockfd));

  int newsockfd;
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(struct sockaddr_in);

  dprint1((stderr, "Cristian Clock Deamon waiting for connections...\n"));
  while (true) {
    if ((newsockfd = accept(clk->sockfd, (struct sockaddr *)&cli_addr,
                            &clilen)) == -1) {
      perror("Cristian Clock error on accepting. Connection stopped?\n");
      return NULL;
    }
    dprint1((stderr, "[Cl %d] Connection accepeted, creating client thread.\n",
             newsockfd));

    pthread_mutex_lock(&clk->access);
    cl_list_add(clk, newsockfd);
    pthread_create(&cl_list_tail(clk)->thread, NULL, crisclk_sv_client_thread,
                   clk);
    pthread_mutex_unlock(&clk->access);
  }
}

void *crisclk_sv_toutth(void *arg) {
  crisclk_sv_t *clk = (crisclk_sv_t *)arg;
  crisclk_cl_t *iterator;

  dprint1((stderr, "Timeout thread running.\n"));

  while (true) {

    // dprint2((stderr, "Timeout scan: identify phase.\n"));

    /**
     * WARNING: the time_t arithmetic is guaranteed to work only with POSIX
     * systems. time_t isn't necessarilly a number in seconds.
     */

    time_t st = time(NULL);

    pthread_mutex_lock(&clk->access);

    for (iterator = clk->clist; iterator; iterator = iterator->next) {
      int i = 0;

      if (iterator->garbag)
        dprint1((stderr, "[Cl %d] Time in garbage: %f.\n", iterator->sockfd,
                 difftime(st, iterator->tmeout)));
      else
        dprint1((stderr, "[Cl %d] Time without requests: %f.\n",
                 iterator->sockfd, difftime(st, iterator->tmeout)));

      if (iterator->garbag &&
          difftime(st, iterator->tmeout) > CRISCLK_GARBAGE_TME) {
        dprint1((stderr, "(X)[Cl %d] Closing connection: %p\n",
                 iterator->sockfd, iterator));
        iterator->remove = true;
        close(iterator->sockfd);
        pthread_cancel(iterator->thread);
        i = i + 1;
      } else if (difftime(st, iterator->tmeout) > CRISCLK_INACTIV_TIMEOUT) {
        dprint1((stderr, "[Cl %d] Adding to garbage due timeout: %p\n",
                 iterator->sockfd, iterator));
        sv_close_connection(clk, iterator, CRISCLK_TMEOUT_CODE);
      }
    }

    dprint2((stderr, "Timeout scan: diconnect phase.\n"));

    for (iterator = clk->clist; iterator; iterator = iterator->next)
      if (iterator->remove)
        cl_list_remove_one(clk, iterator);

    pthread_mutex_unlock(&clk->access);

    if (!clk->sockfd && !clk->clist) // server shutdown
      pthread_exit(NULL);

    sleep(CRISCLK_TMEOUT_THRD_SLEEP);
  }

  return NULL;
}

crisclk_sv_t *crisclk_sv_init(crisclk_sv_t *clk, int port, int backlog) {

  clk->svaddr.sin_family = AF_INET;
  clk->svaddr.sin_port = htons(port);
  clk->svaddr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(clk->svaddr.sin_zero), 8);

  clk->bcklog = backlog;
  clk->clist = NULL;

  return clk;
}

void crisclk_sv_deamon_stop(crisclk_sv_t *clk) {
  dprint1((stderr, "Stopping Cristian Clock Deamon: %p\n", clk));
  sv_close_connection_all(clk);
  close(clk->sockfd);
  clk->sockfd = 0;
  pthread_mutex_destroy(&clk->access);
  pthread_cancel(clk->thread);
  pthread_join(clk->toutth, NULL);
  dprint1((stderr, "Cristian Clock Deamon stopped and offline.\n"));
}

int crisclk_sv_deamon_run(crisclk_sv_t *clk) {

  if (!clk)
    return -1;
  int ret;

  if ((clk->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Error creating Cristian Clock deamon.\n");
    return clk->sockfd;
  }
  if ((ret = setsockopt(clk->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1},
                        sizeof(int))) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed.\n");
    close(clk->sockfd);
    return ret;
  }
  if ((ret = bind(clk->sockfd, (struct sockaddr *)&clk->svaddr,
                  sizeof(clk->svaddr))) < 0) {
    perror("ERROR on binding.\n");
    close(clk->sockfd);
    return ret;
  }
  pthread_mutex_init(&clk->access, NULL);
  if ((ret = listen(clk->sockfd, clk->bcklog)) != 0)
    return ret;
  if ((ret = pthread_create(&clk->thread, NULL, crisclk_sv_deamon, clk)) != 0)
    return ret;
  if ((ret = pthread_create(&clk->toutth, NULL, crisclk_sv_toutth, clk)) != 0)
    return ret;
  return ret;
}