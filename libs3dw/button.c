/*
 * button.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
void s3dw_button_draw(s3dw_widget *widget)
{
	s3dw_button *button = (s3dw_button *)widget;
	float length;
	float vertices[8*3];
	uint32_t polygons[10*4] = {
		0, 4, 5, 0,
		0, 5, 1, 0,
		1, 5, 6, 0,
		1, 6, 2, 0,
		2, 6, 7, 0,
		2, 7, 3, 0,
		3, 7, 4, 0,
		3, 4, 0, 0,
		4, 7, 6, 0,
		4, 6, 5, 0
	};

	button->oid_text = s3d_draw_string(button->text, &length);
	s3d_pep_materials_a(button->oid_text, widget->style->text_mat, 1);

	/* width of the button depends on the length of the text */
	vertices[0*3+0] = 0.0;
	vertices[0*3+1] = 0.0;
	vertices[0*3+2] = 0.0;
	vertices[1*3+0] = 0.0;
	vertices[1*3+1] = -2.0;
	vertices[1*3+2] = 0.0;
	vertices[2*3+0] = length + 1;
	vertices[2*3+1] = -2.0;
	vertices[2*3+2] = 0.0;
	vertices[3*3+0] = length + 1;
	vertices[3*3+1] = 0.0;
	vertices[3*3+2] = 0.0;
	vertices[4*3+0] = 0.25;
	vertices[4*3+1] = -0.25;
	vertices[4*3+2] = 0.25;
	vertices[5*3+0] = 0.25;
	vertices[5*3+1] = -1.75;
	vertices[5*3+2] = 0.25;
	vertices[6*3+0] = length + 0.75;
	vertices[6*3+1] = -1.75;
	vertices[6*3+2] = 0.25;
	vertices[7*3+0] = length + 0.75;
	vertices[7*3+1] = -0.25;
	vertices[7*3+2] = 0.25;
	widget->oid = s3d_new_object();
	s3d_push_materials_a(widget->oid, widget->style->input_mat, 1);
	s3d_push_vertices(widget->oid, vertices, 8);
	s3d_push_polygons(widget->oid, polygons, 10);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_link(button->oid_text, widget->oid);
	s3d_translate(button->oid_text, 0.5, -1.5, 0.30);
	s3d_translate(widget->oid, widget->x, -widget->y, 0);
	widget->width = length + 1;
	widget->height = 2;
}

/* create a new button in the surface */
s3dw_button *s3dw_button_new(const s3dw_surface *surface, const char *text, float posx, float posy)
{
	s3dw_button *button;
	s3dw_widget *widget;
	button = (s3dw_button *)malloc(sizeof(s3dw_button));
	button->text = strdup(text);
	button->onclick = s3dw_nothing;
	widget = s3dw_widget_new((s3dw_widget *)button);
	widget->type = S3DW_TBUTTON;
	widget->x = posx;
	widget->y = posy;
	widget->style = ((s3dw_widget *)surface)->style;

	s3dw_widget_append((s3dw_widget *)surface, widget);
	s3dw_button_draw(widget);
	return(button);
}
/* show, make visible */
void s3dw_button_show(s3dw_widget *widget)
{
	s3dw_button *button = (s3dw_button *)widget;
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(button->oid_text, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* hide */
void s3dw_button_hide(s3dw_widget *widget)
{
	s3dw_button *button = (s3dw_button *)widget;
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(button->oid_text, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* destroy s3d structures of the button */
void s3dw_button_erase(s3dw_widget *widget)
{
	s3dw_button *button = (s3dw_button *)widget;
	s3d_del_object(button->oid_text);
	s3d_del_object(widget->oid);
}

/* destroy the button */
void s3dw_button_destroy(s3dw_widget *widget)
{
	s3dw_button *button = (s3dw_button *)widget;
	s3dw_button_erase(widget);
	free(button->text);
	free(button);
}
/* handle key events */
int s3dw_button_event_key(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(keys))
{
	return(0);
}

/* handle click on a button */
int s3dw_button_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_button *button = (s3dw_button *)widget;
	if ((button->oid_text == oid) || (widget->oid == oid)) {
		button->onclick(widget);
		return(1);
	}
	return(0);
}
