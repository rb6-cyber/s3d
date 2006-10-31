%module s3d
typedef unsigned int uint32_t;
%inline %{
#include <stdint.h>
int s3dpy_init(char *name)
{
	return s3d_init(NULL,NULL,name);
}
int s3dpy_draw_string(char *str)
{
	return s3d_draw_string(str,NULL);
}
%}
%{
#include <s3d.h>
#include <s3d_keysym.h>
%}

%include "../../libs3d/s3d.h"
%include "../../libs3d/s3d_keysym.h"
