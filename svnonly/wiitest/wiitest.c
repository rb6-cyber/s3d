/*
 * wiitest.c
 *
 * Copyright (C) 2007 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of wiitest, ripped from wmdemo.c to
 * See http://s3d.berlios.de/ for more updates.
 *
 * wiitest is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wiitest is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wiitest; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwiid.h>
#include <s3d.h>
#include <math.h> /* sin() */
#include <time.h> /* nanosleep() */

static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */


static float x[3] = {0, 0, 0};
static float move[3] = {0, 0, 0};

static cwiid_err_t err;
static int oid_head;
static cwiid_wiimote_t *wiimote; /* wiimote handle */

static void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}
static void mainloop(void)
{
	struct cwiid_state state; /* wiimote state */
	float z_normvec[3] = { -1, 0, 0};
	float x_normvec[3] = { 0, 1, 0};
	float xrot, zrot;
	float y[3];

	cwiid_get_state(wiimote, &state);

	y[0] = (state.acc[CWIID_X] - 128) / 128.0;
	y[1] = (state.acc[CWIID_Y] - 128) / 128.0;
	y[2] = (state.acc[CWIID_Z] - 128) / 128.0;

	/* smooth it */
	x[0] = (x[0] * 9 + y[0]) / 10;
	x[1] = (x[1] * 9 + y[1]) / 10;
	x[2] = (x[2] * 9 + y[2]) / 10;

	move[0] = ((x[0] - y[0]) + move[0] * 9) / 10;
	move[1] = ((x[2] - y[2]) + move[1] * 9) / 10;
	move[2] = ((y[1] - x[1]) + move[2] * 9) / 10;

	xrot = s3d_vector_angle(x_normvec, x);
	xrot = 90 - (180.0 / M_PI * xrot);

	/* z_normvec[0] = -cos(xrot * M_PI/180) ;
	 z_normvec[1] = 0;
	 z_normvec[2] = +sin(xrot * M_PI/180) ;
	 printf("zn = %3.3f %3.3f %3.3f    ", z_normvec[0], z_normvec[1], z_normvec[2]);*/

	zrot = s3d_vector_angle(z_normvec, x);
	/* take care of inverse cosinus */
	if (x[2] > 0)      zrot = 180 - (180.0 / M_PI * zrot);
	else        zrot = 180 + (180.0 / M_PI * zrot);
	zrot -= 90;

	printf("%3.3f %3.3f %3.3f, zrot = %3.3f, xrot = %3.3f\n", x[0], x[1], x[2], zrot, xrot);

	s3d_rotate(oid_head, xrot, 0, zrot);
	s3d_translate(oid_head, move[0]*10, move[1]*10, move[2]*10);

	nanosleep(&t, NULL);
}

int main(int argc, char *argv[])
{
	bdaddr_t bdaddr; /* bluetooth device address */

	cwiid_set_err(err);

	/* Connect to address given on command-line, if present */
	if (argc > 1) {
		str2ba(argv[1], &bdaddr);
	} else {
		bdaddr = *BDADDR_ANY;
	}

	/* Connect to the wiimote */
	printf("Put Wiimote in discoverable mode now (press 1+2)...\n");

	if (!(wiimote = cwiid_open(&bdaddr, 0))) {
		fprintf(stderr, "Unable to connect to wiimote\n");
		return -1;
	}

	cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN | CWIID_RPT_ACC | CWIID_RPT_IR | CWIID_RPT_NUNCHUK | CWIID_RPT_CLASSIC);


	if (!s3d_init(&argc, &argv, "wiitest")) {
		oid_head = s3d_import_model_file("objs/snow_head.3ds");
		s3d_flags_on(oid_head, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}

	return 0;
}

