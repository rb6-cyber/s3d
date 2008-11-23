/*
 * shm.c
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
#include <stdlib.h>  /* malloc() */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h> /* ntohs() */
#include <errno.h>   /* errno */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309  /* we want struct timespec to be defined */
#endif
#ifndef __USE_POSIX199309
#define __USE_POSIX199309 1
#endif
#include <time.h>   /*  nanosleep() */

#ifdef SHM

#define SHM_SIZE   sizeof(key_t)*2    /* space for the keys */
#define SHM_MAXLOOP  100
#define RB_STD_SIZE  1024*512
static struct buf_t *data_in, *data_out;
static int shmid_in, shmid_out;
static int shm_idle = 0;
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili second */
/* char ftoken[]="/tmp/.s3d_shm";*/

int _shm_init(char *ftoken)
{
	int shmid;
	uint32_t *next_key;
	/* struct shmid_ds *buf; */
	key_t key, key_out, key_in;

	s3dprintf(MED, "connecting to shm token %s", ftoken);
	/* make the key: */
	if ((key = ftok(ftoken, 'R')) == -1) {
		errn("shm_init():ftok()", errno);
		return(1);
	}
	s3dprintf(MED, "init key is 0x%08x", key);

	/* connect to the segment: */
	if ((shmid = shmget(key, SHM_SIZE, 0644)) == -1) {
		errn("shm_init():shmget()", errno);
		return(1);
	}

	/* attach to the segment to get a pointer to it: */
	next_key = (uint32_t*)shmat(shmid, (void *)0, 0);
	if (next_key == (uint32_t *)(-1)) {
		errn("shm_init():shmat()", errno);
		return(1);
	}
	s3dprintf(MED, "right now, next_keys are: %08x, %08x", next_key[0], next_key[1]);
	while ((0 == (key_in = next_key[1])) || (0 == (key_out = next_key[0]))) {}
	next_key[0] = next_key[1] = 0;
	s3dprintf(MED, "right now, next_keys are: %08x, %08x", key_in, key_out);
	/* as we have the new key, we  can detach here now. */
	if (shmdt(next_key) == -1) {
		errn("shm_init():shmdt()", errno);
		return(1);
	}
	/* get input buffer */
	if ((shmid_in = shmget(key_in, RB_STD_SIZE, 0644)) == -1) {
		errn("shm_init():shmget()", errno);
		return(1);
	}
	/* attach to the  in segment to get a pointer to it: */
	data_in = (struct buf_t *) shmat(shmid_in, (void *)0, 0);
	if (data_in == (struct buf_t *)(-1)) {
		errn("shm_init():shmat()", errno);
		return(1);
	}
	/* get output buffer */
	if ((shmid_out = shmget(key_out, RB_STD_SIZE, 0644)) == -1) {
		errn("shm_init():shmget()", errno);
		return(1);
	}
	/* attach to the out segment to get a pointer to it: */
	data_out = (struct buf_t *) shmat(shmid_out, (void *)0, 0);
	if (data_out == (struct buf_t *)(-1)) {
		errn("shm_init():shmat()", errno);
		return(1);
	}
	return(0);
}
int _shm_quit(void)
{
	/* detach from the segment: */
	if (shmdt(data_in) == -1) {
		errn("shm_init():shmdt()", errno);
		return(1);
	}
	if (shmdt(data_out) == -1) {
		errn("shm_init():shmdt()", errno);
		return(1);
	}
	data_in = data_out = NULL;
	return(0);
}
int shm_writen(char *str, int s)
{
	int no_left, no_written, wait = 0;
	no_left = s;
	while (no_left > 0) {
		no_written = shm_write(data_out, str, no_left);
		if (no_written < 0)
			return(no_written);
		no_left -= no_written;
		str += no_written;
		if (wait++ > SHM_MAXLOOP) {
			s3dprintf(HIGH, "shm_writen():waited too long ...");
			return(-1);
		}
		if (wait > 10)
			nanosleep(&t, NULL);
	}
	return(s - no_left);
}
int shm_readn(char *str, int s)
{
	int no_left, no_read, wait = 0;
	no_left = s;
	while (no_left > 0) {
		no_read = shm_read(data_in, str, no_left);
		if (no_read < 0)
			return(no_read);
		if (no_read == 0)
			break;
		no_left -= no_read;
		str += no_read;
		if (wait++ > SHM_MAXLOOP) {
			s3dprintf(HIGH, "shm_readn():waited too long ...");
			return(-1);
		}
		if (wait > 10)
			nanosleep(&t, NULL);
	}
	return(s - no_left);
}
int _shm_net_receive(void)
{
	int      found = 0;
	char     opcode, *buf;
	uint16_t   length;
	struct shmid_ds   d;

	if (data_in == NULL)
		return(found);
	if (data_in->start != data_in->end) {
		if (1 == shm_readn(&opcode, 1)) {
			shm_readn((char *)&length, 2);
			length = ntohs(length);
			buf = (char*)malloc(length);
			shm_readn(buf, length);
			net_prot_in(opcode, length, buf);
			found = 1;
		} else {
			s3dprintf(HIGH, "socket seems to be dead ...");
			s3d_quit();
		}
	} else {
		if (shm_idle++ > SHM_MAX_IDLE) {
			shmctl(shmid_in, IPC_STAT, &d);
			if (d.shm_nattch == 1) { /* we're all alone ... remove it!! */
				s3dprintf(MED, "server vanished ... ");
				s3d_quit();
			} else
				shm_idle = 0;
		}
	}
	return(found);
}
#endif
