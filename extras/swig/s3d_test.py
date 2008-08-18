#!/usr/bin/python
# -*- coding: utf-8; -*-

# Copyright (C) 2007 Simon Wunderlich <dotslash@packetmixer.de>
#
# See http://s3d.berlios.de/ for more updates.
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

from s3d import *
from time import sleep
if 0 == s3dpy_init("hello world"):
	s3d_select_font("vera")
	o = s3dpy_draw_string("hello from python")
	s3d_flags_on(o, S3D_OF_VISIBLE)
	sleep(10)
	s3d_quit()
