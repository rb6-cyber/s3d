/*
 * main.c
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
#include <s3dw.h>	 /* s3dw_ani_mate() */
#include <stdio.h> 	 /*  printf() */
#include <dirent.h>  /*  dirent */
#include <stdlib.h>	 /*  malloc() */
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <time.h>	/* nanosleep() */
static struct timespec t={0,33*1000*1000}; 
t_item root,cam,*focus;

/* save concatting 2 strings, this version takes argument n
 * as the size of the buffer of dest. */
char *mstrncat(char *dest, const char *src, int n)
{
	int i,j;
	dest[n-1]=0;						/* for malformed destinations */
	j=0;
	for (i=strlen(dest);i<(n-1);i++)
	{
		dest[i]=src[j]; 
		if (dest[i]==0) break;
		j++;
	}
	for (;i<n;i++)
		dest[i]=0; /* pad the rest with zero */
	return(dest);
}
/* same as strncpy, but have a terminating zero even if
 * source is too big */
char *mstrncpy(char *dest, const char *src, int n)
{
	strncpy(dest,src,n);
	dest[n-1]=0;
	return(dest);
}
/* writes the path of dir into *path. path should be of type path[M_DIR] */
void get_path(t_item *dir, char *path)
{
	if (dir->parent!=NULL)
	{
		get_path(dir->parent,path);
		mstrncat(path,dir->name,M_DIR);
		mstrncat(path,"/",M_DIR);
	} else
		mstrncpy(path,dir->name,M_DIR);
}
/* finds an item in the tree by oid */
t_item *finditem(t_item *t, int oid)
{
	int i;
	t_item *f;
	if (t->block==oid)		return(t);
	if (t->str==oid)		return(t);
	if (t->close==oid)		return(t);
	if (t->select==oid)		return(t);
	if (t->title==oid)		return(t);
	if (t->titlestr==oid)	return(t);
	if (t->type==T_FOLDER)
		for (i=0;i<t->n_item;i++)
			if ((f=finditem(&(t->list[i]),oid))!=NULL)
				return(f);
	return(NULL);
}
/* info packets handler, we're just interested in the cam */
void object_info(struct s3d_evt *hrmz)
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
void mainloop()
{
	ani_mate();
	s3dw_ani_mate();
	nanosleep(&t,NULL); 
}
int main (int argc, char **argv)
{

	s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
	s3d_set_callback(S3D_EVENT_OBJ_INFO,object_info);
	s3d_set_callback(S3D_EVENT_KEY,key_handler);
	if (!s3d_init(&argc,&argv,"s3dfm"))	
	{
		s3d_select_font("vera");

		
		/* set up file system representation */
		box_init(&root);
		strncpy(root.name,"/",M_NAME);
		focus=&root;
		root.dscale=0.1;
		root.type=T_FOLDER;
		root.str=s3d_draw_string(root.name,&root.len);
		if (root.len<2) root.len=2;
		parse_dir(&root);
		box_expand(&root);
		ani_doit(&root);
		ani_focus(&root);
		
		box_init(&cam); /* a virtual object, just to push the cam throu our animation stack */
		cam.block=0;
		
		
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
