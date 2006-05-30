/*
 * object.c
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
#include <string.h> /* strdup() */

struct s3dw_object *s3dw_object_new()
{
	struct s3dw_object *object=malloc(sizeof(struct s3dw_object));
	object->type=-1;
	object->x=object->dx=0;
	object->y=object->dy=0;
	object->z=object->dz=0;
	object->rx=object->drx=0;
	object->ry=object->dry=0;
	object->rz=object->drz=0;
	object->s=object->ds=1;
	object->o=NULL;
	object->width=0;
	object->height=0;
	object->_surface=NULL;
	object->data.surface=NULL;
	return(object);
}
void s3dw_object_destroy(struct s3dw_object *object)
{
	switch (object->type)
		{
			case S3DW_TBUTTON:
					s3dw_button_destroy(object->data.button);
					break;
			default:
					dprintf(MED,"can't free this type (yet) - memory leak\n");
		}
	free(object);
}
void s3dw_object_event_click(struct s3dw_object *object, unsigned long oid)
{
	switch (object->type)
		{
			case S3DW_TBUTTON:
					s3dw_button_event_click(object->data.button,oid);
					break;
			case S3DW_TLABEL:
					s3dw_label_event_click(object->data.label,oid);
					break;
			case S3DW_TINPUT:
					s3dw_input_event_click(object->data.input,oid);
					break;

		}

}
