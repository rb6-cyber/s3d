/*
 * allocate.h
 *
 * Copyright (C) 2006-2008  Marek Lindner <lindner_marek@yahoo.de>
 *      Thomas Lopatic
 *       Corinna 'Elektra' Aichele
 *       Axel Neumann
 *       Felix Fietkau
 *       Simon Wunderlich <dotslash@packetmixer.de>
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

/* this file is taken from the batman project (www.open-mesh.net/batman)
 * to find heap corruptions... */

void checkIntegrity(void);
void checkLeak(void);
void *debugMalloc(unsigned int length, int tag);
void *debugRealloc(void *memory, unsigned int length, int tag);
void debugFree(void *memoryParameter);
