/*
 * main.c
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
#include <dirent.h>  /*  dirent */
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <stdlib.h>  /*  realloc () */


int parse_dir(t_node *dir)
{
	struct dirent **namelist;
	int n, i;
	int oldn;
	char *nstr = NULL;
	char path[M_DIR];
	char ndir[M_DIR];

	printf("parse_dir( %s )\n", dir->name);
	node_path(dir, path);
	for (i = 0;i < dir->n_sub;i++) {
		dir->sub[i]->check = 1;
	}
	n = scandir(path, &namelist, NULL, alphasort);
	if (n < 0) {
		window_fs_errno(path);
		return(-1);
	} else {
		oldn = dir->n_sub;
		while (n--) {
			nstr = namelist[n]->d_name;
			/* setup kids in the sub */
			for (i = 0;i < oldn;i++) /* see if it's already there */
				if (dir->sub[i])
					if (0 == strcmp(namelist[n]->d_name, dir->sub[i]->name))
						break;
			if ((0 != strcmp(nstr, ".")) && (0 != strcmp(nstr, ".."))) { /* we don't care about those */
				if (i == oldn) { /* it's new, add it, initialize it ... */
					i = dir->n_sub;
					/* i now holds the right index in sub, so we use dir->sub[i]
					 * to reference the new item now... */
					dir->n_sub++;
					dir->sub = (struct _t_node**)realloc(dir->sub , dir->n_sub * sizeof(t_node *));
					dir->sub[i] = (struct _t_node*)malloc(sizeof(t_node));
					node_init(dir->sub[i]);
					strncpy(dir->sub[i]->name, nstr, M_NAME);
					dir->sub[i]->parent = dir;
				}
				/* find out the filetype ... very simple */
				dir->sub[i]->type = T_DUNO;
				dir->sub[i]->pindex = i;
				strncpy(ndir, path, M_DIR);
				mstrncat(ndir, "/", M_DIR);
				strncat(ndir, namelist[n]->d_name, M_DIR);
				if (fs_isdir(ndir))
					dir->sub[i]->type = T_FOLDER;
				dir->sub[i]->check = 0; /* check=0 means we've already processed this item */
			}
			free(namelist[n]);
		}
		free(namelist);
		dir->check = 0;
		for (i = 0;i < dir->n_sub;i++)
			if (dir->sub[i]->check) {
				/* not checked yet... that means the item is not in the reparsed directory, ie vanished.
				 * so we're removing it from our queue */
				node_delete(dir->sub[i]);
				dir->n_sub--;
				dir->sub[i] = dir->sub[dir->n_sub]; /* exchange with the last one */
				dir->sub[i]->pindex = i;
				dir->check = 1;
			}
		/* if we removed something, then shrink the buffer accordingly .. */
		if (dir->check)  dir->sub = (struct _t_node**)realloc(dir->sub , dir->n_sub * sizeof(t_node *));
		dir->parsed = 1;
	}
	return(0);
}

