/*
 * network.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "global.h"
#include <stdio.h>
#include <stdlib.h>   /*  free() */
#include <errno.h>   /*  errno() */
#include <unistd.h>  /* close(), read(),write() */
#include <signal.h>  /* SIGPIPE,SIG_ERR,SIGIO */
#ifdef G_SDL
#include <SDL.h> /* SDL_SetTimer() */
#endif
#ifdef SIGS
#include <signal.h>  /*  sighandler_t SIG_PIPE */
#endif
/*  here go all the network functions */
/*  */
/*  right now, there is only a basic implementation for tcp-scokets. */
/*  upcoming are unix-sockets and ipv6-support */

/*  defines: */

uint8_t ibuf[MAXPLEN]; /* input buffer for a packet */
uint8_t obuf[MAXPLEN]; /* output buffer */
#ifdef SIGS
static int sigio = 0;
#endif

#ifdef SIGS
void sigpipe_handler(int S3DUNUSED(unused))
{
	errs("sigpip_handler()", "there is a broken pipe somewhere");
}

void sigio_handler(int S3DUNUSED(unused))
{
	sigio = 1;
}
#endif
/*  maybe change the errors to fatal errors ... */
int network_init(void)
{
#ifdef SIGS
	/* struct sigaction act;*/
#endif
#ifdef TCP
	tcp_init();
#endif
#ifdef SHM
	shm_init();
#endif
#ifdef SIGS
	if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR)
		errn("network_init():signal()", errno);
	/* act.sa_handler = (sig_t)sigio_handler;
	 if ( sigaction(SIGIO, &act, 0) < 0 )
	  errn("network_init():sigaction()",errno);*/
	if (signal(SIGIO, sigio_handler) == SIG_ERR)
		errn("s3d_init():signal()", errno);
#endif
	return(0);
}
volatile int      turn;
int net_turn_off(int S3DUNUSED(interval))
{
	s3dprintf(VLOW, "Warning: High traffic on Network, interrupting read.");
	turn = 0;
	return(0);
}

/*  this basicly polls for new connection */
int network_main(void)
{
	turn = 1;

#ifdef TCP
#ifdef SIGS
	if (sigio == 1) { /*  as long as there is no locking/threadsafety, do like this ... */
#endif
		tcp_pollport(); /*  this polls for new processes */
#ifdef G_SDL
		SDL_SetTimer(50, (SDL_TimerCallback) net_turn_off);
#endif
		while (turn && tcp_pollproc()) {}  /*  if there is new data, loop please. this is for testing now, and should be combined with timing later .. */
#ifdef G_SDL
		SDL_SetTimer(0, NULL);
#endif

#ifdef SIGS
		sigio = 0;
	}
#endif
#endif
#ifdef SHM
	shm_main();
#endif
	return(0);
}
int n_remove(struct t_process *p)
{
	switch (p->con_type) {
#ifdef SHM
	case CON_SHM:
		shm_remove(p);
		break;
#endif
#ifdef TCP
	case CON_TCP:
		tcp_remove(p->sockid);
		break;
#endif
	}
	p->con_type = CON_NULL;
	return(-1);
}

int n_readn(struct t_process *p, uint8_t *str, int s)
{
	switch (p->con_type) {
#ifdef TCP
	case CON_TCP:
		return(tcp_readn(p->sockid, str, s));
#endif
#ifdef SHM
	case CON_SHM:
		return(shm_readn((struct buf_t *)p->shmsock.data_ctos, str, s));
#endif
	}
	return(-1);
}
int n_writen(struct t_process *p, uint8_t *str, int s)
{
	switch (p->con_type) {
#ifdef TCP
	case CON_TCP:
		return(tcp_writen(p->sockid, str, s));
#endif
#ifdef SHM
	case CON_SHM:
		return(shm_writen((struct buf_t *)p->shmsock.data_stoc, str, s));
#endif
	}
	return(-1);
}

int network_quit(void)
{
#ifdef TCP
	tcp_quit();
#endif
#ifdef SHM
	shm_quit();
#endif
	return(0);
}

