#include "s3d.h"
#include "s3dlib.h"
/*  the s3d callback list */
s3d_cb s3d_cb_list[MAX_CB];
/* the ignore-handler ;) */
static void _s3d_ignore(struct s3d_evt *evt)
{
	/* do plain nothing */
}
/*  sets a callback */
void s3d_set_callback(unsigned char event, s3d_cb func)
{
	s3d_cb_list[(int)event]=func;
	s3d_process_stack();
}
/*  clears a callback, same as s3d_set_callback(event, (s3d_cb) NULL); */
void s3d_clear_callback(unsigned char event)
{
	s3d_cb_list[(int)event]=NULL;
}
/* ignores an event ... */
void s3d_ignore_callback(unsigned char event)
{
	s3d_set_callback(event,_s3d_ignore);
}
s3d_cb s3d_get_callback(unsigned char event)
{
	return(s3d_cb_list[(int)event]);
}
