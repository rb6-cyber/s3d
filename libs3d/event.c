// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <stdint.h>                  /* for uint8_t */
#include <stdlib.h>                  /* for NULL, free */
#include "s3d.h"                     /* for s3d_evt, s3d_get_callback, etc */
#include "s3dlib.h"                  /* for s3dprintf, VLOW, etc */

static struct s3d_evt *s3d_stack;
int cb_lock = 2;  /*  callback lock */

/** \brief push event onto stack
 *
 * Pushes an event onto the event-stack. Usually you don't need to do this
 * manually.
 */
void s3d_push_event(struct s3d_evt *newevt)
{
	struct s3d_evt *p;
	s3d_cb cb;

	s3dprintf(VLOW, "pushed event %d, cb_lock = %d", newevt->event, cb_lock);
	/*  this will always be called for S3D_EVENT_NEW_OBJECT!! */
	if (newevt->event == S3D_EVENT_NEW_OBJECT) {
		_queue_new_object(*((unsigned int *)newevt->buf));
	}
	if (cb_lock == 0) { /*  no recursive event-callbacks, please! */
		if (NULL != (cb = s3d_get_callback(newevt->event))) {
			cb_lock++;   /*  on our way! lock it.. */
			cb(newevt);   /*  .. and call it! */
			cb_lock--;
			/* okay, no new callbacks, unlock now. */
			free(newevt);
			return;
		}
	}
	newevt->next = NULL;
	if (s3d_stack != NULL) {
		for (p = s3d_stack; p->next != NULL; p = p->next) { } /*  go to the end */
		p->next = newevt;
	} else
		s3d_stack = newevt;
}

/** \brief pop event from stack
 *
 * Pops the latest event from the stack. Don't forget to free() both the event
 * and its buffer! Returns a pointer to struct s3d_evt.
 */
struct s3d_evt *s3d_pop_event(void) {
	struct s3d_evt *ret;
	if ((ret = s3d_stack) != NULL)
		s3d_stack = s3d_stack->next;
	return ret;
}

/** \brief find eevnt on stack
 *
 * Finds the latest occurrence of an event, giving the event type as argument.
 * Returns a pointer to struct s3d_evt.
 */
struct s3d_evt *s3d_find_event(uint8_t event) {
	struct s3d_evt *p;
	p = s3d_stack;
	while (p != NULL) {
		if (p->event == event)
			return p;
		p = p->next;
	}
	return NULL;
}

/** \brief delete event from stack
 *
 * Deletes an event, the argument is the pointer to the event which is to be
 * deleted (maybe obtained from s3d_find_event).
 */
int s3d_delete_event(const struct s3d_evt *devt)
{
	struct s3d_evt *previous = NULL;
	struct s3d_evt *next;
	struct s3d_evt *p = s3d_stack;
	while (p != NULL) {
		/* if ((p->event==devt->event) && (p->length==devt->length)) */
		/*  if (0==memcmp(p->buf,devt->buf)) */
		next = p->next;
		if (p == devt) {
			if (p->length > 0)
				free(p->buf);
			if (previous == NULL)
				s3d_stack = p->next;  /*  the first element!! */
			else
				previous->next = p->next;  /*  unlink */

			free(p);
		} else {
			previous = p;
		}
		p = next;
	}
	return -1;
}

/** \brief process all events on stack
 *
 * This function goes through all function of the event-stack and will call
 * functions. this is useful when you define a new function but still have a lot
 * of events of this type on the stack.
 *
 * \deprecated This is probably obsolete
 */
void s3d_process_stack(void)
{
	struct s3d_evt *p;
	s3d_cb cb;
	if (cb_lock > 0) { /* can't do that now. */

		s3dprintf(VLOW, "cb_lock = %d, processing later", cb_lock);
		return;
	}
	s3dprintf(VLOW, "processing stack ...");
	while (NULL != (p = s3d_pop_event())) {
		if ((cb = s3d_get_callback(p->event)) != NULL) {
			cb_lock++;
			cb(p);
			cb_lock--;
		} else {
			/* kick out unprocessed event */

		}
		/*  free */
		if (p->length > 0)
			free(p->buf);
		free(p);
	}
}
