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
#include <s3dw_int.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* strdup() */

/* draw and setup the button */
void s3dw_button_draw(struct s3dw_widget *widget)
{
	struct s3dw_button *button=widget->data.button;
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

	button->_oid_text=s3d_draw_string(button->_text,&length);
	s3d_pep_materials_a(button->_oid_text,widget->_surface->_style->text_mat,1);

	/* width of the button depends on the length of the text */
	vertices[0*3+0]=0.0;			vertices[0*3+1]=0.0;		vertices[0*3+2]=0.0;	
	vertices[1*3+0]=0.0;			vertices[1*3+1]=-2.0;		vertices[1*3+2]=0.0;	
	vertices[2*3+0]=length+1;		vertices[2*3+1]=-2.0;		vertices[2*3+2]=0.0;	
	vertices[3*3+0]=length+1;		vertices[3*3+1]=0.0;		vertices[3*3+2]=0.0;	
	vertices[4*3+0]=0.25;			vertices[4*3+1]=-0.25;		vertices[4*3+2]=0.25;	
	vertices[5*3+0]=0.25;			vertices[5*3+1]=-1.75;		vertices[5*3+2]=0.25;	
	vertices[6*3+0]=length+0.75;	vertices[6*3+1]=-1.75;		vertices[6*3+2]=0.25;	
	vertices[7*3+0]=length+0.75;	vertices[7*3+1]=-0.25;		vertices[7*3+2]=0.25;	
	button->_oid_box=s3d_new_widget();
	s3d_push_materials_a(button->_oid_box,widget->_surface->_style->input_mat,1);
	s3d_push_vertices   (button->_oid_box,vertices,8);
	s3d_push_polygons   (button->_oid_box,polygons,10);
	s3d_link(		   button->_oid_box,widget->_surface->oid);
	s3d_link(		   button->_oid_text,button->_oid_box);
	s3d_translate(button->_oid_text,0.5,-1.5,0.30);
	s3d_translate(button->_oid_box,widget->_x,-widget->_y,0);
	widget->_o=&(button->_oid_box);
	widget->_width=length+1;
	widget->_height=2;
    s3d_flags_on(button->_oid_box,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(button->_oid_text,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

}

/* create a new button in the surface */
struct s3dw_widget *s3dw_button_new(struct s3dw_surface *surface, char *text, float posx, float posy)
{
	struct s3dw_button *button;
	struct s3dw_widget *widget;
	button=(struct s3dw_button *)malloc(sizeof(struct s3dw_button));
	widget=s3dw_widget_new();
	widget->type=S3DW_TBUTTON;
	/* draw button */
	button->_text=strdup(text);
	button->onclick=NULL;
	widget->_x=posx;
	widget->_y=posy;
	widget->data.button=button;
	s3dw_button_draw(widget);
	s3dw_surface_append_obj(surface, widget);
	return(widget);
}
void s3dw_button_erase(struct s3dw_button *button)
{
	s3d_del_widget(button->_oid_text);
	s3d_del_widget(button->_oid_box);

}

/* destroy the button */
void s3dw_button_destroy(struct s3dw_button *button)
{
	s3dw_button_erase(button);
	free(button->_text);
	free(button);
}


void s3dw_button_event_click(struct s3dw_widget *widget, unsigned long oid)
{
	if ((widget->data.button->_oid_text==oid) || (widget->data.button->_oid_box==oid))
	{
		if (widget->data.button->onclick!=NULL)
			widget->data.button->onclick(widget);
	}
	
}
