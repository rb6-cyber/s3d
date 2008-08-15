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
#define N_SMOOTH		8		/* how much data is used to normalize */

static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */


/* "public" wii-data */
static float wii_x[3] = {0, 0, 0};
static float wii_move[3] = {0, 0, 0};
static float wii_xrot = 0;
static float wii_zrot = 0;

/* private wii-data */
static float x_data[N_SMOOTH][3];
static float move_data[N_SMOOTH][3];
static int dataindex;

static cwiid_err_t err;
static cwiid_wiimote_t *wiimote; /* wiimote handle */

static int oid_head;

static void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}


int wii_init(char *addr) 
{
	int i,j;
	bdaddr_t bdaddr; /* bluetooth device address */
	cwiid_set_err(err);

	/* Connect to address given on command-line, if present */
	if (addr != NULL) {
		str2ba(addr, &bdaddr);
	} else {
		/* BDADDR_ANY */
		memset(&bdaddr, 0, sizeof(bdaddr_t));
	}

	/* Connect to the wiimote */
	printf("Put Wiimote in discoverable mode now (press 1+2)...\n");

	if (!(wiimote = cwiid_open(&bdaddr, 0))) {
		fprintf(stderr, "Unable to connect to wiimote\n");
		return -1;
	}

	cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN | CWIID_RPT_ACC | CWIID_RPT_IR | CWIID_RPT_NUNCHUK | CWIID_RPT_CLASSIC);

	for (i = 0; i < N_SMOOTH; i++)
		for (j = 0; j < 3; j++) {
			x_data[i][j] = 0;
			move_data[i][j] = 0;
		}
	dataindex = 0;
	return(0);

}
void wii_calcdata() 
{
	struct cwiid_state state; /* wiimote state */
	float z_normvec[3] = { 0, -1, 0};
	float x_normvec[3] = { 0, 0, 1};
	float y[3];
	int i,j;
	int lastindex;

	cwiid_get_state(wiimote, &state);

	x_data[dataindex][0] = (state.acc[CWIID_X] - 128) / 128.0;
	x_data[dataindex][2] = (state.acc[CWIID_Y] - 128) / 128.0;
	x_data[dataindex][1] = (state.acc[CWIID_Z] - 128) / 128.0;

	lastindex = (dataindex + N_SMOOTH - 1) % N_SMOOTH;
	move_data[dataindex][0] =   x_data[lastindex][0] - x_data[dataindex][0];
	move_data[dataindex][1] =   x_data[lastindex][1] - x_data[dataindex][1];
	move_data[dataindex][2] = - x_data[lastindex][2] + x_data[dataindex][2];
	dataindex = (dataindex + 1)%N_SMOOTH;
	/* smooth it */
	for (j = 0; j < 3; j++) {
		wii_x[j] = 0;
		wii_move[j] = 0;
	}
	for (i = 0; i< N_SMOOTH; i++) 
		for (j = 0; j < 3; j++) {
			wii_x[j] +=    x_data[i][j];
			wii_move[j] += move_data[i][j];
	}
	for (j = 0; j< 3; j++) {
		wii_x[j] /= N_SMOOTH;
		wii_move[j] /= N_SMOOTH;
	}

	wii_xrot = s3d_vector_angle(x_normvec, wii_x);
	wii_xrot = 90 - (180.0 / M_PI * wii_xrot);

	y[0] = wii_x[0];
	y[1] = wii_x[1];
	y[2] = 0;

	wii_zrot = s3d_vector_angle(z_normvec, y);
	/* take care of inverse cosinus */
	if (wii_x[0] < 0)   wii_zrot = 180 - (180.0 / M_PI * wii_zrot);
	else        		wii_zrot = 180 + (180.0 / M_PI * wii_zrot);
}


static void mainloop(void)
{
	wii_calcdata();

	s3d_rotate(oid_head, wii_xrot, 0, wii_zrot);
	s3d_translate(oid_head, wii_move[0]*10, wii_move[1]*10, wii_move[2]*10);

	nanosleep(&t, NULL);
}

int main(int argc, char *argv[])
{
	char *addr;

	if (argc > 1)
		addr = argv[1];
	else
		addr = NULL;

	if (wii_init(addr))
		return(-1);

	if (!s3d_init(&argc, &argv, "wiitest")) {
		oid_head = s3d_import_model_file("objs/snow_head.3ds");
		s3d_flags_on(oid_head, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}

	return 0;
}

