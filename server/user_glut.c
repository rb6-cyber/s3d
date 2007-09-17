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
#include <s3d_keysym.h> /* our very own (haha) keysyms */
/*  local prototypes */
void keyboard(uint8_t key, int x, int y);
void special(int skey, int x, int y);
void mouse_motion(int x, int y);
void passive_mouse_motion(int x, int y);
extern int but;
/*  init user input things for glut */
int user_init_glut()
{
	s3dprintf(MED,"using GLUT for user input");
	glutKeyboardFunc (keyboard);
	glutSpecialFunc (special);
	glutMouseFunc (user_mouse);
	glutMotionFunc(mouse_motion);
	glutPassiveMotionFunc(passive_mouse_motion);
	return(0);
}

void keyboard(uint8_t key, int x, int y)
{
	user_key(key,key,0,0);
	user_key(key,key,0,1);
}
void special(int skey, int x, int y)
{
	uint16_t mkey;
	switch (skey) { /* handle special keys */
	case GLUT_KEY_F1:
		mkey=S3DK_F1;
		break;
	case GLUT_KEY_F2:
		mkey=S3DK_F2;
		break;
	case GLUT_KEY_F3:
		mkey=S3DK_F3;
		break;
	case GLUT_KEY_F4:
		mkey=S3DK_F4;
		break;
	case GLUT_KEY_F5:
		mkey=S3DK_F5;
		break;
	case GLUT_KEY_F6:
		mkey=S3DK_F6;
		break;
	case GLUT_KEY_F7:
		mkey=S3DK_F7;
		break;
	case GLUT_KEY_F8:
		mkey=S3DK_F8;
		break;
	case GLUT_KEY_F9:
		mkey=S3DK_F9;
		break;
	case GLUT_KEY_F10:
		mkey=S3DK_F10;
		break;
	case GLUT_KEY_F11:
		mkey=S3DK_F11;
		break;
	case GLUT_KEY_F12:
		mkey=S3DK_F12;
		break;
	case GLUT_KEY_LEFT:
		mkey=S3DK_LEFT;
		break;
	case GLUT_KEY_RIGHT:
		mkey=S3DK_RIGHT;
		break;
	case GLUT_KEY_UP:
		mkey=S3DK_UP;
		break;
	case GLUT_KEY_DOWN:
		mkey=S3DK_DOWN;
		break;
	case GLUT_KEY_PAGE_UP:
		mkey=S3DK_PAGEUP;
		break;
	case GLUT_KEY_PAGE_DOWN:
		mkey=S3DK_PAGEDOWN;
		break;
	case GLUT_KEY_HOME:
		mkey=S3DK_HOME;
		break;
	case GLUT_KEY_END:
		mkey=S3DK_END;
		break;
	case GLUT_KEY_INSERT:
		mkey=S3DK_INSERT;
		break;
	default:
		mkey=skey;
		break;
	}
	s3dprintf(MED,"special(): %d -> %d",skey,mkey);
	user_key(mkey,0,0,0);
	user_key(mkey,0,0,1);
}
void mouse_motion(int x, int y)
{
	user_mouse(but,2,x,y);
}
void passive_mouse_motion(int x, int y)
{
	user_mouse(-1,-1,x,y);
}
