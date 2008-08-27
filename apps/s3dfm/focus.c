/*
 * focus.c
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
#include <s3d_keysym.h>
#include <stdio.h> /* printf() */
#include <math.h>  /* ceil(), sqrt() */

/* get the scale for the rootbox zoom */
float focus_get_scale(t_node *f)
{
	float scale, s;
	if (f->disp == D_DIR) {
		s = 0.2;
		if (f->parent != NULL)
			scale = 1.f / s * focus_get_scale(f->parent);
		else
			return(1.0);
		root.px -= f->px;
		root.pz -= f->pz;
		root.py -= BOXHEIGHT + f->detached * DETHEIGHT;
		root.px *= 1 / s;
		root.py *= 1 / s;
		root.pz *= 1 / s;
		return(scale);
	} else {
		if (f->parent != NULL)  return(focus_get_scale(f->parent)); /* icons etc */
		else        return(1.0);      /* that should never happen */
	}

}
/* center f for the viewer, therefore moving the root box ... */
void focus_set(t_node *f)
{
	root.px = 0.0;
	root.py = 0.0;
	root.pz = 0.0;
	/* printf("[Z]ooming to %s\n",f->name);*/
	/* box_collapse_grandkids(f);*/
	root.scale = focus_get_scale(f);
	root.py -= 1.5;
	/* printf("[R]escaling to %f\n",root.scale);
	 printf("px: %f py:%f pz: %f\n",root.px,root.py,root.pz);*/

	ani_add(&root);
	node_focus_color(focus, 0);
	node_focus_color(f, 1);
	focus = f;
	if (((cam.dpx - cam.px)* (cam.dpx - cam.px) + (cam.dpy - cam.py)* (cam.dpy - cam.py)
	                + (cam.dpz - cam.pz)* (cam.dpz - cam.pz)) > (10 * 10)) {
		cam.px = 0;
		cam.py = 0;
		cam.pz = 5;
		ani_add(&cam);
	}
}

/* uses a keysym to set the focus new */
void focus_by_key(int keysym)
{
	int i, rowsize;
	if (focus->pindex != -1) {
		switch (focus->disp) {
		case D_DIR:
			switch (keysym) {
			case S3DK_RIGHT:
				/* cycle to the next directory on the ring */
				for (i = focus->pindex - 1;i >= 0;i--)
					if (focus->parent->sub[i]->disp == D_DIR) { /* found a directory before, cycle */
						focus_set(focus->parent->sub[i]);
						break;
					}
				if (i == -1) /* nothing found, wrap to the other side */
					for (i = focus->parent->n_sub - 1;i >= focus->pindex + 1;i--)
						if (focus->parent->sub[i]->disp == D_DIR) { /* found a directory before, cycle */
							focus_set(focus->parent->sub[i]);
							break;
						}
				break;
			case S3DK_LEFT:
				/* cycle to the next directory on the ring */
				for (i = focus->pindex + 1;i < focus->parent->n_sub;i++)
					if (focus->parent->sub[i]->disp == D_DIR) { /* found a directory before, cycle */
						focus_set(focus->parent->sub[i]);
						break;
					}
				if (i == focus->parent->n_sub) /* nothing found, wrap to the other side */
					for (i = 0;i < focus->pindex;i++)
						if (focus->parent->sub[i]->disp == D_DIR) { /* found a directory before, cycle */
							focus_set(focus->parent->sub[i]);
							break;
						}
				break;
			case S3DK_UP:
				/* go in the first entry of this directory, if possible */
				if (focus->n_sub > 0)
					focus_set(focus->sub[0]);
				break;
			case S3DK_DOWN:
				/* go to first icon entry of parent,  or parent itself */
				for (i = focus->parent->n_sub - 1;i >= 0;i--)
					if (focus->parent->sub[i]->disp == D_ICON) { /* found a directory before, cycle */
						focus_set(focus->parent->sub[i]);
						break;
					}
				if (i == 0) /* no icons? go to parent. */
					focus_set(focus->parent);
				break;



			}
			break;
		case D_ICON:
			switch (keysym) {
			case S3DK_LEFT:
				/* search for the next icon on the left side */
				i = focus->pindex;
				do {
					i--;
					if (i < 0) i = focus->parent->n_sub - 1;
				} while (focus->parent->sub[i]->disp != D_ICON);
				focus_set(focus->parent->sub[i]);
				break;
			case S3DK_RIGHT:
				/* search for the next icon on the right side */
				i = focus->pindex;
				do {
					i++;
					if (i >= focus->parent->n_sub) i = 0;
				} while (focus->parent->sub[i]->disp != D_ICON);
				focus_set(focus->parent->sub[i]);
				break;
			case S3DK_UP:
				/* search for the next icon on the left side */
				i = focus->pindex;
				rowsize = ceil(sqrt(focus->parent->n_sub)); /* items per line */
				do {
					i += rowsize;
					if (i >= focus->parent->n_sub) break;
				} while (focus->parent->sub[i]->disp != D_ICON);
				if (i >= focus->parent->n_sub) {
					/* go to the first activated dir above ... */
					for (i = 0;i < focus->parent->n_sub;i++)
						if (focus->parent->sub[i]->disp == D_DIR) { /* found a directory before, cycle */
							focus_set(focus->parent->sub[i]);
							break;
						}
				} else  focus_set(focus->parent->sub[i]);
				break;
			case S3DK_DOWN:
				/* search for the next icon on the left side */
				i = focus->pindex;
				rowsize = ceil(sqrt(focus->parent->n_sub)); /* items per line */
				do {
					i -= rowsize;
					if (i < 0) break;
				} while (focus->parent->sub[i]->disp != D_ICON);
				if (i < 0)
					focus_set(focus->parent);
				else
					focus_set(focus->parent->sub[i]);
				break;

			}
			break;
		}
	} else {
		/* probably root */
		switch (keysym) {
		case S3DK_UP:
			/* go in the first entry of this directory, if possible */
			if (focus->n_sub > 0)
				focus_set(focus->sub[0]);
			break;
		}
	}

}
