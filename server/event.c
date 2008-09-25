/*
 * event.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <stdio.h>  /* sprintf() */
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>  /*  htonl(),htons() */
#endif
#include <string.h>  /*  strlen(),strcpy() */

/*  I don't plan to keep this until the end, but it can show us how */
/*  to interact ... */
int event_obj_click(struct t_process *p, int32_t oid)
{
	uint32_t moid = htonl(oid);
	s3dprintf(MED, "telling client that oid %d got clicked", oid);
	prot_com_out(p, S3D_P_S_CLICK, (uint8_t *)&moid, 4);
	return(0);
}
/*  this functions sends keystroke events to the focused program. */
/*  maybe mcp-keystrokes should be catched here. */
/*  state = 0 -> pressed, 1 -> released */
int event_key_pressed(uint16_t key, uint16_t uni, uint16_t mod, int state)
{
	uint16_t k[4];
	struct t_obj *o;
	k[0] = htons(key);
	k[1] = htons(uni);
	k[2] = htons(mod);
	k[3] = htons(state);
	if (OBJ_VALID(get_proc_by_pid(MCP), focus_oid, o))
		prot_com_out(get_proc_by_pid(o->virtual_pid), S3D_P_S_KEY, (uint8_t *)k, 8);
	prot_com_out(get_proc_by_pid(MCP), S3D_P_S_KEY, (uint8_t *)k, 8); /* mcp always gets a copy */
	return(0);
}


/* mouse button changes are sent to the client */
int event_mbutton_clicked(uint8_t button, uint8_t state)
{
	struct t_obj *o;
	uint8_t b[2];
	b[0] = button;
	b[1] = state;
	if (OBJ_VALID(get_proc_by_pid(MCP), focus_oid, o))
		prot_com_out(get_proc_by_pid(o->virtual_pid), S3D_P_S_MBUTTON, (uint8_t *)&b, 2);
	prot_com_out(get_proc_by_pid(MCP), S3D_P_S_MBUTTON, (uint8_t *)&b, 2); /* mcp always gets a copy */
	return(0);
}
/*  tell the client something about us */
int event_init(struct t_process *p)
{
	char s[S3D_NAME_MAX+3];
	sprintf(s, "%c%c%c%s", S3D_SERVER_MAJOR, S3D_SERVER_MINOR, S3D_SERVER_PATCH, S3D_SERVER_NAME); /* thanks award */
	prot_com_out(p, S3D_P_S_INIT, (uint8_t *)s, strlen(S3D_SERVER_NAME) + 4);
	return(0);
}
/*  this lets a process quit gracefully ... */
int event_quit(struct t_process *p)
{
	prot_com_out(p, S3D_P_S_QUIT, NULL, 0);
	s3dprintf(HIGH, "sending pid %d QUIT signal", p->id);
	process_del(p->id);
	return(0);
}
/* the cam changed?! we should run and tell this the mcp/focused client! */
int event_cam_changed(void)
{
	struct t_process *p;
	struct t_obj  *o;
	p = get_proc_by_pid(MCP);
	event_obj_info(p, 0);
	if (OBJ_VALID(p, focus_oid, o))
		event_obj_info(get_proc_by_pid(o->virtual_pid), 0);
	return(0);
}
/* same for the mouse movement! */
int event_ptr_changed(void)
{
	struct t_process *p;
	struct t_obj  *o;
	p = get_proc_by_pid(MCP);
	event_obj_info(p, get_pointer(p));
	if (OBJ_VALID(p, focus_oid, o)) {
		p = get_proc_by_pid(o->virtual_pid); /* focused program pointer*/
		event_obj_info(p, get_pointer(p));
	}
	return(0);
}

/* inform client about an available shm-segment for the texture */
int event_texshm(struct t_process *p, int32_t oid, int32_t tex)
{
	struct t_obj *o;
	struct {
		int32_t oid, tex, shmid;
		uint16_t tw, th, w, h;
	} __attribute__((__packed__)) shmtex_packet;
	if (OBJ_VALID(p, oid, o)) {
		s3dprintf(LOW, "informing process about new texture on oid %d, texture %d, which is available under id %d\n",
		          oid, tex, o->p_tex[tex].shmid);
		shmtex_packet.oid = htonl(oid);
		shmtex_packet.tex = htonl(tex);
		shmtex_packet.shmid = htonl(o->p_tex[tex].shmid);
		shmtex_packet.tw = htons(o->p_tex[tex].tw);
		shmtex_packet.th = htons(o->p_tex[tex].th);
		shmtex_packet.w = htons(o->p_tex[tex].w);
		shmtex_packet.h = htons(o->p_tex[tex].h);
		prot_com_out(p, S3D_P_S_SHMTEX, (uint8_t *)&shmtex_packet, sizeof(shmtex_packet));
	}
	return(0);
}

/* this should replace the mcp_rep_object() function later ... */
int event_obj_info(struct t_process *p, int32_t oid)
{
	struct {
		int32_t object;
		uint32_t flags;
		float trans_x, trans_y, trans_z;
		float rot_x, rot_y, rot_z;
		float scale;
		float r;
		char name[S3D_NAME_MAX];
	} __attribute__((__packed__)) mo;

	struct t_process *ap;
	struct t_obj *o;
	if (OBJ_VALID(p, oid, o)) {
		mo.object = htonl(oid);
		mo.trans_x = p->object[oid]->translate.x;
		mo.trans_y = p->object[oid]->translate.y;
		mo.trans_z = p->object[oid]->translate.z;

		mo.rot_x = p->object[oid]->rotate.x;
		mo.rot_y = p->object[oid]->rotate.y;
		mo.rot_z = p->object[oid]->rotate.z;

		mo.scale = p->object[oid]->scale;

		mo.r = p->object[oid]->r;

		memset(mo.name, 0, S3D_NAME_MAX);
		switch (o->oflags&OF_TYPE) {
		case OF_VIRTUAL:
			ap = get_proc_by_pid(o->virtual_pid);
			strncpy(mo.name, ap->name, S3D_NAME_MAX);
			break;
		case OF_CAM:
			mo.scale = (float)((float)winw) / winh; /* give aspect ratio to program */
			strncpy(mo.name, "sys_camera0", S3D_NAME_MAX);
			break;
		case OF_POINTER:
			strncpy(mo.name, "sys_pointer0", S3D_NAME_MAX);
			break;

		}
		htonfb(&mo.trans_x, 8);
		prot_com_out(p, S3D_P_S_OINFO, (uint8_t *)&mo, sizeof(mo));
	}
	return(0);
}
