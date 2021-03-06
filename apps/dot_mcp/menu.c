// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
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
static struct menu_entry menu_items[] = {
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
		for (i = 0; i < (sizeof(menu_items) / sizeof(struct menu_entry)); i++) {
			if (act) {
				s3d_flags_on(menu_items[i].icon_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
				s3d_flags_on(menu_items[i].str_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			} else {
				s3d_flags_off(menu_items[i].icon_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
				s3d_flags_off(menu_items[i].str_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			}

		}
		return;
	}
	if (act) {
		for (i = 0; i < (sizeof(menu_items) / sizeof(struct menu_entry)); i++) {
			if ((oid == menu_items[i].icon_oid) || (oid == menu_items[i].str_oid)) {
				size_t len;
				if (0 == strncmp(menu_items[i].path, "LOGOUT", 6)) {
					s3d_quit();
					return;
				}
				strncpy(exec, menu_items[i].path, sizeof(exec));
				exec[sizeof(exec) - 1]= '\0';
				len = strlen(exec);
				if (len < sizeof(exec)) {
					strncat(exec, "> /dev/null 2>&1 &", sizeof(exec) - 1 - len); /* ignoring output, starting in background */
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
	for (i = 0; i < (sizeof(menu_items) / sizeof(struct menu_entry)); i++) {
		if (-1 == (menu_items[i].icon_oid = s3d_import_model_file(menu_items[i].icon)))
			menu_items[i].icon_oid = s3d_new_object();
		menu_items[i].str_oid = s3d_draw_string(menu_items[i].name, NULL);
		s3d_link(menu_items[i].str_oid, menu_items[i].icon_oid);
		s3d_link(menu_items[i].icon_oid, menu_o);
		s3d_translate(menu_items[i].icon_oid, 0, -3 + (-3 * (signed)i), 0);
		s3d_translate(menu_items[i].str_oid, 2, 0, 0);
		/*  s3d_flags_on(menu_items[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		  s3d_flags_on(menu_items[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);*/
		printf("menu item menu[%d], icon_oid=%d, icon_str=%d\n", i, menu_items[i].icon_oid, menu_items[i].str_oid);
	}
	return menu_o;
}
