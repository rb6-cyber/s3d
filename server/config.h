/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

/* use the global config.h */
#include <config-s3d.h>

/*  to be filled with configure options ...  */
/*   */
/*  on which port do we listen? */
#define S3D_PORT 6066
/*  resolution on startup */
#define X_RES 800
#define Y_RES 600
/* how many frames to wait until test the connection if it's still here */
#define MAX_IDLE 50
/*  this is to be set dynamicly later on */
#define VLOW 1
#define LOW  2
#define MED  3
#define HIGH 4
#define VHIGH 5
/*  which is the minimum level of debugmessage we want to see? */
			  /* #define DEBUG LOW *//* should be set with compile, e.g. -DDEBUG=LOW */
