/*
 * animation.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dfm, a s3d file manager.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dfm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dfm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dfm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "s3dfm.h"
#include <s3dw.h>
#include <stdio.h> 	 /*  printf(),NULL */
#include <math.h>	 /*  fabs() */
#define SCALE 	1

/* the animation stack */
static t_item *ani_s[MAXANI];
static int ani_n=0;
extern t_item root,cam;
int moveon=1;

/* get the scale for the rootbox zoom */
float ani_get_scale(t_item *f)
{
	float scale,s;
	s=0.2;
	scale=1/s;
	if (f->parent!=NULL)
		scale=1/s*ani_get_scale(f->parent);
	else
		return(1.0);
	root.px-=f->px;
	root.pz-=f->pz;
	root.py-=BOXHEIGHT+f->detached*DETHEIGHT;
	root.px*=1/s;
	root.py*=1/s;
	root.pz*=1/s;
	
	return(scale);
}
/* center f for the viewer, therefore moving the root box ... */
void ani_focus(t_item *f)
{
	root.px=0.0;
	root.py=0.0;
	root.pz=0.0;
	moveon=1;
/*	printf("[Z]ooming to %s\n",f->name);*/
	box_collapse_grandkids(f);
	root.scale=ani_get_scale(f);
	root.py-=1.5;
/*	printf("[R]escaling to %f\n",root.scale);
	printf("px: %f py:%f pz: %f\n",root.px,root.py,root.pz);*/

	ani_add(&root);
	if (((cam.dpx-cam.px)* (cam.dpx-cam.px) + (cam.dpy-cam.py)* (cam.dpy-cam.py) 
		  + (cam.dpz-cam.pz)* (cam.dpz-cam.pz))	> ( 10 * 10))
	{
		cam.px=0;
		cam.py=0;
		cam.pz=5;
		ani_add(&cam);
	}
}
/* is item f already on stack? */
int ani_onstack(t_item *f)
{
	int i;
	for (i=0;i<ani_n;i++)
		if (ani_s[i]==f)
			return(1);		/* already in list */
	return(0);

}
/* add an item on the animation stack */
void ani_add(t_item *f)
{
	if (ani_n<MAXANI)
	{
		if (ani_onstack(f))
			return;		/* already in list */
		ani_s[ani_n]=f;
		ani_iterate(f);
	/*	printf("[A]ni ADD %d\n",ani_n); */
		ani_n++;
	}
	else /* no place, finish now */
		ani_finish(f,-1);
}
/* delete an item from the animation stack */
void ani_del(int i)
{
	if ((i>=0) && (i<ani_n))
	{
/*		printf("[A]ni DEL %d\n",i);*/
		ani_n--;
		ani_s[i]=ani_s[ani_n]; /* that should also work if i is the last one */
	} else {
		printf("[F]ATAL: can't delete animation!\n");
	}
}
/* well ... */
void ani_doit(t_item *f)
{
	s3d_translate(	f->block, f->dpx,f->dpy,f->dpz);
	s3d_scale(		f->block, f->dscale);
}

/* finish an animation on the stack, stack index i */
void ani_finish(t_item *f, int i)
{
	f->dpx= f->px;
	f->dpy= f->py;
	f->dpz= f->pz;
	f->dscale= f->scale;
	ani_doit(f);
	if (i!=-1)
		ani_del(i);
}
void ani_iterate(t_item *f)
{
	f->dpx=(f->px + f->dpx*ZOOMS)/(ZOOMS+1);
	f->dpy=(f->py + f->dpy*ZOOMS)/(ZOOMS+1);
	f->dpz=(f->pz + f->dpz*ZOOMS)/(ZOOMS+1);
	f->dscale=(f->scale + f->dscale*ZOOMS)/(ZOOMS+1);

}

/* checks if f is good enough */
int ani_check(t_item *f)
{
	float x,y,z;
	x=f->dpx - f->px;
	y=f->dpy - f->py;
	z=f->dpz - f->pz;
	if (((fabs(f->dscale - f->scale)/f->scale)>0.01) || (sqrt(x*x+y*y+z*z) > 0.01))
		return(0);
	return(1);
}
/* doing the whole animation thing */
void ani_mate()
{
	int i;
	t_item *f;
	s3dw_ani_mate();
	for (i=0;i<ani_n;i++)
	{
		f=ani_s[i];
		ani_iterate(f);
		if (ani_check(f))
		{
			ani_finish(f,i);
			i--; /* a new object is here now, take care in the next iteration */
		} else {
			ani_doit(f);
		}
	}
}
