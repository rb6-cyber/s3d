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

void box_draw(t_node *dir)
{
	box_buildblock(dir);
	box_sidelabel(dir);
    s3d_flags_on(dir->oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->objs.close,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->objs.title,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->objs.select,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
    s3d_flags_on(dir->objs.titlestr,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	dir->disp=D_DIR;
	box_draw_icons(dir);

}
/* draw all the icons which are not displayed yet */
void box_draw_icons(t_node *dir)
{
	int i;
	printf("box_draw_icons(%s, %d subs)\n",dir->name, dir->n_sub);
	for (i=0;i<dir->n_sub;i++)
	{
		if (dir->sub[i]->disp==D_NONE)	icon_draw(dir->sub[i]);
	}
	box_order_icons(dir);
}
/* order the icons properly */
void box_order_icons(t_node *dir)
{
	int dps,i;
	dps=ceil(sqrt(dir->n_sub)); /* directories per line */
	for (i=0;i<dir->n_sub;i++)
	{
		printf("ordering icon %s\n",dir->sub[i]->name);
		dir->sub[i]->dpx = -1 +2*  ((float)((int)i%dps)+0.5)/((float)dps);
		dir->sub[i]->dpy = 0.5+((float)((int)i/dps)+0.5)/((float)dps)-0.5;
		dir->sub[i]->dpz = 1.0;
		dir->sub[i]->scale = (float)1.0/((float)dps);
		dir->sub[i]->dscale = 0.001;
		dir->sub[i]->dpx = 0;
		dir->sub[i]->dpy = 0;
		dir->sub[i]->dpz = 1;
		/* make a first setup so there is no flickering */
		s3d_link(dir->sub[i]->oid,dir->oid); /* if it's already displayed, make sure it linked properly ... */
		ani_doit(dir->sub[i]);
		ani_add(dir->sub[i]); /* apply transformation */
		s3d_flags_on(dir->sub[i]->oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		s3d_flags_on(dir->sub[i]->objs.str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

		
	}
}

/* remove old items on the box */
/*
void box_dissolve(t_node *dir)
{
	if (dir->objs.close!=-1)		{	s3d_del_object(dir->objs.close);		dir->objs.close=-1; }
	if (dir->objs.select!=-1)	{	s3d_del_object(dir->objs.select);	dir->objs.select=-1; }
	if (dir->objs.title!=-1)		{	s3d_del_object(dir->objs.title);		dir->objs.title=-1; }
	if (dir->objs.titlestr!=-1)	{	s3d_del_object(dir->objs.titlestr);	dir->objs.titlestr=-1; }
	if (dir->oid!=-1)			s3d_del_object(dir->oid);

}*/



/* places the string at the left side of the cube */
void box_sidelabel(t_node *dir)
{

	s3d_rotate(dir->objs.str,0,90,0);
	s3d_translate(dir->objs.str,1.1,0.3,1);
	s3d_scale(dir->objs.str,(float)1.8/(dir->len));
	s3d_scale(dir->objs.str,(float)1.8/(dir->len));
	s3d_link(dir->objs.str,dir->oid);
	s3d_flags_on(dir->objs.str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
}

/* creates a big block which will hold files and subdirs on top */
int box_buildblock(t_node *dir)
{
	char fname[30];
	char *fullname=fname;
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

	dir->oid=s3d_new_object();
	
	/* draw block outside */
	s3d_push_vertices(dir->oid,vertices,sizeof(vertices)/(3*sizeof(float)));
	s3d_push_material(dir->oid,
						0.5,0.5,0.5,
						0.5,0.5,0.5,
						0.5,0.5,0.5
					);
	s3d_push_material(dir->oid,
						0.5,0.5,0.6,
						0.5,0.5,0.6,
						0.5,0.5,0.6);

	s3d_push_polygon(dir->oid,4,6,5,1);
	s3d_push_polygon(dir->oid,4,7,6,1);

	s3d_push_polygon(dir->oid,0,4,5,0);
	s3d_push_polygon(dir->oid,0,5,1,0);
	
	s3d_push_polygon(dir->oid,3,7,4,0);
	s3d_push_polygon(dir->oid,3,4,0,0);

	s3d_push_polygon(dir->oid,2,6,7,0);
	s3d_push_polygon(dir->oid,2,7,3,0);
	
	s3d_push_polygon(dir->oid,8,9,10,0);
	s3d_push_polygon(dir->oid,8,10,11,0);

	/* draw the select, close buttons ... */
	dir->objs.close=s3d_new_object();
	s3d_push_material(dir->objs.close,
						0.5,0.3,0.3,
						0.5,0.3,0.3,
						0.5,0.3,0.3
					);
	s3d_push_vertices(dir->objs.close,xvertices,sizeof(xvertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->objs.close,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->objs.close,dir->oid);
	
	dir->objs.select=s3d_new_object();
	s3d_push_material(dir->objs.select,
						0.1,0.1,0.3,
						0.1,0.1,0.3,
						0.1,0.1,0.3
					);
	s3d_push_vertices(dir->objs.select,svertices,sizeof(svertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->objs.select,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->objs.select,dir->oid);
	
	/* draw the title string */
	
	dir->objs.title=s3d_new_object();
	s3d_push_material(dir->objs.title,
						0.3,0.3,0.3,
						0.3,0.3,0.3,
						0.3,0.3,0.3
					);
	s3d_push_vertices(dir->objs.title,tvertices,sizeof(tvertices)/(3*sizeof(float)));
	s3d_push_polygons(dir->objs.title,bar_poly,sizeof(bar_poly)/(sizeof(unsigned long)*4));
	s3d_link(dir->objs.title,dir->oid);
	dots_at_start(fullname,30,dir);
	dir->objs.titlestr=s3d_draw_string(fullname,&len);
	if (len>(1.6*5.0))		s3d_scale(dir->objs.titlestr,1.6/len);
	else					s3d_scale(dir->objs.titlestr,0.2);
	s3d_translate(dir->objs.titlestr,-1.0,1.05,1.01);
	s3d_link(dir->objs.titlestr,dir->oid);
	dir->disp=D_DIR;
/*	printf("FULLNAME is [%s]\n",fullname);*/
	return(0);
}
/* display a directoy on the top of another, draw it's icons etc ... */
/*
int box_expand(t_node *dir)
{
	int i;
	float  px,pz;
	int dirn;
	px=pz=0.0;
	printf("box_expand( %s )\n",dir->name);
	if (dir->disp)		undisplay(dir);
	box_buildblock(dir);
	if (dir->parent!=NULL)
		dir->parent->dirs_opened++;
 / * count directories * /
	dirn=0;
	for (i=0;i<dir->n_sub;i++)
	{
		if (dir->sub[i].type==T_FOLDER) dirn++;
	}

	/ * draw icons, if necceasry * /
	for (i=0;i<dir->n_sub;i++)
	{
		if (!dir->sub[i].disp)	icon_draw(dir,i);
		else {
			printf("link %d to the block %d of %s\n",dir->sub[i].block,dir->oid,dir->name);
			s3d_link(dir->sub[i].block,dir->oid); / * if it's already displayed, make sure it linked properly ... * /
		}
	}
	if (dir->parent!=NULL)
	{
		s3d_link(dir->oid,dir->parent->block);
		dir->dpx=0.0;
		dir->dpy=BOXHEIGHT;
		dir->dpz=0.0;
		dir->dscale=0.0;
		box_position_kids(dir->parent);
		ani_doit(dir);
	}
	box_sidelabel(dir);
	return(0);
}

*/
int box_undisplay(t_node *dir)
{
	/*
	int i;
	t_node *par;
	printf("box_undisplay( %s )\n",dir->name);
	for (i=0;i<dir->n_sub;i++)
	{
		if (dir->sub[i].disp==D_ICON)	icon_undisplay(&(dir->sub[i]));
		else if (dir->sub[i].disp!=0)	
				printf("not undisplaying: %s (disp = %d)\n",dir->sub[i].name, dir->sub[i].disp);
	}
	if ((par=dir->parent)!=NULL) / * we can't do this on root.... * /
	{
		for (i=0;i<par->n_item;i++)
			if (&par->list[i]==dir)
				break;
		if (i!=par->n_item) / * if it actually was in the parents item list * /
		{
			icon_draw(par,i);
			s3d_flags_on(dir->oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_flags_on(dir->objs.str,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		}
		par->dirs_opened--;
	} else {
		/ * we're root ... * /
		box_dissolve(dir);
	}
	printf("[U]ndisplayed %s\n",dir->name);
/ * 	dir->dirs_opened=0;* /
	dir->detached=0;
	*/
	return(0);
}
/* undisplay a directory, thus recursively removing the kids.*/
/*
int box_collapse(t_node *dir,int force)
{
	int i;
	int ret;
	printf("box_collapse( %s )\n",dir->name);
	if (&root==dir)
	{
		printf("won't undisplay root window ... \n");
		return(-1);
	}
	if (dir->detached && !force)
		return(1);
	if (dir->disp!=D_DIR)
	{
		printf("[A]lready undisplayed %s, nothing to do ...\n",dir->name);
		return(-1);
	}
	/ * undisplaying kids. ret will be != 0 if any of the kids did not close correctly * /
	ret=0;
	for (i=0;i<dir->n_sub;i++)
		if (dir->sub[i].disp==D_DIR)
			ret|=box_collapse(&dir->sub[i],force);

	if (ret && !force) return(ret); / * if anything got wrong, return here ... * /
	undisplay(dir);
	if (dir->parent!=NULL)
	{
		box_position_kids(dir->parent);
	}
	return(ret);
}
/ * only display dir and its kids, but nothing below. * /
int box_collapse_grandkids(t_node *dir)
{
	int i,j;
	t_node *kid;
	for (i=0;i<dir->n_sub;i++)
		if (dir->sub[i].disp==D_DIR)
		{
			kid=&dir->sub[i];
			for (j=0;j<kid->n_item;j++)
			if (kid->list[j].disp==D_DIR)
				box_collapse(&kid->list[j],0);
		}
	return(0);
}*/
/* orders the directory objects on top of its parent objects 
 * to be called after adding or removing things ...*/
void box_order_subdirs(t_node *dir)
{
	int i,j;
	printf("box_order_subdirs( %s ): %d dirs opened\n",dir->name,dir->dirs_opened);
	switch (dir->dirs_opened)
	{
		case 0: return;
		case 1:
			for (i=0;i<dir->n_sub;i++)
			{
				if (dir->sub[i]->disp==D_DIR)
				{
					dir->sub[i]->px=0.0;
					dir->sub[i]->py=BOXHEIGHT+dir->sub[i]->detached*DETHEIGHT;
					dir->sub[i]->pz=0.0;
					dir->sub[i]->scale=0.2;
					ani_add(dir->sub[i]);
				}
			}
			break;
		default:
			j=0;
			for (i=0;i<dir->n_sub;i++)
			{
				if (dir->sub[i]->disp==D_DIR)
				{
					dir->sub[i]->px=0.8 * sin(((float)j*2*M_PI)/((float)dir->dirs_opened));
					dir->sub[i]->py=BOXHEIGHT+dir->sub[i]->detached*DETHEIGHT;
					dir->sub[i]->pz=0.8 * cos(((float)j*2*M_PI)/((float)dir->dirs_opened));
					dir->sub[i]->scale=0.2;
					ani_add(dir->sub[i]);
					j++;
				}

			}
	}
}
void box_select(t_node *dir)
{
	dir->detached=dir->detached?0:1; /* swapping, not sure if !dir->detached would do the same .. */
	if ((dir->type==T_FOLDER) && dir->disp)
	{
		if (dir->parent!=NULL)
			box_order_subdirs(dir->parent);
	} else {
		/* nothing yet ... */
	}
}
