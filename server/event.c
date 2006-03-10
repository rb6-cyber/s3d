/*
 * event.c
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


#include "global.h"
#include "proto.h"
#include <stdio.h>		/* sprintf() */
#ifdef WIN32 
#include <winsock2.h>
#else
#include <netinet/in.h>  /*  htonl(),htons() */
#endif 
#include <string.h>  /*  strlen(),strcpy() */
extern int focus_oid;
extern int winw,winh; /* to give aspect ratio to the program */
/*  I don't plan to keep this until the end, but it can show us how */
/*  to interact ... */
int event_obj_click(struct t_process *p, uint32_t oid)
{
	uint32_t moid=htonl(oid);
	dprintf(MED,"telling client that oid %d got clicked",oid);
	prot_com_out(p,S3D_P_S_CLICK,(uint8_t *)&moid, 4);
	return(0);
}
/*  this functions sends keystrok events to the focused program. */
/*  maybe mcp-keystrokes should be catched here. */
int event_key_pressed(uint16_t key)
{
	uint16_t k;
	struct t_obj *o;
	k=htons(key);
	if (obj_valid(get_proc_by_pid(MCP),focus_oid,o))
		prot_com_out(get_proc_by_pid(o->n_mat), S3D_P_S_KEY, (uint8_t *)&k, 2);
	return(0);
}
/*  tell the client something about us */
int event_init(struct t_process *p)
{
	char s[NAME_MAX+3];
	sprintf(s,"%c%c%c%s", S3D_SERVER_MAJOR, S3D_SERVER_MINOR, S3D_SERVER_PATCH, S3D_SERVER_NAME); /* thanks award */
	prot_com_out(p,S3D_P_S_INIT, (uint8_t *)s, strlen(S3D_SERVER_NAME)+4);
	return(0);
}
/*  this lets a process quit gracefully ... */
int event_quit(struct t_process *p)
{
	prot_com_out(p, S3D_P_S_QUIT, NULL,0);
	dprintf(HIGH,"sending pid %d  QUIT signal",p->id); 
	process_del(p->id);
	return(0);
}
/* the cam changed?! we should run and tell this the mcp/focused client! */
int event_cam_changed()
{
	struct t_process *p;
	struct t_obj	 *o;
	p=get_proc_by_pid(MCP);
	event_obj_info(p,0);

	if (obj_valid(get_proc_by_pid(MCP),focus_oid,o))
		event_obj_info(get_proc_by_pid(o->n_mat),0);
	return(0);
}
/* this should replace the mcp_rep_object() function later ... */
int event_obj_info(struct t_process *p, uint32_t oid)
{
	struct t_obj_info mo;
	struct t_process *ap;
	struct t_obj *o;
	if (obj_valid(p,oid,o))
	{
		mo.object=htonl(oid);
		mo.trans_x=p->object[oid]->translate.x;
		mo.trans_y=p->object[oid]->translate.y;
		mo.trans_z=p->object[oid]->translate.z;
	
		mo.rot_x=p->object[oid]->rotate.x;
		mo.rot_y=p->object[oid]->rotate.y;
		mo.rot_z=p->object[oid]->rotate.z;
	
		mo.scale=p->object[oid]->scale;
	
		mo.r=p->object[oid]->r;

		memset(mo.name,0,NAME_MAX);
		switch (o->oflags&OF_TYPE)
		{
			case OF_VIRTUAL:
				ap=get_proc_by_pid(o->n_mat);
				strncpy(mo.name,ap->name,NAME_MAX);
				break;
			case OF_CAM:
				mo.scale=(float)((float)winw)/winh; /* give aspect ratio to program */
				strncpy(mo.name,"sys_camera0",NAME_MAX);
				break;
		}
		prot_com_out(p,S3D_P_S_OINFO,(uint8_t *)&mo,sizeof(struct t_obj_info));
	}
	return(0);
}
