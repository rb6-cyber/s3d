/*
 * ui.c
 * 
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include "s3dosm.h"
#include <stdio.h>	/* NULL */
icon_t icons[ICON_NUM]={
		{"objs/accesspoint.3ds",0},
		{"objs/noinetwep.3ds",0},
		{"objs/noinetwpa.3ds",0},
		{"objs/arrow2.3ds",0}
	};

/* load icons, we want to clone each of them later */
void ui_loadicons()
{
	int i;
	for (i=0;i<ICON_NUM;i++)
		icons[i].oid=s3d_import_model_file(icons[i].path);
}

static s3dw_surface *loadwindow=NULL;
static s3dw_label   *loadlabel=NULL;
static s3dw_label   *loadstatus=NULL;

int ui_click(struct s3d_evt *evt)
{
	int oid=(int)*((uint32_t *)evt->buf);
	if (s3dw_handle_click(evt)) {
		printf("s3dw got it?!\n");
	}
	printf("clicked object %d\n",oid);

	return(0);
}
int ui_key(struct s3d_evt *evt)
{
/*	struct s3d_key_event *key=(struct s3d_key_event *)evt->buf;*/
	s3dw_handle_key(evt);
	return(0);
}
int ui_oinfo(struct s3d_evt *evt)
{
	s3dw_object_info(evt);	
	return(0);
}
int ui_init()
{
	printf("ui_init() ...\n");
	ui_loadicons();
	s3d_set_callback(S3D_EVENT_OBJ_CLICK,ui_click);
	s3d_set_callback(S3D_EVENT_KEY,ui_key);
	s3d_set_callback(S3D_EVENT_OBJ_INFO,ui_oinfo);
	return(0);
}

/* initialize the loadwindow or change its caption text */
int load_window(char *text)
{
	if (loadwindow==NULL) /* create it */
	{
		loadwindow=s3dw_surface_new("Now loading ...",20,5);
		loadlabel=s3dw_label_new(loadwindow,text,1,2);
		loadstatus=s3dw_label_new(loadwindow,"",1,3);
		s3dw_show(S3DWIDGET(loadwindow));
	} else {
		s3dw_label_change_text(loadlabel,text);
		s3dw_label_change_text(loadstatus,"");
	}
	return(0);
}
/* remove it if still here */
int load_window_remove()
{
	if (loadwindow!=NULL) {
		s3dw_delete(S3DWIDGET(loadwindow));
		loadwindow=NULL;
		loadlabel=NULL;
	}
	return(0);
}
/* update the load status ... */
int load_update_status(float percent)
{
	char text[128];
	if (loadwindow!=NULL) {
		snprintf(text,128,"%3.1f",percent);
		s3dw_label_change_text(loadstatus,text);
	}
	mainloop();
	return(0);
}
