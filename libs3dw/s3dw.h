/*
 * s3dw.h
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



#define		S3DW_VISIBLE	1
#define 	S3DW_CLICKABLE	2

enum {
	S3DW_TSURFACE,
	S3DW_TBUTTON,
	S3DW_TLABEL,
	S3DW_TINPUT
};

struct s3dw_widget {
	int type;
	int   _flags;
	struct s3dw_surface *_surface;
	union {
		struct s3dw_label 	*label;
		struct s3dw_button  *button;
		struct s3dw_input   *input;
		struct s3dw_surface *surface;
	} data;

	float _x,_y,_z,_s,_rx,_ry,_rz;
	float _dx,_dy,_dz,_ds,_drx,_dry,_drz;
	float _width,_height;
	unsigned long *_o;
};

typedef void (*s3dw_callback)(struct s3dw_widget *);

struct s3dw_button {
	char *_text;
	s3dw_callback onclick;
	unsigned long   _oid_text;
	unsigned long   _oid_box;
	
};
struct s3dw_label {
	char *_text;
	s3dw_callback onclick;
	unsigned long   _oid_text;
	
};
struct s3dw_input {
	char *_text;
	s3dw_callback onclick;
	s3dw_callback onedit;
	unsigned long   _oid_text;
	unsigned long   _oid_box;
	
};

struct s3dw_surface {
	unsigned long		  oid;
	unsigned long		  _oid_title;
	unsigned long		  _oid_tbar;
	int 				  _nobj;
	char				 *title;
	struct s3dw_widget 	**_pobj;
	struct s3dw_style 	 *_style;
};

/* style */
struct s3dw_style {
	char *name;
	float surface_mat[12];
	float input_mat[12];
	float text_mat[12];
	float title_mat[12];
	float title_text_mat[12];
};
/* button.c */
struct s3dw_widget *s3dw_button_new(struct s3dw_surface *surface, char *text, float posx, float posy);
/* label.c */
struct s3dw_widget *s3dw_label_new(struct s3dw_surface *surface, char *text, float posx, float posy);
/* input.c */
struct s3dw_widget *s3dw_input_new(struct s3dw_surface *surface, char *text, float posx, float posy);
/* surface.c */
struct s3dw_widget *s3dw_surface_new(char *title, float width, float height);
void s3dw_surface_delete(struct s3dw_surface *surface);
/* widget.c */
void s3dw_widget_destroy(struct s3dw_widget *widget);
/* event.c */
void s3dw_click_event(struct s3d_evt *evt);
/* animate.c */
void s3dw_ani_mate();
