#ifndef CLIENT_H
#define CLIENT_H

#include <X11/Xlib.h>
#include <intdef.h>

typedef struct _client
{
    Window frame;
    Window client;
    u32 moving;
    u32 drag_x; 
    u32 drag_y;
    char* name;
} client_t;

void client__initialize_map(void);
void client__free_map(void);
void client__forget(Window client);
void client__store(Window window, Window frame, char* name);
client_t* client__retrieve_from(Window window, bool frame);
void client__draw_decor(Window frame, Display* dpy, char* title);
void client__redraw_all_decorations(Display* dpy);
void client__create(Window window, Display* dpy, Window root);
bool client__can_close(XEvent* ev, client_t* client, Display* dpy);

#endif