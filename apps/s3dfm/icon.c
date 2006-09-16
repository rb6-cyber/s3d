/*
 * icon.c
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
#include <stdio.h> 	 /*  printf() */
#include <math.h>	 /*  sin(),cos() */
#include <string.h>  /*  strlen() */
/* draws icon i in the block of dir */
int icon_draw(t_item *dir,int i)
{
	float vertices[]={	-1,-0.5,0,
						-1, 0.5,0,
						 1, 0.5,0,
						 1,-0.5,0,
						-1,-0.5,-1,
						-1, 0.5,-1,
						 1, 0.5,-1,
						 1,-0.5,-1};
	unsigned long polys[]={
				1,3,0,0,				2,3,1,0,
				5,6,2,0,				1,5,2,0,
				2,6,7,0,				2,7,3,0,
				0,3,7,0,				0,7,4,0,
				5,1,0,0,				5,0,4,0	
				};
	float len;
	float d;
	int dps;
	printf("icon_draw( %s )\n",dir->list[i].name);
	dps=ceil(sqrt(dir->n_item)); /* directories per line */
	/* find position for the new block in our directory box */
	dir->list[i].dpx = dir->list[i].px=-1 +2*  ((float)((int)i%dps)+0.5)/((float)dps);
	dir->list[i].dpy = dir->list[i].py=0.5+((float)((int)i/dps)+0.5)/((float)dps)-0.5;
	dir->list[i].dpz = dir->list[i].pz=1.0;
	dir->list[i].scale = dir->list[i].dscale = (float)1.0/((float)dps);
	/* create the block */
	box_dissolve(&(dir->list[i]));
	dir->list[i].block=s3d_new_object();
	s3d_push_vertices(dir->list[i].block,vertices,8);
	d=((int)(((i+(dps+1)%2*(i/dps)))%2))*0.2;
	switch (dir->list[i].type)
	{
		case T_FOLDER:
			s3d_push_material(dir->list[i].block,
									0.4-d,0.4-d,0,
									0.4-d,0.4-d,0,
									0.4-d,0.4-d,0);
			break;
		default:
			s3d_push_material(dir->list[i].block,
									0,0,0.5-d,
									0,0,0.5-d,
									0,0,0.5-d);
	};
	s3d_push_polygons(dir->list[i].block,polys,10);
	s3d_link(dir->list[i].block,dir->block);

	/* draw and position the string */
	if (dir->list[i].str==-1)
	{
		dir->list[i].str=s3d_draw_string(dir->list[i].name,&len);
		if (len<2) len=2;
		dir->list[i].len=len;
	}
	else 
		len=dir->list[i].len;
	s3d_scale(dir->list[i].str,(float)1.8/len);
	s3d_translate(dir->list[i].str,-0.9,-0.3,0.1);
	s3d_rotate(dir->list[i].str,0,0,0);
	s3d_link(dir->list[i].str,dir->list[i].block);
	ani_finish(&dir->list[i],-1); /* apply transformation */
	dir->list[i].disp=D_ICON;
	return(0);
}
int icon_undisplay(t_item *dir)
{
	printf("icon_undisplay( %s )\n",dir->name);
	if (dir->block!=-1)
	{
		s3d_del_object(dir->block);
		dir->block=-1;
	}
	if (dir->str!=-1)
	{
		s3d_del_object(dir->str);
		dir->str=-1;
	}
	dir->disp=0;
	return(0);
}
