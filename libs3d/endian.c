/*
 * endian.c
 *
 * Copyright (C) 2004-2008 S3D contributors
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

#include <arpa/inet.h> /*  htonl(),htons() */
#include <stdint.h>
#include "s3d.h"
#include "s3dlib.h"

/* convert buffer with floats from host to network endianess */
void htonfb(float* netfloat, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		*(uint32_t*)&netfloat[i] = htonl(*(uint32_t*)&netfloat[i]);
	}
}

/* convert buffer with floats from network to host endianess */
void ntohfb(float* netfloat, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		*(uint32_t*)&netfloat[i] = ntohl(*(uint32_t*)&netfloat[i]);
	}
}

/* convert buffer with uint32_ts from host to network endianess */
void htonlb(uint32_t* netint32, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint32[i] = htonl(netint32[i]);
	}
}

/* convert buffer with uint32_ts from network to host endianess */
void ntohlb(uint32_t* netint32, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint32[i] = ntohl(netint32[i]);
	}
}

/* convert buffer with uint16_ts from host to network endianess */
void htonsb(uint16_t* netint16, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint16[i] = htons(netint16[i]);
	}
}

/* convert buffer with uint16_ts from network to host endianess */
void ntohsb(uint16_t* netint16, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint16[i] = ntohs(netint16[i]);
	}
}
