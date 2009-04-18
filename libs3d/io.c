/*
 * io.c
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
#include "proto.h"
#include <stdio.h>    /*  fopen(),fclose(),fileno() */
#include <string.h>   /*  strncpy(),strncmp(),memcpy() */
#include <stdlib.h>   /*  atoi(),malloc(),free() */
#include <sys/stat.h>  /*  fstat() */
#include <unistd.h>   /*  getpid(), fstat() */
#include <errno.h>   /*  errno */
#include <sys/socket.h>  /*  socket() */
#include <getopt.h>   /*  getopt() */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309  /* we want struct timespec to be defined */
#endif
#ifndef __USE_POSIX199309
#define __USE_POSIX199309 1
#endif
#include <time.h>   /*  nanosleep() */

#ifdef SIGS
#include <fcntl.h>   /*  fcntl() */
#define __USE_BSD 1  /* we want sig_t to be defined */
#include <signal.h>   /*  signal.h, SIG_PIPE */
#endif
#include <netinet/in.h>  /*  htons(),htonl() */
#ifndef WIN32
#include <netdb.h>   /*  gethostbyname()  */
#endif

static char    *url = NULL;
/*  this file is the client-lib-implementation which holds the function to connect and control the server. */
#ifdef SIGS
int _s3d_sigio = 0;
static void sigint_handler(int S3DUNUSED(sig), int S3DUNUSED(code))  /*  ... ? */
{
	/*s3d_quit();*/ /* TODO: sometimes no clean quit ?!*/
	exit(-1);
}

#endif
int _s3d_ready = 0;

void s3d_usage(void)
{
	printf("s3d-parameters:\n");
	printf(" --s3d-url <url>: skip S3D environment and connect to this url\n");
	printf(" --help, -h, --s3d-help: this helpful text\n");
}

static int parse_args(int *argc, char ***argv)
{
	int     c;
	int      lopt_idx;
	int     old_argc, curopt;
	struct option long_options[] = {
		{"s3d-url", 1, NULL, 'u'},
		{"help", 0, NULL, 'h'},
		{"s3d-help", 0, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	if ((argc == NULL) || (argv == NULL)) return(0); /* nothing to parse */
	old_argc = *argc;
	optind = 0;
	opterr = 0; /* we don't want to be bothered if there is some error */
	*argc = 1;
	curopt = 1;
	while (-1 != (c = getopt_long(old_argc, *argv, "-h", long_options, &lopt_idx))) {
		switch (c) {
		case 'u':
			if (0 == strcmp(long_options[lopt_idx].name, "s3d-url")) {
				if (optarg) {
					url = optarg;
					s3dprintf(HIGH, "connecting to %s", url);
				}
			}
			break;
		case 'h':
			printf("usage: %s [options]", (*argv)[0]);
			s3d_usage();
			return(-1);
		default:
			/* ignore args which are not for us, but maybe the app which builds on us */
			(*argv)[(*argc)] = (*argv)[curopt];
			(*argc)++;
			break;
		}
		curopt = optind;
	}
	for (c = optind; c < old_argc; c++) {
		(*argv)[(*argc)] = (*argv)[c];
		(*argc)++;
	}
	optind = 0;
	return(0);
}
/*  external functions go here ... */
int s3d_init(int *argc, char ***argv, const char *name)
{
	char     *s;
	char      urlc[256];   /*  this should be enough for an url */
	char      buf[258];    /*  server buffer */
	int      i;
	struct timespec   t = {
		0, 100*1000*1000
	}; /* 100 mili second */

	cb_lock = 1; /* don't bother while initiating ... is set to 0 after INIT packet received. */
	if (NULL != (s = getenv("S3D"))) {
		s3dprintf(VLOW, "at least we have the environment variable ... %s", s);
		url = s;
	}
	parse_args(argc, argv);
	if (url == NULL) { /* no url specified or obtained through arguments */
		/* trying standard ways to connect */
		strncpy(urlc, "s3d:///tmp/.s3d:shm/", 256);
		if (s3d_net_init(urlc) == CON_NULL) {
			strncpy(urlc, "s3d://127.0.0.1:6066/", 256);
			if (s3d_net_init(urlc) == CON_NULL)
				return(-1);
		}
	} else {
		strncpy(urlc, url, 256);  /*  this should keep buffer overflows away, maybe */
		urlc[255] = '\0';  /*  just to make sure */
		if (!strncmp(urlc, "s3d:// ", 6)) {
			if (s3d_net_init(urlc) == CON_NULL) return(-1);
		} else {
			errs("s3d_init()", "invalid url");
			return(-1);
		}
	}
	strncpy(buf, name, 256);  /*  copy the name ... */
	net_send(S3D_P_C_INIT, buf, strlen(buf));

	_queue_init();
	_s3d_texture_init();
#ifdef SIGS
	if (signal(SIGINT, (sig_t)sigint_handler) == SIG_ERR)
		errdn(LOW, "s3d_init():signal()", errno);
	if (signal(SIGTERM, (sig_t)sigint_handler) == SIG_ERR)
		errdn(LOW, "s3d_init():signal()", errno);
#endif
	for (i = 0;i < 100;i++) {
		s3d_net_check(); /* wait for init packet */
		nanosleep(&t, NULL);
		if (_s3d_ready) {
			cb_lock--;
			return(0);
		}
	}
	return(-1);
}
/*  shuts down the socket, clearing the stack */
int s3d_quit(void)
{
	struct s3d_evt *ret;
	_s3d_texture_quit();
	if (con_type != CON_NULL && _s3d_ready) {
		net_send(S3D_P_C_QUIT, NULL, 0);
		switch (con_type) {
#ifdef TCP
		case CON_TCP:
			_tcp_quit();
			break;
#endif
#ifdef SHM
		case CON_SHM:
			_shm_quit();
			break;
#endif
		}
		con_type = CON_NULL;
		_s3d_ready = 0;
		_queue_quit();
		while (NULL != (ret = s3d_pop_event())) s3d_delete_event(ret);  /*  clear the stack ... */
		cb_lock = 0; /* we don't care about old callbacks, now we just quit! */
		ret = (struct s3d_evt *)malloc(sizeof(struct s3d_evt));
		ret->event = S3D_EVENT_QUIT;
		ret->length = 0;
		s3d_push_event(ret);
	}
	return(0);
}
/*  apps should use that as main loop for their programs. */
int s3d_mainloop(void (*f)())
{
	while (con_type != CON_NULL) {
		cb_lock++;   /* no callbacks while we are in mainloop */
		if (f != NULL) f();
		cb_lock--;
		s3d_process_stack();
		s3d_net_check();  /* get any other packets we might have missed */
	}
	return(0);
}
/*  opens a file returning it's filesize  */
/*  and setting *pointer to the buffer. to be freed */
int s3d_open_file(const char *fname, char **pointer)
{
	FILE *fp;
	char *buf = NULL;
	int filesize;
	struct stat bf;
	*pointer = NULL;
	/* if ((fp = fopen(fname, "rb")) == NULL)
	 { errn("s3d_open_file():fopen()",errno); return(0);}
	 if (fseek(fp, 0, SEEK_END) != 0)
	 { errn("s3d_open_file():fseek()",errno); return(0);}
	 if ((filesize = (int)ftell(fp)) == (long)-1)
	 { errn("s3d_open_file():ftell()",errno); return(0);}
	 if (fseek(fp, 0, SEEK_SET) != 0)
	 { errn("s3d_open_file():fseek()",errno); return(0);}*/

	if ((fp = fopen(fname, "rb")) == NULL) {
		errdn(VLOW, "s3d_open_file():fopen()", errno);
		return(-1);
	}
	if (fstat(fileno(fp), &bf)) {
		errdn(VLOW, "s3d_open_file():fstat()", errno);
		return(-1);
	}
	filesize = bf.st_size;
	/* s3dprintf(LOW, "opening %s, filesize is %d",fname, filesize);*/
	if ((buf = (char *)malloc(filesize)) == NULL) {
		errn("s3d_open_3ds_file():malloc()", errno);
		exit(-1);
	}
	fread(buf, 1, filesize, fp);
	fclose(fp);
	*pointer = buf;
	return(filesize);
}
