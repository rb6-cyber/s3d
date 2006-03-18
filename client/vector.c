/*
 * vector.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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

#include <math.h>		/* sqrt() */

/***
 *
 * calculate length of a vector => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Length_of_a_vector
 *
 *   vector   =>   given vector
 *
 *   return length
 *
 ***/

float s3d_vector_length( float vector[] ) {
	return ( sqrt( vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2] ) );

}


/***
 *
 * substract vector1 from vector2 => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Vector_addition_and_subtraction
 *
 *   vector1         =>   given vector1
 *   vector2         =>   given vector2
 *   result_vector   =>   save resulting vector here
 *
 ***/

void s3d_vector_substract( float vector1[], float vector2[], float result_vector[] ) {

	result_vector[0] = vector2[0] - vector1[0];
	result_vector[1] = vector2[1] - vector1[1];
	result_vector[2] = vector2[2] - vector1[2];
}

/***
 *
 * calculate dot product of 2 vectors => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Dot_product
 *
 *   vector1   =>   given vector1
 *   vector2   =>   given vector2
 *
 *   return dot product
 *
 ***/

float s3d_vector_dot_product( float vector1[], float vector2[] ) {
	return ( vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2] );
}

/***
 *
 * calculate angle between 2 vectors => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Dot_product
 *
 *   vector1   =>   given vector1
 *   vector2   =>   given vector2
 *
 *   return angle
 *
 *   NOTE: angle is between 0 and PI, therefore not covering the whole period!
 *
 ***/

float s3d_vector_angle( float vector1[], float vector2[] ) {
	return ( acos( s3d_vector_dot_product( vector1, vector2 ) / ( s3d_vector_length( vector1 ) * s3d_vector_length( vector2 ) ) ) );

}