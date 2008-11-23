/*
 * tcp.c
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
#include <sys/types.h>
#include <stdlib.h>   /*  malloc(), free() */
#include <unistd.h>   /*  read(), write() */
#include <errno.h>   /*  errno */
#include <string.h>   /*  memcpy() */
#include <sys/socket.h>
#include <netinet/in.h>  /*  htons(),htonl() */
#ifndef WIN32
#include <sys/select.h>
#include <netdb.h>   /*  gethostbyname()  */
#endif
#ifdef SIGS
#define __USE_BSD 1  /* we want sig_t and F_SETOWN to be defined */
#include <fcntl.h>   /*  fcntl */
#include <signal.h>   /*  signal(), SIGPIPE, SIGIO */
#endif

static int s3d_socket;   /*  this is the socket which holds the tcp-socket .... */

#ifdef SIGS
static void sigpipe_handler(int S3DUNUSED(sig), int S3DUNUSED(code))  /*  ... ? */
{
	errs("sigpipe_handler()", "there is a broken pipe somewhere");
}
static void sigio_handler(int S3DUNUSED(sig), int S3DUNUSED(code))  /*  ... ? */
{
	_s3d_sigio = 1;
}
#endif

int _tcp_init(char *sv, int pn)
{
	int       sd;
	int      res;
	/* char     *port=NULL;*/
	struct sockaddr_in   sock;
	struct hostent   *server = NULL;
#ifdef SIGS
	_s3d_sigio = 0;
#endif
#ifdef WIN32
	WSADATA datainfo;
	if (WSAStartup(257, &datainfo) != 0) {
		errn("_tcp_init()():startup()", errno);
		return(-1);
	}
#endif
	if ((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		errn("_tcp_init()():socket()", errno);
		return (-1);
	}
	sock.sin_family = AF_INET;
	if (*sv == 0)  /*  no server argument */
		sv = "127.0.0.1";
	if ((server = gethostbyname(sv)))
		memcpy(&sock.sin_addr.s_addr, server->h_addr_list[0], 4);
	else {
		errn("_tcp_init()():gethostbyname()", errno);
		return(-1);
	}
	sock.sin_port = htons(pn);

	res = connect(sd, (struct sockaddr *) & sock, sizeof(struct sockaddr_in));
	if (res < 0) {
		errn("_tcp_init()():connect()", errno);
		return(-1);
	}
	/*    if ( fcntl(sd, F_SETFL, O_ASYNC | O_NONBLOCK) < 0 ) */
	/*   errn("fcntl()",errno); */
#ifdef SIGS
	if (fcntl(sd, F_SETFL, O_ASYNC) < 0)
		errn("fcntl()", errno);
	if (fcntl(sd, F_SETOWN, getpid()) < 0)
		errn("fcntl()", errno);
	if (signal(SIGPIPE, (sig_t)sigpipe_handler) == SIG_ERR)
		errn("_tcp_init():signal()", errno);
	if (signal(SIGIO, (sig_t)sigio_handler) == SIG_ERR)
		errn("_tcp_init():signal()", errno);
#endif
	s3d_socket = sd;
	s3dprintf(MED, "connection to %s:%d established", sv, pn);
	return(0);
}
int _tcp_quit(void)
{
	if (s3d_socket) {
		s3dprintf(MED, "closing socket %d", s3d_socket);
		close(s3d_socket);
		s3d_socket = 0;
	}
	return(0);
}
int tcp_readn(char *str, int s)
{
	int no_left, no_read;
	no_left = s;
	while (no_left > 0) {
		no_read = read(s3d_socket, str, no_left);
		if (no_read < 0)  return(no_read);
		if (no_read == 0) break;
		no_left -= no_read;
		str += no_read;
	}
	return(s - no_left);
}
int tcp_writen(char *str, int s)
{
	int no_left, no_written;
	no_left = s;
	while (no_left > 0) {
		no_written = write(s3d_socket, str, no_left);
		if (no_written <= 0)  return(no_written);
		no_left -= no_written;
		str += no_written;
	}
	return(s - no_left);
}
int _s3d_tcp_net_receive(void)
{
	fd_set     fs_proc;   /*  filedescriptor set for listening port(s) */
	struct timeval   tv;   /*  time structure */
	int      found = 0;
	char     opcode, *buf;
	uint16_t   length;


	if (s3d_socket != -1) {
		FD_ZERO(&fs_proc);
		tv.tv_sec = tv.tv_usec = 0;
		FD_SET(s3d_socket, &fs_proc);

		/* s3dprintf(LOW,"Added %d procceses into file descriptor ...", n); */
		if (select(FD_SETSIZE, &fs_proc, NULL, NULL, &tv) == -1) {
			errn("select()", errno);
		} else {
			/*  data is available */
			if (FD_ISSET(s3d_socket, &fs_proc)) {
				if (1 == tcp_readn(&opcode, 1)) {
					tcp_readn((char *)&length, 2);
					length = ntohs(length);
					buf = (char*)malloc(length);
					tcp_readn(buf, length);
					net_prot_in(opcode, length, buf);
					found = 1;
				} else {
					s3dprintf(HIGH, "socket seems to be dead ...");
					s3d_quit();
				}
			}
		}
	}
	return(found);
}
