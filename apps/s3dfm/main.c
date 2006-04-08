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


#include <s3d.h>
#include <stdio.h> 	 /*  printf() */
#include <dirent.h>  /*  dirent */
#include <stdlib.h>	 /*  malloc() */
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <math.h>	 /*  sin(),cos() */
#include <time.h>	/* nanosleep() */
static struct timespec t={0,10*1000*1000}; /* 10 mili seconds */


#define SH			0.4 /* height of the step */

#define T_DUNO		0
#define T_LOCALDIR	1
#define T_BACKDIR	2
#define T_FOLDER	3
#define T_GEOMETRY	4
#define T_MUSIC		5

#define M_DIR		512
#define M_NAME		256
int folder,geometry,mp3,duno,dot,dotdot;
float dpx,dpy,dpz,dscale;
float px, py, pz, scale;
struct t_item {
	int icon, str;							/* object ids ...*/
	int block;								/* oid of the block */
	int dirs_opened;						/* how many directories are on the block */
	char name[M_NAME];						/* name (e.g. file name) */
	struct t_item *parent;					/* parent item */
	struct t_item *list;					/* list of items  (if it's a subdir)*/
	float px,pz;
	int n_item;								/* number of items in list ( = -1 for normal or not-expanded files) */
	int type;								/* type, determined by extension or file type like dir, pipe, link etc */
	int disp;
};
struct t_item root;
/* draw a block */
int new_block(struct t_item *dir)
{
	float vertices[]=
			{-1,0,-1,
			 -1,0, 1,
			  1,0, 1,
			  1,0,-1,
			 -1,1,-1,
			 -1,1, 1,
			  1,1, 1,
			  1,1,-1};

			 
	dir->block=s3d_new_object();
	s3d_push_vertices(dir->block,vertices,8);
	s3d_push_material(dir->block,
						1,1,1,
						1,1,1,
						1,1,1);
	s3d_push_material(dir->block,
						0.5,1,0.5,
						0.5,1,0.5,
						0.5,1,0.5);

	s3d_push_polygon(dir->block,4,5,6,1);
	s3d_push_polygon(dir->block,4,6,7,1);

	s3d_push_polygon(dir->block,0,4,5,0);
	s3d_push_polygon(dir->block,0,5,1,0);
	
	s3d_push_polygon(dir->block,3,7,4,0);
	s3d_push_polygon(dir->block,3,4,0,0);

	s3d_push_polygon(dir->block,2,6,7,0);
	s3d_push_polygon(dir->block,2,7,3,0);
	return(0);

}
/* orders the directory objects on top of its parent objects 
 * to be called after adding or removing things ...*/
void placeontop(struct t_item *dir)
{
	int i,j;
	printf("placeontop dir %s, %d\n",dir->name,dir->dirs_opened);
	switch (dir->dirs_opened)
	{
		case 0: return;
		case 1:
			for (i=0;i<dir->n_item;i++)
			{
				if (dir->list[i].disp)
				{
					printf("raising %d\n", i);
					dir->list[i].px=0.0;
					dir->list[i].pz=0.0;
					s3d_translate(dir->list[i].block,0,1,0);
					s3d_scale(dir->list[i].block,0.2);
				}
			}
			break;
		default:
			j=0;
			for (i=0;i<dir->n_item;i++)
			{
				if (dir->list[i].disp)
				{
					printf("raising %d\n", i);
					dir->list[i].px=0.8 * sin(((float)j*2*M_PI)/((float)dir->dirs_opened));
					dir->list[i].pz=0.8 * cos(((float)j*2*M_PI)/((float)dir->dirs_opened));
					s3d_translate(dir->list[i].block,dir->list[i].px,1,dir->list[i].pz);
					s3d_scale(dir->list[i].block,0.2);
					j++;
				}

			}
	}
}
int display_dir(struct t_item *dir)
{
	int i;
	float  px,pz;
	int dirn, dps;
	float vertices[]={	-1,-0.5,0,
						-1, 0.5,0,
						 1, 0.5,0,
						 1,-0.5,0};
	float d;
	px=pz=0.0;
	if (dir->disp)
		return(-1); /* already displayed ... */ 
	s3d_del_object(dir->block);
	new_block(dir);
	if (dir->parent!=NULL)
		dir->parent->dirs_opened++;
	dir->dirs_opened=0;
 /* count directories */
	dirn=0;
	for (i=0;i<dir->n_item;i++)
	{
		if (dir->list[i].type==T_FOLDER)
			dirn++;
	}
	dps=ceil(sqrt(dir->n_item)); /* directories per line */
	printf("directories per line: %d\n",dps);
	for (i=0;i<dir->n_item;i++)
	{
		dir->list[i].px=((float)((int)i%dps)+0.5)/((float)dps)-0.5;
		dir->list[i].pz=((float)((int)i/dps)+0.5)/((float)dps)-0.5;
		dir->list[i].block=s3d_new_object();
		s3d_link(dir->list[i].block,dir->block);
		s3d_push_vertices(dir->list[i].block,vertices,4);
		d=((int)i%2)*0.2;
		switch (dir->list[i].type)
		{
			case T_FOLDER:
				s3d_push_material(dir->list[i].block,
										1-d,1-d,0,
										1-d,1-d,0,
										1-d,1-d,0);
				break;
			default:
				s3d_push_material(dir->list[i].block,
										0,0,1-d,
										0,0,1-d,
										0,0,1-d);
		};
		s3d_push_polygon(dir->list[i].block,0,1,2,0);
		s3d_push_polygon(dir->list[i].block,0,2,3,0);
		s3d_scale(dir->list[i].block,(float)1.0/((float)dps));
		s3d_translate(dir->list[i].block,dir->list[i].px*2,dir->list[i].pz+0.5,1.0);
	}
	dir->disp=1;
	if (dir->parent!=NULL)
	{
		s3d_link(dir->block,dir->parent->block);
		placeontop(dir->parent);
	}
	for (i=0;i<dir->n_item;i++)
		s3d_flags_on(dir->list[i].block,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->block,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	return(0);
}
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
			list[n].type=T_DUNO;
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
			list[n].n_item=-1;
			list[n].parent=dir;
			list[n].disp=0;
			list[n].icon=-1;
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
	if (t->block==oid)
		return(t);
	if (t->type==T_FOLDER)
		for (i=0;i<t->n_item;i++)
			if ((f=finditem(&(t->list[i]),oid))!=NULL)
				return(f);
	return(NULL);
}
/* gets the scale by multiplying scales */

float get_scale(struct t_item *f)
{
	float scale,s;
	s=0.2;
	scale=1.0/s;
	if (f->parent!=NULL)
		scale=1/s*get_scale(f->parent);
	else
		return(1.0);
	px-=f->px;
	pz-=f->pz;
	py-=1;
	printf("[S]cale factor of %s: %f\n",f->name,1/s);
	px*=1/s;
	py*=1/s;
	pz*=1/s;
	
	return(scale);
}
#define SCALE 	1
void rescale(struct t_item *f)
{
	px=0.0;
	py=0.0;
	pz=0.0;
	printf("[Z]ooming to %s\n",f->name);
	scale=get_scale(f);
	printf("[R]escaling to %f\n",scale);
	printf("px: %f py:%f pz: %f\n",px,py,pz);
}

void object_click(struct s3d_evt *evt)
{
	int oid;
	struct t_item *f;
	oid=(int)*((unsigned long *)evt->buf);
	if (NULL!=(f=finditem(&root,oid)))
	{
		if (f->type==T_FOLDER)
		{
			printf("[F]ound, expanding %s\n",f->name);
			parse_dir(f);
			display_dir(f);
			rescale(f);
		} else
			printf("[F]ound, but is no folder\n");
	} else {
		printf("[C]ould not find :/\n");
	}
}
#define ZOOMS 	5
void mainloop()
{
	dpx=(px+dpx*ZOOMS)/(ZOOMS+1);
	dpy=(py+dpy*ZOOMS)/(ZOOMS+1);
	dpz=(pz+dpz*ZOOMS)/(ZOOMS+1);
	dscale=(scale+dscale*ZOOMS)/(ZOOMS+1);
	s3d_translate(root.block,dpx*SCALE,-3.0+SCALE*dpy,dpz*SCALE);
	s3d_scale(root.block,dscale*SCALE);

	nanosleep(&t,NULL); 
}
int main (int argc, char **argv)
{
	int i;
	px=py=pz=0.0;
	dpx=dpy=dpz=0.0;
	dscale=scale=1.0;
	if (!s3d_init(&argc,&argv,"filebrowser"))	
	{
		i=0;
		 /*  load the object files */
		folder=s3d_import_3ds_file("objs/folder.3ds");
		geometry=s3d_import_3ds_file("objs/geometry.3ds");
		mp3=s3d_import_3ds_file("objs/notes.3ds");
		duno=s3d_import_3ds_file("objs/duno.3ds");
		dot=s3d_import_3ds_file("objs/dot.3ds");
		dotdot=s3d_import_3ds_file("objs/dotdot.3ds");
		s3d_select_font("vera");
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
		
		/* set up file system representation */
		strncpy(root.name,"/",M_NAME);
		root.parent=NULL;
		root.type=T_FOLDER;
		root.px=root.pz=0.0;
		root.dirs_opened=0;
		parse_dir(&root);
		rescale(&root);
		display_dir(&root);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
