/*
 * node.c
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
#include <string.h>	/* strncpy(), index() */
#include <stdio.h>	/* printf() */
/* find the node to a path, return NULL if not parsed yet */
t_node *node_getbypath(char *path)
{
	char p[M_DIR];
	char *s,*match;
	t_node *cur;
	int i;
	
	if (path==NULL) return NULL;
	if (path[0]=='/')
	{
		strncpy(p,path,M_DIR);
		s=p+1;
		cur=&root;
	} else return NULL; /* TODO: also process local paths. right now, we are to lazy */
	printf("processing rest of string %s\n",s);
	match=s;
	while ((s=index(s,'/'))!=NULL) { /* while we have slashes inside */
		s[0]=0; /* mark the slash with space */
		s++;	/* move to the next */
		/* parse ... */
		printf("looking for a match for %s\n",match);
		for (i=0;i<cur->n_sub;i++)
			if (0==strcmp(cur->sub[i]->name,match))
			{ /* found !! */
				cur=cur->sub[i]; /* forward */
				match=s; /* select next */
				break;
			}
		if (i==cur->n_sub) {
			printf("found no match for %s :(\n",match);
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
	if (t->oid==oid)			return(t);
	if (t->objs.str==oid)		return(t);
	if (t->objs.close==oid)		return(t);
	if (t->objs.select==oid)	return(t);
	if (t->objs.title==oid)		return(t);
	if (t->objs.titlestr==oid)	return(t);
	if (t->type==T_FOLDER)
		for (i=0;i<t->n_sub;i++)
			if ((f=node_getbyoid(t->sub[i],oid))!=NULL)
				return(f);
	return(NULL);
}
/* writes the path of dir into *path. path should be of type path[M_DIR] */
void node_path(t_node *dir, char *path)
{
	if (dir->parent!=NULL)
	{
		node_path(dir->parent,path);
		mstrncat(path,dir->name,M_DIR);
		mstrncat(path,"/",M_DIR);
	} else
		mstrncpy(path,dir->name,M_DIR);
}
/* clear the node */
int node_init(t_node *dir)
{
	dir->parent=NULL;
	dir->sub=NULL;
	dir->n_sub=0;

	dir->oid=-1;
	dir->objs.str=-1;
	dir->objs.close=-1;
	dir->objs.select=-1;
	dir->objs.title=-1;
	dir->objs.titlestr=-1;

	dir->len=0;
	dir->disp=0;
	dir->parsed=0;

	dir->px=root.pz=0.0;
	dir->dirs_opened=0;
	dir->type=T_DUNO;
	dir->px=dir->py=dir->pz=0.0;
	dir->dpx=dir->dpy=dir->dpz=0.0;
	dir->scale=dir->dscale=1.0;
	dir->detached=0;

	return(0);
}
/* general undisplay routine. does not handle anything recursively... */
int node_undisplay(t_node *dir)
{
	switch (dir->disp)
	{
		case D_DIR: return(box_undisplay(dir));break;
		case D_ICON:return(icon_undisplay(dir));break;
		default:	return(-1);
	}
	dir->disp=0;
}

/* delete a node and all its kids internally, remove the graphics, reorder the parents etc ... */
int node_delete(t_node *dir)
{
	/* TODO: IMPLEMENT IT, DAMNIT */
	/*
	int i;
	printf("node_free( %s )\n",t->name);
	switch (t->disp)
	{
			case D_DIR:  box_collapse(t,1); / * collapse this and its kids * /
			case D_ICON: icon_undisplay(t);
	}
	if (t->n_item>0) {
		for (i=0;i<t->n_item;i++)
			node_free(&(t->sub[i]));
		free(t->sub);
	}
	t->n_item=0;
	*/
	return(0);
}
/* node select handles click on the detach button. selected items can be moved, copied etc.*/
void node_select(t_node *dir)
{
	printf("node_select(%s)\n",dir->name);
	dir->detached=dir->detached?0:1; /* swapping, not sure if !dir->detached would do the same .. */
	printf("dir->type = %d\n",dir->disp);
	switch (dir->disp)
	{
		case D_DIR:
			if (dir->parent!=NULL)
				box_order_subdirs(dir->parent);
			break;
		case D_ICON:
			dir->pz=dir->detached*0.2+1.0;
			ani_add(dir);
			break;
		/* nothing yet ... */
	}
}
