/*
 * label.c
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

void s3dw_label_draw(struct s3dw_widget *widget)
{
	struct s3dw_label *label=widget->data.label;
	float length;
	label->_oid_text=s3d_draw_string(label->_text,&length);
	s3d_pep_materials_a(label->_oid_text,widget->_surface->_style->text_mat,1);
	s3d_link(label->_oid_text,widget->_surface->oid);
	s3d_translate(label->_oid_text,widget->_x,-widget->_y,0.1);
	widget->_width=length+1;
	widget->_height=2;

    s3d_flags_on(label->_oid_text,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

}
/* create a new label in the surface */
struct s3dw_widget *s3dw_label_new(struct s3dw_surface *surface, char *text, float posx, float posy)
{
	struct s3dw_label *label;
	struct s3dw_widget *widget;
	label=(struct s3dw_label *)malloc(sizeof(struct s3dw_label));
	widget=s3dw_widget_new();
	widget->type=S3DW_TBUTTON;
	/* draw label */
	label->_text=strdup(text);
	label->onclick=NULL;
	widget->_x=posx;
	widget->_y=posy;
	widget->data.label=label;
	widget->_o=&(label->_oid_text);
	s3dw_label_draw(widget);
	s3dw_surface_append_obj(surface, widget);
	return(widget);
}

void s3dw_label_erase(struct s3dw_label *label)
{
	s3d_del_widget(label->_oid_text);
}
/* destroy the label */
void s3dw_label_destroy(struct s3dw_label *label)
{
	s3dw_label_erase(label);
	free(label->_text);
	free(label);
}


void s3dw_label_event_click(struct s3dw_widget *widget, unsigned long oid)
{
	if (widget->data.label->_oid_text==oid)
	{
		if (widget->data.label->onclick!=NULL)
			widget->data.label->onclick(widget);
	}
	
}
