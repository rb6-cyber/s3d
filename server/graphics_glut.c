/*
 * graphics_glut.c
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


/*  this is the file for operation with glut ... */
/*  maybe we are able to choose if we want to use glut or sdl or any other lib later ... */

#include "global.h"
#include <stdio.h>		/* NULL */
#include <GL/glut.h> 	 /*  all the glut functions */
#include <GL/gl.h>		 /*  of course, the gl header */
/*  glut version of graphics init ... */
int graphics_init_glut()
{
	/* XXX: Faking argc and argv is probably not a good idea. */
	int argc=1;
	char *argv[]={"s3d", NULL};
	s3dprintf(MED,"Using GLUT for GL/windowing ...");
 	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize (X_RES, Y_RES);
	glutCreateWindow("grmbl");
    glutIdleFunc(one_time);
	if (0!=(atexit(quit)))
		s3dprintf(MED,"Error in setting Exit function ...");
	glutDisplayFunc(graphics_main);
	glutReshapeFunc(graphics_reshape);
	return(0);
}
/*  nothing to be done ... */
int graphics_quit_glut()
{
	return(0);	
}
