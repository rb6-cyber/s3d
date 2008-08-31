/*
 * vector.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *                         Marek Lindner <lindner_marek@yahoo.de>
 *                         Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <math.h>  /* sqrt() */
#include "s3d.h"

/**
 * Calculates and returns the length of the given vector (which should be of the
 * type float[3]). More info on wikipedia
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_length(const float vector[])
{

	return (sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]));

}

/**
 * Subtracts vector1 from vector2, writing result into result_vector. All vectors
 * should have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
void s3d_vector_subtract(const float vector1[], const float vector2[], float result_vector[])
{

	result_vector[0] = vector2[0] - vector1[0];
	result_vector[1] = vector2[1] - vector1[1];
	result_vector[2] = vector2[2] - vector1[2];

}

/**
 * Calculates and returns the dot product of vector1 and vector2. All vectors
 * should have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_dot_product(const float vector1[], const float vector2[])
{
	return (vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2]);
}

/**
 * Calculates and returns the cross product of vector1 and vector2. All vectors
 * should have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
void s3d_vector_cross_product(const float vector1[], const float vector2[], float result_vector[])
{
	result_vector[0] = vector1[1] * vector2[2] - vector1[2] * vector2[1];
	result_vector[1] = vector1[2] * vector2[0] - vector1[0] * vector2[2];
	result_vector[2] = vector1[0] * vector2[1] - vector1[1] * vector2[0];
}

/**
 * Calculates and returns the angle between vector1 and vector2. Please note that
 * the resulting angle is between 0 and PI, therefore not covering the whole
 * period! To convert in degrees just do result*180/M_PI. All vectors should
 * have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_angle(const float vector1[], const float vector2[])
{

	return (acos(s3d_vector_dot_product(vector1, vector2) / (s3d_vector_length(vector1) * s3d_vector_length(vector2))));

}

/**
 * Given obj_pos and cam_pos in the format float[3], angle_rad about which angle
 * the object should be rotated around the y-axis so that it faces the camera.
 * This might become handy if you have some text floating in space and want it
 * to face the camera.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_angle_to_cam(const float obj_pos[], const float cam_pos[], float *angle_rad)
{

	float angle, tmp_mov_vec[3], desc_norm_vec[3] = { 0.0, 0.0, -1.0 };


	tmp_mov_vec[0] = cam_pos[0] - obj_pos[0];
	tmp_mov_vec[1] = 0;   /* we are not interested in the y value */
	tmp_mov_vec[2] = cam_pos[2] - obj_pos[2];

	angle = s3d_vector_angle(desc_norm_vec, tmp_mov_vec);

	/* take care of inverse cosinus */
	if (tmp_mov_vec[0] > 0) {
		*angle_rad = 90.0 / M_PI - angle;
		angle = 180 - (180.0 / M_PI * angle);
	} else {
		*angle_rad = 90.0 / M_PI + angle;
		angle = 180 + (180.0 / M_PI * angle);
	}

	return angle;

}
