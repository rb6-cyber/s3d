/*
 * fly.c
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
#include <stdio.h>	/* NULL */
#include <string.h> /* strncpy() */
static void _get_pos(t_node *work, t_node *f)
{
	if (f->parent!=NULL) _get_pos(work,f->parent);
	work->px+=f->px										* work->scale;
	work->pz+=f->pz										* work->scale;
	work->py+=f->py										* work->scale;
	printf("%s: %3.3f %3.3f %3.3f - %3.3f\n",f->name, f->px, f->py, f->pz, f->scale );
	printf("### %3.3f %3.3f %3.3f - %3.3f)\n", work->px, work->py, work->pz, work->scale);

	if (f->parent==NULL)	work->scale = f->scale;
	else					work->scale = work->scale * f->scale;
	
}

int fly_set_absolute_position(t_node *node)
{
	t_node work;
	node_init(&work);
	work.px = node->px;
	work.py = node->py;
	work.pz = node->pz;
	printf("selected node %s: %3.3f %3.3f %3.3f, %3.3f\n",node->name, node->px, node->py, node->pz, node->scale);
	work.parent = node->parent;
	strncpy(work.name,node->name,M_NAME);
	_get_pos(&work, node);
	printf("absolute position: %f %f %f - %f\n", work.px, work.py, work.pz, work.scale);
	return(0);
}
