/*
 * button.c
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
#include <stdlib.h> /* malloc() */
#include <string.h> /* strdup() */

/* create a new button in the surface */
struct s3dw_button *s3dw_button_new(struct s3dw_surface *surface, char *text, int posx, int posy)
{
	struct s3dw_button *button;
	struct s3dw_object *object;
	float length;
	float vertices[8*3];
	unsigned long polygons[10*4]={
			0,4,5,0,
			0,5,1,0,
			1,5,6,0,
			1,6,2,0,
			2,6,7,0,
			2,7,3,0,
			3,7,4,0,
			3,4,0,0,
			4,7,6,0,
			4,6,5,0
	};
	button=(struct s3dw_button *)malloc(sizeof(struct s3dw_button));
	object=(struct s3dw_object *)malloc(sizeof(struct s3dw_object));
	object->type=S3DW_TBUTTON;
	/* draw button */
	button->_text=strdup(text);
	button->_flags=0;
	s3d_select_font("vera");
	button->_oid_text=s3d_draw_string(button->_text,&length);
	s3d_pep_materials_a(button->_oid_text,surface->_style->text_mat,1);
	button->_oid_box=s3d_new_object();
	/* width of the button depends on the length of the text */
	vertices[0*3+0]=0.0;			vertices[0*3+1]=0.0;		vertices[0*3+2]=0.0;	
	vertices[1*3+0]=0.0;			vertices[1*3+1]=-2.0;		vertices[1*3+2]=0.0;	
	vertices[2*3+0]=length+1;		vertices[2*3+1]=-2.0;		vertices[2*3+2]=0.0;	
	vertices[3*3+0]=length+1;		vertices[3*3+1]=0.0;		vertices[3*3+2]=0.0;	
	vertices[4*3+0]=0.25;			vertices[4*3+1]=-0.25;		vertices[4*3+2]=0.25;	
	vertices[5*3+0]=0.25;			vertices[5*3+1]=-1.75;		vertices[5*3+2]=0.25;	
	vertices[6*3+0]=length+0.75;	vertices[6*3+1]=-1.75;		vertices[6*3+2]=0.25;	
	vertices[7*3+0]=length+0.75;	vertices[7*3+1]=-0.25;		vertices[7*3+2]=0.25;	
	s3d_push_materials_a(button->_oid_box,surface->_style->input_mat,1);
	s3d_push_vertices   (button->_oid_box,vertices,8);
	s3d_push_polygons   (button->_oid_box,polygons,10);
	s3d_link(		   button->_oid_box,surface->_oid);
	s3d_link(		   button->_oid_text,button->_oid_box);
	s3d_translate(button->_oid_text,0.5,-1.5,0.30);
	s3d_translate(button->_oid_box,posx,-posy,0);
	object->posx=posx;
	object->posy=posy;
	object->width=length+1;
	object->height=2;
    s3d_flags_on(button->_oid_box,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(button->_oid_text,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

	s3dw_surface_append_obj(surface, object);
	return(button);
}
/* destroy the button */
void s3dw_button_destroy(struct s3dw_button *button)
{
	s3d_del_object(button->_oid_text);
	s3d_del_object(button->_oid_box);
	free(button->_text);
	free(button);
}


