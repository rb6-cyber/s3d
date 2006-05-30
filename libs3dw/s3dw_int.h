/*
 * s3dw_int.h
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

#include <s3dlib.h> /* dprintf() */
#define MAXANI		16
#define ZOOMS		5
extern	struct s3dw_object  **psurf;
extern	int					  nsurf;

/* object.c */
struct s3dw_object *s3dw_object_new();
void s3dw_object_destroy(struct s3dw_object *object);
void s3dw_object_event_click(struct s3dw_object *object, unsigned long oid);
/* surface.c */
void s3dw_surface_event_click(struct s3dw_surface *surface, unsigned long oid);
void s3dw_surface_append_obj(struct s3dw_surface *surface, struct s3dw_object *object);
void s3dw_surface_event_click(struct s3dw_surface *surface, unsigned long oid);
/* button.c */
void s3dw_button_destroy(struct s3dw_button *button);
void s3dw_button_event_click(struct s3dw_button *button, unsigned long oid);
/* label.c */
void s3dw_label_destroy(struct s3dw_label *label);
void s3dw_label_event_click(struct s3dw_label *label, unsigned long oid);
/* input.c */
void s3dw_input_destroy(struct s3dw_input *input);
void s3dw_input_event_click(struct s3dw_input *input, unsigned long oid);

/* style.c */
extern struct s3dw_style def_style;
/* animate.c */
int  _s3dw_ani_onstack(struct s3dw_object *f);
void _s3dw_ani_add(struct s3dw_object *f);
void _s3dw_ani_del(int i);
void _s3dw_ani_doit(struct s3dw_object *f);
void _s3dw_ani_finish(struct s3dw_object *f, int i);
void _s3dw_ani_iterate(struct s3dw_object *f);
int  _s3dw_ani_check(struct s3dw_object *f);
