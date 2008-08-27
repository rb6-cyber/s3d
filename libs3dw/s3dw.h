/*
 * s3dw.h
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

#ifndef LIBS3DW_H
#define LIBS3DW_H
#ifndef LIBS3D_H
#include <s3d.h>
#endif
#include <stdint.h>  /* [u]intXX_t type definitions*/

#ifdef HAVE_GCCVISIBILITY
#define S3DWEXPORT __attribute__ ((visibility("default")))
#else
#define S3DWEXPORT
#endif

/* we want this widget visible, as long as the widgets below are also visible.
 * on for all widgets, except surfaces which have to be switched visible
 * with s3dw_show() */
#define  S3DW_VISIBLE 1
/* widget should accept input. that's on by default. */
#define  S3DW_ACTIVE  2
/* tells us if the widget is currently displayed */
#define  S3DW_ONSCREEN 256
/* if the surface (or widget) is already properly arranged */
#define  S3DW_ARRANGED 512
/* follow the camera */
#define  S3DW_FOLLOW_CAM 1024
/* turn to the camera */
#define  S3DW_TURN_CAM 2048
/* just a typecaster to beatify code. use it if you like */
#define  S3DWIDGET(x) ((s3dw_widget *)x)

enum {
	S3DW_TROOT,
	S3DW_TCAM,
	S3DW_TSURFACE,
	S3DW_TBUTTON,
	S3DW_TLABEL,
	S3DW_TINPUT,
	S3DW_TTEXTBOX,
	S3DW_TSCROLLBAR,
	S3DW_NTYPES
};
typedef struct _s3dw_widget  s3dw_widget;
typedef struct _s3dw_button  s3dw_button;
typedef struct _s3dw_label   s3dw_label;
typedef struct _s3dw_textbox   s3dw_textbox;
typedef struct _s3dw_scrollbar  s3dw_scrollbar;
typedef struct _s3dw_input   s3dw_input;
typedef struct _s3dw_surface   s3dw_surface;
typedef struct _s3dw_style   s3dw_style;
typedef void (*s3dw_callback)(s3dw_widget *);


struct _s3dw_widget {
	/* private .. */
	int      type;
	s3dw_widget *parent;
	s3dw_style  *style;
	int       nobj; /* number of children objects */
	s3dw_widget    **pobj; /* pointer to list of children objects */
	int    focus;   /* index of the widget focused in pobj */
	int      flags;   /* flags like visibility */
	float    ax, ay, az;  /* current position for animation */
	float    as;   /* current scale factor */
	float    arx, ary, arz; /* current rotation */
	float    width, height; /* width and height of the widget, outer size */
	uint32_t  oid;   /* the main object which is used for transformations etc ...*/
	/* public */
	void  *ptr;   /* a pointer to a user structure, to use in callbacks etc */
	float    x, y, z; /* position, relative to the surface usually */
	float    s;    /* scale factor */
	float    rx, ry, rz;  /* rotation around the axis */
};


struct _s3dw_button {
	/* private */
	s3dw_widget   widget;
	char    *text;
	uint32_t      oid_text;
	/* public */
	s3dw_callback   onclick;
};
struct _s3dw_label {
	/* private */
	s3dw_widget   widget;
	char    *text;
	/* public */
	s3dw_callback   onclick;

};
struct _s3dw_scrollbar {
	/* private */
	s3dw_widget   widget;
	float    pos, max;
	int     type; /* 0 = horizontal, 1 = vertical */
	int     loid, roid, baroid;
	/* public */
	s3dw_callback   lonclick;
	s3dw_callback   ronclick;

};

struct _s3dw_textbox {
	/* private */
	s3dw_widget   widget;
	s3dw_scrollbar *scroll_vertical,
	*scroll_horizontal;
	char    *text;
	int    n_lineoids, *p_lineoids;
	int    window_x, window_y;
	/* public */
	s3dw_callback   onclick;

};

struct _s3dw_input {
	/* private */
	s3dw_widget   widget;
	char    *text;
	uint32_t     oid_text;
	/* public */
	s3dw_callback   onclick;
	s3dw_callback   onedit;
};

struct _s3dw_surface {
	/* private */
	s3dw_widget     widget;
	uint32_t     oid_title;
	uint32_t     oid_tbar;
	char     *title;
};

/* style */
struct _s3dw_style {
	char *name;     /* name of the style ... kind of redundant */
	char *fontface;    /* font face for all used fonts */
	float surface_mat[12];  /* material for the surface background */
	float input_mat[12];  /* material for buttonboxes and other widgets */
	float inputback_mat[12]; /* material for inputfield background */
	float text_mat[12];   /* material for the text on buttons and inputs */
	float title_mat[12];  /* material for the title bar */
	float title_text_mat[12]; /* material for the text on the title bar */
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

	/* button.c */
	S3DWEXPORT s3dw_button   *s3dw_button_new(const s3dw_surface *surface, const char *text, float posx, float posy);
	S3DWEXPORT s3dw_label    *s3dw_label_new(const s3dw_surface *surface, const char *text, float posx, float posy);
	S3DWEXPORT s3dw_input    *s3dw_input_new(const s3dw_surface *surface, float width, float posx, float posy);
	S3DWEXPORT s3dw_textbox   *s3dw_textbox_new(const s3dw_surface *surface, const char *text, float posx, float posy, float width, float height);
	S3DWEXPORT char     *s3dw_input_gettext(s3dw_input *input);
	S3DWEXPORT void      s3dw_input_change_text(s3dw_input *input, const char *text);
	S3DWEXPORT void      s3dw_label_change_text(s3dw_label *label, const char *text);
	S3DWEXPORT s3dw_surface   *s3dw_surface_new(const char *title, float width, float height);

	S3DWEXPORT s3dw_widget   *s3dw_getroot(void);
	S3DWEXPORT void     s3dw_moveit(s3dw_widget *widget);
	S3DWEXPORT void      s3dw_delete(s3dw_widget *widget);
	S3DWEXPORT void      s3dw_show(s3dw_widget *widget);
	S3DWEXPORT void      s3dw_focus(s3dw_widget *focus);

	S3DWEXPORT void      s3dw_textbox_scrollup(s3dw_textbox *textbox);
	S3DWEXPORT void      s3dw_textbox_scrolldown(s3dw_textbox *textbox);
	S3DWEXPORT void      s3dw_textbox_scrollleft(s3dw_textbox *textbox);
	S3DWEXPORT void      s3dw_textbox_scrollright(s3dw_textbox *textbox);
	S3DWEXPORT void      s3dw_textbox_scrollto(s3dw_textbox *textbox, int x, int y);
	S3DWEXPORT void      s3dw_textbox_change_text(s3dw_textbox *textbox, const char *text);

	S3DWEXPORT int      s3dw_handle_click(const struct s3d_evt *event);
	S3DWEXPORT int      s3dw_handle_key(const struct s3d_evt *event);
	S3DWEXPORT int      s3dw_object_info(struct s3d_evt *event);

	S3DWEXPORT void      s3dw_ani_mate(void);

#if defined(__cplusplus) || defined(c_plusplus)
} /* extern "C" */
#endif

#endif
