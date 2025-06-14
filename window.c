#include <X11/Xlib.h>
#include <string.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "window.h"
#include "client.h"

void window__draw_decorations(client_t* c, Display* dpy)
{
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, c->client, &attr);
    int w = attr.width + BORDER_WIDTH * 2;
    int h = attr.height + TITLEBAR_HEIGHT + BORDER_WIDTH * 2;
    cairo_save(c->cr);
    cairo_set_operator(c->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(c->cr, 0, 0, 0, 0); // fully transparent background
    cairo_paint(c->cr);

    // Draw semi-transparent black border
    cairo_set_source_rgba(c->cr, 0, 0, 0, 0.7);
    cairo_rectangle(c->cr, 0, 0, w, h);
    cairo_set_line_width(c->cr, BORDER_WIDTH);
    cairo_stroke(c->cr);

    // Draw title bar background
    cairo_rectangle(c->cr, BORDER_WIDTH, BORDER_WIDTH, w - BORDER_WIDTH, TITLEBAR_HEIGHT);
    cairo_set_source_rgba(c->cr, 0.1, 0.1, 0.3, 0.8);
    cairo_fill(c->cr);

    // Draw window title text
    cairo_set_source_rgb(c->cr, 1, 1, 1);
    cairo_select_font_face(c->cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(c->cr, 14);
    cairo_move_to(c->cr, BORDER_WIDTH + 5, BORDER_WIDTH + TITLEBAR_HEIGHT - 5);

    XTextProperty prop;
    if (XGetWMName(dpy, c->client, &prop) && prop.value) {
        char *title = (char *)prop.value;
        cairo_show_text(c->cr, title);
        XFree(prop.value);
    } else {
        cairo_show_text(c->cr, "No Title");
    }

    // Draw close button (red square)
    cairo_rectangle(c->cr, w - BORDER_WIDTH - 20, BORDER_WIDTH + 2, 16, 16);
    cairo_set_source_rgba(c->cr, 0.8, 0, 0, 1);
    cairo_fill(c->cr);

    cairo_restore(c->cr);
    cairo_surface_flush(c->surface);
    XFlush(dpy);
    /*GC gc = XCreateGC(dpy, frame, 0, NULL);
    XSetForeground(dpy, gc, 0x888888);
    XFillArc(dpy, frame, gc, 0, 0, CORNER_RADIUS, CORNER_RADIUS, -180*64, -90*64);
    //XFillRectangle(dpy, frame, gc, 0, 0, 800, TITLEBAR_HEIGHT);

    XSetForeground(dpy, gc, 0xFF0000);
    //XFillRectangle(dpy, frame, gc, 2, 2, CLOSE_WIDTH, TITLEBAR_HEIGHT - 4);

    XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));
    if (title)
        XDrawString(dpy, frame, gc, 30, 15, title, strlen(title));

    XFreeGC(dpy, gc);*/
    return;
}