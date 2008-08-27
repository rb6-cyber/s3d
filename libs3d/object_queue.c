/*
 * object_queue.c
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
#include <stdlib.h>  /*  malloc(),free(), realloc() */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309  /* we want struct timespec to be defined */
#endif
#ifndef __USE_POSIX199309
#define __USE_POSIX199309 1
#endif
#include <time.h>   /*  nanosleep() */

/*  objects are requested before beeing used for having fast  */
/*  access when needed. this also makes things more asynchronous, */
/*  therefore faster (I hope). */

#define Q_UNUSED ((unsigned int)~0)      /*  unused slot magic number */
#define MAX_REQ  100      /*  don't request more than that. */
static unsigned int *queue;     /*  the object id's */
static int queue_size = 0;   /*  the size of the object queue */
static int requested;      /*  counter of how many addtional */
/*  objects have been requested */
static struct timespec t = {
	0, 10*1000
}; /* 10 micro seconds */
/*  initializes the object queue */
int _queue_init(void)
{
	int i;
	queue_size = 1;
	requested = 0;
	queue = (unsigned int*)malloc(sizeof(unsigned int) * queue_size);
	for (i = 0;i < queue_size;i++) {
		queue[i] = Q_UNUSED;
	}
	_queue_fill();
	return(0);
}
/*  checks the queue empty slots and requests new ones if needed */
int _queue_fill(void)
{
	int i;
	for (i = 0;i < queue_size;i++)
		if (queue[i] == Q_UNUSED)
			net_send(S3D_P_C_NEW_OBJ, NULL, 0);
	return(0);
}
/*  we have a new object from the server, trying to find a place for it */
int _queue_new_object(unsigned int oid)
{
	int i;
	/*  s3dprintf(LOW,"having a new object (%d) in the queue!!",oid); */
	for (i = 0;i < queue_size;i++)
		if (queue[i] == Q_UNUSED) {
			/*    s3dprintf(LOW,"placing it at position %d",i); */
			queue[i] = oid;
			requested--;
			return(0);
		}
	if (queue_size == 0) return(-1);  /*  already quit. */
	/*  if we reach here, all slots all taken.  */
	/*  s3dprintf(LOW,"no place for object, resizing stack.",i); */
	queue = (unsigned int*)realloc(queue, sizeof(unsigned int) * (queue_size + 1));
	queue_size += 1;
	requested--;
	queue[queue_size-1] = oid;
	return(0);
}
/*  an object is requested!! give one out: */
unsigned int _queue_want_object(void)
{
	unsigned int ret;
	int i, j;
	j = 0;
	do {
		for (i = 0;i < queue_size;i++)
			if (queue[i] != Q_UNUSED) {
				ret = queue[i];
				queue[i] = Q_UNUSED;
				net_send(S3D_P_C_NEW_OBJ, NULL, 0);  /*  we already can request a new one. */
				return(ret);
			}
		/*  if we reach this point, our queue is empty. */
		/*  as other request should have sent S3D_P_C_NEW_OBJ-requests,  */
		/*  we request one more object than needed to satisfy more load in future. */
		if (queue_size == 0) return(-1);  /*  already quit. */
		if (requested < MAX_REQ) {
			net_send(S3D_P_C_NEW_OBJ, NULL, 0);
			requested++;
		}
		s3d_net_check();
		nanosleep(&t, NULL);
	} while (j++ < TIMEOUT);

	errds(LOW, "_queue_want_object()", "timeout is reached. server is extremly slow/laggy or dead");
	return(-1);
}
/*  cleans up */
int _queue_quit(void)
{
	if (queue != NULL) {
		free(queue);
		queue = NULL;
	}
	queue_size = 0;
	return(0);
}
