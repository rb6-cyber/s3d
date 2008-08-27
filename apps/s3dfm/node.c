/*
 * node.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <string.h> /* strncpy(), index() */
#include <stdio.h> /* printf() */
#include <stdlib.h> /* free() */
/* find the node to a path, return NULL if not parsed yet */
t_node *node_getbypath(const char *path)
{
	char p[M_DIR];
	char *s, *match;
	t_node *cur;
	int i;

	if (path == NULL) return NULL;
	if (path[0] == '/') {
		strncpy(p, path, M_DIR - 1);
		s = p + 1;
		cur = &root;
	} else return NULL; /* TODO: also process local paths. right now, we are to lazy */
	p[strlen(p)+1] = 0; /* extra terminating 0, to be sure */
	printf("processing rest of string %s\n", s);
	match = s;
	while (*s != 0) { /* while search string is not empty */
		if ((s = index(s, '/')) != NULL) {
			s[0] = 0; /* mark the slash with space */
			s++; /* move to the next */
		} else {
			s = match + strlen(match); /* select terminating 0 */
		}
		/* parse ... */
		printf("looking for a match for %s, rest is %s\n", match, s);
		for (i = 0;i < cur->n_sub;i++)
			if (0 == strcmp(cur->sub[i]->name, match)) { /* found !! */
				cur = cur->sub[i]; /* forward */
				match = s; /* select next */
				break;
			}
		if (i == cur->n_sub) {
			printf("found no match for %s :(\n", match);
			return NULL; /* not found */
		}
	}
	return(cur);
}
/* finds an node in the tree by oid */
t_node *node_getbyoid(t_node *t, int oid)
{
	int i;
	t_node *f;
	if (t->oid == oid)   return(t);
	if (t->objs.str == oid)  return(t);
	if (t->objs.close == oid)  return(t);
	if (t->objs.select == oid) return(t);
	if (t->objs.title == oid)  return(t);
	if (t->objs.titlestr == oid) return(t);
	if (t->type == T_FOLDER)
		for (i = 0;i < t->n_sub;i++)
			if ((f = node_getbyoid(t->sub[i], oid)) != NULL)
				return(f);
	return(NULL);
}
/* writes the path of dir into *path. path should be of type path[M_DIR] */
void node_path(t_node *dir, char *path)
{
	if (dir->parent != NULL) {
		node_path(dir->parent, path);
		if (dir->parent->parent != NULL)
			mstrncat(path, "/", M_DIR);
		mstrncat(path, dir->name, M_DIR);
	} else
		mstrncpy(path, dir->name, M_DIR);
}
/* clear the node */
int node_init(t_node *dir)
{
	dir->parent = NULL;
	dir->sub = NULL;
	dir->n_sub = 0;

	dir->oid = -1;
	dir->objs.str = -1;
	dir->objs.close = -1;
	dir->objs.select = -1;
	dir->objs.title = -1;
	dir->objs.titlestr = -1;
	dir->objs.strlen = 0;

	dir->disp = D_NONE;
	dir->parsed = 0;

	dir->pindex = -1;
	dir->check = 0;
	dir->dirs_opened = 0;
	dir->type = T_DUNO;
	dir->px = dir->py = dir->pz = 0.0;
	dir->dpx = dir->dpy = dir->dpz = 0.0;
	dir->scale = dir->dscale = 1.0;
	dir->detached = 0;

	return(0);
}
/* general undisplay routine. does not handle anything recursively... */
int node_undisplay(t_node *dir)
{
	switch (dir->disp) {
	case D_DIR:
		return(box_undisplay(dir));
		break;
	case D_ICON:
		return(icon_undisplay(dir));
		break;
	default:
		return(-1);
	}
}

/* delete a node and all its kids internally, remove the graphics, reorder the parents etc ... */
int node_delete(t_node *dir)
{
	int i;
	if (dir->parent == NULL) {
		printf("won't delete root window!\n");
		return(-1);
	}
	/* delete all the kids */
	if (dir->n_sub > 0) {
		for (i = 0;i < dir->n_sub;i++)
			node_delete(dir->sub[i]);
		free(dir->sub);
	}
	/* move focus upward, this should go up with the recursion */
	if (focus == dir) focus_set(dir->parent); /* do this before deleting the contents, its better ... */
	switch (dir->disp) {
	case D_DIR:
		box_undisplay(dir);
	case D_ICON:
		icon_undisplay(dir);
	}

	if (-1 != (i = ani_onstack(dir))) ani_del(i); /* tell animation stack too */
	free(dir);
	return(0);
}
/* node select handles click on the detach button. selected items can be moved, copied etc.*/
void node_select(t_node *dir)
{

	dir->detached = dir->detached ? 0 : 1; /* swapping, not sure if !dir->detached would do the same .. */
	switch (dir->disp) {
	case D_DIR:
		if (focus != dir) {
			dir->detached = dir->detached ? 0 : 1; /* swap again, we actually don't want to have it detachedf now. */
			focus_set(dir);
		}
		if (dir->parent != NULL)
			box_order_subdirs(dir->parent);
		break;
	case D_ICON:
		if (dir->type == T_FOLDER) {
			dir->detached = 0;
			if (!parse_dir(dir))
				box_expand(dir);
		} else {
			dir->pz = dir->detached * 0.2 + 1.0;
			ani_add(dir);
		}
		focus_set(dir);
		break;
	}
}
/* change color etc if a node is focused */
void node_focus_color(t_node *node, int on)
{
	switch (node->disp) {
	case D_DIR:
		box_focus_color(node, on);
		break;
	case D_ICON:
		icon_focus_color(node, on);
		break;
	}
}
/* get the directory of a node */
t_node *node_getdir(t_node *node)
{
	if (node->type == T_FOLDER) return(node);
	else return(node->parent);
}
