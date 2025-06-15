#include <X11/Xlib.h>
#include <string.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include "window.h"
#include "client.h"

void window__draw_decorations(client_t* c, Display* dpy)
{
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, c->frame, &attr);
    int w = attr.width;
    int h = attr.height;
    cairo_save(c->cr);
    cairo_set_operator(c->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(c->cr, 0, 0, 0, 0); // fully transparent background
    cairo_paint(c->cr);

    cairo_set_source_rgba(c->cr, 1,1,1,0.7);
    cairo_new_path(c->cr);
    cairo_arc(c->cr, w-CORNER_RADIUS, CORNER_RADIUS+TITLEBAR_HEIGHT, CORNER_RADIUS, -90*(3.14/180), 0);
    cairo_arc(c->cr, w-CORNER_RADIUS, h-CORNER_RADIUS, CORNER_RADIUS, 0, 90*(3.14/180));
    cairo_arc(c->cr, CORNER_RADIUS, h-CORNER_RADIUS, CORNER_RADIUS, 90*(3.14/180), 180*(3.14/180));
    cairo_arc(c->cr, CORNER_RADIUS, CORNER_RADIUS+TITLEBAR_HEIGHT, CORNER_RADIUS, 180*(3.14/180), 270*(3.14/180));
    cairo_close_path(c->cr);
    cairo_fill(c->cr);

    // Draw semi-transparent black border
    /*cairo_set_source_rgba(c->cr, 0, 0, 0, 0.7);
    cairo_rectangle(c->cr, 0, TITLEBAR_HEIGHT, w, h);
    cairo_set_line_width(c->cr, CORNER_RADIUS);
    cairo_stroke(c->cr);

    // Draw title bar background
    cairo_rectangle(c->cr, 0, 0, w, TITLEBAR_HEIGHT);
    cairo_set_source_rgba(c->cr, 0.1, 0.1, 0.3, 0.8);
    cairo_fill(c->cr);

    // Draw window title text*/
    cairo_set_source_rgb(c->cr, 1, 1, 1);
    cairo_select_font_face(c->cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(c->cr, 14);
    cairo_move_to(c->cr, BORDER_WIDTH + 5, TITLEBAR_HEIGHT - 5);

    XTextProperty prop;
    char* name = strdup("No title");
    int count = 0;
    char** list = NULL;
    if (XGetWMName(dpy, c->client, &prop) && prop.value)
    {
        if (prop.encoding == XA_STRING)
        {
            free(name);
            name = strdup((char *)prop.value);
        }
        
        else if (XmbTextPropertyToTextList(dpy, &prop, &list, &count) >= Success && count > 0)
        {
            free(name);
            name = strdup(list[0]);
            XFreeStringList(list);
            list = NULL;
        }

        XFree(prop.value);
    }
    
    cairo_show_text(c->cr, name);
    // Draw close button (red square)
    cairo_rectangle(c->cr, w - CLOSE_WIDTH, 5, 16, 16);
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