// TODO: keep-alive messages.
// TODO: get timeout value message.

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
#define CRISCLK_TIME_OUT_CODE 1

#include "dropboxClientCrisclk.h"

// CLIENT

crisclk_t *crisclk_cl_init(crisclk_t *clk, char *host, int port) {
  dprint1((stderr, "crisclk_cl_init\n"));
  if (!clk)
    return NULL;

  if ((clk->server = gethostbyname(host)) == NULL) {
    fprintf(stderr, "ERROR, no such host: %s\n", host);
    exit(0);
  }

  clk->serv_addr.sin_family = AF_INET;
  clk->serv_addr.sin_port = htons(port);
  clk->serv_addr.sin_addr = *((struct in_addr *)clk->server->h_addr);
  bzero(&(clk->serv_addr.sin_zero), 8);

  clk->connected = false;

  return clk;
}

void crisclk_cl_connect(crisclk_t *clk) {
  dprint1((stderr, "crisclk_cl_connect: "));
  if (!clk->connected) {

    if ((clk->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "Cristian Clock error opening socket: %d\n", clk->sockfd);
      exit(0);
    }

    printf("%d\n", clk->sockfd);

    if (connect(clk->sockfd, (struct sockaddr *)&clk->serv_addr,
                sizeof(clk->serv_addr)) < 0) {
      fprintf(stderr, "Cristian Clock error connecting.\n");
      exit(0);
    }

    clk->connected = true;
  }
}

void crisclk_cl_disconnect(crisclk_t *clk) {

  if (clk->connected) {
    dprint1((stderr, "crisclk_cl_disconnect\n"));
    send_data(CRISCLK_DISCONNECT, clk->sockfd, sizeof(CRISCLK_DISCONNECT));
    clk->connected = false;
    close(clk->sockfd);
  }
}

time_t crisclk_cl_get_time(crisclk_t *clk) {
  dprint1((stderr, "crisclk_cl_get_time\n"));

  bool persistent = clk->connected;
  if (!persistent)
    crisclk_cl_connect(clk);

  struct buffer *b = malloc(sizeof(struct buffer));
  time_t t0, t1, ts, tf;

  time(&t0);
  send_data(CRISCLK_REQUEST, clk->sockfd, sizeof(CRISCLK_REQUEST));
  b = read_data(clk->sockfd); // time(&ts);
  time(&t1);

  /**
   * WARNING: the time_t arithmetic is guaranteed to work only with POSIX
   * systems. time_t isn't necessarilly a number in seconds.
   */
  ts = *(time_t *)b->data;
  tf = ts + (t1 - t0) / 2;

  if (ts == CRISCLK_DISCONNECT_CODE) {
    fprintf(stderr, "WARNING: Cristian Clock server closed connection (server "
                    "shutdown).\n");
    crisclk_cl_disconnect(clk);
    return ts;
  } else if (ts == CRISCLK_TIME_OUT_CODE) {
    fprintf(stderr,
            "WARNING: Cristian Clock server closed connection (timeout).\n");
    return ts;
  }

  dprint1((stderr, "tf = ts+(t1-t0)/2\ntf = %ld+(%ld-%ld)/2\ntf = %ld\n", ts,
           t1, t0, tf));

  if (!persistent)
    crisclk_cl_disconnect(clk);

  return tf;
}