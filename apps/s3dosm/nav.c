/*
 * nav.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "s3dosm.h"
#include <s3d.h>
#include <string.h> /* strcmp() */
#include <stdlib.h> /* strtod() */
#include <stdio.h> /* printf() */
#include <math.h> /* fabs() */

static int oidx;
int oidy;
static float lat, lon, tlat, tlon;
/* load rotation centers */
void nav_init(void)
{
	lat = lon = tlat = tlon = 0.0;
	oidx = s3d_new_object();
	oidy = s3d_new_object();
	s3d_link(oidy, oidx);
	s3d_translate(oidx, 0, -ESIZE*RESCALE - VIEWHEIGHT, 0);
	s3d_scale(oidx, RESCALE);
}
/* center to given latitude longitude */
void nav_center(float la, float lo)
{
	tlat = la;
	tlon = lo;
}
void nav_campos(float campos[3], float earthpos[3])
{

	float tmp1[3], tmp2[3];
	float alpha;

	tmp1[0] = campos[0];
	tmp1[1] = campos[1] + ESIZE * RESCALE + VIEWHEIGHT;
	tmp1[2] = campos[2];

	alpha = (90 - lat) * M_PI / 180.0;
	tmp2[0] =  tmp1[0];
	tmp2[1] =  tmp1[1] * cos(alpha) - tmp1[2] * sin(alpha);
	tmp2[2] =  tmp1[1] * sin(alpha) + tmp1[2] * cos(alpha);

	alpha = lon * M_PI / 180.0;
	tmp1[0] =  tmp2[0] * cos(alpha) + tmp2[2] * sin(alpha);
	tmp1[1] =  tmp2[1];
	tmp1[2] =  - tmp2[0] * sin(alpha) + tmp2[2] * cos(alpha);



	earthpos[0] = tmp1[0];
	earthpos[1] = tmp1[1];
	earthpos[2] = tmp1[2];

}
void nav_main(void)
{
	float x[3];
	if ((fabs(tlat - lat) > 0.00001) && (fabs(tlon - lon) > 0.00001)) {
		if (lat == 0.0 && lon == 0.0) {
			lat = tlat;
			lon = tlon;
		} else {
			lat = (tlat + lat * 15) / 16;
			lon = (tlon + lon * 15) / 16;
		}
	} else {
		lat = tlat;
		lon = tlon;
	}
	s3d_rotate(oidy, 0, -lon, 0);
	s3d_rotate(oidx, -(90 - lat), 0, 0);
	calc_earth_to_eukl(lon, lat, 0, x);
}

static int get_center(void *data, int argc, char **argv, char **azColName)
{
	float *med = (float *)data;
	int i;
	med[0] = 0;
	med[1] = 0;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(azColName[i], "la"))   med[0] = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "lo"))  med[1] = strtod(argv[i], NULL);
		}
	}
	return(0);
}
/* returns the heading in degress of position P1 -> P2 */
float get_heading(float la1, float lo1, float la2, float lo2)
{
	float p1_north[3], p1[3], p2[3];
	float dir[3], north[3];
	float angle;
	int i;
	calc_earth_to_eukl(la1, lo1, 0, p1);
	calc_earth_to_eukl(la2, lo2, 0, p2);
	calc_earth_to_eukl(la1 + 1, lo1, 0, p1_north);
	for (i = 0;i < 3;i++)  north[i] = p1_north[i] - p1[i];
	for (i = 0;i < 3;i++)  dir[i] = p2[i] - p1[i];
	angle = s3d_vector_angle(dir, north);
	angle = angle * 180.0 / M_PI;
	if ((lo2 > lo1) || (lo1 - lo2 > 180.0))  angle = 360 - angle;
	return(angle);
}
/* find some good center on our own */
void nav_autocenter(void)
{
	float med[2];
	char query[] = "SELECT avg(longitude) as lo, avg(latitude) as la FROM node; ";
	db_exec(query, get_center, med);
	nav_center(med[0], med[1]);
	printf("center to %f,%f\n", med[0], med[1]);
}
