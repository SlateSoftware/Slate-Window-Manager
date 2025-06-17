#ifndef CLIENT_H
#define CLIENT_H

#include <X11/Xlib.h>
#include <cairo/cairo-xlib.h>
#include <intdef.h>

typedef struct _client
{
    Window frame;
    Window client;
    u32 state;
    u32 drag_x; 
    u32 drag_y;
    u32 frame_x;
    u32 frame_y;
    u32 frame_w;
    u32 frame_h;
    u32 client_w;
    u32 client_h;
    cairo_surface_t* surface;
    cairo_t* cr;
    char* name;
} client_t;

#define MOVE_MASK (1 << 7)
#define RESIZE_REGION_MASK (0b111 << 0)
#define IS_RESIZING_MASK (1 << 2)
#define BOTTOM_LEFT_RESIZE_MASK (0b00 << 0)
#define BOTTOM_RIGHT_RESIZE_MASK (0b01 << 0)
#define TOP_RIGHT_RESIZE_MASK (0b10 << 0)
#define TOP_LEFT_RESIZE_MASK (0b11 << 0)

void client__initialize_map(void);
void client__free_map(void);
void client__forget(client_t* client);
void client__store(client_t* client);
client_t* client__retrieve_from(Window window, bool frame);
void client__redraw_all_decorations(Display* dpy);
void client__create(Window window, Display* dpy, Window root);
bool client__can_close(XEvent* ev, client_t* client, Display* dpy);
bool client__can_resize(XEvent* ev, client_t* client, Display* dpy, bool* is_resizing);
cairo_surface_t* client__get_cairo_surface(Drawable frame, Display* dpy, u32 w, u32 h, Visual* visual);

#endif