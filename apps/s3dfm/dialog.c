/*
 * dialog.c
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dfm, a s3d file manager.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dfm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dfm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dfm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3dfm.h"
#include <s3d_keysym.h>
#include <s3dw.h>
#include <stdio.h> 	/* NULL, printf() */
#include <string.h> /* strlen() */
extern t_item *focus;
/* keyevent handler */
void key_handler(struct s3d_evt *evt)
{
	struct s3d_key_event *keys=(struct s3d_key_event *)evt->buf;
	switch (keys->keysym)
	{
		case S3DK_F1:
				{
				char path[M_DIR];
				get_path(focus,path);
				info_window(path);
				}
	}
	s3dw_handle_key(evt);
}

/* object click handler */
void object_click(struct s3d_evt *evt)
{
	int oid;
	t_item *f;
	s3dw_handle_click(evt);
	oid=(int)*((unsigned long *)evt->buf);
	if (NULL!=(f=finditem(&root,oid)))
	{
		if (f->close==oid)
		{
			box_collapse(f,1);
/*			if (f->parent!=NULL)
				ani_focus(f->parent);*/
			return;
		}
		if (f->select==oid)
		{
			printf("[S]electing %s\n",f->name);
			box_select(f);
			return;
		}
		if (f->type==T_FOLDER)
		{
			printf("[F]ound, expanding %s\n",f->name);
			parse_dir(f);
			box_expand(f);
			focus=f;
			ani_focus(f);
		} else
			printf("[F]ound, but is %s no folder\n",f->name);
	} else {
		printf("[C]ould not find :/\n");
	}
}
void close_win(s3dw_widget *button)
{
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}

void info_window(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	char string1[M_DIR];
	char string2[M_DIR];
	int b,d,f;
	float l;
	snprintf(string1,M_DIR,"Info for %s",path);
	fs_approx(path, &f, &d, &b);
	snprintf(string2 ,M_DIR,"%d bytes in %d files and %d Directories",b,f,d);
	
	l=((strlen(string1)>strlen(string2)) ? strlen(string1) :strlen(string2))*0.7;
	
	infwin=s3dw_surface_new("Info Window",l,12);

	s3dw_label_new(infwin,string1,1,2);
	s3dw_label_new(infwin,string2,1,4);

	button=s3dw_button_new(infwin,"OK",l/2-1,6);
	/* clicking on the button will exit ... */
	button->onclick=close_win;
	/* of couse, show it */
	s3dw_show(S3DWIDGET(infwin));

}
