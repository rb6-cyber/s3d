// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
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
	{
		0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0
	},
	/* input_mat */
	{
		0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0,
		0.7, 0.7, 0.7, 1.0
	},
	/* inputback_mat */
	{
		0.9, 0.9, 0.9, 1.0,
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

