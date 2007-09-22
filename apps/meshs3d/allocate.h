/*
 * allocate.h
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *                    Marek Lindner <lindner_marek@yahoo.de>
 *
 * This file is part of meshs3d, an olsr/batman topology visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * olsrs3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * olsrs3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef _ALLOCATE_H
#define _ALLOCATE_H 1
#include <stdint.h>



void checkIntegrity(void);
void checkLeak(void);
void *debugMalloc(uint32_t length, int32_t tag);
void *debugRealloc(void *memory, uint32_t length, int32_t tag);
void debugFree(void *memoryParameter, int32_t tag);

#endif
