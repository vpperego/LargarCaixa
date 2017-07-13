#ifndef __CRISCLK_CL_H__
#define __CRISCLK_CL_H__

#define CRISCLK_DEBUG			0
#define CRISCLK_DEFAULT_PORT	55001

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
 * CRISTIAN CLOCK - CLIENT SIDE
 */

typedef struct crisclk_s {
	bool 				connected;
	int					sockfd;
	struct hostent		*server;
	struct sockaddr_in	serv_addr;
} crisclk_t;

/**
 * Initializes a crisclk_t with useful info to request a timestem from a Cristian Clock server.
 * @param	crisclk_t 	*clk	Clock useful info are stored in the location specified by this parameter;
 * @param	char		*host	Dns or IP of Cristian Clock server;
 * @param	int			port	Server's port.
 * @return						Returns the clock structure initialized.
 */
crisclk_t *crisclk_cl_init(crisclk_t *clk, char *host, int port);

/**
 * Establishes persistent connection with a Cristian Clock Server.
 * @param	crisclk_t 	*clk	Clock initialized with crisclk_cl_init().
 */
void crisclk_cl_connect(crisclk_t *clk);

/**
 * Used to get a timestamp from a Cristian Clock Server.
 * ATTENTION: for non-persistent connection, just initialize a clock (crisclk_t) and call crisclk_cl_get_time() function.
 * For persistent connections crisclk_cl_connect() must be called before crisclk_cl_get_time() calls.
 * ATTENTION: Persistent connections have a timeout of ten seconds, that is, if no successful crisclk_cl_get_time() call is
 * made for 10 seconds since last one, the server will close the connection.
 * The connection will start and finish at each crisclk_cl_get_time() call.
 * @param	crisclk_t 	*clk	Clock initialized with crisclk_cl_init().
 * @return						If successfull, the crisclk_cl_get_time() function will return a timestamp greater than zero.
 * 								Otherwise it will return 0 (zero) for server shutdown and 1 (one) for timeout.
 */
time_t crisclk_cl_get_time(crisclk_t *clk);

/**
 * Disconnects persistent connection with a Cristian Clock server.
 * This is only needed when a persistent connection is established, that is, when crisclk_cl_connect() is used.
 * @param	crisclk_t 	*clk	Clock initialized with crisclk_cl_init().
 */
void crisclk_cl_disconnect(crisclk_t *clk);

#endif