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


/* clear the dirs attributes */
int node_init(t_item *dir)
{
	dir->parent=NULL;
	dir->list=NULL;
	dir->n_item=-1;

	dir->block=-1;
	dir->str=-1;
	dir->close=-1;
	dir->select=-1;
	dir->title=-1;
	dir->titlestr=-1;

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
int parse_dir(t_item *dir)
{
	t_item *list;
	struct dirent **namelist;
	int n,i;
	char *ext,*nstr;
	char path[M_DIR];
	char ndir[M_DIR]; 
	
	printf("parse_dir( %s )",dir->name);
/*	if (dir->parsed) return(-1);*/
	get_path(dir,path);
/*	printf("scanning %s\n",path);*/
    n = i = scandir(path, &namelist, 0, alphasort);
	
    if (n < 0)
	{
        perror("scandir");
		return(-1);
	} else {
		list=malloc(sizeof(t_item)*i);
		dir->list=list;
		dir->n_item=n;
        while(n--) {
			/* setup kids in the list */
			node_init(&list[n]);
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
					   if (0==strncmp(ext,".model",strlen(ext)<4?strlen(ext):4))
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
		dir->parsed=1;
   	}
	return(0);
}
void parse_again(t_item *dir)
{
	int oldn,redisp;
	t_item *oldlist;
	int i,j;
	oldlist=dir->list;
	oldn   =dir->n_item;
	redisp=0;
	printf("parse_again( %s )\n",dir->name);
	if (dir->disp==D_DIR)	/* undisplay it later */
	{ 
		box_undisplay(dir); 
		redisp=1; 
	}
	parse_dir(dir);

	printf("oldn = %d\n",oldn);
	if (oldn>0)
	{
		/* find old, already displayed contents, and copy the data, or remove them if 
		 * deleted */
		
		for (i=0;i<oldn;i++)
		{
			if (oldlist[i].disp == D_DIR)	/* old things expanded as directory */
			{
				/* see if it's also in the new list */
				for (j=0;j<dir->n_item;j++)
				{
					if (0==strcmp(oldlist[i].name,dir->list[j].name))
					{
						printf("we still have %s - %s (%d,%d) , copy to new list ... \n",dir->list[j].name, oldlist[i].name,j,i);
						memcpy(&(dir->list[j]),&oldlist[i],sizeof(t_item));
						break; /* found */
					}
				}
				if (j == dir->n_item) /* not found, collapse it */
					node_free(&oldlist[i]);
				else {}/* don't collapse it!! keep as it is */
			} else node_free(&oldlist[i]);
		}
		free(oldlist);

		printf("finished copying old content, redisplay if neccesary\n");
		if (redisp)
		{
			printf("redisplay ... (opened: %d) ", dir->dirs_opened);
			/* if it was displayed, redisplay it ... */
			box_expand(dir);
			printf("done (opened: %d)\n", dir->dirs_opened);
		}
	}	
}
void node_free(t_item *t)
{
	int i;
	printf("node_free( %s )\n",t->name);
	switch (t->disp)
	{
			case D_DIR:  box_collapse(t,1); /* collapse this and its kids */
			case D_ICON: icon_undisplay(t);
	}
	if (t->n_item>0) {
		for (i=0;i<t->n_item;i++)
			node_free(&(t->list[i]));
		free(t->list);
	}
	t->n_item=0;
}

