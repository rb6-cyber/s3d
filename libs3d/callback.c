// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <stddef.h>                  /* for NULL */
#include <stdint.h>                  /* for uint8_t */
#include "s3d.h"                     /* for s3d_cb, s3d_process_stack */
#include "s3dlib.h"                  /* for MAX_CB, S3DUNUSED */

static int _s3d_ignore(struct s3d_evt *evt);
/*  the s3d callback list */
/* i know it's ugly, but it's better to have ugly code somewhere than provoke
 * race conditions in the applications code */
#define S3D_CBNIL (s3d_cb)NULL
s3d_cb s3d_cb_list[MAX_CB] = {
	S3D_CBNIL, _s3d_ignore, _s3d_ignore, _s3d_ignore, _s3d_ignore, _s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	_s3d_ignore, _s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	_s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL
};
/* the ignore-handler ;) */
static int _s3d_ignore(struct s3d_evt *S3DUNUSED(evt))
{
	/* do plain nothing */
	return 0;
}

/** \brief sets a callback
 *
 * Sets a callback for a certain event. this is very useful for event-oriented
 * applications. event callbacks will not interrupt each other or the mainloop.
 *
 * \remarks Defining callbacks will only work after calling s3d_init()
 *
 * \code
 * #include <inttypes.h>
 *
 * void obj_click(struct s3d_evt event)
 * {
 *         printf("object id %"PRIu32" got clicked", *((uint32_t *)event->buf));
 * }
 * ...
 * s3d_set_callback(S3D_EVENT_NEW_OBJECT, obj_click);
 * // this will tell you when a object got clicked
 * \endcode
 */
void s3d_set_callback(uint8_t event, s3d_cb func)
{
	s3d_cb_list[(int)event] = func;
	s3d_process_stack();
}

/** \brief clears a callback
 *
 * Clears the callback which is associated with the event.
 */
void s3d_clear_callback(uint8_t event)
{
	s3d_cb_list[(int)event] = S3D_CBNIL;
}

/** \brief ignores an event
 *
 * Sets the callback on ignore, that means it won't be queued up for later use.
 * An incoming event of this type will simply be skipped.
 */
void s3d_ignore_callback(uint8_t event)
{
	s3d_set_callback(event, _s3d_ignore);
}

/** \brief get callback of event
 *
 * Returns the Callback-function of the event.
 *
 * \code
 * struct s3d_evt e;
 * ...
 * s3d_get_callback(S3D_EVENT_KEY)(e);
 * // will call the key-handling function with argument e.
 * \endcode
 */
s3d_cb s3d_get_callback(uint8_t event)
{
	return s3d_cb_list[(int)event];
}
