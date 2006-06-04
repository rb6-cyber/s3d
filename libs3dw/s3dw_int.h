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
#define MAXANI		128
#define ZOOMS		5
/* constructor and handler callbacks */
typedef int (*s3dw_click_callback)(s3dw_widget *, unsigned long);
typedef int (*s3dw_key_callback)(  s3dw_widget *, struct s3d_key_event *);
s3dw_callback 		s3dwcb_show[S3DW_NTYPES];
s3dw_callback 		s3dwcb_hide[S3DW_NTYPES];
s3dw_callback 		s3dwcb_destroy[S3DW_NTYPES];
s3dw_click_callback s3dwcb_click[S3DW_NTYPES];
s3dw_key_callback	s3dwcb_key[S3DW_NTYPES];

/* root.c */
s3dw_widget *s3dw_getroot();
void s3dw_nothing(s3dw_widget *widget);
void s3dw_root_destroy(s3dw_widget *widget);
int s3dw_root_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_root_event_key(s3dw_widget *widget, struct s3d_key_event *keys);
/* widget.c */
s3dw_widget *s3dw_widget_new();
void s3dw_widget_append(s3dw_widget *parent, s3dw_widget *widget);
void s3dw_widget_visible(s3dw_widget *widget);
int s3dw_widget_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_widget_event_key(s3dw_widget *widget, struct s3d_key_event *keys);
/* surface.c */
void s3dw_surface_destroy(s3dw_widget *widget);
void s3dw_surface_draw(s3dw_widget *widget);
void s3dw_surface_erase(s3dw_widget *widget);
void s3dw_surface_show(s3dw_widget *widget);
void s3dw_surface_hide(s3dw_widget *widget);
int s3dw_surface_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_surface_event_key(s3dw_widget *widget, struct s3d_key_event *keys);
/* button.c */
void s3dw_button_destroy(s3dw_widget *widget);
void s3dw_button_draw(s3dw_widget *widget);
void s3dw_button_erase(s3dw_widget *widget);
void s3dw_button_show(s3dw_widget *widget);
void s3dw_button_hide(s3dw_widget *widget);
int s3dw_button_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_button_event_key(s3dw_widget *widget, struct s3d_key_event *keys);

/* label.c */
void s3dw_label_destroy(s3dw_widget *widget);
void s3dw_label_draw(s3dw_widget *widget);
void s3dw_label_erase(s3dw_widget *widget);
void s3dw_label_show(s3dw_widget *widget);
void s3dw_label_hide(s3dw_widget *widget);
int s3dw_label_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_label_event_key(s3dw_widget *widget, struct s3d_key_event *keys);

/* input.c */
void s3dw_input_destroy(s3dw_widget *widget);
void s3dw_input_draw(s3dw_widget *widget);
void s3dw_input_erase(s3dw_widget *widget);
void s3dw_input_show(s3dw_widget *widget);
void s3dw_input_hide(s3dw_widget *widget);
unsigned long s3dw_input_draw_string(s3dw_widget *widget);
int s3dw_input_event_click(s3dw_widget *widget, unsigned long oid);
int s3dw_input_event_key(s3dw_widget *widget, struct s3d_key_event *keys);

/* style.c */
extern s3dw_style def_style;
/* animate.c */
int  s3dw_ani_onstack(s3dw_widget *f);
void s3dw_ani_add(s3dw_widget *f);
void s3dw_ani_del(int i);
void s3dw_ani_doit(s3dw_widget *f);
void s3dw_ani_finish(s3dw_widget *f, int i);
void s3dw_ani_iterate(s3dw_widget *f);
int  s3dw_ani_check(s3dw_widget *f);
/* arrange.c */
void s3dw_arrange();
void s3dw_arr_widgetcenter(s3dw_widget *widget, float *center);
void s3dw_arr_normdir(float *dir);
