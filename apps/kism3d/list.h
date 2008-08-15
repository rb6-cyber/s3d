/*
 * list.h
 *
 * Copyright (C) 2006  Marek Lindner <lindner_marek@yahoo.de>
 *
 * This file is part of kism3d, an 802.11 visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * kism3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kism3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>

struct list_head {
	struct list_head *next, *prev;
};



#define DEFINE_LIST_HEAD(name) \
 struct list_head name = { &(name), &(name) }

#define INIT_LIST_HEAD(ptr) do { \
 (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)


/**
 * list_for_each - iterate over a list
 * @pos: the &struct list_head to use as a loop counter.
 * @head: the head for your list.
 */
#define list_for_each(pos, head) \
 for (pos = (head)->next; pos != (head); \
         pos = pos->next)


/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos: the &struct list_head to use as a loop counter.
 * @n:  another &struct list_head to use as temporary storage
 * @head: the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
 for (pos = (head)->next, n = pos->next; pos != (head); \
  pos = n, n = pos->next)


/**
 * list_entry - get the struct for this entry
 * @ptr: the &struct list_head pointer.
 * @type: the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
 ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))



/**
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *n,
                              struct list_head *prev,
                              struct list_head *next)
{
	next->prev = n;
	n->next = next;
	n->prev = prev;
	prev->next = n;
}



/**
 * list_add_tail - add a new entry
 * @n: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *n, struct list_head *head)
{
	__list_add(n, head->prev, head);
}



/**
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}



/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

