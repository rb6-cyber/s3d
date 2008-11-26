/*
 * filebrowser.c
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




#include <s3d.h>
#include <stdio.h>   /*  printf() */
#include <dirent.h>  /*  dirent */
#include <stdlib.h>  /*  malloc() */
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <unistd.h>  /*  chdir() */
#include <math.h>  /*  sin(),cos() */
#include <time.h> /* nanosleep() */
#include <sys/stat.h>
#include "example.h" /* S3DUNUSED */
static struct timespec t = {
	0, 100*1000*1000
}; /* 100 mili seconds */

#define T_DUNO  0
#define T_LOCALDIR 1
#define T_BACKDIR 2
#define T_FOLDER 3
#define T_GEOMETRY 4
#define T_MUSIC  5
#define T_MOVIE  6

#define M_DIR  512
#define M_NAME  256

#define DEG2RAD(x) (x * ((float)M_PI / 180.0f))

static int folder, geometry, mp3, movie, duno, dot, dotdot;
struct t_item {
	int icon_oid, descr_oid, pie_oid;
	char name[M_NAME];
	int type;
};
static struct t_item *item;
static int n_item = 0;
static int display_dir(const char *dir, int S3DUNUSED(depth), int  posx, int posy, int posz)
{
	struct dirent **namelist;
	int n, i;
	int  px, py, pz;
	char *ext;
	char *nstr;
	float alpha, al, radius, f;
	char ndir[M_DIR+1];
	struct stat s;
	if (n_item) {
		printf("freeing %d old items\n", n_item);
		for (i = 0;i < n_item;i++) {
			printf("deleting %d and %d\n", item[i].icon_oid, item[i].descr_oid);
			s3d_del_object(item[i].descr_oid);
			s3d_del_object(item[i].icon_oid);
			s3d_del_object(item[i].pie_oid);
		}
		free(item);

	}
	n = i = scandir(dir, &namelist, NULL, alphasort);
	if (n < 0) {
		perror("scandir");
		return(-1);
	} else {
		item = (struct t_item*)malloc(sizeof(struct t_item) * i);
		n_item = i;
		while (n--) {
			item[n].type = T_DUNO;
			nstr = namelist[n]->d_name;
			strncpy(item[n].name, nstr, M_NAME);
			if ((0 == strncmp(nstr, ".", 1)) && (strlen(nstr) == 1))
				item[n].type = T_LOCALDIR;
			else if (0 == strncmp(nstr, "..", strlen(nstr) < 2 ? strlen(nstr) : 2))
				item[n].type = T_BACKDIR;
			else {
				ext = strrchr(nstr, '.');
				strncpy(ndir, dir, M_DIR);
				ndir[M_DIR] = 0;  /* just in case */
				strncat(ndir, "/", M_DIR - strlen(ndir));
				strncat(ndir, namelist[n]->d_name, M_DIR - strlen(ndir));
				/*     printf("displaying %s\n",ndir); */
				stat(ndir, &s);
				if (S_ISDIR(s.st_mode)) {
					item[n].type = T_FOLDER;
				}
				else
				{
					if (ext != NULL) {
						if (0 == strncmp(ext, ".3ds", strlen(ext) < 4 ? strlen(ext) : 4))
							item[n].type = T_GEOMETRY;
						else if (0 == strncmp(ext, ".mp3", strlen(ext) < 4 ? strlen(ext) : 4))
							item[n].type = T_MUSIC;
						else if (0 == strncmp(ext, ".mpg", strlen(ext) < 4 ? strlen(ext) : 4))
							item[n].type = T_MOVIE;

					}
				}
			}
			switch (item[n].type) {
			case T_LOCALDIR:
				item[n].icon_oid = s3d_clone(dot);
				break;
			case T_BACKDIR:
				item[n].icon_oid = s3d_clone(dotdot);
				break;
			case T_FOLDER:
				item[n].icon_oid = s3d_clone(folder);
				break;
			case T_GEOMETRY:
				item[n].icon_oid = s3d_clone(geometry);
				break;
			case T_MUSIC:
				item[n].icon_oid = s3d_clone(mp3);
				break;
			case T_MOVIE:
				item[n].icon_oid = s3d_clone(movie);
				break;
			default:
				printf("don't know type, defaulting to duno %d...\n", duno);
				item[n].icon_oid = s3d_clone(duno);
				break;
			}

			alpha = ((360.0f * n) / ((float)i));
			if (n_item < 5)
				radius = ((50) / ((float)M_PI * 4));
			else
				radius = ((n_item * 10) / ((float)M_PI * 4));
			px = posx - sinf(DEG2RAD(alpha)) * radius;
			py = posy;
			pz = posz - cosf(DEG2RAD(alpha)) * radius;

			item[n].pie_oid = s3d_new_object();
			s3d_push_vertex(item[n].pie_oid, 0, -2, 0);
			al = ((360.0f * (n - 0.5f)) / ((float)i));
			s3d_push_vertex(item[n].pie_oid,
			                posx - sinf(DEG2RAD(al))*radius, -2, posz - cosf(DEG2RAD(al))*radius);
			al = ((360.0f * (n + 0.5f)) / ((float)i));
			s3d_push_vertex(item[n].pie_oid,
			                posx - sinf(DEG2RAD(al))*radius, -2, posz - cosf(DEG2RAD(al))*radius);

			f = 1.0f - 0.05f * (n % 2);
			switch (item[n].type) {
			case T_LOCALDIR:
				s3d_push_material(item[n].pie_oid,  0, f, 0,   0.5, 0.5, 0.5,  f, f, f);
				break;
			case T_BACKDIR:
				s3d_push_material(item[n].pie_oid,  0, f / 2, 0,  0.5, 0.5, 0.5,  f, f, f);
				break;
			case T_FOLDER:
				s3d_push_material(item[n].pie_oid,  f, f, 0,   0.5, 0.5, 0.0,  f, f, 1);
				break;
			default:
				s3d_push_material(item[n].pie_oid,  f, f, f,   0.5, 0.5, 0.5,  f, f, f);
				break;
			}
			s3d_push_polygon(item[n].pie_oid, 0, 2, 1, 0);

			s3d_push_vertex(item[n].pie_oid, (float)pz, -2.f, 0.f);
			s3d_translate(item[n].icon_oid, (float)px, (float)py, (float)pz);
			s3d_rotate(item[n].icon_oid, 0, alpha, 0);
			s3d_flags_on(item[n].icon_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			s3d_flags_on(item[n].pie_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			item[n].descr_oid = s3d_draw_string(nstr, NULL);
			s3d_link(item[n].descr_oid, item[n].icon_oid);
			s3d_translate(item[n].descr_oid, -1, -2, 0);
			/*    r=s3d_get_radius(p); */
			/*    s3d_scale(p,1.0/r,1.0/r,1.0/r); */
			s3d_flags_on(item[n].descr_oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			printf("string %d linked to %d\n", item[n].descr_oid, item[n].icon_oid);
			free(namelist[n]);
		}
		free(namelist);
	}
	return(0);
}

static int object_click(struct s3d_evt *evt)
{
	int i, oid;
	char execstr[256];
	oid = (int) * ((unsigned long *)evt->buf);
	printf("!!!!!!!!! clicked object %d\n", oid);
	for (i = 0;i < n_item;i++) {
		if (((oid == item[i].icon_oid) || (oid == item[i].descr_oid)) ||
		                (oid == item[i].pie_oid)) {
			switch (item[i].type) {
			case T_BACKDIR:
			case T_FOLDER:
			case T_LOCALDIR:
				printf("going into %s\n", item[i].name);
				chdir(item[i].name);
				display_dir(".", 0, 0, 0, 0);
				return(0);
				break;
			case T_GEOMETRY:
				printf("loading geometry %s\n", item[i].name);
				snprintf(execstr, 256, "modelloader \"%s\"&\n", item[i].name);
				system(execstr);
				return(0);
				break;
			case T_MOVIE:
				printf("playing %s\n", item[i].name);
				snprintf(execstr, 256, "mplayer -vo s3d \"%s\"&\n", item[i].name);
				system(execstr);
				return(0);
				break;


			}
		}
	}
	return(0);
}
static void mainloop(void)
{
	nanosleep(&t, NULL);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "filebrowser")) {
		/*  load the object files */
		folder = s3d_import_model_file("objs/folder.3ds");
		geometry = s3d_import_model_file("objs/geometry.3ds");
		mp3 = s3d_import_model_file("objs/notes.3ds");
		movie = s3d_import_model_file("objs/film.3ds");
		duno = s3d_import_model_file("objs/duno.3ds");
		dot = s3d_import_model_file("objs/dot.3ds");
		dotdot = s3d_import_model_file("objs/dotdot.3ds");
		s3d_select_font("vera");
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, object_click);
		display_dir(".", 0, 0, 0, 0);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
