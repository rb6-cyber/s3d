// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include "s3dfm.h"
#include <s3d.h> /* NULL */
#include <stdio.h> /* NULL */
#include <string.h> /* strncpy() */
#include <stdlib.h> /* malloc() */
#include <string.h> /* strncpy() */
static void _get_pos(t_node *work, t_node *f)
{
	if (f->parent != NULL) _get_pos(work, f->parent);
	work->px += f->px          * work->scale;
	work->pz += f->pz          * work->scale;
	work->py += f->py          * work->scale;
	/*
	printf("%s: %3.3f %3.3f %3.3f - %3.3f\n",f->name, f->px, f->py, f->pz, f->scale );
	printf("### %3.3f %3.3f %3.3f - %3.3f)\n", work->px, work->py, work->pz, work->scale);
	*/
	if (f->parent == NULL) work->scale = f->scale;
	else     work->scale = work->scale * f->scale;

}

int fly_set_absolute_position(t_node *node)
{
	t_node work;
	work.px = 0;
	work.py = 0;
	work.pz = 0;
	work.scale = 1.0;
	_get_pos(&work, node);
	node->px = work.px;
	node->py = work.py;
	node->pz = work.pz;
	node->scale = work.scale;
	/* printf("node coordinates: %3.3f %3.3f %3.3f %3.3f\n",node->px,node->py,node->pz,node->scale);*/
	return 0;
}
/* create a copy of *node as an icon (block) which can be moved for animation ... */
t_node *fly_create_anode(t_node *node)
{
	t_node *work;
	work = (t_node*)malloc(sizeof(t_node));
	node_init(work);
	work->parent = node->parent;
	work->scale = node->scale;
	work->px = node->px;
	work->py = node->py;
	work->pz = node->pz;
	work->type = node->type;
	mstrncpy(work->name, node->name, M_NAME);
	fly_set_absolute_position(work);
	icon_draw(work);

	work->dpx = work->px;
	work->dpy = work->py;
	work->dpz = work->pz;
	work->dscale = work->scale;
	work->py = 2 * work->scale - work->py; /* invert */
	s3d_flags_on(work->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(work->objs.str, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	ani_add(work);
	return work;
}
