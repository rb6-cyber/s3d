#include "s3d.h"
#include "s3dlib.h"
#include "proto.h"	 
#include <stdlib.h>	 /*  malloc(),free(), realloc() */
#include <unistd.h>	 /*  usleep() */
/*  objects are requested before beeing used for having fast  */
/*  access when needed. this also makes things more asynchronous, */
/*  therefore faster (I hope). */

#define Q_UNUSED	-1				 /*  unused slot magic number */
#define MAX_REQ		100				 /*  don't request more than that. */
static unsigned int *queue;			 /*  the object id's */
static int queue_size=0;			 /*  the size of the object queue */
static int requested;				 /*  counter of how many addtional */
									 /*  objects have been requested */
/*  initializes the object queue */
int _queue_init()
{
	int i;
	queue_size=1;
	requested=0;
	queue=malloc(sizeof(unsigned int)*queue_size);
	for (i=0;i<queue_size;i++)
	{
		queue[i]=Q_UNUSED;
	}
	_queue_fill();
	return(0);
}
/*  checks the queue empty slots and requests new ones if needed */
int _queue_fill()
{
	int i;
	for (i=0;i<queue_size;i++)
		if (queue[i]==Q_UNUSED)
			net_send(S3D_P_C_NEW_OBJ,NULL,0);
	return(0);
}
/*  we have a new object from the server, trying to find a place for it */
int _queue_new_object(unsigned int oid)
{
	int i;
/* 	dprintf(LOW,"having a new object (%d) in the queue!!",oid); */
	for (i=0;i<queue_size;i++)
		if (queue[i]==Q_UNUSED)
		{
/* 			dprintf(LOW,"placing it at position %d",i); */
			queue[i]=oid;
			return(0);
		}
	if (queue_size==0) return(-1);  /*  already quit. */
	 /*  if we reach here, all slots all taken.  */
/* 	dprintf(LOW,"no place for object, resizing stack.",i); */
	queue=realloc(queue,sizeof(unsigned int)*(queue_size+1));
	queue_size+=1;
	queue[queue_size-1]=oid;
	return(0);
}
/*  an object is requested!! give one out: */
unsigned int _queue_want_object()
{
	unsigned int ret;
	int i;
	do {
	for (i=0;i<queue_size;i++)
		if (queue[i]!=Q_UNUSED)
		{
			ret=queue[i];
			queue[i]=Q_UNUSED;
			net_send(S3D_P_C_NEW_OBJ,NULL,0);  /*  we already can request a new one. */
			return(ret);
		}
	 /*  if we reach this point, our queue is empty. */
	 /*  as other request should have sent S3D_P_C_NEW_OBJ-requests,  */
	 /*  we request one more object than needed to satisfy more load in future. */
	if (queue_size==0) return(-1);  /*  already quit. */
	if (requested<MAX_REQ)
	{
		net_send(S3D_P_C_NEW_OBJ,NULL,0);
		requested++;
	}
	s3d_net_check();
	usleep(10);
	} while(i++<TIMEOUT);

	errds(LOW,"_queue_want_object()","timeout is reached. server is extremly slow/laggy or dead");	
	return(-1);
}
/*  cleans up */
int _queue_quit()
{
	if (queue!=NULL)
	{
		free(queue);
		queue=NULL;
	}
	queue_size=0;
	return(0);
}
