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
	S3DW_TINPUT
};

struct s3dw_button {
	char *_text;
	int   _flags;
	int   _oid_text;
	int   _oid_box;
};
struct s3dw_input {
	char *_text;
	int   _flags;
};

struct s3dw_object {
	int type;
	float posx,posy,width,height;
	union {
		struct s3dw_button *button;
		struct s3dw_input  *input;
	} object;
};

struct s3dw_surface {
	int 				  _oid;
	int 				  _flags;
	int 				  _nobj;
	struct s3dw_object 	**_pobj;
	struct s3dw_style 	 *_style;
};

/* style */
struct s3dw_style {
	char *name;
	float surface_mat[12];
	float input_mat[12];
	float text_mat[12];
};
/* button.c */
struct s3dw_button *s3dw_button_new(struct s3dw_surface *surface, char *text, int posx, int posy);
void s3dw_button_destroy(struct s3dw_button *button);
/* surface.c */
struct s3dw_surface *s3dw_surface_new();
void s3dw_surface_destroy(struct s3dw_surface *surface);
void s3dw_surface_append_obj(struct s3dw_surface *surface, struct s3dw_object *object);
/* style.c */
extern struct s3dw_style def_style;
