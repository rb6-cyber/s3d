/*
 * box.c
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
#include <math.h>	 /*  sin(),cos() */
#include <string.h>  /*  strlen() */

/* clear the dirs attributes */
int box_init(struct t_item *dir)
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

	dir->px=root.pz=0.0;
	dir->dirs_opened=0;
	dir->type=T_DUNO;
	dir->px=dir->py=dir->pz=0.0;
	dir->dpx=dir->dpy=dir->dpz=0.0;
	dir->scale=dir->dscale=1.0;
	dir->detached=0;

	return(0);
}
/* draws icon i in the block of dir */
int box_icon(struct t_item *dir,int i)
{
	float vertices[]={	-1,-0.5,0,
						-1, 0.5,0,
						 1, 0.5,0,
						 1,-0.5,0,
						-1,-0.5,-1,
						-1, 0.5,-1,
						 1, 0.5,-1,
						 1,-0.5,-1};
	unsigned long polys[]={
				1,3,0,0,				2,3,1,0,
				5,6,2,0,				1,5,2,0,
				2,6,7,0,				2,7,3,0,
				0,3,7,0,				0,7,4,0,
				5,1,0,0,				5,0,4,0	
				};
	float len;
	float d;
	int dps;
	dps=ceil(sqrt(dir->n_item)); /* directories per line */
	/* find position for the new block in our directory box */
	dir->list[i].dpx = dir->list[i].px=-1 +2*  ((float)((int)i%dps)+0.5)/((float)dps);
	dir->list[i].dpy = dir->list[i].py=0.5+((float)((int)i/dps)+0.5)/((float)dps)-0.5;
	dir->list[i].dpz = dir->list[i].pz=1.0;
	dir->list[i].scale = dir->list[i].dscale = (float)1.0/((float)dps);
	/* create the block */
	if (dir->list[i].close!=-1)		{	s3d_del_object(dir->list[i].close);		dir->list[i].close=-1; }
	if (dir->list[i].select!=-1)	{	s3d_del_object(dir->list[i].select);	dir->list[i].select=-1; }
	if (dir->list[i].title!=-1)		{	s3d_del_object(dir->list[i].title);		dir->list[i].title=-1; }
	if (dir->list[i].titlestr!=-1)	{	s3d_del_object(dir->list[i].titlestr);	dir->list[i].titlestr=-1; }
	if (dir->list[i].block!=-1)			s3d_del_object(dir->list[i].block);
	dir->list[i].block=s3d_new_object();
	s3d_push_vertices(dir->list[i].block,vertices,8);
	d=((int)(((i+(dps+1)%2*(i/dps)))%2))*0.2;
	switch (dir->list[i].type)
	{
		case T_FOLDER:
			s3d_push_material(dir->list[i].block,
									0.4-d,0.4-d,0,
									0.4-d,0.4-d,0,
									0.4-d,0.4-d,0);
			break;
		default:
			s3d_push_material(dir->list[i].block,
									0,0,0.5-d,
									0,0,0.5-d,
									0,0,0.5-d);
	};
	s3d_push_polygons(dir->list[i].block,polys,10);
	s3d_link(dir->list[i].block,dir->block);

	/* draw and position the string */
	if (dir->list[i].str==-1)
	{
		dir->list[i].str=s3d_draw_string(dir->list[i].name,&len);
		if (len<2) len=2;
		dir->list[i].len=len;
	}
	else 
		len=dir->list[i].len;
	s3d_scale(dir->list[i].str,(float)1.8/len);
	s3d_translate(dir->list[i].str,-0.9,-0.3,0.1);
	s3d_rotate(dir->list[i].str,0,0,0);
	s3d_link(dir->list[i].str,dir->list[i].block);
	ani_finish(&dir->list[i],-1); /* apply transformation */
	return(0);
}


/* places the string at the left side of the cube */
void box_sidelabel(struct t_item *dir)
{

	s3d_rotate(dir->str,0,90,0);
	s3d_translate(dir->str,1.1,0.3,1);
	s3d_scale(dir->str,(float)1.8/(dir->len));
	s3d_scale(dir->str,(float)1.8/(dir->len));
	s3d_link(dir->str,dir->block);
	s3d_flags_on(dir->str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
}

/* creates a big block which will hold files and subdirs on top */
int box_buildblock(struct t_item *dir)
{
	char fname[30];
	char *fullname=fname;
	struct t_item *d;
	int i,j;
	float len;
	float vertices[]=
			{-BHP,0,-BHP,
			 -BHP,0, BHP,
			  BHP,0, BHP,
			  BHP,0,-BHP,
			 -BHP,BHH,-BHP,
			 -BHP,BHH, BHP,
			  BHP,BHH, BHP,
			  BHP,BHH,-BHP,
			 -1,0, 0.8,
			 -1,BOXHEIGHT, 0.8,
			  1,BOXHEIGHT, 0.8,
			  1,0, 0.8
				};
	float xvertices[]=
			{
			  0.8,BHH-0.2, 0.8,
			  0.8,BHH    , 0.8,
			  BHP,BHH    , 0.8,
			  BHP,BHH-0.2, 0.8,
			  0.8,BHH-0.2, 1.0,
			  0.8,BHH    , 1.0,
			  BHP,BHH    , 1.0,
			  BHP,BHH-0.2, 1.0
			 };
	float svertices[]=
			{
			  0.6,BHH-0.2, 0.8,
			  0.6,BHH    , 0.8,
			  0.8,BHH    , 0.8,
			  0.8,BHH-0.2, 0.8,
			  0.6,BHH-0.2, 1.0,
			  0.6,BHH    , 1.0,
			  0.8,BHH    , 1.0,
			  0.8,BHH-0.2, 1.0
			 };
	float tvertices[]=
			{
			  -BHP,BHH-0.2, 0.8,
			  -BHP,BHH    , 0.8,
			  0.6, BHH    , 0.8,
			  0.6, BHH-0.2, 0.8,
			  -BHP,BHH-0.2, 1.0,
			  -BHP,BHH    , 1.0,
			  0.6, BHH    , 1.0,
			  0.6, BHH-0.2, 1.0
			 };
	unsigned long bar_poly[]={
		4,5,6,0,
		4,6,7,0,
		3,7,4,0,
		3,4,0,0
	};
/*	printf("new block for %s\n",dir->name);*/

	dir->block=s3d_new_object();
	s3d_push_vertices(dir->block,vertices,sizeof(vertices)/(3*sizeof(float)));
	s3d_push_material(dir->block,
						0.5,0.5,0.5,
						0.5,0.5,0.5,
						0.5,0.5,0.5
					);
	s3d_push_material(dir->block,
						0.5,0.5,0.6,
						0.5,0.5,0.6,
						0.5,0.5,0.6);

	s3d_push_polygon(dir->block,4,6,5,1);
	s3d_push_polygon(dir->block,4,7,6,1);

	s3d_push_polygon(dir->block,0,4,5,0);
	s3d_push_polygon(dir->block,0,5,1,0);
	
	s3d_push_polygon(dir->block,3,7,4,0);
	s3d_push_polygon(dir->block,3,4,0,0);

	s3d_push_polygon(dir->block,2,6,7,0);
	s3d_push_polygon(dir->block,2,7,3,0);
	
	s3d_push_polygon(dir->block,8,9,10,0);
	s3d_push_polygon(dir->block,8,10,11,0);

	dir->close=s3d_new_object();
	s3d_push_material(dir->close,
						0.5,0.3,0.3,
						0.5,0.3,0.3,
						0.5,0.3,0.3
					);
	s3d_push_vertices(dir->close,xvertices,sizeof(xvertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->close,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->close,dir->block);
	
	dir->select=s3d_new_object();
	s3d_push_material(dir->select,
						0.1,0.1,0.3,
						0.1,0.1,0.3,
						0.1,0.1,0.3
					);
	s3d_push_vertices(dir->select,svertices,sizeof(svertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->select,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->select,dir->block);
	
	dir->title=s3d_new_object();
	s3d_push_material(dir->title,
						0.3,0.3,0.3,
						0.3,0.3,0.3,
						0.3,0.3,0.3
					);
	s3d_push_vertices(dir->title,tvertices,sizeof(tvertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->title,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->title,dir->block);
	i=28;
	fullname[29]=0;
	d=dir;
	do {
		j=strlen(d->name)-1;
		if (NULL!=(d->parent))
		{
			fullname[i]='/';
			i--;
		}
		while ((i >= 0) && (j >= 0))
		{
			fullname[i]=d->name[j];
			j--;
			i--;
		}
		if (i<0) 
			break;


	} while ((d=d->parent)!=NULL);
	if (i<0)
		fullname[0]=fullname[1]='.';
	else 
		fullname=(char *)fullname+i+1; /* jump to start of the string */
	dir->titlestr=s3d_draw_string(fullname,&len);
	if (len>(1.6*5.0))
		s3d_scale(dir->titlestr,1.6/len);
	else
		s3d_scale(dir->titlestr,0.2);
	s3d_translate(dir->titlestr,-1.0,1.05,1.01);
	s3d_link(dir->titlestr,dir->block);
/*	printf("FULLNAME is [%s]\n",fullname);*/
	return(0);
}

/* display a directoy on the top of another, draw it's icons etc ... */
int box_expand(struct t_item *dir)
{
	int i;
	float  px,pz;
	int dirn;
	px=pz=0.0;
	if (dir->disp)
		return(-1); /* already displayed ... */ 
	s3d_del_object(dir->block);
	box_buildblock(dir);
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
	for (i=0;i<dir->n_item;i++)
	{
		box_icon(dir,i);
	}
	dir->disp=1;
	if (dir->parent!=NULL)
	{
		s3d_link(dir->block,dir->parent->block);
		dir->dpx=0.0;
		dir->dpy=BOXHEIGHT;
		dir->dpz=0.0;
		dir->dscale=0.0;
		box_position_kids(dir->parent);
		ani_doit(dir);
	}
	for (i=0;i<dir->n_item;i++)
	{
		s3d_flags_on(dir->list[i].block,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		s3d_flags_on(dir->list[i].str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	}
	box_sidelabel(dir);
    s3d_flags_on(dir->block,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->close,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->title,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->select,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->titlestr,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	return(0);
}

/* undisplay a directory, thus recursively removing the kids.*/
int box_collapse(struct t_item *dir,int force)
{
	int i;
	int ret;
	struct t_item *par;
	if (&root==dir)
	{
		printf("won't undisplay root window ... \n");
		return(-1);
	}
	if (dir->detached && !force)
		return(1);
	if (dir->disp==0)
	{
/*		printf("[A]lready undisplayed, nothing to do ...\n");*/
		return(-1);
	}
	ret=0;
	for (i=0;i<dir->n_item;i++)
		if (dir->list[i].disp)
			ret|=box_collapse(&dir->list[i],force);

	if (ret && !force) return(ret);
	for (i=0;i<dir->n_item;i++)
	{
		if (dir->list[i].block!=-1)
		{
			s3d_del_object(dir->list[i].block);
			dir->list[i].block=-1;
		}
		if (dir->list[i].str!=-1)
		{
			s3d_del_object(dir->list[i].str);
			dir->list[i].str=-1;
		}
	}
	if ((par=dir->parent)!=NULL) /* should never be because there we don't process root */
	{
		for (i=0;i<par->n_item;i++)
			if (&par->list[i]==dir)
				break;
		if (i!=par->n_item)
		{
			box_icon(par,i);
			s3d_flags_on(dir->block,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_flags_on(dir->str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		}
		par->dirs_opened--;
	}
	dir->dirs_opened=0;
	dir->disp=0;
	dir->detached=0;
	if (dir->parent!=NULL)
	{
		box_position_kids(dir->parent);
	}
	return(ret);
}
/* only display dir and its kids, but nothing below. */
int box_collapse_grandkids(struct t_item *dir)
{
	int i,j;
	struct t_item *kid;
	for (i=0;i<dir->n_item;i++)
		if (dir->list[i].disp)
		{
			kid=&dir->list[i];
			for (j=0;j<kid->n_item;j++)
				box_collapse(&kid->list[j],0);
		}
	return(0);
}
/* orders the directory objects on top of its parent objects 
 * to be called after adding or removing things ...*/
void box_position_kids(struct t_item *dir)
{
	int i,j;
/*	printf("placeontop dir %s, %d\n",dir->name,dir->dirs_opened);*/
	switch (dir->dirs_opened)
	{
		case 0: return;
		case 1:
			for (i=0;i<dir->n_item;i++)
			{
				if (dir->list[i].disp)
				{
					dir->list[i].px=0.0;
					dir->list[i].py=BOXHEIGHT+dir->list[i].detached*DETHEIGHT;
					dir->list[i].pz=0.0;
					dir->list[i].scale=0.2;
					ani_add(&dir->list[i]);
				}
			}
			break;
		default:
			j=0;
			for (i=0;i<dir->n_item;i++)
			{
				if (dir->list[i].disp)
				{
					dir->list[i].px=0.8 * sin(((float)j*2*M_PI)/((float)dir->dirs_opened));
					dir->list[i].py=BOXHEIGHT+dir->list[i].detached*DETHEIGHT;
					dir->list[i].pz=0.8 * cos(((float)j*2*M_PI)/((float)dir->dirs_opened));
					dir->list[i].scale=0.2;
					ani_add(&dir->list[i]);
					j++;
				}

			}
	}
}
void box_select(struct t_item *dir)
{
	dir->detached=dir->detached?0:1; /* swapping, not sure if !dir->detached would do the same .. */
	if ((dir->type==T_FOLDER) && dir->disp)
	{
		if (dir->parent!=NULL)
			box_position_kids(dir->parent);
	} else {
		/* nothing yet ... */
	}
}
