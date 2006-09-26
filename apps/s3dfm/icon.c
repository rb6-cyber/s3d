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
/* draws icon dir, which is number i (used for nice coloring) */
int icon_draw(t_node *dir)
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
	/* find position for the new block in our directory box */
	/* create the block */
	dir->oid=s3d_new_object();
	s3d_push_vertices(dir->oid,vertices,8);
	switch (dir->type)
	{
		case T_FOLDER:
			s3d_push_material(dir->oid,
									0.4,0.4,0,
									0.4,0.4,0,
									0.4,0.4,0);
			break;
		default:
			s3d_push_material(dir->oid,
									0,0,0.5,
									0,0,0.5,
									0,0,0.5);
	};
	s3d_push_polygons(dir->oid,polys,10);

	/* draw and position the string */
	if (dir->objs.str==-1)
	{
		dir->objs.str=s3d_draw_string(dir->name,&len);
		if (len<2) len=2;
		dir->len=len;
	}
	else 
		len=dir->len;
	s3d_scale(dir->objs.str,(float)1.8/len);
	s3d_translate(dir->objs.str,-0.9,-0.3,0.1);
	s3d_rotate(dir->objs.str,0,0,0);
	s3d_link(dir->objs.str,dir->oid);
	dir->disp=D_ICON;
	return(0);
}
int icon_undisplay(t_node *dir)
{
	if (dir->oid!=-1)
	{
		s3d_del_object(dir->oid);
		dir->oid=-1;
	}
	if (dir->objs.str!=-1)
	{
		s3d_del_object(dir->objs.str);
		dir->objs.str=-1;
	}
	dir->disp=0;
	return(0);
}
