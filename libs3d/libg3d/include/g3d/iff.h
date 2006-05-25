/* $Id: iff.h,v 1.1.2.3 2006/01/23 16:43:18 dahms Exp $ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005, 2006  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __G3D_IFF_H__
#define __G3D_IFF_H__

#include <stdio.h>
#include <glib.h>

#define G3D_IFF_MKID(a,b,c,d) ( \
	(((guint32)(a))<<24) | \
	(((guint32)(b))<<16) | \
	(((guint32)(c))<< 8) | \
	(((guint32)(d))    ) )

G_BEGIN_DECLS

/**
 * g3d_iff_open:
 * @filename: file name of IFF file
 * @id: top level ID (out)
 * @len: length of top level container (out)
 *
 * Opens an IFF file, checks it and reads its top level container.
 *
 * Returns: the file pointer of open file or NULL in case of an error
 */
FILE *g3d_iff_open(const gchar *filename, guint32 *id, guint32 *len);

/**
 * g3d_iff_readchunk:
 * @f: the open IFF file pointer
 * @id: ID of chunk (out)
 * @len: length of chunk (excluding header) (out)
 *
 * Reads one chunk header from an IFF file.
 *
 * Returns: real length of chunk including header and possible padding byte
 */
int g3d_iff_readchunk(FILE *f, guint32 *id, guint32 *len);

G_END_DECLS

#endif
