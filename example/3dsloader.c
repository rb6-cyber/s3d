/*
 * 3dsloader.c
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
#include <stdio.h>  /*  NULL */
#include <unistd.h>  /* usleep() */
int i,oid;
void mainloop()
{
	s3d_rotate(oid,0,i,0);
	i++;
	usleep(100000);
}
void object_click(struct s3d_evt *evt)
{
	s3d_quit();
}
	
int main (int argc, char **argv)
{
	if (argc<2)
	{
		printf("usage: %s [somefile.3ds]\n",argv[0]);
		return(-1);
	}
	if (!s3d_init(&argc,&argv,"3dsloader"))	
	{
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
	    if (-1!=(oid=s3d_import_3ds_file(argv[1])))
		{
		    s3d_flags_on(oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_mainloop(mainloop);
		} else {
			printf("file not found ... \n");
		}
		s3d_quit();
	}
	return(0);
}
