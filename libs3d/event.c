/*
 * event.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <stdlib.h>		 /*  malloc(), free() */

struct s3d_evt *s3d_stack;
int cb_lock=2;	 /*  callback lock */
void s3d_push_event(struct s3d_evt *newevt)
{
	struct s3d_evt *p;
	s3d_cb cb;

	s3dprintf(MED,"pushed event %d",newevt->event);
	 /*  this will always be called for S3D_EVENT_NEW_OBJECT!! */
	if (newevt->event==S3D_EVENT_NEW_OBJECT)
	{
		_queue_new_object(*((unsigned int *)newevt->buf));	
	}
	if (cb_lock)  /*  no recursive event-callbacks, please! */
	{	
		cb_lock=2;  /*  we want our event processed after this finishs */
	} else 	{
		if (NULL!=(cb=s3d_get_callback(newevt->event)))
		{
			cb_lock=1;		 /*  on our way! lock it.. */
			cb(newevt);		 /*  .. and call it! */
			if (cb_lock==2)  /*  there were other callbacks? we process: */
			{
				cb_lock=0;
				s3d_process_stack();
			}
			free(newevt);
			return;
		}
	}
	newevt->next=NULL;
	if (s3d_stack!=NULL)
	{
		for (p=s3d_stack;p->next!=NULL;p=p->next);  /*  go to the end */
		p->next=newevt;
	} else
		s3d_stack=newevt;
}
struct s3d_evt *s3d_pop_event()
{
	struct s3d_evt *ret;
	if ((ret=s3d_stack)!=NULL)
		s3d_stack=s3d_stack->next;
	return ret;
}
struct s3d_evt *s3d_find_event(unsigned char event)
{
	struct s3d_evt *p;
	p=s3d_stack;
	while (p!=NULL)
	{
		if (p->event==event)
			return(p);
		p=p->next;
	}
	return(NULL);
}
int s3d_delete_event(struct s3d_evt *devt)
{
	struct s3d_evt *previous=NULL;
	struct s3d_evt *p=s3d_stack;
	while (p!=NULL)
	{
		 /* if ((p->event==devt->event) && (p->length==devt->length)) */
		 /* 	if (0==memcmp(p->buf,devt->buf)) */
		if (p==devt)
			{
				if (p->length>0) 
					free(p->buf);
				if (previous==NULL) 
					s3d_stack=p->next;  /*  the first element!! */
				else
					previous->next=p->next;  /*  unlink */
				free(p);
			}
		previous=p;
		p=p->next;
	}
	return(-1);
}
/*  this function checks the stack for callbacks. */
void s3d_process_stack()
{
	struct s3d_evt *p,*pre,*t;
	s3d_cb cb;
	p=s3d_stack;
	pre=NULL;
	if (cb_lock!=0) /* can't do that now. */
	{
		cb_lock=2; /* request later processing */
		return;
	}
	s3dprintf(LOW,"processing stack ...");
	while (p!=NULL)
	{
		
		if ((cb=s3d_get_callback(p->event))!=NULL)
		{
			cb_lock=1;
			cb(p);
			cb_lock=0;
			if (pre!=NULL)   /*  there is a previous element */
				pre->next=p->next;
			else			 /*  it's the first element */
				s3d_stack=p->next;
			t=p->next;
			 /*  free */
			if (p->length>0) 
				free(p->buf);
			free(p);	
			p=t;
			
		}
		else 
		{
			pre=p;
			p=p->next;
		}
	}
}
