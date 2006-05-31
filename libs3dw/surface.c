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
#include <string.h> /* strdup() */

struct s3dw_object **psurf=NULL;
int					  nsurf=0;

void s3dw_surface_draw(struct s3dw_object *object)
{
	struct s3dw_surface *surface=object->data.surface;
	int textlen;
	float length;
	float vertices[8*3]={
		0,0,0,
		1,0,0,
		1,1,0,
		0,1,0,
		0,0,1,
		1,0,1,
		1,1,1,
		0,1,1
	};
	float sver[8*3], tver[8*3];
	unsigned long polygon[10*4]={
		0,1,2,0,
		0,2,3,0,
		1,5,6,0,
		1,6,2,0,
		2,6,7,0,
		2,7,3,0,
		4,0,3,0,
		4,3,7,0,
		5,4,7,0,
		5,7,6,0
	};
	unsigned long tpol[10*4];
	int i;

	surface->oid=s3d_new_object();
	surface->_oid_tbar=s3d_new_object();
	s3d_select_font("vera");
	surface->_oid_title=s3d_draw_string(surface->title,&length);
	while (length > (object->_width+1))
	{
		dprintf(HIGH,"%f > %f",length,object->_width+1);
		textlen=strlen(surface->title);
		if (length>((object->_width+1)*1.3))
			textlen=textlen*((object->_width+1)*1.1/length);
		if (textlen>4)
		{
			surface->title[textlen-2]=0;
			surface->title[textlen-3]='.';
			surface->title[textlen-4]='.';
			s3d_del_object(surface->_oid_title);
			surface->_oid_title=s3d_draw_string(surface->title,&length);
		} else {
			break;
		}
	}
 	/* prepare vertices */
	for (i=0;i<8;i++)
	{
		sver[i*3 + 0]=vertices[i*3+0] * object->_width;
		sver[i*3 + 1]=vertices[i*3+1] * -object->_height;
		sver[i*3 + 2]=vertices[i*3+2] * -1;
		tver[i*3 + 0]=vertices[i*3+0] * object->_width;
		tver[i*3 + 1]=vertices[i*3+1];
		tver[i*3 + 2]=vertices[i*3+2] * -1;
	}
	/* swap */
	for (i=0;i<10;i++)
	{
	   tpol[i*4 + 0]=polygon[i*4 + 1];
	   tpol[i*4 + 1]=polygon[i*4 + 0];
	   tpol[i*4 + 2]=polygon[i*4 + 2];
	   tpol[i*4 + 3]=polygon[i*4 + 3];
	}
	object->_o=&(surface->oid);
	s3d_push_vertices(surface->oid,sver,8);
	s3d_push_vertices(surface->_oid_tbar,tver,8);
	s3d_push_materials_a(surface->oid      ,surface->_style->surface_mat,1);
	s3d_push_materials_a(surface->_oid_tbar,surface->_style->title_mat,1);
	s3d_pep_materials_a(surface->_oid_title,surface->_style->title_text_mat,1);
	s3d_push_polygons(surface->oid,polygon,10);
	s3d_push_polygons(surface->_oid_tbar,tpol,10);
	s3d_link(surface->_oid_tbar,surface->oid);
	s3d_link(surface->_oid_title,surface->_oid_tbar);
	s3d_translate(surface->_oid_title,0.5,0.2,0.1);
    s3d_flags_on(surface->oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(surface->_oid_title,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(surface->_oid_tbar,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);


}
/* create a new surface */
struct s3dw_object *s3dw_surface_new(char *title, float width, float height)
{
	struct s3dw_surface *surface;
	struct s3dw_object  *object;
			
	surface=(struct s3dw_surface *)malloc(sizeof(struct s3dw_surface));
	surface->_nobj=0;
	surface->_pobj=NULL;
	surface->_style=&def_style;
	surface->title=strdup(title);

	nsurf++;
	psurf=realloc(psurf,sizeof(struct s3dw_object **)*nsurf);
	object=s3dw_object_new();
	object->type=S3DW_TSURFACE;
	object->data.surface=surface;
	object->_width=width;
	object->_height=height;
	psurf[nsurf-1]=object;
	s3dw_surface_draw(object);
	return(object);
}
void s3dw_surface_erase(struct s3dw_surface *surface)
{
	s3d_del_object(surface->oid);
	s3d_del_object(surface->_oid_tbar);
	s3d_del_object(surface->_oid_title);
}
/* destroy the surface */
void s3dw_surface_destroy(struct s3dw_surface *surface)
{
	int i;
	if (surface->_nobj>0)
	{
		for (i=0;i<surface->_nobj;i++)
		{
			s3dw_object_destroy(surface->_pobj[i]);
		}
		free(surface->_pobj);
	}

	s3dw_surface_erase(surface);
	free(surface->title);
	free(surface);
}
/* properly delete, take care of the carrying structure ... */
void s3dw_surface_delete(struct s3dw_surface *surface)
{
	int i;
	for (i=0;i<nsurf;i++) /* search ... */
		if (psurf[i]->data.surface==surface) /* ... and destroy */
		{
			s3dw_surface_erase(surface);
			free(psurf[i]);
			psurf[i]=psurf[nsurf-1]; /* swap last element to the to be deleted one */
			nsurf--;
			break;
		}
}
/* append an object */
void s3dw_surface_append_obj(struct s3dw_surface *surface, struct s3dw_object *object)
{
	surface->_nobj++;
	surface->_pobj=realloc(surface->_pobj,sizeof(struct s3dw_object *)*surface->_nobj);
	surface->_pobj[surface->_nobj-1]=object;
	object->_surface=surface;
}
/* test objects of the surface for clicks */
void s3dw_surface_event_click(struct s3dw_object *object, unsigned long oid)
{
	int i;
	if (object->data.surface->oid==oid)
	{
		dprintf(MED,"body %s clicked",object->data.surface->title);
	}
	if ((object->data.surface->_oid_tbar==oid) || (object->data.surface->_oid_title==oid))
	{
		dprintf(MED,"title %s clicked",object->data.surface->title);
	}
	for (i=0;i<object->data.surface->_nobj;i++)
		s3dw_object_event_click(object->data.surface->_pobj[i],oid);
}

