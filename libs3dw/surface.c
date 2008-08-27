/*
 * surface.c
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
#include <math.h> /* cos(), sin() */
#define POPUPDIST 20

void s3dw_surface_draw(s3dw_widget *widget)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	int textlen;
	float length;
	float vertices[8*3] = {
		0, 0, 0,
		1, 0, 0,
		1, 1, 0,
		0, 1, 0,
		0, 0, 1,
		1, 0, 1,
		1, 1, 1,
		0, 1, 1
	};
	float sver[8*3], tver[8*3];
	uint32_t polygon[10*4] = {
		0, 1, 2, 0,
		0, 2, 3, 0,
		1, 5, 6, 0,
		1, 6, 2, 0,
		2, 6, 7, 0,
		2, 7, 3, 0,
		4, 0, 3, 0,
		4, 3, 7, 0,
		5, 4, 7, 0,
		5, 7, 6, 0
	};
	uint32_t tpol[10*4];
	int i;

	widget->oid = s3d_new_object();
	surface->oid_tbar = s3d_new_object();
	s3d_select_font("vera");
	surface->oid_title = s3d_draw_string(surface->title, &length);
	while (length > (widget->width + 1)) {
		s3dprintf(HIGH, "%f > %f", length, widget->width + 1);
		textlen = strlen(surface->title);
		if (length > ((widget->width + 1)*1.3))
			textlen = textlen * ((widget->width + 1) * 1.1 / length);
		if (textlen > 4) {
			surface->title[textlen-2] = 0;
			surface->title[textlen-3] = '.';
			surface->title[textlen-4] = '.';
			s3d_del_object(surface->oid_title);
			surface->oid_title = s3d_draw_string(surface->title, &length);
		} else {
			break;
		}
	}
	/* prepare vertices */
	for (i = 0;i < 8;i++) {
		sver[i*3 + 0] = vertices[i*3+0] * widget->width;
		sver[i*3 + 1] = vertices[i*3+1] * -widget->height;
		sver[i*3 + 2] = vertices[i*3+2] * -1;
		tver[i*3 + 0] = vertices[i*3+0] * widget->width;
		tver[i*3 + 1] = vertices[i*3+1];
		tver[i*3 + 2] = vertices[i*3+2] * -1;
	}
	/* swap */
	for (i = 0;i < 10;i++) {
		tpol[i*4 + 0] = polygon[i*4 + 1];
		tpol[i*4 + 1] = polygon[i*4 + 0];
		tpol[i*4 + 2] = polygon[i*4 + 2];
		tpol[i*4 + 3] = polygon[i*4 + 3];
	}
	s3d_push_vertices(widget->oid, sver, 8);
	s3d_push_vertices(surface->oid_tbar, tver, 8);
	s3d_push_materials_a(widget->oid      , widget->style->surface_mat, 1);
	s3d_push_materials_a(surface->oid_tbar, widget->style->title_mat, 1);
	s3d_pep_materials_a(surface->oid_title, widget->style->title_text_mat, 1);
	s3d_push_polygons(widget->oid, polygon, 10);
	s3d_push_polygons(surface->oid_tbar, tpol, 10);
	s3d_link(surface->oid_tbar, widget->oid);
	s3d_link(surface->oid_title, surface->oid_tbar);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_translate(surface->oid_title, 0.5, 0.2, 0.1);
	s3d_scale(widget->oid, widget->as);
	s3d_translate(widget->oid, widget->ax, widget->ay, widget->az);
}
/* show the surface */
void s3dw_surface_show(s3dw_widget *widget)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(surface->oid_title, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(surface->oid_tbar, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* hides the surface */
void s3dw_surface_hide(s3dw_widget *widget)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(surface->oid_title, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(surface->oid_tbar, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* create a new surface */
s3dw_surface *s3dw_surface_new(const char *title, float width, float height)
{
	s3dw_surface *surface;
	s3dw_widget  *widget;
	float f1[3];

	surface = (s3dw_surface *)malloc(sizeof(s3dw_surface));
	surface->title = strdup(title);
	widget = s3dw_widget_new((s3dw_widget *)surface);
	widget->type = S3DW_TSURFACE;
	widget->width = width;
	widget->height = height;
	widget->as = 0.01;
	s3dw_arr_widgetcenter(widget, f1);
	s3dw_widget_append(s3dw_getroot(), widget);
	widget->x = -f1[0] + _s3dw_cam->x - sin(_s3dw_cam->ry * M_PI / 180) *  cos(_s3dw_cam->rx * M_PI / 180) * POPUPDIST;
	widget->y = -f1[1] + _s3dw_cam->y +             sin(_s3dw_cam->rx * M_PI / 180) * POPUPDIST;
	widget->z = -f1[2] + _s3dw_cam->z - cos(_s3dw_cam->ry * M_PI / 180) *  cos(_s3dw_cam->rx * M_PI / 180) * POPUPDIST;
	widget->ax = widget->x;
	widget->ay = widget->y;
	widget->az = widget->z;
	widget->flags |= S3DW_FOLLOW_CAM | S3DW_TURN_CAM;
	s3dw_surface_draw(widget);
	s3dw_ani_needarr();
	s3dw_ani_add(widget);
	return(surface);
}
/* delete objects in the s3d context */
void s3dw_surface_erase(s3dw_widget *widget)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	s3d_del_object(widget->oid);
	s3d_del_object(surface->oid_tbar);
	s3d_del_object(surface->oid_title);
}
/* destroy the surface */
void s3dw_surface_destroy(s3dw_widget *widget)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	s3dw_surface_erase(widget);
	free(surface->title);
	free(surface);
}
/* handle key events */
int s3dw_surface_event_key(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(keys))
{
	return(0);
}
/* test widgets of the surface for clicks */
int s3dw_surface_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_surface *surface = (s3dw_surface *)widget;
	if (widget->oid == oid) {
		s3dw_focus(widget);
		s3dprintf(MED, "body %s clicked", surface->title);
		return(1);
	}
	if ((surface->oid_tbar == oid) || (surface->oid_title == oid)) {
		s3dw_focus(widget);
		s3dprintf(MED, "title %s clicked", surface->title);
		return(1);
	}
	return(0);
}

