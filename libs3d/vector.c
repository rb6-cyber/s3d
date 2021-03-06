// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2004-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2004-2015  Andreas Langer <an.langer@gmx.de>
 */

#include <math.h>  /* sqrt() */
#include "s3d.h"

/** \brief calculate length of vector
 *
 * Calculates and returns the length of the given vector (which should be of the
 * type float[3]). More info on wikipedia
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_length(const float vector[])
{

	return sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

}

/** \brief subtract two vectors
 *
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

/** \brief calculate dot product of two vectors
 *
 * Calculates and returns the dot product of vector1 and vector2. All vectors
 * should have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_dot_product(const float vector1[], const float vector2[])
{
	return vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
}

/** \brief calculate cross product of two vectors
 *
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

/** \brief calculate angle between two vectors
 *
 * Calculates and returns the angle between vector1 and vector2. Please note that
 * the resulting angle is between 0 and PI, therefore not covering the whole
 * period! To convert in degrees just do result*180/M_PI. All vectors should
 * have the format float[3]. More info on wikipedia.
 * http://en.wikipedia.org/wiki/Vector_(spatial)
 */
float s3d_vector_angle(const float vector1[], const float vector2[])
{

	return acos(s3d_vector_dot_product(vector1, vector2) / (s3d_vector_length(vector1) * s3d_vector_length(vector2)));

}

/** \brief calculate angle between vector and cam
 *
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
