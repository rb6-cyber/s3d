/*
 * event.c
 * 
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
#include <stdio.h> /* printf() */



int typeinput=0;

/* info packets handler, we're just interested in the cam */
void event_oinfo(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf=(struct s3d_obj_info *)hrmz->buf;
	if ((inf->object==0) && (!ani_onstack(&cam)))
	{
		cam.dpx=inf->trans_x;
		cam.dpy=inf->trans_y;
		cam.dpz=inf->trans_z;
	}
	s3dw_object_info(hrmz);
}

/* keyevent handler */
void event_key(struct s3d_evt *evt)
{
	struct s3d_key_event *keys=(struct s3d_key_event *)evt->buf;
	char path[M_DIR];
	if (typeinput) {	/* we have some inputfield now and want the s3dw to handle our input */	
			printf("inputting text ...\n");
			s3dw_handle_key(evt); 
			return; 
	}
	node_path(focus,path);
	switch (keys->keysym)
	{
		case 'i':
		case 'I':
				{
				window_info(path);
				}
				break;
		case 'r':
		case 'R':
				{/* refresh this window ... */
					printf("[R]efreshing %s\n",focus->name);
/*					parse_again(focus);*/
					ani_focus(focus);
				}
				break;
		case S3DK_F1:
				window_help();
				break;
		case S3DK_F5:
				window_copy(path);
				break;
		case S3DK_F6:
				window_move(path);
				break;
		case S3DK_F7:
				window_mkdir(path);
				break;


	}
	s3dw_handle_key(evt);
}

/* object click handler */
void event_click(struct s3d_evt *evt)
{
	int oid;
	t_node *f;
	s3dw_handle_click(evt);
	oid=(int)*((unsigned long *)evt->buf);
	if (NULL!=(f=node_getbyoid(&root,oid)))
	{
		if (f->objs.close==oid)
		{
			box_close(f,1);
			return;
		}
		if (f->objs.select==oid)
		{
			printf("[S]electing %s\n",f->name);
			node_select(f);
			return;
		}
		switch (f->disp)
		{
			case D_DIR:
				printf("[F]ound, Already displayed - ani_focus( %s )\n",f->name);
				ani_focus(f);
				break;
			case D_ICON:
				if (f->type==T_FOLDER)
				{
					parse_dir(f);
					box_expand(f);	
					ani_focus(f);
				} else {
					node_select(f);
				}
				break;
		}
	} else {
/*		printf("[C]ould not find :/\n");*/
	}
}

