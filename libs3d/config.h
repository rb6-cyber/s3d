/*
 * config.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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


/* use the global config.h */
#include <config-s3d.h>

/*  this is definitly the better way to pick fonts. */

#define WITH_FONTCONFIG
/*  our level of debug messages */
/* #define DEBUG   LOW */  /*  standard debug level, should be set with compiler, e.g. -DDEBUG=LOW */
#define SHM_MAX_IDLE 200 /* maximum wait for server timeout */
#define SEI_SS   200 /* seidel algorithm maximum point number */
