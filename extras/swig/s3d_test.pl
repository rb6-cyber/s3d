#!/usr/bin/perl

# SPDX-License-Identifier: GPL-2.0-or-later
# SPDX-FileCopyrightText: 2007-2015  Simon Wunderlich <sw@simonwunderlich.de>

use s3d;
if (0==s3d::s3dpy_init("hello perl"))
{
	s3d::s3d_select_font("vera");
	$o= s3d::s3dpy_draw_string("hello from perl");
	print("object $o\n");
	s3d::s3d_flags_on($o,$s3d::S3D_OF_VISIBLE);
	sleep(10);
	s3d::s3d_quit();
}
