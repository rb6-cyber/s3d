/*
 * dialog.c
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
#include <stdio.h>  /* NULL, printf() */
#include <string.h> /* strlen() */
#include <stdlib.h> /* realloc(),malloc() */
#include <errno.h>  /* errno */
#include <pthread.h> /* pthread_create() */
#include <sys/stat.h> /* mkdir() */
#include <sys/types.h> /* mkdir() */

static s3dw_input  *input;
static filelist *fp;
static char destdir[M_DIR];
static t_node *destnode = NULL;

extern int typeinput;
int fs_lock = 0;
static pthread_t filethread;


void close_win(s3dw_widget *button)
{
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
/* add all selected dirs in the new filelist */
static int get_selected(filelist *fp, t_node *dir)
{
	int i;
	char *s;
	for (i = 0;i < dir->n_sub;i++) {
		if (dir->sub[i]->sub != NULL) get_selected(fp, dir->sub[i]); /* scan subdir */
		if (dir->sub[i]->detached) {
			fp->n++;
			fp->p = (struct _t_file*)realloc(fp->p, sizeof(t_file) * fp->n);
			s = (char*)malloc(M_DIR);
			node_path(dir->sub[i], s);
			fp->p[fp->n - 1].name = s;
			if ((fs_lock == TYPE_COPY) || (fs_lock == TYPE_MOVE))
				fp->p[fp->n - 1].anode = fly_create_anode(dir->sub[i]);
			else
				fp->p[fp->n - 1].anode = NULL;
			fp->p[fp->n - 1].size = 0; /*TODO: later */
			fp->p[fp->n - 1].state = STATE_NONE;

		}
	}
	return(0);
}
void window_help(void)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin = s3dw_surface_new("Help Window", 12, 12);
	s3dw_label_new(infwin, "F1 - This Help Window", 1, 2);
	s3dw_label_new(infwin, "F5 - Copy", 1, 3);
	s3dw_label_new(infwin, "F6 - Move", 1, 4);
	s3dw_label_new(infwin, "F7 - Create Directory", 1, 5);
	s3dw_label_new(infwin, "F8 - Unlink", 1, 5);
	s3dw_label_new(infwin, "R - Refresh", 1, 6);
	s3dw_label_new(infwin, "I - Info", 1, 7);

	button = s3dw_button_new(infwin, "OK", 4, 10);
	button->onclick = close_win;
	s3dw_show(S3DWIDGET(infwin));

}
void window_fs_another(void)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin = s3dw_surface_new("Error", 20, 8);
	s3dw_label_new(infwin, "Sorry, another FS Action is in Progress", 1, 2);
	button = s3dw_button_new(infwin, "OK", 5, 5);
	button->onclick = close_win;
	s3dw_show(S3DWIDGET(infwin));
}
void window_fs_nothing(void)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin = s3dw_surface_new("Error", 12, 8);
	s3dw_label_new(infwin, "Nothing selected :(", 1, 2);
	button = s3dw_button_new(infwin, "OK", 5, 5);
	button->onclick = close_win;
	s3dw_show(S3DWIDGET(infwin));
}
static void window_fs_confirm_error(s3dw_widget *button)
{
	fs_err.state = ESTATE_NONE;
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */

}
void window_fs_errno(const char *errmsg)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	char string[M_DIR];
	float l;
	snprintf(string, M_DIR, "%s: %s", errmsg, strerror(errno));
	l = strlen(string) * 0.7;
	infwin = s3dw_surface_new("Error", l, 8);
	s3dw_label_new(infwin, string, 1, 2);
	button = s3dw_button_new(infwin, "OK", l / 2 - 1, 5);
	button->onclick = close_win;
	s3dw_show(S3DWIDGET(infwin));
}

void window_fs_abort(s3dw_widget *button)
{
	/* delete a filelist, if there was any */
	if (fp != NULL) {
		fl_del(fp);
		fp = NULL;
	}
	typeinput = 0;
	fs_lock = TYPE_NONE;
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
static void* thread_start(void *S3DFMUNUSED(ptr))
{
	switch (fs_lock) {
	case TYPE_COPY:
		printf("starting a copy process in the thread ... \n");
		destnode = node_getbypath(destdir);
		fs_fl_copy(fp, destdir);
		printf("done\n");
		break;
	case TYPE_UNLINK:
		printf("unlinking some files ... \n");
		fs_fl_unlink(fp);
		printf("done\n");
		break;
	}
	fs_lock = TYPE_FINISHED;
	return(NULL);

}
/* start the thread, as filesystem stuff is locked ... */
void window_fs(s3dw_widget *button)
{
	pthread_create(&filethread, NULL, thread_start, NULL);
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
void window_copy(const char *S3DFMUNUSED(path))
{
	s3dw_surface *infwin;
	s3dw_button  *okbutton, *abortbutton;
	float l;

	int i;
	unsigned int m;

	if (fs_lock)  {
		window_fs_another();
		return;
	}
	fs_lock = TYPE_COPY;
	fp = (filelist*)malloc(sizeof(filelist));
	fp->n = 0;
	fp->p = NULL;
	get_selected(fp, &root);
	printf("selected %d nodes\n", fp->n);
	if (fp->n == 0) {
		window_fs_nothing();
		free(fp);
		fp = NULL;
		return;
	}
	/* get the longest item on the list */
	m = 10;
	for (i = 0;i < fp->n;i++) {
		if (strlen(fp->p[i].name) > m) m = strlen(fp->p[i].name);
		printf("%d: %s\n", i, fp->p[i].name);
	}

	l = (m + 3) * 0.7;
	infwin = s3dw_surface_new("Copy Window", l, fp->n + 8);
	s3dw_label_new(infwin, "Copy: ", 1, 1);
	for (i = 0;i < fp->n;i++)
		s3dw_label_new(infwin, fp->p[i].name, 3, 2 + i);
	s3dw_label_new(infwin, "to:", 1, fp->n + 3);
	node_path(node_getdir(focus), destdir);
	s3dw_label_new(infwin, destdir, 3, fp->n + 4);

	okbutton = s3dw_button_new(infwin, "OK", l / 2 - 3, fp->n + 5);
	okbutton->onclick = window_fs;
	abortbutton = s3dw_button_new(infwin, "abort", l / 2, fp->n + 5);
	abortbutton->onclick = window_fs_abort;

	s3dw_show(S3DWIDGET(infwin));

}


void window_unlink(void)
{
	s3dw_surface *infwin;
	s3dw_button  *okbutton, *abortbutton;
	float l;

	int i;
	unsigned int m;

	if (fs_lock)  {
		window_fs_another();
		return;
	}
	fs_lock = TYPE_UNLINK;
	fp = (filelist*)malloc(sizeof(filelist));
	fp->n = 0;
	fp->p = NULL;
	get_selected(fp, &root);
	printf("selected %d nodes\n", fp->n);
	if (fp->n == 0) {
		window_fs_nothing();
		free(fp);
		fp = NULL;
		return;
	}
	/* get the longest item on the list */
	m = 10;
	for (i = 0;i < fp->n;i++)
		if (strlen(fp->p[i].name) > m) m = strlen(fp->p[i].name);

	l = (m + 3) * 0.7;
	infwin = s3dw_surface_new("Unlink Window", l, fp->n + 8);
	s3dw_label_new(infwin, "Unlink: ", 1, 1);
	for (i = 0;i < fp->n;i++)
		s3dw_label_new(infwin, fp->p[i].name, 3, 2 + i);

	okbutton = s3dw_button_new(infwin, "OK", l / 2 - 3, fp->n + 3);
	okbutton->onclick = window_fs;
	abortbutton = s3dw_button_new(infwin, "Abort", l / 2, fp->n + 3);
	abortbutton->onclick = window_fs_abort;

	s3dw_show(S3DWIDGET(infwin));

}
void window_fs_mkdir(s3dw_widget *button)
{
	char *dir;
	t_node *item;
	dir = s3dw_input_gettext(input);
	printf("creating Directory ...%s\n", dir);
	if (-1 == mkdir(dir, 0777)) /* umask ?! */
		window_fs_errno("could not create directory");
	else {
		/* success, now refresh it */
		item = node_getbypath(dir);
		if (item == NULL) {
			printf("cannot refresh\n");
		} else {
			printf("refreshing %s\n", item->name);
			/*   parse_again(item);*/
		}
	}
	fs_lock = 0;
	window_fs_abort(button); /* finish */

}
void window_mkdir(const char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *okbutton, *abortbutton;
	char string1[M_DIR];
	float l;
	if (fs_lock) {
		window_fs_another();
		return;
	}
	snprintf(string1, M_DIR, "Create Directory in %s", path);
	l = strlen(string1) * 0.7;
	infwin = s3dw_surface_new("Create Directory", l, 8);
	s3dw_label_new(infwin, string1, 1, 2);
	input = s3dw_input_new(infwin, 10, 1, 3);
	s3dw_input_change_text(input, path);
	s3dw_focus(S3DWIDGET(input));
	s3dw_focus(S3DWIDGET(infwin));
	typeinput = 1;
	fs_lock = 1;
	okbutton = s3dw_button_new(infwin, "OK", l / 2 - 3, 5);
	okbutton->onclick = window_fs_mkdir;
	abortbutton = s3dw_button_new(infwin, "abort", l / 2, 5);
	abortbutton->onclick = window_fs_abort;
	s3dw_show(S3DWIDGET(infwin));
}

void window_move(const char *S3DFMUNUSED(path))
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	if (fs_lock) {
		window_fs_another();
		return;
	}
	infwin = s3dw_surface_new("Info Window", 20, 8);
	s3dw_label_new(infwin, "Sorry, moving is not implemented yet.. :(", 1, 2);
	button = s3dw_button_new(infwin, "Too bad", 7, 5);
	button->onclick = close_win;
	s3dw_show(S3DWIDGET(infwin));

}
/* a small window which counts directories/files and displays the result */
void window_info(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	char string1[M_DIR];
	char string2[M_DIR];
	int b, d, f;
	char bd[M_DIR];
	float l;
	snprintf(string1, M_DIR, "Info for %s", path);
	fs_approx(path, &f, &d, &b);
	dotted_int(bd, b);
	snprintf(string2 , M_DIR, "%s bytes in %d files and %d Directories", bd, f, d);

	l = ((strlen(string1) > strlen(string2)) ? strlen(string1) : strlen(string2)) * 0.7;

	infwin = s3dw_surface_new("Info Window", l, 12);

	s3dw_label_new(infwin, string1, 1, 2);
	s3dw_label_new(infwin, string2, 1, 4);

	button = s3dw_button_new(infwin, "OK", l / 2 - 1, 6);
	/* clicking on the button will exit ... */
	button->onclick = close_win;
	/* of couse, show it */
	s3dw_show(S3DWIDGET(infwin));
}
/* check if a file operation is finished and clean up */
void window_fsani(void)
{
	int i;
	t_node *node, dummy;
	if (fs_lock != TYPE_NONE) {
		/* get current position of our destination node */
		if (destnode != NULL) {
			node = destnode;
			node_init(&dummy);
			dummy.parent = node->parent;
			dummy.scale = node->scale;
			dummy.px = node->px;
			dummy.py = node->py;
			dummy.pz = node->pz;
			dummy.type = node->type;
			fly_set_absolute_position(&dummy);
		} else {
			dummy.px = 0;
			dummy.py = 0;
			dummy.pz = 0;
			dummy.scale = 0.01;
		}
		if (fp != NULL) {
			for (i = 0;i < fp->n;i++) {
				if (fp->p[i].state == STATE_FINISHED) { /* we can go and clean up now. */
					if (NULL != (node = node_getbypath(fp->p[i].name))) {
						printf("[CLEANUP] for node %s (%s)\n", node->name, fp->p[i].name);
						node->detached = 0;
						if (node->parent != NULL) {
							parse_dir(node->parent);
							switch (node->disp) {
							case D_ICON:
								box_order_icons(node->parent);
								break;
							case D_DIR:
								box_order_subdirs(node->parent);
								break;
							}
						}
					} else
						printf("node %s already vanished ...\n", fp->p[i].name);

					fp->p[i].state = STATE_CLEANED;
				}
				if (fp->p[i].state > STATE_NONE) {
					if (destnode != NULL) {
						fp->p[i].anode->px = dummy.px;
						fp->p[i].anode->py = dummy.py;
						fp->p[i].anode->pz = dummy.pz;
						ani_add(fp->p[i].anode);
					}

				}
			}
		}
		if (fs_lock == TYPE_FINISHED) {
			printf("filesystem stuff is finisheed, cleaning up");
			if (fp != NULL) {
				fl_del(fp);
				fp = NULL;
			}
			typeinput = 0;
			fs_lock = TYPE_NONE;
			if (destnode != NULL)
				if (destnode->disp == D_DIR) { /* it usually is opened */
					printf("reordering icons on destnode ...\n");
					box_order_icons(destnode);
				}
			destnode = NULL;
		}
		if (fs_err.state == ESTATE_RISE) {
			s3dw_surface *infwin;
			s3dw_button  *button;
			char errmsg[M_DIR];
			float l;
			fs_err.state = ESTATE_WAIT_FOR_CONFIRM;
			snprintf(errmsg, M_DIR, "Error \"%s\" on %s", fs_err.message, fs_err.file);
			l = s3d_strlen(errmsg) + 2;
			infwin = s3dw_surface_new("Error", l, 8);
			s3dw_label_new(infwin, errmsg, 1, 2);
			button = s3dw_button_new(infwin, "OK", l / 2 - 1, 5);
			button->onclick = window_fs_confirm_error;
			s3dw_show(S3DWIDGET(infwin));
			printf("fs_err is active ... message: %s\n", errmsg);
		}
	}
}
