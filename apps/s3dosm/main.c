/*
 * main.c
 * 
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
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


#include <stdio.h>		 /*  snprintf(), printf(), NULL */
#include <s3d.h>
#include "s3dosm.h"
#include <time.h>	 /*  nanosleep(), struct tm, time_t...  */

void mainloop()
{
	struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
	nanosleep(&t,NULL); 
}
int init(int argc, char **argv)
{
	if (db_init(":memory:")) return(-1);
	if (db_create()) return(-1);
	if (process_args(argc,argv)) return(-1);
	nav_init();
	nav_autocenter();
	draw_all_layers();
	return(0);
}
int quit()
{
	s3d_quit();
	db_quit();
	return(0);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc,&argv,"s3dosm"))
	{
		if (!init(argc, argv)) s3d_mainloop(mainloop);
		quit();
	} else return(-1);
	return(0);
}
