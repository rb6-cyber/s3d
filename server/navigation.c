/*
 * navigation.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <math.h> /* atan() */

void navi_right(void)
{
	navi_pos(1, 0);
}
void navi_left(void)
{
	navi_pos(-1, 0);
}
void navi_fwd(void)
{
	navi_pos(0, 1);
}
void navi_back(void)
{
	navi_pos(0, -1);
}
/* simple movements, not needed currently
void navi_rot_right()
{
 cam.rotate.y=cam.rotate.y+2;
 if (cam.rotate.y>360) cam.rotate.y-=360;
}
void navi_rot_left()
{
 cam.rotate.y=cam.rotate.y-2;
 cam.rotate.y=(cam.rotate.y<0)?cam.rotate.y+360:cam.rotate.y;
}
void navi_rot_up()
{
 cam.rotate.x=(cam.rotate.x+2);
 if (cam.rotate.x>90) cam.rotate.x=90;
}
void navi_rot_down()
{
 cam.rotate.x=cam.rotate.x-2;
 if (cam.rotate.x<-90) cam.rotate.x=-90;
}*/
void navi_pos(int xdif, int ydif)
{
	float tv[3];
	struct t_obj *cam;
	cam = get_proc_by_pid(MCP)->object[0];
	tv[0] = cam->translate.x;
	tv[1] = cam->translate.y;
	tv[2] = cam->translate.z;

	tv[0] += ydif * sin((-cam->rotate.y * M_PI) / 180);
	tv[2] -= ydif * cos((-cam->rotate.y * M_PI) / 180);

	tv[0] -= xdif * cos((-cam->rotate.y * M_PI) / 180);
	tv[2] -= xdif * sin((-cam->rotate.y * M_PI) / 180);
	obj_translate(get_proc_by_pid(MCP), 0, tv);
}
void navi_rot(int xdif, int ydif)
{
	float rv[3];
	struct t_obj *cam;
	cam = get_proc_by_pid(MCP)->object[0];
	rv[0] = (cam->rotate.x + ydif);
	rv[1] = (cam->rotate.y + xdif);
	rv[2] = 0.0F;
	if (rv[0] > 90)  rv[0] = 90;
	if (rv[0] < -90)  rv[0] = -90;
	if (rv[1] > 360)  rv[1] -= 360;
	if (rv[1] < 0)  rv[1] += 360;
	obj_rotate(get_proc_by_pid(MCP), 0, rv);
}
void ptr_move(int x, int y)
{
	float tv[3], rv[3], xf, yf;
	struct t_process *p;
	int ptr;
	if (winw > winh) {
		xf = winw / (float)winh;
		yf = 1;
	} else {
		xf = 1;
		yf = winh / (float)winw;
	}
	tv[0] = (2.0 * x / ((float)winw) - 1.0) * xf;
	tv[1] = -(2.0 * y / ((float)winh) - 1.0) * yf;
	tv[2] = -1;
	rv[0] = 1.5 * 180 / M_PI * atan(tv[1] / 2); /* TODO: Hm, this is not really correct ... */
	rv[1] = 1.5 * 180 / M_PI * -atan(tv[0] / 2);
	rv[2] = 0;
	p = get_proc_by_pid(MCP);
	if (-1 != (ptr = get_pointer(p))) {
		obj_translate(p, ptr, tv);
		obj_rotate(p, ptr, rv);
	}
}
