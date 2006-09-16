/*
 * dialog.c
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
#include <s3dw.h>
#include <stdio.h> 	/* NULL, printf() */
#include <string.h> /* strlen() */
#include <stdlib.h> /* realloc(),malloc() */
#include <errno.h>  /* errno */
#include <sys/stat.h> /* mkdir() */
#include <sys/types.h> /* mkdir() */
extern t_item *focus;
filelist fp={NULL,0};
int typeinput=0;
/* keyevent handler */
void key_handler(struct s3d_evt *evt)
{
	struct s3d_key_event *keys=(struct s3d_key_event *)evt->buf;
	char path[M_DIR];
	if (typeinput) {	/* we have some inputfield now and want the s3dw to handle our input */	
			printf("inputting text ...\n");
			s3dw_handle_key(evt); 
			return; 
	}
	get_path(focus,path);
	switch (keys->keysym)
	{
		case 'i':
		case 'I':
				{
				window_info(path);
				}
				break;
		case 'r':
		case 'R':
				{/* refresh this window ... */
					printf("[R]efreshing %s\n",focus->name);
					parse_again(focus);
					ani_focus(focus);
				}
				break;
		case S3DK_F1:
				window_help();
				break;
		case S3DK_F5:
				window_copy(path);
				break;
		case S3DK_F6:
				window_move(path);
				break;
		case S3DK_F7:
				window_mkdir(path);
				break;


	}
	s3dw_handle_key(evt);
}

/* object click handler */
void object_click(struct s3d_evt *evt)
{
	int oid;
	t_item *f;
	s3dw_handle_click(evt);
	oid=(int)*((unsigned long *)evt->buf);
	if (NULL!=(f=finditem(&root,oid)))
	{
		if (f->close==oid)
		{
			box_collapse(f,1);
/*			if (f->parent!=NULL)
				ani_focus(f->parent);*/
			return;
		}
		if (f->select==oid)
		{
			printf("[S]electing %s\n",f->name);
			box_select(f);
			return;
		}
		if (f->type==T_FOLDER)
		{
			if (f->disp == D_DIR)
			{
				printf("[F]ound, Already displayed - ani_focus( %s )\n",f->name);
			} else {
				if (!f->parsed)	parse_dir(f);
				box_expand(f);
			}
			focus=f;
			ani_focus(f);
		} else
			printf("[F]ound, but %s is no folder\n",f->name);
	} else {
/*		printf("[C]ould not find :/\n");*/
	}
}
void close_win(s3dw_widget *button)
{
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
/* add some dots to an integer value for better readability */
void dotted_int(char *s,unsigned int i)
{
	char st[M_DIR];
	int p;
	p=0;
	st[0]=0;
	while (i>0)
	{
		if ((p+1)%4==0) {
			st[p]='.';
			p++;
		}
		st[p]=(i%10)+'0';
		i=i/10;
		p++;
	}
	if (p>0) p--;
	st[p+1]=0;
	for (i=0;i<p+1;i++)
		s[i]=st[p-i];
	s[p+1]=0;
}
/* add all selected dirs in the new filelist */
int get_selected(filelist *fp, t_item *dir)
{
	int i;
	char *s;
	for (i=0;i<dir->n_item;i++)
	{
		if (dir->list[i].list!=NULL)	get_selected(fp,&(dir->list[i])); /* scan subdir */
		if (dir->list[i].detached)
		{
			fp->n++;
			fp->p=realloc(fp->p,sizeof(char *) * fp->n);
			s=malloc(M_DIR);
			get_path(&(dir->list[i]),s);
			fp->p[fp->n - 1]=s;
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
	infwin=s3dw_surface_new("Error",12,8);
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
	int i;
	for (i=0;i<fp.n;i++)
		free(fp.p[i]);
	if (fp.p!=NULL) free(fp.p);
	fp.n=0;
	fp.p=NULL;
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

	if (fp.n!=0) 	{	window_fs_another(); 	return; }
	fp.n=0;
	fp.p=NULL;
	get_selected(&fp,&root);
	printf("selected %d nodes\n",fp.n);
	if (fp.n == 0)	{	window_fs_nothing();	return;	}
	m=10;
	for (i=0;i<fp.n;i++)
	{
		if (strlen(fp.p[i])>m) m=strlen(fp.p[i]);
		printf("%d: %s\n",i,fp.p[i]);
	}

	l=(m+3)*0.7;
	infwin=s3dw_surface_new("Copy Window",l,fp.n+8);
	s3dw_label_new(infwin,"Copy: ",1,1);
	for (i=0;i<fp.n;i++)
		s3dw_label_new(infwin,fp.p[i],3,2+i);
	s3dw_label_new(infwin,"to:",1,fp.n+3);
	get_path(focus,destdir);
	s3dw_label_new(infwin,destdir,3,fp.n+4);

	okbutton=s3dw_button_new(infwin,"OK",l/2-3,fp.n+5);
	okbutton->onclick=window_fs_abort;
	abortbutton=s3dw_button_new(infwin,"abort",l/2,fp.n+5);
	abortbutton->onclick=window_fs_abort;

	s3dw_show(S3DWIDGET(infwin));

}
s3dw_input	 *input;
void window_fs_mkdir(s3dw_widget *button)
{
	char *dir;
	t_item *item;
	dir=s3dw_input_gettext(input);
	printf("creating Directory ...%s\n",dir);
	if (-1==mkdir(dir,0777)) /* umask ?! */
		window_fs_errno("could not create directory");
	else {
		/* success, now refresh it */
		item=get_item(dir);
		if (item==NULL)
		{
			printf("cannot refresh\n");
		} else {
			printf("refreshing %s\n",item->name);
			parse_again(item);

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
	if (fp.n!=0) {window_fs_another(); return; }
	snprintf(string1,M_DIR,"Create Directory in %s",path);
	l=strlen(string1)*0.7;
	infwin=s3dw_surface_new("Create Directory",l,8);
	s3dw_label_new(infwin,string1,1,2);
	input=s3dw_input_new(infwin,10,1,3);
	s3dw_input_change_text(input, path);
	s3dw_focus(S3DWIDGET(input));
	s3dw_focus(S3DWIDGET(infwin));
	typeinput=1;
	okbutton=s3dw_button_new(infwin,"OK",l/2-3,fp.n+5);
	okbutton->onclick=window_fs_mkdir;
	abortbutton=s3dw_button_new(infwin,"abort",l/2,fp.n+5);
	abortbutton->onclick=window_fs_abort;
	s3dw_show(S3DWIDGET(infwin));

}

void window_move(char *path)
{
	s3dw_surface *infwin;
	s3dw_button  *button;
	if (fp.n!=0) {window_fs_another(); return; }
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
