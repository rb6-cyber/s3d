#!/usr/bin/perl

# Copyright (C) 2007-2012  Simon Wunderlich <dotslash@packetmixer.de>
#
# See http://s3d.sourceforge.net/ for more updates.
#
# testvis is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# testvis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with testvis; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

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
