#!/usr/bin/python
# -*- coding: utf-8; -*-

# SPDX-License-Identifier: GPL-2.0-or-later
# SPDX-FileCopyrightText: 2007-2015  Simon Wunderlich <sw@simonwunderlich.de>

from s3d import *
from time import sleep
if 0 == s3dpy_init("hello world"):
	s3d_select_font("vera")
	o = s3dpy_draw_string("hello from python")
	s3d_flags_on(o, S3D_OF_VISIBLE)
	sleep(10)
	s3d_quit()
