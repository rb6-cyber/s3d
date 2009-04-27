/*
 * menu.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of dot_mcp, a mcp for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * dot_mcp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * dot_mcp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dot_mcp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include "s3d.h"
#include "dot_mcp.h"
#include <unistd.h> /* fork(), execl() */
#include <stdio.h> /* printf() */
#include <stdlib.h> /* exit() */
#include <string.h> /* strlen(),strncpy(), strncat() */
struct menu_entry {
	const char *icon, *name, *path;
	int icon_oid, str_oid;
};

static int go = -1;
static int act;
static struct menu_entry menu[] = {
	{"objs/comp.3ds", "terminal", "s3dvt",    0, 0},
	{"objs/comp.3ds", "meshs3d", "meshs3d",    0, 0},
	{"objs/comp.3ds", "s3d_x11gate", "s3d_x11gate",  0, 0},
	{"objs/comp.3ds", "s3dfm", "s3dfm",  0, 0},
	{"objs/comp.3ds", "logout", "LOGOUT",     0, 0},
};

void menu_click(int oid)
{
	unsigned int i;
	char exec[256];
	printf("%d got clicked\n", oid);
	if (oid == go) {
		act = !act;
		for (i = 0;i < (sizeof(menu) / sizeof(struct menu_entry));i++) {
			if (act) {
				s3d_flags_on(menu[i].icon_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
				s3d_flags_on(menu[i].str_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			} else {
				s3d_flags_off(menu[i].icon_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
				s3d_flags_off(menu[i].str_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			}

		}
		return;
	}
	if (act) {
		for (i = 0;i < (sizeof(menu) / sizeof(struct menu_entry));i++) {
			if ((oid == menu[i].icon_oid) || (oid == menu[i].str_oid)) {
				int len;
				if (0 == strncmp(menu[i].path, "LOGOUT", 6)) {
					s3d_quit();
					return;
				}
				strncpy(exec, menu[i].path, 256);
				exec[255]= '\0';
				len = strlen(exec);
				if (len < 256) {
					strncat(exec, "> /dev/null 2>&1 &", 255 - len); /* ignoring output, starting in background */
					printf("executing [%s]\n", exec);
					system(exec);
				} else {
					fprintf(stderr, "path too long to execute\n");
				}
				return;
			}
		}
	}
}

int menu_init(void)
{
	unsigned int i;
	int menu_o;
	menu_o = s3d_new_object();
	act = 0; /* menu deactived */
	go = s3d_import_model_file("objs/s3dstart.3ds");
	s3d_flags_on(go, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_link(go, menu_o);
	for (i = 0; i < (sizeof(menu) / sizeof(struct menu_entry)); i++) {
		if (-1 == (menu[i].icon_oid = s3d_import_model_file(menu[i].icon)))
			menu[i].icon_oid = s3d_new_object();
		menu[i].str_oid = s3d_draw_string(menu[i].name, NULL);
		s3d_link(menu[i].str_oid, menu[i].icon_oid);
		s3d_link(menu[i].icon_oid, menu_o);
		s3d_translate(menu[i].icon_oid, 0, -3 + (-3 * (signed)i), 0);
		s3d_translate(menu[i].str_oid, 2, 0, 0);
		/*  s3d_flags_on(menu[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		  s3d_flags_on(menu[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);*/
		printf("menu item menu[%d], icon_oid=%d, icon_str=%d\n", i, menu[i].icon_oid, menu[i].str_oid);
	}
	return(menu_o);
}
