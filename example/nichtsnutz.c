/*
 * nichtsnutz.c
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
#include <s3d_keysym.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

static int object, foll;
static float al, r, alpha = 0.0, angle;
static float CamPosition[2][3],
TmpMove[3],
Tmp[3],
RotCam[2][3],
CatPos[3];

static float length;

static void mainloop(void)
{

	al = (alpha * (float)M_PI / 180.f);
	r = 5.0f;

	CatPos[0] = sinf(al) * r;
	CatPos[1] = 0;
	CatPos[2] = cosf(al) * r;
	s3d_translate(object, CatPos[0] , CatPos[1], CatPos[2]);
	s3d_rotate(object, 0, alpha, 0);
	alpha = alpha + 0.1f;
	if (alpha > 360.0f) alpha = 0.0f;

	length = s3d_vector_length(CatPos);


	RotCam[0][0] = (CatPos[0] * 12.0f) / length;
	RotCam[0][1] = (CatPos[1] * 12.0f) / length;
	RotCam[0][2] = (CatPos[2] * 12.0f) / length;


	if (foll) {

		CamPosition[0][0] = ((CamPosition[0][0] * 4 + RotCam[0][0]) / 5);
		CamPosition[0][1] = ((CamPosition[0][1] * 4 + RotCam[0][1]) / 5);
		CamPosition[0][2] = ((CamPosition[0][2] * 4 + RotCam[0][2]) / 5);
		s3d_translate(0, CamPosition[0][0], CamPosition[0][1], CamPosition[0][2]);

		TmpMove[0] = 0.0;
		TmpMove[1] = 0.0;
		TmpMove[2] = -1.0;

		Tmp[0] = CamPosition[0][0] - CatPos[0];
		Tmp[1] = 0.0;
		Tmp[2] = CamPosition[0][2] - CatPos[2];

		angle = s3d_vector_angle(Tmp, TmpMove);
		angle = (CatPos[0] > 0) ? (180.f - (180.f / (float)M_PI * angle)) : (180.f + (180.f / (float)M_PI * angle));
		printf("%f %f\n", angle, al);

		CamPosition[1][1] = (CamPosition[1][1] * 4 + angle) / 5;
		s3d_rotate(0, CamPosition[1][0], CamPosition[1][1], CamPosition[1][2]);
	}



	nanosleep(&t, NULL);
}

static int object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf = (struct s3d_obj_info *)hrmz->buf;

	if (inf->object == 0) {
		CamPosition[0][0] = inf->trans_x;
		CamPosition[0][1] = inf->trans_y;
		CamPosition[0][2] = inf->trans_z;
		CamPosition[1][0] = inf->rot_x;
		CamPosition[1][1] = inf->rot_y;
		CamPosition[1][2] = inf->rot_z;
	}
	return(0);
}



static int keypress(struct s3d_evt *event)
{
	int key;
	key = *((unsigned short *)event->buf);
	switch (key) {
	default:
		;
	}
	return(0);
}


int main(int argc, char **argv)
{

	if (!s3d_init(&argc, &argv, "running cat")) {
		s3d_set_callback(S3D_EVENT_KEY, keypress);
		s3d_set_callback(S3D_EVENT_OBJ_INFO, object_info);

		object = s3d_import_model_file("objs/katze_body.3ds");
		s3d_flags_on(object, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
