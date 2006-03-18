/*
 * user_glut.c
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


#include "global.h"
#include <GL/glut.h> 	 /*  all the glut functions */
/*  local prototypes */
void keyboard(unsigned char key, int x, int y);
void special(int skey, int x, int y);
void mouse_motion(int x, int y);
void passive_mouse_motion(int x, int y);
extern int but;
/*  init user input things for glut */
int user_init_glut()
{
	dprintf(MED,"using GLUT for user input");
	glutKeyboardFunc (keyboard);
	glutSpecialFunc (special);
	glutMouseFunc (user_mouse);
    glutMotionFunc(mouse_motion);
	glutPassiveMotionFunc(passive_mouse_motion);
	return(0);
}

void keyboard(unsigned char key, int x, int y)
{
	user_key(key,0);
}
void special(int skey, int x, int y)
{
	user_key(skey,0);
}
void mouse_motion(int x, int y)
{
	user_mouse(but,2,x,y);
}
void passive_mouse_motion(int x, int y)
{
	user_mouse(-1,-1,x,y);
}
