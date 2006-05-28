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
	struct s3dw_object *_object;
	int   _flags;
	void (*onclick)(struct s3dw_button *);
	unsigned long   _oid_text;
	unsigned long   _oid_box;
	
};
struct s3dw_input {
	char *_text;
	int   _flags;
	struct s3dw_object *_object;
};

struct s3dw_object {
	int type;
	float x,y,z,s,rx,ry,rz;
	float dx,dy,dz,ds,drx,dry,drz;
	float width,height;
	unsigned long *o;
	struct s3dw_surface *_surface;
	union {
		struct s3dw_button  *button;
		struct s3dw_input   *input;
		struct s3dw_surface *surface;
	} data;
};

struct s3dw_surface {
	unsigned long		  oid;
	unsigned long		  _oid_title;
	unsigned long		  _oid_tbar;
	int 				  _flags;
	int 				  _nobj;
	char				 *title;
	struct s3dw_object   *_object;
	struct s3dw_object 	**_pobj;
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
struct s3dw_button *s3dw_button_new(struct s3dw_surface *surface, char *text, float posx, float posy);
/* surface.c */
struct s3dw_surface *s3dw_surface_new(char *title, float width, float height);
void s3dw_surface_delete(struct s3dw_surface *surface);
/* event.c */
void s3dw_click_event(struct s3d_evt *evt);
/* animate.c */
void s3dw_ani_mate();
