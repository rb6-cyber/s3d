/*
 * network.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3d.h"
#include "s3dlib.h"
#include <string.h>   /*  memcpy() */
#include <stdlib.h>   /*  malloc(), free() */
#include <unistd.h>   /*  read(), write() */
#include <errno.h>   /*  errno */
#include <netinet/in.h>  /*  htons(),htonl() */
int con_type = CON_NULL;
#ifdef TCP
static int _s3d_net_receive(void);
#endif

int net_send(uint8_t opcode, char *buf, uint16_t length)
{
	char *ptr;
	/*  char *buff; */
	char buff[65539];  /*  uint16_t really shouldn't be bigger ;) */
	*(buff) = opcode;
	ptr = buff + 1;
	*((uint16_t *) ptr) = htons(length);
	if (length != 0)
		memcpy(buff + 3, buf, length);
	switch (con_type) {
#ifdef SHM
	case CON_SHM:
		shm_writen(buff, length + 3);
		break;
#endif
#ifdef TCP
	case CON_TCP:
		tcp_writen(buff, length + 3);
		break;
#endif
	}
	return(0);
}
/* handler for socket based connection types */
#ifdef TCP
static int _s3d_net_receive(void)
{
	return(_s3d_tcp_net_receive());
}
#endif
int s3d_net_check(void)
{
	switch (con_type) {
#ifdef TCP
	case CON_TCP:
#ifdef SIGS
		if (_s3d_sigio) {
#endif
			while (_s3d_net_receive()) {}
#ifdef SIGS
			_s3d_sigio = 0;
		}
#endif
		break;
#endif
#ifdef SHM
	case CON_SHM:
		while (_shm_net_receive()) {}
		break;
#endif
	}
	s3d_process_stack();
	return(0);
}
int s3d_net_init(char *urlc)
{
	char    *s, *sv, *port = NULL;
	char    *first_slash = NULL;
#ifdef TCP
	int      pn = 0;
#endif
	int      tcp, shm;
	tcp = shm = 1; /* everything is possible, yet */

	/*  doing a very bad server/port extraction, but I think it'll work ... */
	s = sv = urlc + 6;  /*  getting to the "real" thing */
	/* while (((*s!='/') && (*s!=0)) && (s<(urlc-6))) */
	while (*s != 0) {
		if (*s == '/') {
			if (first_slash == NULL)
				first_slash = s;
			if (port != NULL)
				break;
		}
		if (*s == ':') { /*  there is a port in here */
			port = s + 1;
			*s = 0;  /*  NULL the port  */
		}
		s++;
	}

	*s = 0;
	if (port == NULL) {
		shm = 0;
		if (first_slash != NULL)
			*first_slash = 0;
	} else {
		if (first_slash < port)
			tcp = 0;
		else
			if (first_slash != NULL)
				*first_slash = 0;
		if (!strncmp(port, "shm", 3)) {
			tcp = 0; /* null the others */
		} else {
			shm = 0;
		}
	}
#ifdef SHM
	if (shm) {
		if (!strncmp(port, "shm", 3))
			if (!_shm_init(sv)) return(con_type = CON_SHM);
	}
#endif
#ifdef TCP
	if (tcp) {
		pn = 6066;
		if (port != NULL) {
			if (!(pn = atoi(port))) { /*  I hope atoi is safe enough. */
				errn("s3d_init():atoi()", errno);
				pn = 6066;
			}
		}
		if (!_tcp_init(sv, pn)) return(con_type = CON_TCP);
	}
#endif
	return(CON_NULL);
}
