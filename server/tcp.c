/*
 * tcp.c
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
#ifdef TCP
#include <errno.h>   /*  errno */
#include <string.h>   /*  memset() */
#ifdef WIN32  /*  sohn wars */
#include <winsock2.h>
#else  /* sohn wars */
#include <sys/types.h>   /* fd_set, FD*, socket, accept ... */
#include <sys/socket.h>  /* socket, accept ... */
#include <sys/select.h>  /* fd_set,FD* */
#include <sys/time.h>  /* fd_set,FD* */
#include <netinet/in.h>  /* ntohs(),htons(),htonl(),ntohl() */
#include <arpa/inet.h>   /* network */
#endif   /*  sohn wars */
#include <time.h>   /*  select() timeval things */
#include <fcntl.h>   /*  fcntl(),F_SETOWN */
#ifndef F_SETOWN /* somehow it is not set with -ansi */
#define F_SETOWN 8
#endif
#include <unistd.h>   /*  read(),write(),getpid(),close() */
#include <stdlib.h>   /*  malloc(),free() */
#include <stdint.h>

static int tcp_sockid;
int tcp_init(void)
{
	int yes = 1;
	struct sockaddr_in my_addr;
	s3dprintf(LOW, "server: creating socket");
#ifdef WIN32  /*  sohn wars */
	WSADATA datainfo;
	if (WSAStartup(257, &datainfo) != 0)
		errnf("startup()", 0);
#endif  /*  auch sohn */
	if ((tcp_sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		errnf("socket()", errno);

	s3dprintf(LOW, "server: binding my local socket");
	/*  allow addresses to be reused */
	/*  this seems to have something to do with servers using one port */
	if (setsockopt(tcp_sockid, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		errn("setsockopt(...,SO_REUSEADDR...)", errno);
	memset((char *) &my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(S3D_PORT);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY);
	if (bind(tcp_sockid , (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
		errnf("bind()", errno);
	if (listen(tcp_sockid, 5) < 0)
		errnf("listen()", errno);
#ifdef SIGS
	if (fcntl(tcp_sockid, F_SETFL, O_ASYNC | O_NONBLOCK) < 0)
		errnf("fcntl()", errno);
	if (fcntl(tcp_sockid, F_SETOWN, getpid()) < 0)
		errnf("fcntl()", errno);
#endif
	return(0);

}
int tcp_quit(void)
{
	close(tcp_sockid);
#ifdef WIN32
	WSACleanup();
#endif
	return(0);
}
/*  watches the port for new connections */
int tcp_pollport(void)
{
	fd_set     fs_port;   /*  filedescriptor set for listening port(s) */
	int      newsd;   /*  new socket descriptor */
	struct timeval   tv;   /*  time structure */
	/* struct t_con   *new_con; */ /*  pointer to new connection */
	struct t_process *new_p;   /*  pointer to new process */
	struct sockaddr   client_addr;  /*  new client's address */
	socklen_t    clilen = sizeof(client_addr); /*  length of client's address */
	/* int i; */
	FD_ZERO(&fs_port);
	FD_SET(tcp_sockid, &fs_port);
select_again:
	tv.tv_sec = tv.tv_usec = 0;
	if (select(FD_SETSIZE, &fs_port, NULL, NULL, &tv) < 0) {
		if (errno == EINTR) { /*  interruption by some evil signal, just do again :) */
			errn("tcp_pollport():select()", errno);
			goto select_again;  /*  oh no, a goto!! that's evil */
		} else
			errn("tcp_pollport():select()", errno);
	} else
		if (FD_ISSET(tcp_sockid, &fs_port)) { /* redundant, I guess */
			s3dprintf(HIGH, "select(): new connection!!");
			if ((newsd = accept(tcp_sockid , (struct sockaddr *) & client_addr, &clilen)) < 0)
				errn("accept()", errno);
			else {
#ifdef SIGS
				if (fcntl(newsd, F_SETFL, O_ASYNC) < 0)
					errnf("fcntl()", errno);
				if (fcntl(newsd, F_SETOWN, getpid()) < 0)
					errnf("fcntl()", errno);
#endif
				new_p = process_add();
				new_p->con_type = CON_TCP;
				new_p->sockid = newsd;
				s3dprintf(HIGH, "registered new connection %d as pid %d", new_p->sockid, new_p->id);
			}
		}
	return(0);
}
/*  this is about looking for new data on the sockets */
/*  returns 1 when there was new data. */
int tcp_pollproc(void)
{
	fd_set     fs_proc;   /*  filedescriptor set for listening port(s) */
	struct timeval   tv;   /*  time structure */
	struct t_process *p;
	int      found = 0;
	int      i, unfinished, n, off;
	off = 0;
	do {
		FD_ZERO(&fs_proc);
		unfinished = 0;
		n = 0;
		for (i = off;i < procs_n;i++) {
			p = &procs_p[i];
			if (p->con_type == CON_TCP) {
				FD_SET(p->sockid, &fs_proc);
				n++;
				if (n >= FD_SETSIZE) { /* don't overflow the setsize! */
					off = i;
					unfinished = 1;
					break;
				}
			}
		}
		/*  maybe having a global fd_set for all the processes would have been better */
		/*  than generating them new in every poll. to be optimized... */
select_again_poll:
		tv.tv_sec = tv.tv_usec = 0;
		if (select(FD_SETSIZE, &fs_proc, NULL, NULL, &tv) == -1) {
			if (errno == EINTR) {
				errn("tcp_pollproc():select()", errno);
				goto select_again_poll;
			} else {
				errn("tcp_pollproc():select()", errno);
			}
		} else {
			/*  data is available */
			for (i = 0;i < procs_n;i++) {
				p = &procs_p[i];
				if (p->con_type == CON_TCP) {
					if (FD_ISSET(p->sockid, &fs_proc)) {
						FD_CLR(p->sockid, &fs_proc); /*  clear it from the fd */
						tcp_prot_com_in(p);
						found = 1;
					}
				}
			}
		}
	} while (unfinished);
	return(found);
}
/* read some data from the line, pushes it into the buffer and calls prot_com_in */
int tcp_prot_com_in(struct t_process *p)
{
	uint16_t length;
	if (3 == tcp_readn(p->sockid, ibuf, 3)) {
		length = ntohs(*((uint16_t *)((uint8_t *)ibuf + 1)));
		s3dprintf(VLOW, "command %d, length %d", ibuf[0], length);
		if (length > 0) {
			tcp_readn(p->sockid, ibuf + sizeof(int_least32_t), length);   /*  uint16_t is limited to 65536, so  */
			/*  length can't be bigger than that ... lucky */
		}
		prot_com_in(p, ibuf);
	} else {
		s3dprintf(LOW, "tcp_prot_com_in():n_readn():fd seems to be dead (pid %d, sock %d)", p->id, p->sockid);
		process_del(p->id);
	}
	return(0);
}
/*  shamelessly ripped from simple ftp server */
int tcp_readn(int sock, uint8_t *str, int s)
{
	int no_left, no_read;
	no_left = s;
	while (no_left > 0) {
		no_read = read(sock, str, no_left);
		if (no_read < 0) {
			errn("read()", errno);
			return(no_read);
		}
		if (no_read == 0) break;
		no_left -= no_read;
		str += no_read;
	}
	return(s - no_left);
}
int tcp_writen(int sock, uint8_t *str, int s)
{
	int no_left, no_written;
	no_left = s;
	while (no_left > 0) {
		no_written = write(sock, str, no_left);
		if (no_written <= 0) {
			errn("write()", errno);
			return(no_written);
		}
		no_left -= no_written;
		str += no_written;
	}
	return(s - no_left);
}
int tcp_remove(int sock)
{
	return(close(sock));
}
#endif
