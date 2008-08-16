#include <s3d.h>
#include <X11/Xlib.h>       /* Ximage, Display, X*() */
#include <X11/Xutil.h>       /* XDestroyImage() */
#include <X11/Xatom.h>
#include <config-s3d.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>
#ifndef COMPUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define COMPUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define COMPUNUSED(x) /* x */
#else
#define COMPUNUSED(x) x
#endif
#endif

#define MAXEVENTS 50  /* maximum events per loop. */
#define SCREEN_SCALE	5.0

/* must be 2^x */
#define TEXW 256
#define TEXH 256
#define TEXNUM(win, x, y) \
  ((((win->attr.height + TEXH - 1)& ~(TEXH-1))/TEXH) * ((int)(x/TEXH)) + ((int)(y/TEXW)))
#define MIN(x,y)	(((x)<(y))?(x):(y))
#define MAX(x,y)	(((x)>(y))?(x):(y))


struct window {
	Window        id;
	XWindowAttributes    attr;   /* position, size etc. */
	XImage     *image;
	Damage      damage;  /* damage notification */
	Pixmap		pix;
	int			geometry_update_needed;
	int     	content_update_needed;
	XRectangle	content_update;
	int			oid;
	int			no;
	int 		mapped;
	char		*name;

	struct window     *next;
};

/* comptest.c */
extern int screen_width;
extern int screen_height;
extern int screen_oid;
void deco_box(struct window *win);
/* window.c */
void window_map(struct window *win);
void window_set_position(struct window *win);
void window_restack(struct window *win, Window above);
struct window *window_find(Window id);
struct window *window_add(Display *dpy, Window id);
void window_remove(Window id);
void window_update_content(struct window *win);
void window_update_geometry(struct window *win);
void window_unmap(struct window *win);

extern struct window   *window_head;
/* x11.c */
void x11_always_on_top(Window win);
void event(void);
int xinit(void);
int error(Display *COMPUNUSED(dpy), XErrorEvent *event);
int print_event(Display *COMPUNUSED(dpy), XEvent *event);
void x11_addwindows();
char *x11_get_prop(Window win, char *prop);
char *x11_get_name(Window win); 
void print_properties(Window win);
extern Display *dpy;