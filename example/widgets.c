/*
 * widgets.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */




#include <s3d.h>
#include <stdio.h>  /* NULL */
#include <time.h>	/* nanosleep() */
static struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
int i,oid;
#define		S3DW_VISIBLE	1
#define 	S3DW_CLICKABLE	2

struct s3dw_button {
	char *_text;
	int   _flags;
};
struct s3dw_input {
	char *_text;
	int   _flags;
};

struct s3dw_object {
	int type;
	int posx,posy,width,height;
	union {
		struct s3dw_button *button;
		struct s3dw_input  *input;
	};
};

struct s3dw_surface {
	int _oid;
	int _nobj;
	int _flags;
	struct obj *_pobj;
};
void mainloop()
{
	s3d_rotate(oid,0,i,0);
	i++;
	nanosleep(&t,NULL); 
}
void object_click(struct s3d_evt *evt)
{
	s3d_quit();
}
	
struct s3dw_surface *s3dw_surface_new()
{
	struct s3dw_surface *ret;
	ret=(struct s3dw_surface *)malloc(sizeof(struct s3dw_surface));
	ret->_oid=-1;
	ret->_flags=0;
	ret->_nobj=0;
	ret->_pobj=NULL;
}
void s3dw_surface_destroy(struct s3dw_surface *surface)
{
	free(surface);
}
void s3dw_surface_append_obj(struct s3dw_surface *surface, struct s3dw_object *object, int posx, int posy)
{
	
}
struct s3dw_button *s3dw_button_new(struct s3dw_surface *surface, char *text, int posx, int posy)
{
	struct s3dw_button *button;
	struct s3dw_object *object;
	button=(struct s3dw_button *)malloc(sizeof(struct s3dw_button));
	object=(struct s3dw_object *)malloc(sizeof(struct s3dw_object));
	button->_text=strdup(text);
	button->_flags=0;
	object->_type=S3DW_TBUTTON;
	object->posx=posx;
	object->posy=posy;
	object->width=5;
	object->height=1;
	s3dw_surface_append_obj(surface, object);
	return(button);
}
int main (int argc, char **argv)
{
	return(0);
}

