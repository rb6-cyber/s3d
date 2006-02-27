/*
 * config.h
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


/*  to be filled with configure options ...  */
/*   */
/*  on which port do we listen? */
#define S3D_PORT	6066
/*  resolution on startup */
#define X_RES	800
#define Y_RES	600
/* how many frames to wait until test the connection if it's still here */
#define MAX_IDLE	50
/*  this is to be set dynamicly later on */
#define VLOW	1
#define	LOW		2
#define MED		3
#define HIGH	4
#define	VHIGH	5
/*  which is the minimum level of debugmessage we want to see? */
#define DEBUG	LOW
#ifndef DEBUG
#define errds(...) /* nothing */
#define dprintf(...) /* nothing */
#endif

/*  which subsystem do we use for rendering and ? */

/* GLUT is the GL utility library which you obtain at 
 * http://freeglut.sourceforge.net/  
 */
#define G_GLUT	1
/* SDL is a framework for simple media access which contains
 * opengl support besides music, cdrom etc.
 */
/*  #define G_SDL	2 */
/*  do we want signals? usually, yes. it makes network things  */
/*  with polling a lot faster and  */
/*  we can go down properly on a terminate signal... */
/*  windows does not support that, so ... */
#ifndef WIN32
#define SIGS	1
#define SHM		1
#endif
#define TCP		1
