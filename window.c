#include <X11/Xlib.h>
#include <string.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <unistd.h>
#include "core.h"
#include "window.h"
#include "client.h"

const int MIN_WIDTH = 100;
const int MIN_HEIGHT = 50;

char* window__get_name(Window client, Display* dpy)
{
    XTextProperty prop;
    char* name = strdup("No title");
    int count = 0;
    char** list = NULL;
    if (XGetWMName(dpy, client, &prop) && prop.value)
    {
        if (prop.encoding == XA_STRING)
        {
            free(name);
            name = strdup((char*)prop.value);
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
    return name;
}

void window__draw_decorations(client_t* c, Display* dpy, int w, int h,  bool update_title)
{
    int _h = h;
    int _w = w;
    if (!_h && !_w)
    {
        _h = c->frame_h;
        _w = c->frame_w;
    }
    logf("w = %d", w);
    logf("h = %d", h);
    logf("_w = %d", _w);
    logf("_h = %d", _h);

    if (_w < 0 || _h < 0)
        return;

    cairo_save(c->cr);
    cairo_set_operator(c->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(c->cr, 0, 0, 0, 0);
    cairo_paint(c->cr);

    cairo_set_source_rgba(c->cr, 1,1,1,0.7);
    cairo_new_path(c->cr);
    cairo_arc(c->cr, _w-CORNER_RADIUS, CORNER_RADIUS+TITLEBAR_HEIGHT, CORNER_RADIUS, -90*(3.14/180), 0);
    cairo_arc(c->cr, _w-CORNER_RADIUS, _h-CORNER_RADIUS, CORNER_RADIUS, 0, 90*(3.14/180));
    cairo_arc(c->cr, CORNER_RADIUS, _h-CORNER_RADIUS, CORNER_RADIUS, 90*(3.14/180), 180*(3.14/180));
    cairo_arc(c->cr, CORNER_RADIUS, CORNER_RADIUS+TITLEBAR_HEIGHT, CORNER_RADIUS, 180*(3.14/180), 270*(3.14/180));
    cairo_close_path(c->cr);
    cairo_fill(c->cr);


    /*cairo_rectangle(c->cr, CORNER_RADIUS, TITLEBAR_HEIGHT, _w - CORNER_RADIUS * 2, CORNER_RADIUS);
    cairo_fill(c->cr);
    cairo_rectangle(c->cr, 0, TITLEBAR_HEIGHT+CORNER_RADIUS+_h, _w - CORNER_RADIUS * 2, CORNER_RADIUS);
    cairo_fill(c->cr);*/
    /*cairo_rectangle(c->cr, 0, TITLEBAR_HEIGHT+CORNER_RADIUS, CORNER_RADIUS, _h);
    cairo_fill(c->cr);
    cairo_rectangle(c->cr, _w+CORNER_RADIUS, TITLEBAR_HEIGHT+CORNER_RADIUS, CORNER_RADIUS, _h);
    cairo_fill(c->cr);*/
    
    
    /*cairo_set_source_rgba(c->cr, 0, 0, 0, 0.7);
    cairo_rectangle(c->cr, 0, TITLEBAR_HEIGHT, _w, _h);
    cairo_set_line_width(c->cr, CORNER_RADIUS);
    cairo_stroke(c->cr);
    
    cairo_rectangle(c->cr, 0, 0, _w, TITLEBAR_HEIGHT);
    cairo_set_source_rgba(c->cr, 0.1, 0.1, 0.3, 0.8);
    cairo_fill(c->cr);*/
    
    cairo_set_source_rgb(c->cr, 1, 1, 1);
    cairo_select_font_face(c->cr, "DM Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_status_t status = cairo_status(c->cr);
    if (status != CAIRO_STATUS_SUCCESS)
    {
        elogf("[Cairo error %d] %s", status, cairo_status_to_string(status));
    }
    
    cairo_set_font_size(c->cr, 14);
    cairo_move_to(c->cr, BORDER_WIDTH + 5, TITLEBAR_HEIGHT - 5);
    status = cairo_status(c->cr);
    if (status != CAIRO_STATUS_SUCCESS)
    {
        elogf("[Cairo error %d] %s", status, cairo_status_to_string(status));
    }
    if (update_title)
    {
        free(c->name);
        c->name = window__get_name(c->client, dpy);
    }
    
    cairo_show_text(c->cr, c->name);
    status = cairo_status(c->cr);
    if (status != CAIRO_STATUS_SUCCESS)
    {
        elogf("[Cairo error %d] %s", status, cairo_status_to_string(status));
    }
    
    cairo_rectangle(c->cr, _w - CLOSE_WIDTH, 5, 16, 16);
    cairo_set_source_rgba(c->cr, 0.8, 0, 0, 1);
    cairo_fill(c->cr);
    printf("%p (%ux%u=%u)\n", c->icon, c->icon[0], c->icon[1], (c->icon[0] * c->icon[1]));
    if (c->icon && (c->icon[0] * c->icon[1]) > 0)
    {
        u32 icon_width = c->icon[0];
        u32 icon_height = c->icon[1];
        cairo_surface_t* icon_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, icon_width, icon_height);
        u32* surface_data = (u32*)cairo_image_surface_get_data(icon_surface);
        memcpy(surface_data, &c->icon[2], icon_width * icon_height);
        cairo_surface_mark_dirty(icon_surface);
        cairo_save(c->cr);

        /*const int icon_x = 4;
        const int icon_y = 4;
        const int icon_size = 100; *

        /*cairo_translate(c->cr, icon_x, icon_y);

        double sx = (double)icon_size / icon_width;
        double sy = (double)icon_size / icon_height;
        cairo_scale(c->cr, sx, sy);*/

        cairo_set_source_surface(c->cr, icon_surface, 0, 0);
        cairo_paint(c->cr);
        cairo_surface_flush(icon_surface);
    }

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

void window__handle_resize_event(XEvent* ev, client_t* c, Display* dpy)
{
    int dx = ev->xmotion.x_root - c->drag_x;
    int dy = ev->xmotion.y_root - c->drag_y;
    switch (c->state & (RESIZE_REGION_MASK & ~IS_RESIZING_MASK))
    {
        case BOTTOM_LEFT_RESIZE_MASK:
        {
            if (c->frame_w - dx < MIN_WIDTH || c->frame_h + dy < MIN_WIDTH) return;
            XMoveResizeWindow(dpy, c->frame, c->frame_x + dx, c->frame_y, c->frame_w - dx, c->frame_h + dy);
            XResizeWindow(dpy, c->client, c->client_w - dx, c->client_h + dy);
            /*cairo_surface_destroy(c->surface);
            cairo_destroy(c->cr);
            c->surface = client__get_cairo_surface(c->frame, dpy, c->frame_w - dx, c->frame_h + dy, NULL);
            c->cr = cairo_create(c->surface);*/
            window__draw_decorations(c, dpy, c->frame_w - dx, c->frame_h + dy, false);
            return;
        }
        case BOTTOM_RIGHT_RESIZE_MASK:
        {
            if (c->frame_w + dx < MIN_WIDTH || c->frame_h + dy < MIN_WIDTH) return;
            XResizeWindow(dpy, c->frame, c->frame_w + dx, c->frame_h + dy);
            XResizeWindow(dpy, c->client, c->client_w + dx, c->client_h + dy);
            /*cairo_surface_destroy(c->surface);
            cairo_destroy(c->cr);
            c->surface = client__get_cairo_surface(c->frame, dpy, c->frame_w + dx, c->frame_h + dy, NULL);
            c->cr = cairo_create(c->surface);*/
            window__draw_decorations(c, dpy, c->frame_w + dx, c->frame_h + dy, false);
            return;
        }
        case TOP_RIGHT_RESIZE_MASK:
        {
            if (c->frame_w + dx < MIN_WIDTH || c->frame_h - dy < MIN_WIDTH) return;
            XMoveResizeWindow(dpy, c->frame, c->frame_x, c->frame_y + dy, c->frame_w + dx, c->frame_h - dy);
            XResizeWindow(dpy, c->client, c->client_w + dx, c->client_h - dy);
            /*cairo_surface_destroy(c->surface);
            cairo_destroy(c->cr);
            c->surface = client__get_cairo_surface(c->frame, dpy, c->frame_w + dx, c->frame_h - dy, NULL);
            c->cr = cairo_create(c->surface);*/
            window__draw_decorations(c, dpy, c->frame_w + dx, c->frame_h - dy, false);
            return;
        }
        case TOP_LEFT_RESIZE_MASK:
        {
            if (c->frame_w - dx < MIN_WIDTH || c->frame_h - dy < MIN_WIDTH) return;
            XMoveResizeWindow(dpy, c->frame, c->frame_x + dx, c->frame_y + dy, c->frame_w - dx, c->frame_h - dy);
            XResizeWindow(dpy, c->client, c->client_w - dx, c->client_h - dy);
            /*cairo_surface_destroy(c->surface);
            cairo_destroy(c->cr);
            c->surface = client__get_cairo_surface(c->frame, dpy, c->frame_w - dx, c->frame_h - dy, NULL);
            c->cr = cairo_create(c->surface);*/
            window__draw_decorations(c, dpy, c->frame_w - dx, c->frame_h - dy, false);
            return;
        }
    }
    usleep(16000);
    return;
}

u32* window__get_icon(Window client, Display* dpy)
{
    Atom net_wm_icon = XInternAtom(dpy, "_NET_WM_ICON", false);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* data = NULL;

    if (XGetWindowProperty(
            dpy, 
            client, 
            net_wm_icon, 
            0, 
            ~0L,
            false, 
            XA_CARDINAL,
            &actual_type, 
            &actual_format, 
            &nitems, 
            &bytes_after, 
            &data
        ) == Success
    ) 
    {
        return (u32*)data;
    }
    else
    {
        elogf("[X11 error] Cannot retrieve icon for window %lu", client);
        return NULL;
    }
}