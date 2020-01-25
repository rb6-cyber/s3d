// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <stdio.h>   /*  snprintf(), printf(), NULL */
#include <s3d.h>
#include <s3dw.h>
#include "s3dosm.h"
#include <time.h>  /*  nanosleep(), struct tm, time_t...  */
static int ready = 0;

void mainloop(void)
{
	struct timespec t = {
		0, 100*1000*1000
	}; /* 100 mili seconds */
	if (ready) {
		nanosleep(&t, NULL);
		gps_main();
		nav_main();
		s3dw_ani_mate();
	}
}
static int init(int argc, char **argv)
{
	s3d_select_font("vera");
	ui_init();
	if (db_init(":memory:")) return -1;
	if (db_create()) return -1;
	if (process_args(argc, argv)) return -1;
	nav_init();
	nav_autocenter();
	draw_all_layers();
	gps_init("localhost");
	ready = 1;
	return 0;
}
static int quit(void)
{
	ready = 0;
	gps_quit();
	s3d_quit();
	db_quit();
	return 0;
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "s3dosm")) {
		if (!init(argc, argv)) s3d_mainloop(mainloop);
		quit();
	} else return -1;
	return 0;
}
