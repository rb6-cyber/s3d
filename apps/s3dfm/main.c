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
static struct timespec t={0,100*1000*1000}; /* 100 mili seconds */


#define SH			0.4 /* height of the step */

#define T_DUNO		0
#define T_LOCALDIR	1
#define T_BACKDIR	2
#define T_FOLDER	3
#define T_GEOMETRY	4
#define T_MUSIC		5

#define M_DIR		512
#define M_NAME		256
int folder,geometry,mp3,duno,dot,dotdot,dirstep;
struct t_item {
	int icon, str;							/* object ids ...*/
	char name[M_NAME];						/* name (e.g. file name) */
	struct t_item *parent;					/* parent item */
	struct t_item *list;					/* list of items  (if it's a subdir)*/
	float scale;
	float px,pz;
	int n_item;								/* number of items in list ( = -1 for normal or not-expanded files) */
	int type;								/* type, determined by extension or file type like dir, pipe, link etc */
	int disp;
};
struct t_item root;

int display_dir(struct t_item *dir)
{
	int i;
	float  px,pz;
	float dss; /* dirstep size */
	int dirn, dirc,dps;
	int icon;
	px=pz=0.0;
	if (dir->disp)
		return(-1); /* already displayed ... */ 

 /* count directories */
	dirn=0;
	for (i=0;i<dir->n_item;i++)
	{
		if (dir->list[i].type==T_FOLDER)
			dirn++;
	}
	dps=(dirn+3)/4; /* directories per side */
	dirc=0;			/* no directories counted yet */
	dss=1.0/(2.0*dps+1.0);
	dir->scale=dss;
	printf("having %d directories, %d per side. one is scaled down to %3.3f \n",dirn,dps,dss);
	for (i=0;i<dir->n_item;i++)
	{
		if (dir->list[i].type==T_FOLDER)
		{
			icon=dir->list[i].icon=s3d_clone(dirstep);
			printf("processing %d [%s] icon %d\n",dirc,dir->list[i].name,icon);
			s3d_scale(icon,dss);
		    s3d_flags_on(icon,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_link(icon,dir->icon);
			/* castle style:
			 * direcotires are clockwise oredered, starting from the lower left corner on the 
			 * underlying dirstep */
			switch (dirc/dps)
			{
				case 0:
					px=-1.0+dss;
					pz=-1.0+(dirc*2+0.5)*2*dss;
					break;
				case 1:
					px=-1.0+((dirc-dps)*2+0.5)*2*dss;
					pz=1.0-dss;
					break;
				case 2:
					px=1.0-dss;
					pz=1.0-((dirc-2*dps)*2+0.5)*2*dss;
					break;
				case 3:
					px=1.0-((dirc-dps*3)*2+0.5)*2*dss;
					pz=-1.0+dss;
					break;
			}
			s3d_translate(icon, px,SH,pz);
			dir->list[i].px=px;
			dir->list[i].pz=pz;
			dir->list[i].str=s3d_draw_string(dir->list[i].name,NULL);
			s3d_translate(	dir->list[i].str, px,1.0+SH,pz);
			s3d_link(		dir->list[i].str,icon);
			s3d_flags_on(	dir->list[i].str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			dirc++;
			
        }
	}
	dir->disp=1;
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
			list[n].scale=1.0f;
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
	if (t->icon==oid)
		return(t);
	if (t->type==T_FOLDER)
		for (i=0;i<t->n_item;i++)
			if ((f=finditem(&(t->list[i]),oid))!=NULL)
				return(f);
	return(NULL);
}
float px, py, pz;
/* gets the scale by multiplying scales */
float get_scale(struct t_item *f)
{
	float scale;
	scale=1/f->scale;
	if (f->parent!=NULL)
		scale=1/f->scale*get_scale(f->parent);
	px+=f->px;
	pz+=f->pz;
	py+=SH;
	printf("scale factor of %s: %f\n",f->name,1/f->scale);
	px*=1/f->scale;
	py*=1/f->scale;
	pz*=1/f->scale;
	
	
	return(scale);
}
#define SCALE 	1
void rescale(struct t_item *f)
{
	float scale=1.0;
	px=0.0;
	py=0.0;
	pz=0.0;
	if (f->parent!=NULL)
	{
		scale=get_scale(f);
	}
	s3d_scale(root.icon,scale*SCALE);
	printf("rescaling to %f\n",scale);
	printf("px: %f py:%f pz: %f\n",px,py,pz);
	s3d_translate(root.icon,px*SCALE,-3.0-SCALE*py,pz*SCALE);
}

void object_click(struct s3d_evt *evt)
{
	int oid;
	struct t_item *f;
	oid=(int)*((unsigned long *)evt->buf);
	printf("! clicked object %d\n",oid);
	if (NULL!=(f=finditem(&root,oid)))
	{
		if (f->type==T_FOLDER)
		{
			printf("found, expanding %s\n",f->name);
			parse_dir(f);
			display_dir(f);
			rescale(f);
		}
	}
}
void mainloop()
{
	nanosleep(&t,NULL); 
}
int main (int argc, char **argv)
{
	int i;
	if (!s3d_init(&argc,&argv,"filebrowser"))	
	{
		i=0;
		 /*  load the object files */
		dirstep=s3d_import_3ds_file("objs/dirstep.3ds");
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
		root.icon=s3d_clone(dirstep);
		root.px=root.pz=0.0;
		root.scale=1.0;
		rescale(&root);
	    s3d_flags_on(root.icon,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		parse_dir(&root);
		display_dir(&root);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
