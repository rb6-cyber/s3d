// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <s3d.h>
#include "s3dosm.h"
#include <stdio.h>  /* printf() */
#ifdef HAVE_GPS
static int user_icon = -1, user_icon_rotator = -1;
#include <gps.h>  /* gps_*() */

#include <errno.h>  /* errno */
#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* strlen() */
#include <stdio.h>   /*  snprintf(), printf(), NULL */
#include <time.h>  /*  nanosleep(), struct tm, time_t...  */
#include <math.h> /* fabs(), finite () */
#include <fcntl.h>  /* fcntl() */
#include <unistd.h> /* fcntl() */
static struct gps_data_t  *dgps;
static int       frame = 0;
static int       lastfix = 0;
static int       gps_active = 0;
static int      gps_info = -1;
static float      lat, lon, tlat, tlon; /* we have the same in nav.c, this one is for the user icon ... */
static float     lat_old, lon_old;
static float     speed_old = 0.0;
void      show_gpsdata(struct gps_data_t *dgps);
static void     show_position(struct gps_data_t *dgps);

void show_gpsdata(struct gps_data_t *dgps)
{
#if GPSD_API_MAJOR_VERSION >= 9
	if (0 == dgps->online.tv_sec &&
	    0 == dgps->online.tv_nsec)
#else
	if (!dgps->online)
#endif
		printf("WARNING: no connection to gps device\n");

	printf("[%d] lat/long: [%f|%f], altitude %f\n", frame, dgps->fix.latitude, dgps->fix.longitude, dgps->fix.altitude);
	printf("speed [kph]: %f\n", dgps->fix.speed / KNOTS_TO_KPH);
	printf("used %d/%d satellits\n", dgps->satellites_used, dgps->satellites_visible);

#if GPSD_API_MAJOR_VERSION >= 10
	switch (dgps->fix.status) {
#else
	switch (dgps->status) {
#endif
	case STATUS_NO_FIX:
		printf("status: no fix\n");
		break;
	case STATUS_FIX:
		printf("status: fix\n");
		break;
#ifdef STATUS_DGPS_FIX
	case STATUS_DGPS_FIX:
		printf("status: dgps fix\n");
		break;
#endif
	}

	switch (dgps->fix.mode) {
	case MODE_NOT_SEEN:
		printf("mode: not seen yet\n");
		break;
	case MODE_NO_FIX:
		printf("mode: no fix\n");
		break;
	case MODE_2D:
		printf("mode: 2d fix\n");
		break;
	case MODE_3D:
		printf("mode: 3d fix\n");
		break;
	}
}
#define BUFSIZE  1024
static void show_position(struct gps_data_t *dgps)
{
	int fix = 1;
	float la, lo, heading, speed, slen;
	char buf[BUFSIZE+1];

#if GPSD_API_MAJOR_VERSION >= 9
	if (0 == dgps->online.tv_sec &&
	    0 == dgps->online.tv_nsec)
#else
	if (!dgps->online)
#endif
		fix = 0;

	switch (dgps->fix.mode) {
	case MODE_NOT_SEEN:
		fix = 0;
		break;
	case MODE_NO_FIX:
		fix = 0;
		break;
	}

	la = dgps->fix.latitude;
	lo = dgps->fix.longitude;
	heading = -dgps->fix.track;
	speed = dgps->fix.speed;

	tlat = la;
	tlon = lo;
	if (fix) {
		printf("have a fix\n");
		nav_center(la, lo);
		if (!finitef(heading)) {
			heading = get_heading(lat_old, lon_old, la, lo);
			if (!lastfix)   {
				s3d_scale(user_icon, 1.0 / RESCALE);
			}
		}
		if (finitef(heading))  s3d_rotate(user_icon, 0, heading, 0); /* wrong rotation? */
		if (finitef(speed)) {
			/* print some information */
			snprintf(buf, BUFSIZE, "speed: %3.2f km/h", speed*3.6);
			speed_old = speed;
		} else
			snprintf(buf, BUFSIZE, "speed: NA (old: %3.2f km/h)", speed_old*3.6);

		if (gps_info != -1) s3d_del_object(gps_info);
		gps_info = s3d_draw_string(buf, &slen);
		s3d_translate(gps_info, -slen / 2, 1, 0);
		s3d_link(gps_info, user_icon);
		s3d_flags_on(gps_info, S3D_OF_VISIBLE);
	}


	lat_old = la;
	lon_old = lo;
	lastfix = fix;
}
int gps_init(const char *gpshost)
{
	int sock_opts;
	const char *err_str;
#if GPSD_API_MAJOR_VERSION >= 5
	dgps = malloc(sizeof(*dgps));
	if (dgps != NULL)
		if (gps_open(gpshost, "2947", dgps)) {
			free(dgps);
			dgps = NULL;
		}
#else
	dgps = gps_open(gpshost, "2947");
#endif
	if (dgps == NULL) {
		switch (errno) {
		case NL_NOSERVICE:
			err_str = "can't get service entry";
			break;
		case NL_NOHOST:
			err_str = "can't get host entry";
			break;
		case NL_NOPROTO:
			err_str = "can't get protocol entry";
			break;
		case NL_NOSOCK:
			err_str = "can't create socket";
			break;
		case NL_NOSOCKOPT:
			err_str = "error SETSOCKOPT SO_REUSEADDR";
			break;
		case NL_NOCONNECT:
			err_str = "can't connect to host";
			break;
		default:
			err_str = "Unknown";
			break;
		}
		/*  printf("no connection to gpsd\n");*/
		fprintf(stderr, "s3dosm: no gpsd running or network error: %d, %s\n" ,  errno, err_str);
		return -1;
	}
	sock_opts = fcntl(dgps->gps_fd, F_GETFL, 0);
	fcntl(dgps->gps_fd, F_SETFL, sock_opts | O_NONBLOCK);

	user_icon = s3d_clone(icons[ICON_ARROW].oid);
	user_icon_rotator = s3d_new_object();
	s3d_link(user_icon, user_icon_rotator);
	s3d_link(user_icon_rotator, oidy);
	s3d_flags_on(user_icon, S3D_OF_VISIBLE);
	s3d_scale(user_icon, 1.0 / RESCALE);
	tlat = lat = lat_old = 0.0;
	tlon = lon = lon_old = 0.0;
	gps_active = 1;
	gps_stream(dgps, WATCH_ENABLE, NULL);
	return 0;
}
int gps_main(void)
{
	if (gps_active && ((frame % 6) == 0)) {

#if GPSD_API_MAJOR_VERSION >= 7
		if (gps_read(dgps, NULL, 0) < 0) {
#elif GPSD_API_MAJOR_VERSION >= 5
		if (gps_read(dgps) < 0) {
#else
		if (gps_poll(dgps) < 0) {
#endif
			if (errno != EWOULDBLOCK) {

				printf("read error on server socket\n");
				gps_quit();
			}
		}

		/*show_gpsdata(dgps);*/
		show_position(dgps);
	}
	if ((fabs(tlat - lat) > 0.00001) && (fabs(tlon - lon) > 0.00001)) {
		if (lat == 0.0 && lon == 0.0) {
			lat = tlat;
			lon = tlon;
		} else {
			lat = (tlat + lat * 7) / 8;
			lon = (tlon + lon * 7) / 8;
		}
	} else {
		tlat = lat;
		tlon = lon;
	}
	draw_translate_icon(user_icon_rotator, lat, lon);
	frame++;
	return 0;
}
int gps_quit(void)
{
	if (gps_active) {
		printf("deactivating gps-connection ...\n");
		gps_active = 0;
		gps_close(dgps);
#if GPSD_API_MAJOR_VERSION >= 5
		free(dgps);
		dgps = NULL;
#endif
	}
	return 0;
}
#else

int gps_init(const char *S3DOSMUNUSED(gpshost))
{
	printf("GPS support not compiled in!\n");
	return 0;
}
int gps_main(void)
{
	return 0;
}
int gps_quit(void)
{
	return 0;
}
#endif
