/*
 * menu.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3d.h"
#include "dot_mcp.h"
#include <unistd.h> /* fork(), execl() */
#include <stdio.h> /* printf() */
#include <stdlib.h> /* exit() */
#include <string.h> /* strlen(),strncpy(), strncat() */
struct menu_entry {
	char *icon, *name, *path;
	int icon_oid, str_oid;
};
static int go=-1;
static int act;
static struct menu_entry menu[]={
		{"objs/comp.3ds","filebrowser","./filebrowser",	0,0},
		{"objs/comp.3ds","terminal","s3dvt",				0,0},
		{"objs/comp.3ds","clock","s3dclock",				0,0}
};
void menu_click(int oid)
{
	unsigned int i;
	char exec[256];
	printf("%d got clicked\n",oid);
	if (oid==go)
	{
		act=!act;
		for (i=0;i<(sizeof(menu)/sizeof(struct menu_entry));i++)
		{
			if (act)
			{
				s3d_flags_on(menu[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
				s3d_flags_on(menu[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			} else {
				s3d_flags_off(menu[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
				s3d_flags_off(menu[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			}

		}
		return;
	}
	if (act)
	{
		for (i=0;i<(sizeof(menu)/sizeof(struct menu_entry));i++)
		{
			if ((oid==menu[i].icon_oid) || (oid==menu[i].str_oid))
			{
				strncpy(exec,menu[i].path,256);
				strncat(exec,"> /dev/null 2>&1 &",256); /* ignoring output, starting in background */
				system(exec);
				return;
			}
		}
	}
}
int menu_init ()
{
	int i,menu_o;
	menu_o=s3d_new_object();
	act=0; /* menu deactived */
	go=s3d_import_3ds_file("objs/go_button.3ds");
	s3d_flags_on(go,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	s3d_link(go,menu_o);
	for (i=0;i<(sizeof(menu)/sizeof(struct menu_entry));i++)
	{
		if (-1==(menu[i].icon_oid=s3d_import_3ds_file(menu[i].icon)))
				menu[i].icon_oid=s3d_new_object();
		menu[i].str_oid=s3d_draw_string(menu[i].name,NULL);
		s3d_link(menu[i].str_oid,menu[i].icon_oid);
		s3d_link(menu[i].icon_oid,menu_o);
		s3d_translate(menu[i].icon_oid,0,-3+(-3*i),0);
		s3d_translate(menu[i].str_oid,2,0,0);
/*		s3d_flags_on(menu[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		s3d_flags_on(menu[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);*/
		printf("menu item menu[%d], icon_oid=%d, icon_str=%d\n",i,menu[i].icon_oid,menu[i].str_oid);
	}
	return(menu_o);
}


