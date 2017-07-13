#ifndef __CRISCLK_SV_H__
#define __CRISCLK_SV_H__

#define CRISCLK_DEBUG			0
#define CRISCLK_DEFAULT_PORT	55001
#define CRISCLK_DEFAULT_BACKLOG 50

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "dropboxSharedSocket.h"

/**
 * CRISTIAN CLOCK - SERVER SIDE
 */

typedef struct crisclk_cl_s {
    pthread_t 			thread;
	int					sockfd;
	time_t				tmeout;
	bool				garbag;
	bool				remove;
	struct crisclk_cl_s	*next;
} crisclk_cl_t;

typedef struct crisclk_sv_s {
	struct sockaddr_in	svaddr;
	int					bcklog;
	int					sockfd;
	pthread_t			thread;
	pthread_t			toutth;
	pthread_mutex_t		access;
	crisclk_cl_t		*clist;
} crisclk_sv_t;

/**
 * Initializes a crisclk_sv_t structure with useful info to start a Cristian Clock deamon.
 * This deamon treats client requests.
 * @param	crisclk_sv_t 	*clk		Clock useful info are stored in the location specified by this parameter.
 * @param	int 			port		Connection port;
 * @param	int				backlog		Connection backlog;
 * @return								Returns the clock structure initialized.
 */
crisclk_sv_t *crisclk_sv_init(crisclk_sv_t *clk, int port, int backlog);

/**
 * Starts a deamon thread configured in *clk structure initialized with crisclk_sv_init().
 * @param	crisclk_sv_t	*clk		Clock being started;
 * @return								If successful, the crisclk_sv_deamon_run() function will return zero.
 * 										Otherwise an error number will be returned to indicate the error.
 */
int crisclk_sv_deamon_run(crisclk_sv_t *clk);

/**
 * Diconnect all client connections and stop the deamon thread.
 * The clock can be started again using crisclk_sv_deamon_run() function.
 * @param	crisclk_sv_t	*clk		Clock being stopped.
 */
void crisclk_sv_deamon_stop(crisclk_sv_t *clk);

#endif