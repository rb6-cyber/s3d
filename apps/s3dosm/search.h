/*
 * search.h
 *
 * Copyright (C) 2006-2008 Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of the olsrs3d, an olsr topology visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * olsrs3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * olsrs3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define NOTHING 0  /* nothing ;) */
#define ABORT 1   /* widget has no focus */
#define WIDGET 2  /* focus on widget */
#define FOLLOW 3  /* follow search result */

void create_search_widget(float x, float y, float z);
void move_search_widget(float x, float y, float z);
void move_to_search_widget(float cam_position_t[], float cam_position_r[]);
void move_to_return_point(float cam_position_t[], float cam_position_r[]);
void set_return_point(float cam_position_t[], float cam_position_r[]);
int get_search_status(void);
void set_search_status(int stat);
void search_widget_write(int key);
void set_node_root(struct olsr_node *root);
void follow_node(float cam_position_t[], float cam_position_r[], float rotate);
void follow_node_by_click(struct olsr_node *olsr_node);

void show_search_window();
