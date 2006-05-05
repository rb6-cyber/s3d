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
#include <stdio.h> 	 /*  printf() */
#include <dirent.h>  /*  dirent */
#include <stdlib.h>	 /*  malloc() */
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <time.h>	/* nanosleep() */
static struct timespec t={0,33*1000*1000}; /* 30 fps */
struct t_item root;

void get_path(struct t_item *dir, char *path)
{
	if (dir->parent!=NULL)
	{
		get_path(dir->parent,path);
		strncat(path,dir->name,M_DIR);
		strncat(path,"/",M_DIR);
	} else
		strncpy(path,dir->name,M_DIR);
}
int parse_dir(struct t_item *dir)
{
	struct t_item *list;
	struct dirent **namelist;
	int n,i;
	char *ext,*nstr;
	char path[M_DIR];
	char ndir[M_DIR];  
	if (dir->n_item>0) /* refusing */
		return(-1);
	get_path(dir,path);
	printf("scanning %s\n",path);
    n = i = scandir(path, &namelist, 0, alphasort);
    if (n < 0)
	{
        perror("scandir");
		return(-1);
	} else {
		if (dir->n_item>0)
			free(dir->list); /* this is a refresh, free old items */
		list=malloc(sizeof(struct t_item)*i);
		dir->list=list;
		dir->n_item=n;
		printf("found %d items, processing ...\n",n);
        while(n--) {

			box_init(&list[n]);
			nstr=namelist[n]->d_name;
			strncpy(list[n].name,nstr,M_NAME);
 		    if ((0==strncmp(nstr,".",1)) && (strlen(nstr)==1))
				list[n].type=T_LOCALDIR;
			else if (0==strncmp(nstr,"..",strlen(nstr)<2?strlen(nstr):2))
			   list[n].type=T_BACKDIR;
			else {
				ext=strrchr(nstr,'.');
				strncpy(ndir,path,M_DIR);
		    	strncat(ndir,namelist[n]->d_name,M_DIR);
			    if ((namelist[n]->d_type==DT_DIR) ||
					((namelist[n]->d_type==DT_UNKNOWN) && (opendir(ndir)!=NULL)))
						list[n].type=T_FOLDER;
				else 
				{
				   if (ext!=NULL)
				   {
					   if (0==strncmp(ext,".3ds",strlen(ext)<4?strlen(ext):4))
							   list[n].type=T_GEOMETRY;
					   else if (0==strncmp(ext,".mp3",strlen(ext)<4?strlen(ext):4))
							   list[n].type=T_MUSIC;
				   }
				}
			}
			list[n].parent=dir;
        	free(namelist[n]);
		}
		free(namelist);
   	}
	return(0);
}
/* finds an item in the tree by oid */
struct t_item *finditem(struct t_item *t, int oid)
{
	int i;
	struct t_item *f;
	if (t->block==oid)		return(t);
	if (t->str==oid)		return(t);
	if (t->close==oid)		return(t);
	if (t->type==T_FOLDER)
		for (i=0;i<t->n_item;i++)
			if ((f=finditem(&(t->list[i]),oid))!=NULL)
				return(f);
	return(NULL);
}
/* gets the scale by multiplying scales */



void object_click(struct s3d_evt *evt)
{
	int oid;
	struct t_item *f;
	oid=(int)*((unsigned long *)evt->buf);
	if (NULL!=(f=finditem(&root,oid)))
	{
		if (f->close==oid)
		{
			box_collapse(f);
			if (f->parent!=NULL)
				ani_focus(f->parent);
			return;
		}
		if (f->type==T_FOLDER)
		{
			printf("[F]ound, expanding %s\n",f->name);
			parse_dir(f);
			box_expand(f);
			ani_focus(f);
		} else
			printf("[F]ound, but is %s no folder\n",f->name);
	} else {
		printf("[C]ould not find :/\n");
	}
}
void mainloop()
{
	nanosleep(&t,NULL); 
	ani_mate();
}
int main (int argc, char **argv)
{
	if (!s3d_init(&argc,&argv,"s3dfm"))	
	{
		s3d_select_font("vera");
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
		
		/* set up file system representation */
		box_init(&root);
		strncpy(root.name,"/",M_NAME);
		root.dscale=0.1;
		root.type=T_FOLDER;
		root.str=s3d_draw_string(root.name,&root.len);
		if (root.len<2) root.len=2;
		parse_dir(&root);
		box_expand(&root);
		ani_doit(&root);
		ani_focus(&root);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
