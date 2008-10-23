/*
 * widget.c
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

#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* strdup() */
s3dw_widget *s3dw_widget_new(s3dw_widget *widget)
{
	widget->type = -1;
	widget->x = widget->ax = 0;
	widget->y = widget->ay = 0;
	widget->z = widget->az = 0;
	widget->rx = widget->arx = 0;
	widget->ry = widget->ary = 0;
	widget->rz = widget->arz = 0;
	widget->s = widget->as = 1;
	widget->width = 0;
	widget->height = 0;
	widget->nobj = 0;
	widget->pobj = NULL;
	widget->parent = NULL;
	widget->ptr = NULL;
	widget->focus = -1;
	widget->flags = S3DW_ACTIVE;
	widget->oid = -1;
	return(widget);
}
/* widget clicked, call specific function and check kids */
int s3dw_widget_event_click(s3dw_widget *widget, uint32_t oid)
{
	int i;
	s3dprintf(VLOW, "processing click event for widget %10p of type %d, oid %d (%d), subobjects: %d", (void*)widget, widget->type, widget->oid, oid, widget->nobj);
	if (s3dwcb_click[widget->type](widget, oid)) return(1);
	for (i = 0;i < widget->nobj;i++)
		if (s3dw_widget_event_click(widget->pobj[i], oid)) return(1);
	return(0);
}
/* widget received key,,call specific function and check (focused) kids */
int s3dw_widget_event_key(s3dw_widget *widget, struct s3d_key_event *keys)
{
	if (s3dwcb_key[widget->type](widget, keys)) return(1);
	if (widget->focus != -1)
		if (s3dw_widget_event_key(widget->pobj[widget->focus], keys)) return(1);
	return(0);
}


/* append an widget */
void s3dw_widget_append(s3dw_widget *parent, s3dw_widget *widget)
{
	parent->nobj++;
	parent->pobj = (s3dw_widget**)realloc(parent->pobj, sizeof(s3dw_widget **) * (parent->nobj));
	parent->pobj[parent->nobj-1] = widget;
	widget->parent = parent;
	widget->style = parent->style;
	if (!(parent->flags&S3DW_VISIBLE))
		widget->flags |= S3DW_VISIBLE;
}
/* removes an widget from it's parent, should have been appended before */
static void s3dw_widget_remove(s3dw_widget *widget)
{
	s3dw_widget *parent = widget->parent;
	int i, stackpos;

	stackpos = s3dw_ani_stackpos(widget);
	if (stackpos != -1)
		s3dw_ani_del(stackpos);
	if (parent == NULL) return;

	for (i = 0;i < parent->nobj;i++) /* search ... */
		if (parent->pobj[i] == widget) { /* ... and destroy */
			if (parent->focus == i)     parent->focus = -1;
			if (parent->focus == (parent->nobj - 1)) parent->focus = i;
			parent->pobj[i] = parent->pobj[parent->nobj-1]; /* swap last element to the to be deleted one */
			parent->nobj--;
		}
}
/* properly delete the object, removing kids, own structure and link from parent. */
void s3dw_delete(s3dw_widget *widget)
{
	s3dw_widget_remove(widget);
	/* remove kids */
	while (widget->nobj > 0) /* will decrease as child-delete will call s3dw_widget_remove() */
		s3dw_delete(widget->pobj[0]);
	free(widget->pobj);
	s3dwcb_destroy[widget->type](widget); /* type-specific destroy */
}
/* toggle a widget visible and show it */
void s3dw_show(s3dw_widget *widget)
{
	widget->flags |= S3DW_VISIBLE;
	s3dw_widget_visible(widget);
}
void s3dw_focus(s3dw_widget *focus)
{
	int i;
	for (i = 0;i < focus->parent->nobj;i++)
		if (focus->parent->pobj[i] == focus) {
			focus->parent->focus = i;
			return;
		}
}

/* show visible kids */
void s3dw_widget_visible(s3dw_widget *widget)
{
	int i;
	s3dw_widget *kid;
	for (i = 0;i < widget->nobj;i++) {
		kid = widget->pobj[i];
		if (widget->flags&S3DW_VISIBLE)
			s3dw_widget_visible(kid);
	}
	widget->flags |= S3DW_ONSCREEN;
	s3dwcb_show[widget->type](widget);
}
/* apply the moves ... */
void     s3dw_moveit(s3dw_widget *widget)
{
	s3dw_ani_add(widget);
}
