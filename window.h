#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>
#include "client.h"
#define CORNER_RADIUS 20
#define TITLEBAR_HEIGHT 24
#define BORDER_SIZE 2
#define BORDER_WIDTH 20
#define CLOSE_WIDTH 20

void window__draw_decorations(client_t* c, Display* dpy);

#endif