/*
 * mcp.c
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
#include "proto.h"     /*  for S3D_P_OBJECT, to be integrated in proto.c */
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>   /*  htonl() */
#endif
#include <string.h>    /*  strncpy() */

/*  this interacts with the actual mcp client */
struct mcp_object {
	uint32_t object;
	float trans_x, trans_y, trans_z;
	float r;
	/*  char event; */
	char name[S3D_NAME_MAX];
};
#define MCP_NEW_OBJECT 1
/*  call when a new mcp connects */
int mcp_init(void)
{
	struct t_process *p;
	uint32_t i;
	p = get_proc_by_pid(MCP);
	i = p->n_obj;
	while (i--) {
		if (p->object[i] != NULL)
			switch (p->object[i]->oflags&OF_TYPE) {
			case OF_VIRTUAL:
				mcp_rep_object(i);
				break;
			case OF_CAM:
				event_obj_info(p, i);
				break;
			}
	}
	mcp_focus(-1);
	return(0);
}
/*  report the mcp about our object */
int mcp_rep_object(int32_t mcp_oid)
{
	struct mcp_object mo;
	struct t_process *p, *ap;
	p = get_proc_by_pid(MCP);
	mo.object = htonl(mcp_oid);
	mo.trans_x = p->object[mcp_oid]->translate.x;
	mo.trans_y = p->object[mcp_oid]->translate.y;
	mo.trans_z = p->object[mcp_oid]->translate.z;
	mo.r = p->object[mcp_oid]->r;

	htonfb(&mo.trans_x, 4);
	/*  mo.event=MCP_NEW_OBJECT; */
	ap = get_proc_by_pid(p->object[mcp_oid]->virtual_pid);
	strncpy(mo.name, ap->name, S3D_NAME_MAX);
	prot_com_out(p, S3D_P_MCP_OBJECT, (uint8_t *)&mo, sizeof(struct mcp_object));
	return(0);
}
/* tells the mcp that some program vanished ... */
int mcp_del_object(int32_t mcp_oid)
{
	int32_t oid = htonl(mcp_oid);
	if (mcp_oid == focus_oid) {
		s3dprintf(MED, "lost the focus of mcp-oid %d", mcp_oid);
		mcp_focus(-1);
	}
	prot_com_out(get_proc_by_pid(MCP), S3D_P_MCP_DEL_OBJECT, (uint8_t *)&oid, 4);
	return(0);
}
/* sets a new focus */
int mcp_focus(int oid)
{
	struct t_process *p;
	struct t_obj *o;
	focus_oid = -1;
	p = get_proc_by_pid(MCP);
	s3dprintf(MED, "request to focus %d", oid);
	if (OBJ_VALID(p, oid, o))
		if (o->oflags&OF_VIRTUAL) {
			focus_oid = oid;
			obj_pos_update(p, 0, 0);
		}
	return(0);
}

/*  clean up functions after the mcp left */

/* TODO: move things from object.c to this place!
int mcp_quit(void)
{
 return(0);
}
*/
