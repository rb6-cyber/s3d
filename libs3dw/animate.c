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
static struct s3dw_object *ani_s[MAXANI];
static int ani_n=0;
int moveon=1;

/* is item f already on stack? */
int _s3dw_ani_onstack(struct s3dw_object *f)
{
	int i;
	for (i=0;i<ani_n;i++)
		if (ani_s[i]==f)
			return(1);		/* already in list */
	return(0);

}
/* add an item on the animation stack */
void _s3dw_ani_add(struct s3dw_object *f)
{
	if (ani_n<MAXANI)
	{
		if (_s3dw_ani_onstack(f))
			return;		/* already in list */
		ani_s[ani_n]=f;
		_s3dw_ani_iterate(f);
		ani_n++;
	}
	else /* no place, finish now */
		_s3dw_ani_finish(f,-1);
}
/* delete an item from the animation stack */
void _s3dw_ani_del(int i)
{
	if ((i>=0) && (i<ani_n))
	{
/*		printf("[A]ni DEL %d\n",i);*/
		ani_n--;
		ani_s[i]=ani_s[ani_n]; /* that should also work if i is the last one */
	} else {
		dprintf(MED,"[F]ATAL: can't delete animation!\n");
	}
}
/* well ... */
void _s3dw_ani_doit(struct s3dw_object *f)
{
	s3d_translate(	*(f->_o), f->_dx,f->_dy,f->_dz);
	s3d_scale(		*(f->_o), f->_ds);
}

/* finish an animation on the stack, stack index i */
void _s3dw_ani_finish(struct s3dw_object *f, int i)
{
	f->_dx= f->_x;
	f->_dy= f->_y;
	f->_dz= f->_z;
	f->_ds= f->_s;
	_s3dw_ani_doit(f);
	if (i!=-1)
		_s3dw_ani_del(i);
}
void _s3dw_ani_iterate(struct s3dw_object *f)
{
	f->_dx=(f->_x + f->_dx*ZOOMS)/(ZOOMS+1);
	f->_dy=(f->_y + f->_dy*ZOOMS)/(ZOOMS+1);
	f->_dz=(f->_z + f->_dz*ZOOMS)/(ZOOMS+1);
	f->_ds=(f->_s + f->_ds*ZOOMS)/(ZOOMS+1);

}

/* checks if f is good enough */
int _s3dw_ani_check(struct s3dw_object *f)
{
	float x,y,z;
	x=f->_dx - f->_x;
	y=f->_dy - f->_y;
	z=f->_dz - f->_z;
	if (((fabs(f->_ds - f->_s)/f->_s)>0.01) || (sqrt(x*x+y*y+z*z) > 0.01))
		return(0);
	return(1);
}
/* doing the whole animation thing */
void s3dw_ani_mate()
{
	int i;
	struct s3dw_object *f;
	for (i=0;i<ani_n;i++)
	{
		f=ani_s[i];
		_s3dw_ani_iterate(f);
		if (_s3dw_ani_check(f))
		{
			_s3dw_ani_finish(f,i);
			i--; /* a new object is here now, take care in the next iteration */
		} else {
			_s3dw_ani_doit(f);
		}
	}
}

