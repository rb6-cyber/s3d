/*
 * animate.c
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d Widgets, a Widget Library for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d Widgets is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * s3d Widgets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d Widgets; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>
#include <math.h>

/* the animation stack */
static s3dw_widget *ani_s[MAXANI];
static int ani_n=0;
static int animation_on=0;

/* is item f already on stack? */
int s3dw_ani_onstack(s3dw_widget *f)
{
	int i;
	for (i=0;i<ani_n;i++)
		if (ani_s[i]==f)
			return(1);		/* already in list */
	return(0);

}
/* add an item on the animation stack */
void s3dw_ani_add(s3dw_widget *f)
{
	if ((ani_n<MAXANI) && (animation_on))
	{
		if (s3dw_ani_onstack(f))
			return;		/* already in list */
		ani_s[ani_n]=f;
		s3dw_ani_iterate(f);
		ani_n++;
	}
	else /* no place, finish now */
		s3dw_ani_finish(f,-1);
}
/* delete an item from the animation stack */
void s3dw_ani_del(int i)
{
	if ((i>=0) && (i<ani_n))
	{
/*		printf("[A]ni DEL %d\n",i);*/
		ani_n--;
		ani_s[i]=ani_s[ani_n]; /* that should also work if i is the last one */
	} else {
		s3dprintf(MED,"[F]ATAL: can't delete animation!\n");
	}
}
/* well ... */
void s3dw_ani_doit(s3dw_widget *f)
{
	s3d_translate(	f->oid, f->ax,f->ay,f->az);
	s3d_rotate(		f->oid, f->arx,f->ary,f->arz);
	s3d_scale(		f->oid, f->as);
}

/* finish an animation on the stack, stack index i */
void s3dw_ani_finish(s3dw_widget *f, int i)
{
	f->ax= f->x;
	f->ay= f->y;
	f->az= f->z;
	f->as= f->s;
	s3dw_ani_doit(f);
	if (i!=-1)
		s3dw_ani_del(i);
}
void s3dw_ani_iterate(s3dw_widget *f)
{
	f->ax=(f->x + f->ax*ZOOMS)/(ZOOMS+1);
	f->ay=(f->y + f->ay*ZOOMS)/(ZOOMS+1);
	f->az=(f->z + f->az*ZOOMS)/(ZOOMS+1);
	f->as=(f->s + f->as*ZOOMS)/(ZOOMS+1);

}

/* checks if f is good enough */
int s3dw_ani_check(s3dw_widget *f)
{
	float x,y,z;
	x=f->ax - f->x;
	y=f->ay - f->y;
	z=f->az - f->z;
	if (((fabs(f->as - f->s)/f->s)>0.01) || (sqrt(x*x+y*y+z*z) > 0.01))
		return(0);
	return(1);
}
/* doing the whole animation thing */
void s3dw_ani_mate()
{
	int i;
	s3dw_widget *f;
	animation_on=1;			/* animation is activated */
	for (i=0;i<ani_n;i++)
	{
		f=ani_s[i];
		s3dw_ani_iterate(f);
		if (s3dw_ani_check(f))
		{
			s3dw_ani_finish(f,i);
			i--; /* a new widget is here now, take care in the next iteration */
		} else {
			s3dw_ani_doit(f);
		}
	}
}

