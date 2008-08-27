/*
 * animation.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <stdio.h>   /*  printf(),NULL */
#include <math.h>  /*  fabs() */
#define SCALE  1

/* the animation stack */
static t_node *ani_s[MAXANI];
static int ani_n = 0;

/* is node f already on stack? */
int ani_onstack(t_node *f)
{
	int i;
	for (i = 0;i < ani_n;i++)
		if (ani_s[i] == f)
			return(i);  /* already in list */
	return(-1);

}
/* add an node on the animation stack */
void ani_add(t_node *f)
{
	if (ani_n < MAXANI) {
		if (-1 != ani_onstack(f))
			return;  /* already in list */
		ani_s[ani_n] = f;
		ani_iterate(f);
		/* printf("[A]ni ADD %d\n",ani_n); */
		ani_n++;
	} else /* no place, finish now */
		ani_finish(f, -1);
}
/* delete an node from the animation stack */
void ani_del(int i)
{
	if ((i >= 0) && (i < ani_n)) {
		/*  printf("[A]ni DEL %d\n",i);*/
		ani_n--;
		ani_s[i] = ani_s[ani_n]; /* that should also work if i is the last one */
	} else {
		printf("[F]ATAL: can't delete animation!\n");
	}
}
/* well ... */
void ani_doit(t_node *f)
{
	s3d_translate(f->oid, f->dpx, f->dpy, f->dpz);
	s3d_scale(f->oid, f->dscale);
}

/* finish an animation on the stack, stack index i */
void ani_finish(t_node *f, int i)
{
	f->dpx = f->px;
	f->dpy = f->py;
	f->dpz = f->pz;
	f->dscale = f->scale;
	ani_doit(f);
	if (i != -1)
		ani_del(i);
}
void ani_iterate(t_node *f)
{
	f->dpx = (f->px + f->dpx * ZOOMS) / (ZOOMS + 1);
	f->dpy = (f->py + f->dpy * ZOOMS) / (ZOOMS + 1);
	f->dpz = (f->pz + f->dpz * ZOOMS) / (ZOOMS + 1);
	f->dscale = (f->scale + f->dscale * ZOOMS) / (ZOOMS + 1);

}

/* checks if f is good enough */
int ani_check(t_node *f)
{
	float x, y, z;
	x = f->dpx - f->px;
	y = f->dpy - f->py;
	z = f->dpz - f->pz;
	if (((fabs(f->dscale - f->scale) / f->scale) > 0.01) || (sqrt(x*x + y*y + z*z) > 0.01))
		return(0);
	return(1);
}
/* doing the whole animation thing */
void ani_mate(void)
{
	int i;
	t_node *f;
	s3dw_ani_mate();
	for (i = 0;i < ani_n;i++) {
		f = ani_s[i];
		if (f->oid == -1) { /* kick out bad animations */
			ani_del(i);
			i--;
		} else {
			ani_iterate(f);
			if (ani_check(f)) {
				ani_finish(f, i);
				i--; /* a new object is here now, take care in the next iteration */
			} else {
				ani_doit(f);
			}
		}
	}
}
