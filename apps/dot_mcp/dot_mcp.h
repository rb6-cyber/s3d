/*
 * dot_mcp.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of dot_mcp, a mcp for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * dot_mcp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * dot_mcp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dot_mcp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config-s3d.h>

#ifndef DOTMCPUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define DOTMCPUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define DOTMCPUNUSED(x) /* x */
#else
#define DOTMCPUNUSED(x) x
#endif
#endif

int menu_init(void);
void menu_click(int oid);
