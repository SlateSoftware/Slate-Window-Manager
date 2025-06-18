#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>
#include "client.h"
#define CORNER_RADIUS 20
#define TITLEBAR_HEIGHT 24
#define BORDER_SIZE 2
#define BORDER_WIDTH 20
#define CLOSE_WIDTH 20

char* window__get_name(Window client, Display* dpy);
u32* window__get_icon(Window client, Display* dpy);
void window__draw_decorations(client_t* c, Display* dpy, int w, int h, bool update_title);
void window__handle_resize_event(XEvent* ev, client_t* c, Display* dpy);

#endif