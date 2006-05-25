/*
 * surface.c
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
#include <stdlib.h> /* malloc() */
/* create a new surface */
struct s3dw_surface *s3dw_surface_new()
{
	struct s3dw_surface *ret;
	ret=(struct s3dw_surface *)malloc(sizeof(struct s3dw_surface));
	ret->_oid=s3d_new_object();
	ret->_flags=0;
	ret->_nobj=0;
	ret->_pobj=NULL;
	ret->_style=&def_style;
	return(ret);
}

/* destroy the surface */
void s3dw_surface_destroy(struct s3dw_surface *surface)
{
	int i;
	s3d_del_object(surface->_oid);
	if (surface->_nobj>0)
	{
		for (i=0;i<surface->_nobj;i++)
		{
			switch (surface->_pobj[i]->type)
			{
				case S3DW_TBUTTON:
						s3dw_button_destroy(surface->_pobj[i]->object.button);
						free(surface->_pobj[i]);
						break;
				default:
						printf("can't free this type (yet) - memory leak\n");
			}
		}
		free(surface->_pobj);
	}
	free(surface);
}
/* append an object */
void s3dw_surface_append_obj(struct s3dw_surface *surface, struct s3dw_object *object)
{
	surface->_nobj++;
	surface->_pobj=realloc(surface->_pobj,sizeof(struct s3dw_object *)*surface->_nobj);
	surface->_pobj[surface->_nobj-1]=object;
}

