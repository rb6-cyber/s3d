/*
 * box.c
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
#include <stdio.h>   /*  printf() */
#include <math.h>  /*  sin(),cos() */
#include <string.h>  /*  strlen() */

void box_draw(t_node *dir)
{
	box_buildblock(dir);
	box_sidelabel(dir);
	ani_doit(dir);
	s3d_flags_on(dir->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(dir->objs.close, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(dir->objs.title, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(dir->objs.select, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(dir->objs.titlestr, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	dir->disp = D_DIR;
	box_draw_icons(dir);
}

/* draw all the icons which are not displayed yet */
void box_draw_icons(t_node *dir)
{
	int i;
	printf("box_draw_icons(%s, %d subs)\n", dir->name, dir->n_sub);
	for (i = 0;i < dir->n_sub;i++) {
		if (dir->sub[i]->disp == D_NONE) icon_draw(dir->sub[i]);
	}
	box_order_icons(dir);
}


/* places the string at the left side of the cube */
void box_sidelabel(t_node *dir)
{
	float len;
	if (dir->objs.str == -1) {
		dir->objs.str = s3d_draw_string(dir->name, &len);
		if (len < 2) len = 2;
		dir->objs.strlen = len;
	}
	s3d_rotate(dir->objs.str, 0, 90, 0);
	s3d_translate(dir->objs.str, 1.1, 0.3, 1);
	s3d_scale(dir->objs.str, (float)1.8 / (dir->objs.strlen));
	s3d_scale(dir->objs.str, (float)1.8 / (dir->objs.strlen));
	s3d_link(dir->objs.str, dir->oid);
	s3d_flags_on(dir->objs.str, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* gives another color for the focused box */
void box_focus_color(t_node *dir, int on)
{

	s3d_pep_material(dir->oid,
	                 0.5 + on*0.3, 0.5 + on*0.3, 0.5 + on*0.3,
	                 0.5 + on*0.3, 0.5 + on*0.3, 0.5 + on*0.3,
	                 0.5 + on*0.3, 0.5 + on*0.3, 0.5 + on*0.3);

}
/* creates a big block which will hold files and subdirs on top */
int box_buildblock(t_node *dir)
{
	char fname[30];
	char *fullname = fname;
	float len;
	float vertices[] = { -BHP, 0, -BHP,
	                     -BHP, 0, BHP,
	                     BHP, 0, BHP,
	                     BHP, 0, -BHP,
	                     -BHP, BHH, -BHP,
	                     -BHP, BHH, BHP,
	                     BHP, BHH, BHP,
	                     BHP, BHH, -BHP,
	                     -1, 0, 0.8,
	                     -1, BOXHEIGHT, 0.8,
	                     1, BOXHEIGHT, 0.8,
	                     1, 0, 0.8
	                   };
	float xvertices[] = {
		0.8, BHH - 0.2, 0.8,
		0.8, BHH    , 0.8,
		BHP, BHH    , 0.8,
		BHP, BHH - 0.2, 0.8,
		0.8, BHH - 0.2, 1.0,
		0.8, BHH    , 1.0,
		BHP, BHH    , 1.0,
		BHP, BHH - 0.2, 1.0
	};
	float svertices[] = {
		0.6, BHH - 0.2, 0.8,
		0.6, BHH    , 0.8,
		0.8, BHH    , 0.8,
		0.8, BHH - 0.2, 0.8,
		0.6, BHH - 0.2, 1.0,
		0.6, BHH    , 1.0,
		0.8, BHH    , 1.0,
		0.8, BHH - 0.2, 1.0
	};
	float tvertices[] = {
		-BHP, BHH - 0.2, 0.8,
		-BHP, BHH    , 0.8,
		0.6, BHH    , 0.8,
		0.6, BHH - 0.2, 0.8,
		-BHP, BHH - 0.2, 1.0,
		-BHP, BHH    , 1.0,
		0.6, BHH    , 1.0,
		0.6, BHH - 0.2, 1.0
	};
	uint32_t bar_poly[] = {
		4, 5, 6, 0,
		4, 6, 7, 0,
		3, 7, 4, 0,
		3, 4, 0, 0
	};
	/* printf("new block for %s\n",dir->name);*/

	dir->oid = s3d_new_object();

	/* draw block outside */
	s3d_push_vertices(dir->oid, vertices, sizeof(vertices) / (3*sizeof(float)));
	s3d_push_material(dir->oid,
	                  0.5, 0.5, 0.5,
	                  0.5, 0.5, 0.5,
	                  0.5, 0.5, 0.5
	                 );

	/* top */
	s3d_push_polygon(dir->oid, 4, 6, 5, 0);
	s3d_push_polygon(dir->oid, 4, 7, 6, 0);
	/* bottom */
	s3d_push_polygon(dir->oid, 8, 11, 3, 0);
	s3d_push_polygon(dir->oid, 8, 3, 0, 0);


	/* left */
	s3d_push_polygon(dir->oid, 0, 4, 5, 0);
	s3d_push_polygon(dir->oid, 0, 5, 1, 0);

	/* back */
	s3d_push_polygon(dir->oid, 3, 7, 4, 0);
	s3d_push_polygon(dir->oid, 3, 4, 0, 0);

	/* right */
	s3d_push_polygon(dir->oid, 2, 6, 7, 0);
	s3d_push_polygon(dir->oid, 2, 7, 3, 0);

	/* front */
	s3d_push_polygon(dir->oid, 8, 9, 10, 0);
	s3d_push_polygon(dir->oid, 8, 10, 11, 0);
	/* left inner side */
	s3d_push_polygon(dir->oid, 1, 5, 9, 0);
	s3d_push_polygon(dir->oid, 1, 9, 8, 0);

	/* right inner side */
	s3d_push_polygon(dir->oid, 2, 11, 10, 0);
	s3d_push_polygon(dir->oid, 2, 10, 6, 0);

	/* top inner side */
	s3d_push_polygon(dir->oid, 9, 5, 6, 0);
	s3d_push_polygon(dir->oid, 9, 6, 10, 0);




	/* draw the select, close buttons ... */
	dir->objs.close = s3d_new_object();
	s3d_push_material(dir->objs.close,
	                  0.5, 0.3, 0.3,
	                  0.5, 0.3, 0.3,
	                  0.5, 0.3, 0.3
	                 );
	s3d_push_vertices(dir->objs.close, xvertices, sizeof(xvertices) / (3*sizeof(float)));
	s3d_push_polygons(dir->objs.close, bar_poly, sizeof(bar_poly) / (sizeof(uint32_t)*4));
	s3d_link(dir->objs.close, dir->oid);

	dir->objs.select = s3d_new_object();
	s3d_push_material(dir->objs.select,
	                  0.1, 0.1, 0.3,
	                  0.1, 0.1, 0.3,
	                  0.1, 0.1, 0.3
	                 );
	s3d_push_vertices(dir->objs.select, svertices, sizeof(svertices) / (3*sizeof(float)));
	s3d_push_polygons(dir->objs.select, bar_poly, sizeof(bar_poly) / (sizeof(uint32_t)*4));
	s3d_link(dir->objs.select, dir->oid);

	/* draw the title string */

	dir->objs.title = s3d_new_object();
	s3d_push_material(dir->objs.title,
	                  0.3, 0.3, 0.3,
	                  0.3, 0.3, 0.3,
	                  0.3, 0.3, 0.3
	                 );
	s3d_push_vertices(dir->objs.title, tvertices, sizeof(tvertices) / (3*sizeof(float)));
	s3d_push_polygons(dir->objs.title, bar_poly, sizeof(bar_poly) / (sizeof(uint32_t)*4));
	s3d_link(dir->objs.title, dir->oid);
	dir->objs.titlestr = s3d_draw_string(dots_at_start(fullname, 30, dir), &len);
	if (len > (1.6*5.0))  s3d_scale(dir->objs.titlestr, 1.6 / len);
	else     s3d_scale(dir->objs.titlestr, 0.2);
	s3d_translate(dir->objs.titlestr, -1.0, 1.05, 1.01);
	s3d_link(dir->objs.titlestr, dir->oid);
	dir->disp = D_DIR;
	/* printf("FULLNAME is [%s]\n",fullname);*/
	return(0);
}
/* display a directoy on the top of another */
int box_expand(t_node *dir)
{
	printf("box_expand( %s )\n", dir->name);
	switch (dir->disp) {
	case D_DIR:
		return(0); /* already done */
	case D_ICON:
		icon_undisplay(dir); /* undisplay previously displayed types, like icons etc */
		break;
	case D_NONE:
		break; /* ignore */
	default:
		return(-1); /* panic */
	}
	dir->dpx = 0.0;
	dir->dpy = BOXHEIGHT;
	dir->dpz = 0.0;
	dir->dscale = 0.01;
	box_draw(dir);

	/* initialize position on the parent */
	if (dir->parent != NULL) {
		dir->parent->dirs_opened++;
		s3d_link(dir->oid, dir->parent->oid);
		box_order_subdirs(dir->parent);
	}
	return(0);
}
/* remove s3d-objects of a directory node */
int box_undisplay(t_node *dir)
{
	printf("box_undisplay( %s )\n", dir->name);
	if (dir->objs.close != -1)  {
		s3d_del_object(dir->objs.close);
		dir->objs.close = -1;
	}
	if (dir->objs.select != -1)  {
		s3d_del_object(dir->objs.select);
		dir->objs.select = -1;
	}
	if (dir->objs.title != -1)  {
		s3d_del_object(dir->objs.title);
		dir->objs.title = -1;
	}
	if (dir->objs.titlestr != -1)  {
		s3d_del_object(dir->objs.titlestr);
		dir->objs.titlestr = -1;
	}
	if (dir->oid != -1)    {
		s3d_del_object(dir->oid);
	}
	/* keep this. icons also needs the *same* string */
	/* if (dir->objs.str!=-1)   {  s3d_del_object(dir->objs.str); dir->objs.str=-1; }*/
	dir->disp = D_NONE;
	return(0);
}
/* the opposite effect of box_expand, e.g. transforming the box back to an icon */
int box_unexpand(t_node *dir)
{
	printf("box_unexpand( %s )\n", dir->name);
	if (dir->parent == NULL) /* we can't do this on root.... */
		return(-1);
	dir->detached = 0;
	box_undisplay(dir);
	icon_draw(dir);
	dir->parent->dirs_opened--;
	box_order_icons(dir->parent);
	box_order_subdirs(dir->parent);
	return(0);
}

/* undisplay a directory, thus recursively removing the kids.*/
/* if force is 1, even the directory is removed even if it still have selected kids */
int box_close(t_node *dir, int force)
{
	int i;
	int ret;
	printf("box_close( %s )\n", dir->name);
	if (&root == dir) {
		printf("won't close down root box ... \n");
		return(-1);
	}
	if (dir->detached && !force) return(1);
	if (dir->disp != D_DIR) { /* that should not be happening ... */
		printf("[A]lready undisplayed %s, nothing to do ...\n", dir->name);
		return(-1);
	}
	/* closing kids. ret will be != 0 if any of the kids did not close correctly */
	ret = 0;
	for (i = 0;i < dir->n_sub;i++)
		if (dir->sub[i]->disp == D_DIR)
			ret |= box_close(dir->sub[i], force);
	if (ret && !force) { /* if anything got wrong, return here ... */
		box_order_subdirs(dir);
		return(ret);
	} else {
		/* also remove the icons */
		if (focus == dir)   focus_set(dir->parent);
		for (i = 0;i < dir->n_sub;i++)
			if (dir->sub[i]->disp == D_ICON) {
				icon_undisplay(dir->sub[i]);
				dir->detached = 0;
				if (focus == dir->sub[i])
					focus_set(dir->parent);
			}
		box_unexpand(dir);
	}
	return(ret);
}
/*
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
	int i, j;
	printf("box_order_subdirs( %s ): %d dirs opened\n", dir->name, dir->dirs_opened);
	switch (dir->dirs_opened) {
	case 0:
		return;
	case 1:
		for (i = 0;i < dir->n_sub;i++) {
			if (dir->sub[i]->disp == D_DIR) {
				dir->sub[i]->px = 0.0;
				dir->sub[i]->py = BOXHEIGHT + dir->sub[i]->detached * DETHEIGHT;
				dir->sub[i]->pz = 0.0;
				dir->sub[i]->scale = 0.2;
				ani_add(dir->sub[i]);
			}
		}
		break;
	default:
		j = 0;
		for (i = 0;i < dir->n_sub;i++) {
			if (dir->sub[i]->disp == D_DIR) {
				dir->sub[i]->px = 0.8 * sin(((float)j * 2 * M_PI) / ((float)dir->dirs_opened));
				dir->sub[i]->py = BOXHEIGHT + dir->sub[i]->detached * DETHEIGHT;
				dir->sub[i]->pz = 0.8 * cos(((float)j * 2 * M_PI) / ((float)dir->dirs_opened));
				dir->sub[i]->scale = 0.2;
				ani_add(dir->sub[i]);
				j++;
			}
		}
	}
}
/* order the icons properly */
void box_order_icons(t_node *dir)
{
	int dps, i;
	dps = ceil(sqrt(dir->n_sub)); /* directories per line */
	for (i = 0;i < dir->n_sub;i++) {
		if (dir->sub[i]->disp == D_ICON) {
			dir->sub[i]->px = -1 + 2 * ((float)((int)i % dps) + 0.5) / ((float)dps);
			dir->sub[i]->py = 0.5 + ((float)((int)i / dps) + 0.5) / ((float)dps) - 0.5;
			dir->sub[i]->pz = dir->sub[i]->detached * 0.2 + 1.0;
			dir->sub[i]->scale = (float)1.0 / ((float)dps);
			s3d_link(dir->sub[i]->oid, dir->oid); /* if it's already displayed, make sure it linked properly ... */
			ani_finish(dir->sub[i], -1);    /* copy to the current animation state */
			s3d_flags_on(dir->sub[i]->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			s3d_flags_on(dir->sub[i]->objs.str, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		}
	}
}
