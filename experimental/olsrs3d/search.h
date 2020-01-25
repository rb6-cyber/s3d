/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2006-2015  Andreas Langer <an.langer@gmx.de>
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#define NOTHING 0  /* nothing ;) */
#define ABORT 1   /* widget has no focus */
#define WIDGET 2  /* focus on widget */
#define FOLLOW 3  /* follow search result */

struct olsr_node;

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

void show_search_window(void);

#endif /* _SEARCH_H_ */
