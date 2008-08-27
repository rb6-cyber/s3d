/*
 * cull.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 * code originated from  http://www.racer.nl/reference/vfc.htm
 * which is (C) Ruud van Gaal
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
#include <math.h> /* sqrt() */
#include <GL/gl.h> /* glGetFloatv() */
struct t_plane {
	struct t_vertex n;
	float d;
};
#define LEFT 0
#define RIGHT 1
#define TOP  2
#define BOTTOM 3
#define PNEAR 4
#define PFAR 5
static struct t_plane frustumPlane[6];
void cull_get_planes(void)
{
	t_mtrx m, mproj, mmodel;
	struct t_plane *p;
	int i;
	float d;


	/* get matrices from opengl */
	glGetFloatv(GL_MODELVIEW_MATRIX, mmodel);
	glGetFloatv(GL_PROJECTION_MATRIX, mproj);

	mySetMatrix(mproj);
	myMultMatrix(mmodel);
	myGetMatrix(m); /* multiply and have the result in m */

	p = &frustumPlane[RIGHT];
	p->n.x = m[3] - m[0];
	p->n.y = m[7] - m[4];
	p->n.z = m[11] - m[8];
	p->d = m[15] - m[12];

	p = &frustumPlane[LEFT];
	p->n.x = m[3] + m[0];
	p->n.y = m[7] + m[4];
	p->n.z = m[11] + m[8];
	p->d = m[15] + m[12];

	p = &frustumPlane[BOTTOM];
	p->n.x = m[3] + m[1];
	p->n.y = m[7] + m[5];
	p->n.z = m[11] + m[9];
	p->d = m[15] + m[13];

	p = &frustumPlane[TOP];
	p->n.x = m[3] - m[1];
	p->n.y = m[7] - m[5];
	p->n.z = m[11] - m[9];
	p->d = m[15] - m[13];

	p = &frustumPlane[PFAR];
	p->n.x = m[3] - m[2];
	p->n.y = m[7] - m[6];
	p->n.z = m[11] - m[10];
	p->d = m[15] - m[14];

	p = &frustumPlane[PNEAR];
	p->n.x = m[3] + m[2];
	p->n.y = m[7] + m[6];
	p->n.z = m[11] + m[10];
	p->d = m[15] + m[14];

	/* Normalize all plane normals */
	for (i = 0;i < 6;i++) {
		p = &frustumPlane[i];
		d = sqrt(p->n.x * p->n.x + p->n.y * p->n.y + p->n.z * p->n.z);
		if (d != 0.0) {
			p->n.x /= d;
			p->n.y /= d;
			p->n.z /= d;
			p->d /= d;
		}
	}
}

int cull_sphere_in_frustum(struct t_vertex *center, float radius)
{
	int i;
	struct t_plane *p;
	for (i = 0;i < 6;i++) {
		p = &frustumPlane[i];
		if (p->n.x*center->x + p->n.y*center->y + p->n.z*center->z + p->d <= -radius) {
			/*   s3dprintf(MED,"out of %d plane (n %f %f %f |d %f)",i,p->n.x,p->n.y,p->n.z,p->d);*/
			return 0; /* sorry, no ... */
		}
	}
	return 1; /* it's inside */
}

