/*
 * style.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d Widgets, a Widget Library for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d Widgets is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * s3d Widgets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d Widgets; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>


/* default style */
s3dw_style def_style = {
	/* name */
	"default",
	/* font face */
	"vera",
	/* surface_mat */
	{0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0
	},
	/* input_mat */
	{0.7, 0.7, 0.7, 1.0,
	 0.7, 0.7, 0.7, 1.0,
	 0.7, 0.7, 0.7, 1.0
	},
	/* inputback_mat */
	{0.9, 0.9, 0.9, 1.0,
	 0.9, 0.9, 0.9, 1.0,
	 0.9, 0.9, 0.9, 1.0
	},

	/* text_mat */
	{
		0.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		0.0, 0.0, 0.0, 1.0
	},
	/* title_mat */
	{
		0.0, 0.4, 0.8, 1.0,
		1.0, 1.0, 1.0, 1.0,
		0.0, 0.4, 0.8, 1.0
	},
	/* title_text_mat */
	{
		0.0, 0.0, 0.0, 1.0,
		0.0, 0.0, 0.0, 1.0,
		0.0, 0.0, 0.0, 1.0
	}
};

