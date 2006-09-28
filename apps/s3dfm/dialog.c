/*
 * dialog.c
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
#include <s3d_keysym.h>
#include <stdio.h> 	/* NULL, printf() */
#include <string.h> /* strlen() */
#include <stdlib.h> /* realloc(),malloc() */
#include <errno.h>  /* errno */
#include <sys/stat.h> /* mkdir() */
#include <sys/types.h> /* mkdir() */

static s3dw_input	 *input;
static filelist *fp;

extern int typeinput;


void close_win(s3dw_widget *button)
{
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
/* add all selected dirs in the new filelist */
int get_selected(filelist *fp, t_node *dir)
{
	int i;
	char *s;
	for (i=0;i<dir->n_sub;i++)
	{
		if (dir->sub[i]->sub!=NULL)	get_selected(fp,dir->sub[i]); /* scan subdir */
		if (dir->sub[i]->detached)
		{
			fp->n++;
			fp->p=realloc(fp->p,sizeof(t_file) * fp->n);
			s=malloc(M_DIR);
			node_path(dir->sub[i],s);
			fp->p[fp->n - 1].name=s;
			fp->p[fp->n - 1].anode=fly_create_anode(dir->sub[i]);
			fp->p[fp->n - 1].size=0; /*TODO: later */
		}
	}
	return(0);
}
void window_help()
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin=s3dw_surface_new("Help Window",12,12);
	s3dw_label_new(infwin,"F1 - This Help Window",1,2);
	s3dw_label_new(infwin,"F5 - Copy",1,3);
	s3dw_label_new(infwin,"F6 - Move",1,4);
	s3dw_label_new(infwin,"F7 - Create Directory",1,5);
	s3dw_label_new(infwin,"R - Refresh",1,6);
	s3dw_label_new(infwin,"I - Info",1,7);

	button=s3dw_button_new(infwin,"OK",4,10);
	button->onclick=close_win;
	s3dw_show(S3DWIDGET(infwin));

}
void window_fs_another()
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin=s3dw_surface_new("Error",20,8);
	s3dw_label_new(infwin,"Sorry, another FS Action is in Progress",1,2);
	button=s3dw_button_new(infwin,"OK",5,5);
	button->onclick=close_win;
	s3dw_show(S3DWIDGET(infwin));
}
void window_fs_nothing()
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	infwin=s3dw_surface_new("Error",12,8);
	s3dw_label_new(infwin,"Nothing selected :(",1,2);
	button=s3dw_button_new(infwin,"OK",5,5);
	button->onclick=close_win;
	s3dw_show(S3DWIDGET(infwin));

}
void window_fs_errno(char *errmsg)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	char string[M_DIR];
	float l;
	snprintf(string,M_DIR,"%s: %s",errmsg,strerror(errno));
	l=strlen(string)*0.7;
	infwin=s3dw_surface_new("Error",l,8);
	s3dw_label_new(infwin,string,1,2);
	button=s3dw_button_new(infwin,"OK",l/2-1,5);
	button->onclick=close_win;
	s3dw_show(S3DWIDGET(infwin));
}

void window_fs_abort(s3dw_widget *button)
{
	/* delete a filelist, if there was any */
	if (fp!=NULL)
	{
		fl_del(fp);
		fp=NULL;
	}
	typeinput=0;
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
void window_copy(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *okbutton,*abortbutton;
	float l;
	char destdir[M_DIR];

	int i,m;

	if (fp!=NULL) 	{	window_fs_another(); 	return; }
	fp=malloc(sizeof(filelist));
	fp->n=0;
	fp->p=NULL;
	get_selected(fp,&root);
	printf("selected %d nodes\n",fp->n);
	if (fp->n == 0)	{	window_fs_nothing();	free(fp); fp=NULL;return;	}
	m=10;
	for (i=0;i<fp->n;i++)
	{
		if (strlen(fp->p[i].name)>m) m=strlen(fp->p[i].name);
		printf("%d: %s\n",i,fp->p[i].name);
	}

	l=(m+3)*0.7;
	infwin=s3dw_surface_new("Copy Window",l,fp->n+8);
	s3dw_label_new(infwin,"Copy: ",1,1);
	for (i=0;i<fp->n;i++)
		s3dw_label_new(infwin,fp->p[i].name,3,2+i);
	s3dw_label_new(infwin,"to:",1,fp->n+3);
	node_path(focus,destdir);
	s3dw_label_new(infwin,destdir,3,fp->n+4);

	okbutton=s3dw_button_new(infwin,"OK",l/2-3,fp->n+5);
	okbutton->onclick=window_fs_abort;
	abortbutton=s3dw_button_new(infwin,"abort",l/2,fp->n+5);
	abortbutton->onclick=window_fs_abort;

	s3dw_show(S3DWIDGET(infwin));

}
void window_fs_mkdir(s3dw_widget *button)
{
	char *dir;
	t_node *item;
	dir=s3dw_input_gettext(input);
	printf("creating Directory ...%s\n",dir);
	if (-1==mkdir(dir,0777)) /* umask ?! */
		window_fs_errno("could not create directory");
	else {
		/* success, now refresh it */
		item=node_getbypath(dir);
		if (item==NULL)
		{
			printf("cannot refresh\n");
		} else {
			printf("refreshing %s\n",item->name);
/*			parse_again(item);*/
		}
	}
	window_fs_abort(button); /* finish */

}
void window_mkdir(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *okbutton,*abortbutton;
	char string1[M_DIR];
	float l;
	if (fp!=NULL) {window_fs_another(); return; }
	snprintf(string1,M_DIR,"Create Directory in %s",path);
	l=strlen(string1)*0.7;
	infwin=s3dw_surface_new("Create Directory",l,8);
	s3dw_label_new(infwin,string1,1,2);
	input=s3dw_input_new(infwin,10,1,3);
	s3dw_input_change_text(input, path);
	s3dw_focus(S3DWIDGET(input));
	s3dw_focus(S3DWIDGET(infwin));
	typeinput=1;
	okbutton=s3dw_button_new(infwin,"OK",l/2-3,5);
	okbutton->onclick=window_fs_mkdir;
	abortbutton=s3dw_button_new(infwin,"abort",l/2,5);
	abortbutton->onclick=window_fs_abort;
	s3dw_show(S3DWIDGET(infwin));

}

void window_move(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	if (fp!=NULL) {window_fs_another(); return; }
	infwin=s3dw_surface_new("Info Window",20,8);
	s3dw_label_new(infwin,"Sorry, moving is not implemented yet.. :(",1,2);
	button=s3dw_button_new(infwin,"Too bad",7,5);
	button->onclick=close_win;
	s3dw_show(S3DWIDGET(infwin));

}
/* a small window which counts directories/files and displays the result */
void window_info(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	char string1[M_DIR];
	char string2[M_DIR];
	int b,d,f;
	char bd[M_DIR];
	float l;
	snprintf(string1,M_DIR,"Info for %s",path);
	fs_approx(path, &f, &d, &b);
	dotted_int(bd,b);
	snprintf(string2 ,M_DIR,"%s bytes in %d files and %d Directories",bd,f,d);
	
	l=((strlen(string1)>strlen(string2)) ? strlen(string1) :strlen(string2))*0.7;
	
	infwin=s3dw_surface_new("Info Window",l,12);

	s3dw_label_new(infwin,string1,1,2);
	s3dw_label_new(infwin,string2,1,4);

	button=s3dw_button_new(infwin,"OK",l/2-1,6);
	/* clicking on the button will exit ... */
	button->onclick=close_win;
	/* of couse, show it */
	s3dw_show(S3DWIDGET(infwin));
}
