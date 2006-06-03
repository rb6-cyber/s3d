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


/* we want this widget visible, as long as the widgets below are also visible. 
 * on for all widgets, except surfaces which have to be switched visible 
 * with s3dw_show() */
#define		S3DW_VISIBLE	1
/* widget should accept input. that's on by default. */
#define 	S3DW_ACTIVE		2
/* tells us if the widget is currently displayed */
#define		S3DW_ONSCREEN	256
/* just a typecaster to beatify code. use it if you like */
#define 	S3DWIDGET(x)	(s3dw_widget *)x

enum {
	S3DW_TROOT,
	S3DW_TSURFACE,
	S3DW_TBUTTON,
	S3DW_TLABEL,
	S3DW_TINPUT,
	S3DW_NTYPES
};
typedef struct _s3dw_widget 	s3dw_widget;
typedef struct _s3dw_button 	s3dw_button;
typedef struct _s3dw_label  	s3dw_label;
typedef struct _s3dw_input  	s3dw_input;
typedef struct _s3dw_surface  	s3dw_surface;
typedef struct _s3dw_style  	s3dw_style;
typedef void (*s3dw_callback)(s3dw_widget *);


struct _s3dw_widget {
	/* it's all private .. */
	int   		 type;
	s3dw_widget *parent; 
	s3dw_style  *style;
	int 				  nobj; /* number of children objects */
	s3dw_widget		 	**pobj; /* pointer to list of children objects */
	int 		 focus;			/* index of the widget focused in pobj */
	int   		 flags;			/* flags like visibility */
	float 		 x,y,z;			/* position, relative to the surface usually */
	float 		 ax,ay,az;		/* current position for animation */
	float 		 s;				/* scale factor */
	float 		 as;			/* current scale factor */
	float 		 rx,ry,rz;		/* rotation around the axis */
	float 		 arx,ary,arz;   /* current rotation */
	float 		 width,height;	/* width and height of the widget, outer size */
	unsigned long oid;			/* the main object which is used for transformations etc ...*/
};


struct _s3dw_button {
	/* private */
	s3dw_widget 	 widget;
	char 			*text;
	unsigned long    oid_text;
	/* public */
	s3dw_callback 	 onclick;
};
struct _s3dw_label {
	/* private */
	s3dw_widget 	 widget;
	char 			*text;
	unsigned long    oid_text;
	/* public */
	s3dw_callback 	 onclick;
	
};
struct _s3dw_input {
	/* private */
	s3dw_widget 	 widget;
	char 			*text;
	unsigned long    oid_text;
	/* public */
	s3dw_callback 	 onclick;
	s3dw_callback 	 onedit;
};

struct _s3dw_surface {
	/* private */
	s3dw_widget 		  widget;
	unsigned long		  oid_title;
	unsigned long		  oid_tbar;
	char				 *title;
};

/* style */
struct _s3dw_style {
	char *name;					/* name of the style ... kind of redundant */
	char *fontface;				/* font face for all used fonts */
	float surface_mat[12];		/* material for the surface background */
	float input_mat[12];		/* material for buttonboxes and other widgets */
	float inputback_mat[12];	/* material for inputfield background */
	float text_mat[12];			/* material for the text on buttons and inputs */
	float title_mat[12];		/* material for the title bar */
	float title_text_mat[12];	/* material for the text on the title bar */
};
/* button.c */
s3dw_button 		*s3dw_button_new(s3dw_surface *surface, char *text, float posx, float posy);
s3dw_label	 		*s3dw_label_new(s3dw_surface *surface, char *text, float posx, float posy);
s3dw_input 			*s3dw_input_new(s3dw_surface *surface, float width, float posx, float posy);
char 				*s3dw_input_gettext(s3dw_input *input);
void 				 s3dw_input_change_text(s3dw_input *input, char *text);
s3dw_surface 		*s3dw_surface_new(char *title, float width, float height);

void s3dw_delete(s3dw_widget *widget);
void s3dw_show(s3dw_widget *widget);
void s3dw_focus(s3dw_widget *focus);

void s3dw_handle_click(struct s3d_evt *evt);
void s3dw_handle_key(struct s3d_evt *evt);

void s3dw_ani_mate();