/*
 * main.c
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


#include <s3d.h>
#include <s3d_keysym.h>
#include "dot_mcp.h"
#include <stdint.h>  /*  uint32_t */
#include <unistd.h>  /*  sleep() */
#include <stdlib.h>  /*  free(), malloc() */
#include <string.h>  /*  strncpy() */
#include <stdio.h>  /*  printf() */
#include <math.h> /* sin(), cos() */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 33*1000*1000
};

#define bsize  0.2
struct tver {
	float x, y, z;
};
static struct tver campos, camrot;
static float xdif = 0, ydif = 0;

struct app {
	int oid, oid_c;
	float r;
	int init;
	float trans_x, trans_y, trans_z;
	float textw;
	char name[256];
	int min_but, close_but, title;
	int sphere;
	struct app *next;
};
static int ego_mode = 0;
static float asp = 1.0;
static struct app *apps = NULL;
static float bottom = -1.0;
static float left = -1.0;
static float zoom = 5.0;
static int n_app = 0;
static int rot_flag = 0;
static struct app *focus = NULL;
static float focus_r = 0;
static float alpha = 0;
static int rotate, reset, min_but, close_but, sphere, menu = -1;

void place_apps(void);

#define SIDES 60
#define RINGS 60
static int greentorus(void)
{
	int o, i, j;
	float R, r, a;
	float ia, ja, iap, jap;
	float v[SIDES*RINGS*3];
	float n[SIDES*12]; /* normals */
	uint32_t l[SIDES*RINGS*6];
	o = s3d_new_object();
	R = 100; /* outer radius */
	r = 100; /* inner radius */
	a = M_PI / 180;
	s3d_push_material_a(o, 0.2, 0.6, 0.2, 0.5,
	                    1  , 1  , 1  , 0.5,
	                    0.2, 0.6, 0.2, 0.5);
	for (i = 0;i < RINGS;i++) {
		for (j = 0;j < SIDES;j++) {
			ia = a * ((float)i * 360.0 / RINGS);
			ja = a * ((float)j * 360.0 / SIDES);
			iap = a * ((float)(i + 1) * 360.0 / RINGS);
			jap = a * ((float)(j + 1) * 360.0 / SIDES);

			v[i*SIDES*3+ j*3 +0] = (R + r * cos(ja)) * cos(ia);
			v[i*SIDES*3+ j*3 +1] = r * sin(ja);
			v[i*SIDES*3+ j*3 +2] = (R + r * cos(ja)) * sin(ia);

			l[i*SIDES*6+ j*6 +0] = i * SIDES + j;
			l[i*SIDES*6+ j*6 +1] = i * SIDES + (j + 1) % SIDES;
			l[i*SIDES*6+ j*6 +2] = 0;
			l[i*SIDES*6+ j*6 +3] = i * SIDES + j;
			l[i*SIDES*6+ j*6 +4] = ((i + 1) % RINGS) * SIDES + j;
			l[i*SIDES*6+ j*6 +5] = 0;

			n[j*12 +0] = R * r * cos(ja) *    cos(ia) + r * r * cos(ja) * cos(ia) * cos(ia);
			n[j*12 +1] = R * r * sin(ja) *    cos(ia) + r * r * sin(ja) * cos(ia) * cos(ia);
			n[j*12 +2] = R * r * sin(ia)    + r * r * sin(ia) * cos(ia);
			n[j*12 +3] = R * r * cos(jap) *    cos(ia) + r * r * cos(jap) * cos(ia) * cos(ia);
			n[j*12 +4] = R * r * sin(jap) *    cos(ia) + r * r * sin(jap) * cos(ia) * cos(ia);
			n[j*12 +5] = R * r * sin(ia)    + r * r * sin(ia) * cos(ia);

			n[j*12 +6] = R * r * cos(ja) *    cos(ia) + r * r * cos(ja) * cos(ia) * cos(ia);
			n[j*12 +7] = R * r * sin(ja) *    cos(ia) + r * r * sin(ja) * cos(ia) * cos(ia);
			n[j*12 +8] = R * r * sin(ia)    + r * r * sin(ia) * cos(ia);
			n[j*12 +9] = R * r * cos(ja) *    cos(iap) + r * r * cos(ja) * cos(iap) * cos(iap);
			n[j*12 +10] = R * r * sin(ja) *    cos(iap) + r * r * sin(ja) * cos(iap) * cos(iap);
			n[j*12 +11] = R * r * sin(iap)    + r * r * sin(iap) * cos(iap);
		}
		s3d_push_vertices(o, &v[i*SIDES*3], SIDES);
		s3d_push_lines(o,   &l[i*SIDES*6], SIDES*2);
		s3d_pep_line_normals(o, n, SIDES*2);
	}
	/* s3d_push_vertices(o,v,SIDES*RINGS);
	 s3d_push_lines(o,   l,SIDES*RINGS*2);*/

	s3d_flags_on(o, S3D_OF_VISIBLE);
	return(o);
}

static void set_focus(struct app *a)
{
	if (focus != a)
		/* resetting old focus */
		if (focus != NULL) {
			printf("unfocusing app name %s\n", focus->name);
			s3d_scale(focus->oid, 1 / focus->r);
			s3d_scale(focus->sphere, focus->r);

			s3d_flags_on(focus->sphere, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			s3d_flags_off(focus->min_but, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			s3d_translate(focus->title, -focus->textw - 1.2, 0.0, 0);
			s3d_translate(focus->close_but, bsize*focus->textw / 2, 1.2, 0);
			s3d_link(focus->close_but, focus->sphere);
			s3d_link(focus->oid, 0);
		}
	focus = a;
	if (a == NULL) {
		focus_r = n_app;
		s3d_mcp_focus(-1);
		s3d_flags_off(rotate, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_flags_off(reset, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		rot_flag = 0;
	} else {
		/* set the new focus app up */
		s3d_translate(a->title, -a->textw - 9.6, 0.0, 0);
		s3d_translate(reset, -7.2, 0.0, 0);
		s3d_translate(rotate, -4.8, 0.0, 0);
		s3d_translate(a->min_but, -2.4, 0.0, 0);
		s3d_link(a->close_but, 0);
		s3d_link(rotate, a->close_but);
		s3d_link(reset, a->close_but);
		s3d_flags_on(a->min_but, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_flags_on(rotate, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_flags_on(reset, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_flags_off(a->sphere, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_unlink(a->oid);
		s3d_rotate(a->oid,  0, 0, 0);
		s3d_translate(a->oid, 0, 0, 0);
		focus_r = a->r;
		s3d_scale(a->oid, 1);
		s3d_mcp_focus(a->oid);
	}
	place_apps();
}

static int add_app(struct app *a)
{
	struct app *prev = NULL, *a2 = apps;
	while ((a2) != NULL) {
		prev = a2;
		if (a2->oid == a->oid) { /*  already added, so free() and return */
			free(a);
			return(1);
		}
		a2 = a2->next;  /*  go to the end */
	}
	a->next = NULL;
	if (prev == NULL)
		apps = a;
	else
		prev->next = a;
	n_app += 1;
	return(0);
}

static struct app* find_app(int oid)
{
	struct app *a = apps;
	while (a != NULL) {
		if (oid == a->oid)
			break;
		a = a->next;
	}
	return(a);
}

static void* del_app(int oid)
{
	struct app *prev = NULL, *a = apps;
	while ((a != NULL)  && (a->oid != oid)) {
		prev = a;
		a = a->next;
	}
	if (a != NULL) { /*  found ... */
		if (a->init) {
			s3d_del_object(a->close_but);
			s3d_del_object(a->min_but);
			s3d_del_object(a->title);
			s3d_del_object(a->sphere);
			s3d_del_object(a->oid);
		}
		if (prev == NULL)
			apps = a->next;  /*  new head */
		else
			prev->next = a->next;
		n_app--;
		if (focus == a) {
			set_focus(NULL);
			focus = NULL;
		}
		free(a);
		place_apps();
	}
	return(a);
}

static int stop(struct s3d_evt* DOTMCPUNUSED(evt))
{
	s3d_quit();
	return(0);
}

void place_apps(void)
{
	struct app *a = apps;
	int j = 0;
	while (a != NULL) {
		if (a->init) {
			/*   printf("placing app [%d,'%s'], oid %d, r=%f\n",j,a->name,a->oid,a->r); */
			if (focus == a) {
				s3d_translate(a->close_but, (-left)*zoom - 0.4, (-bottom)*zoom - 0.4, -zoom);
			} else {
				s3d_translate(a->oid, zoom*(left) + j*2 + 1.0, zoom*bottom + 1.0, -zoom);
				s3d_rotate(a->oid, 0, 10, 0);
				j++;
			}
		}
		a = a->next;
	}
	s3d_translate(menu, left*zoom + 0.4, (-bottom)*zoom - 0.4, -zoom);

	s3d_rotate(menu, 0 , 30, 0);
}

static int mcp_object(struct s3d_evt *hrmz)
{
	struct mcp_object *mo;
	struct app *a;
	mo = (struct mcp_object *)hrmz->buf;
	if (NULL == (a = find_app(mo->object))) {
		printf("adding new object ......");
		a = (struct app*)malloc(sizeof(struct app));
		a->oid = mo->object;
		a->r = mo->r;
		strncpy(a->name, mo->name, 256);
		a->init = 0;
		add_app(a);
		place_apps();
		printf("..%s\n", a->name);
	} else {
		/*  printf("updating app %d\n",a->oid);*/
		a->trans_x = mo->trans_x;
		a->trans_y = mo->trans_y;
		a->trans_z = mo->trans_z;
		a->r = mo->r;
		if (a->init) {
			if (a == focus) {
				focus_r = a->r;
			} else {
				s3d_scale(a->sphere, a->r);
				s3d_scale(a->oid, 1 / a->r);
			}
		}
		place_apps();
	}
	return(0);
}

static void app_init(struct app *a)
{
	printf("building some window decorations on %d ['%s']\n", a->oid, a->name);
	printf("radius of object %d is %f\n", a->oid, a->r);
	s3d_scale(a->oid,  1 / a->r);

	a->sphere = s3d_clone(sphere);
	s3d_scale(a->sphere, a->r);
	s3d_link(a->sphere, a->oid);
	s3d_flags_on(a->sphere, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);

	a->title = s3d_draw_string(a->name, &a->textw);

	a->close_but = s3d_clone(close_but);
	s3d_translate(a->close_but, bsize*a->textw / 2.0, 1.2f, 0.0f);
	s3d_link(a->close_but, a->sphere);
	s3d_flags_on(a->close_but, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);


	s3d_translate(a->title, -a->textw - 1.2f, 0.0f, 0.0f);
	s3d_link(a->title, a->close_but);
	s3d_flags_on(a->title, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);

	a->min_but = s3d_clone(min_but);
	s3d_link(a->min_but, a->close_but);


	s3d_scale(a->close_but, bsize);

	s3d_link(a->oid, 0);
	a->init = 1;
	/* if (focus==NULL)
	  set_focus(a);
	 else*/
	place_apps();
}

static int mcp_del_object(struct s3d_evt *hrmz)
{
	struct mcp_object *mo;
	mo = (struct mcp_object *)hrmz->buf;
	del_app(mo->object);
	return(0);
}

static int object_click(struct s3d_evt *hrmz)
{
	struct app *a;
	unsigned int i;
	int oid;
	oid = *((int *)hrmz->buf);
	a = apps;
	i = 0;
	if (oid == rotate) {
		rot_flag = !rot_flag;
		return(0);
	}
	if (oid == reset) {
		s3d_translate(0, 0.0, 0.0, 5.0);
		s3d_rotate(0, 0, 0, 0);
	}
	while (a != NULL) {
		if (oid == a->close_but) {
			del_app(a->oid);
			return(0);
		} else  if (oid == a->min_but) {
			if (a == focus) {
				set_focus(NULL); /* nothing is focused now */
			}
			return(0);
		} else  if (((oid == a->title) || (oid == a->sphere)) || (oid == a->oid)) {
			printf("giving focus to [%s], %d\n", a->name, oid);
			set_focus(a);
			return(0);
		}
		i++;
		a = a->next;
	}
	menu_click(oid);
	return(0);
}

static int object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf = (struct s3d_obj_info *)hrmz->buf;
	if (inf->object == 0) {
		campos.x = inf->trans_x;
		campos.y = inf->trans_y;
		campos.z = inf->trans_z;
		camrot.x = inf->rot_x;
		camrot.y = inf->rot_y;
		camrot.z = inf->rot_z;

		if (asp != inf->scale) {
			asp = inf->scale;
			printf("screen aspect: %f\n", asp);
			if (asp > 1.0) { /* wide screen */
				bottom = -1.0;
				left = -asp;
			} else {  /* high screen */
				bottom = (-1.0 / asp);
				left = -1.0;
			}
			place_apps(); /* replace apps */
		}
	}
	return(0);
}

static void mainloop(void)
{
	struct app *a;
	float al, r;
	int i;
	a = apps;
	i = 0;
	while (a != NULL) {
		if (!a->init)
			app_init(a);
		i++;
		a = a->next;
	}
	if (rot_flag) {
		al = (alpha * M_PI / 180);
		r = (focus_r > 20.0) ? 20.0 : focus_r;
		s3d_translate(0, sin(al)*(r + 5), 0, cos(al)*(r + 5));
		s3d_rotate(0, 0, alpha, 0);
		alpha = alpha + 0.1;
		if (alpha > 360.0) alpha = 0.0;
	}
	if (ego_mode) {
		if ((ydif != 0) || (xdif != 0)) {
			campos.x += ydif * sin((camrot.y * M_PI) / 180);
			campos.z += ydif * cos((camrot.y * M_PI) / 180);
			campos.x += xdif * cos((-camrot.y * M_PI) / 180);
			campos.z += xdif * sin((-camrot.y * M_PI) / 180);
			campos.y += ydif * sin((-camrot.x * M_PI) / 180);
			s3d_translate(0, campos.x, campos.y, campos.z);
		}
	}
	nanosleep(&t, NULL);
}

static int keydown(struct s3d_evt *event)
{
	struct s3d_key_event *keys = (struct s3d_key_event *)event->buf;
	switch (keys->keysym) {
	case S3DK_F2:
		ego_mode = (ego_mode + 1) % 2;
		xdif = 0;
		ydif = 0;
		printf("ego mode %d\n", ego_mode);
		break;
	case 'w':
		ydif += -1.0;
		break;
	case 'a':
		xdif += -1.0;
		break;
	case 's':
		ydif += 1.0;
		break;
	case 'd':
		xdif += 1.0;
		break;
	}
	return(0);
}

static int keyup(struct s3d_evt *event)
{
	struct s3d_key_event *keys = (struct s3d_key_event *)event->buf;
	switch (keys->keysym) {
	case 'w':
		ydif -= -1.0;
		break;
	case 'a':
		xdif -= -1.0;
		break;
	case 's':
		ydif -= 1.0;
		break;
	case 'd':
		xdif -= 1.0;
		break;
	}
	return(0);

}

int main(int argc, char **argv)
{
	s3d_set_callback(S3D_EVENT_OBJ_INFO, object_info);
	s3d_set_callback(S3D_MCP_OBJECT, mcp_object);
	s3d_set_callback(S3D_EVENT_QUIT, stop);
	s3d_set_callback(S3D_MCP_DEL_OBJECT, mcp_del_object);
	s3d_set_callback(S3D_EVENT_OBJ_CLICK, object_click);
	s3d_set_callback(S3D_EVENT_KEYDOWN, keydown);
	s3d_set_callback(S3D_EVENT_KEYUP, keyup);

	if (!s3d_init(&argc, &argv, "mcp")) {
		if (!((argc > 1) && (0 == strcmp(argv[1], "--notorus"))))
			greentorus(); /* just call ... */

		if (s3d_select_font("vera")) {
			printf("font not found\n");
		}
		min_but = s3d_import_model_file("objs/btn_minimize.3ds");
		rotate = s3d_import_model_file("objs/btn_rotate.3ds");
		reset = s3d_import_model_file("objs/reset.3ds");
		close_but = s3d_import_model_file("objs/btn_close.3ds");
		sphere = s3d_import_model_file("objs/ringsystem.3ds");
		menu = menu_init();
		s3d_link(menu, 0);
		s3d_scale(menu, bsize);
		place_apps();
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
