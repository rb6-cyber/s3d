/*
 * shm.c
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
#ifdef G_SDL
#include <SDL.h> /* SDL_SetTimer() */
#endif
#ifdef SHM
#include <stdio.h>  /* printf(),fopen(),fclose() */
#include <unistd.h> /* unlink(),usleep() */
#include <stdlib.h> /* realloc(),free() */
#include <string.h> /* memcpy() */
#include <signal.h> /* signal() */
#include <errno.h> /* errno */
#ifdef WIN32  /*  sohn wars */
#include <winsock2.h>
#else  /* sohn wars */
#include <netinet/in.h>  /* ntohs(),htons(),htonl(),ntohl() */
#endif   /*  sohn wars */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h> /* nanosleep() */
#include <stdint.h>

static struct t_shmcb waiting_comblock;

static key_t *data = NULL;
static char ftoken[] = "/tmp/.s3d";
static int shmid;
static int mkey; /* increasing key */

static int shm_new_comblock(key_t *data);

int shm_next_key(void)
{
	mkey = mkey + 1;
	return(mkey);
}
int shm_init(void)
{
	FILE *fp;
	key_t key;

	/* create an empty token file */
	fp = fopen(ftoken, "wb");
	fclose(fp);
	/* make the key: */
	if ((key = ftok(ftoken, 'R')) == -1) {
		errnf("shm_init():ftok()", errno);
		return(1);
	}
	s3dprintf(LOW, "shm_init(): init key is 0x%08x", key);
	mkey = key;

	/* connect to (and possibly create) the segment: */
	if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
		errnf("shm_init():shmget()", errno);
		return(1);
	}

	/* attach to the segment to get a pointer to it: */
	data = (key_t *)shmat(shmid, (void *)0, 0);
	if (data == (key_t *)(-1)) {
		errnf("shm_init():shmat()", errno);
		return(1);
	}
	shm_new_comblock(data);
	return(0);
}
static void comblock_init(struct t_shmcb *p_cb)
{
	p_cb->shmid_ctos = -1;
	p_cb->shmid_stoc = -1;
	p_cb->key_stoc = -1;
	p_cb->key_ctos = -1;
	p_cb->data_ctos = NULL;
	p_cb->data_stoc = NULL;
}
/* registers a communication block, and sets waiting_comblock */
static int shm_new_comblock(key_t *data)
{
	struct t_shmcb *mycb;
	comblock_init(&waiting_comblock);
	mycb = &waiting_comblock;
	mycb->key_stoc = shm_next_key();
	mycb->key_ctos = shm_next_key();
	s3dprintf(MED, "shm_open_comblock():stoc: %08x, ctos: %08x", mycb->key_stoc, mycb->key_ctos);
	/* connect & create the client to server segment: */
	if ((mycb->shmid_ctos = shmget(mycb->key_ctos, RB_STD_SIZE, 0644 | IPC_CREAT)) == -1) {
		errn("shm_open_comblock:shmget()", errno);
		return(1);
	}
	mycb->data_ctos = (char*)shmat(mycb->shmid_ctos, (void *)0, 0);
	if (mycb->data_ctos == (char *)(-1)) {
		errn("shm_open_comblock:shmat()", errno);
		return(1);
	}
	/* connect & create the client to server segment: */
	if ((mycb->shmid_stoc = shmget(mycb->key_stoc, RB_STD_SIZE, 0644 | IPC_CREAT)) == -1) {
		errn("shm_open_comblock:shmget()", errno);
		return(1);
	}
	mycb->data_stoc = (char*)shmat(mycb->shmid_stoc, (void *)0, 0);
	if (mycb->data_stoc == (char *)(-1)) {
		errn("shm_open_comblock:shmat()", errno);
		return(1);
	}

	/* init ringbuffers */
	ringbuf_init(mycb->data_stoc, RB_STD_SIZE);
	ringbuf_init(mycb->data_ctos, RB_STD_SIZE);
	data[0] = mycb->key_ctos;
	data[1] = mycb->key_stoc;
	mycb->idle = 0;
	s3dprintf(LOW, "shm_open_comblock():data: %08x, %08x", data[0], data[1]);
	return(0);
}

int shm_quit(void)
{
	/* detach from the segment: */
	s3dprintf(LOW, "shm_quit()...");
	unlink(ftoken);
	if (data != NULL) {
		data[0] = data[1] = 0;
		data = NULL;
		s3dprintf(MED, "shm_quit():removing init block");
		if (shmdt(data) == -1)
			errn("shm_quit():shmdt()", errno);
		if (shmctl(shmid, IPC_RMID, NULL) == -1)
			errn("shm_quit():shmctl()", errno);
	}
	return(0);
}
int shm_remove(struct t_process *p)
{
	s3dprintf(MED, "shm_remove(): removing pid %d", p->id);
	s3dprintf(MED, "shm_remove():freeing keys: %08x, %08x", p->shmsock.key_ctos, p->shmsock.key_stoc);
	if (shmdt(p->shmsock.data_ctos) == -1)
		errn("shm_rmove():shmdt()", errno);
	if (shmctl(p->shmsock.shmid_ctos, IPC_RMID, NULL) == -1)
		errn("shm_quit():shmctl()", errno);
	if (shmdt(p->shmsock.data_stoc) == -1)
		errn("shm_quit():shmdt()", errno);
	if (shmctl(p->shmsock.shmid_stoc, IPC_RMID, NULL) == -1)
		errn("shm_quit():shmctl()", errno);
	return(0);
}


int shm_main(void)
{
	int      i/*,found*/;
	struct buf_t   *dai; /* data in, data out */
	struct t_process *new_p;
	struct shmid_ds   d;
	int iterations;
	/* do*/
	{
		/*  found=0;*/
		iterations = 0;
		for (i = 0;i < procs_n;i++) {
			iterations++;
#ifdef G_SDL
			SDL_SetTimer(100, (SDL_TimerCallback) net_turn_off);
#endif
			if (procs_p[i].con_type == CON_SHM) {
				dai = (struct buf_t *) procs_p[i].shmsock.data_ctos;
				if (dai->start != dai->end) {
					/*     found=1;*/
					procs_p[i].shmsock.idle = 0;
					shm_prot_com_in(&procs_p[i]);
					if (turn)
						i--; /* evil hack: decrease i so it will be our turn again in the next round */
					else {
						s3dprintf(MED, "client %d [%s] seems to want to keep us busy ... ", i, procs_p[i].name);
						SDL_SetTimer(100, (SDL_TimerCallback) net_turn_off); /* restart timer */
						turn = 1; /* don't decrease, it's next connections turn */
					}
				} else {
					if (procs_p[i].shmsock.idle++ > MAX_IDLE) { /* maybe the function timed out somehow ...? let's check ...*/
						shmctl(procs_p[i].shmsock.shmid_ctos, IPC_STAT, &d);
						if (d.shm_nattch == 1) { /* we're all alone ... remove it!! */
							s3dprintf(MED, "client [%s] detached, removing ... ", procs_p[i].name);
							process_del(procs_p[i].id);
						} else {
							procs_p[i].shmsock.idle = 0;
						}
					}
				}
			}
			if (iterations > 500) {
				turn = 0;
				iterations = 0;
			}
			/*   s3dprintf(MED, "iterations: %d, turn = %d, i = %d\n", iterations, turn, i);*/
		}
#ifdef G_SDL
		SDL_SetTimer(0, NULL);
#endif
	} /*while (found);*/
	if ((data[0] == 0) && (data[1] == 0)) {
		new_p = process_add();
		new_p->con_type = CON_SHM;
		memcpy(&new_p->shmsock, &waiting_comblock, sizeof(struct t_shmcb));
		s3dprintf(HIGH, "shm_main():registered new connection (keys %d, %d) as pid %d", new_p->shmsock.key_ctos, new_p->shmsock.key_stoc, new_p->id);
		s3dprintf(LOW, "shm_main():new client attached! allocating shm block for further clients ...");
		if (shm_new_comblock(data))
			return(1);
	}
	return(0);
}
int shm_prot_com_in(struct t_process *p)
{
	uint16_t length;
	struct buf_t *dai;
	dai = (struct buf_t *)p->shmsock.data_ctos;
	if (dai != NULL)
		/* if ((pid=get_proc_by_dai( */
		if (3 == shm_readn(dai, ibuf, 3)) {
			length = ntohs(*((uint16_t *)((uint8_t *)ibuf + 1)));
			s3dprintf(VLOW, "command %d, length %d", ibuf[0], length);
			if (length > 0) {
				shm_readn(dai, ibuf + sizeof(int_least32_t), length);
			}
			prot_com_in(p, ibuf);
		}
	return(0);
}
#define SHM_MAXLOOP  20
static struct timespec t = {
	0, 1000*1000
}; /* 1 mili seconds */
int shm_writen(struct buf_t *rb, uint8_t *buf, int n)
{
	int no_left, no_written, wait = 0;
	no_left = n;
	while (no_left > 0) {
		no_written = shm_write(rb, (char *)buf, no_left);
		if (no_written <= 0)
			return(no_written);
		no_left -= no_written;
		buf += no_written;
		if (wait++ > SHM_MAXLOOP) {
			s3dprintf(HIGH, "shm_writen():waited too long ...");
			return(-1);
		}
		if (wait > 10)  nanosleep(&t, NULL);
	}
	return(n - no_left);
}
int shm_readn(struct buf_t *rb, uint8_t *buf, int n)
{
	int no_left, no_read, wait = 0;
	no_left = n;
	while (no_left > 0) {
		no_read = shm_read(rb, (char *)buf, no_left);
		if (no_read < 0)
			return(no_read);
		if (no_read == 0)
			break;
		no_left -= no_read;
		buf += no_read;
		if (wait++ > SHM_MAXLOOP) {
			s3dprintf(HIGH, "shm_readn():waited too long ...");
			return(-1);
		}
		if (wait > 10)  nanosleep(&t, NULL);
	}
	return(n - no_left);
}
#endif
