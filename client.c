#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <evec.h>
#include <string.h>
#include "client.h"

#define TITLEBAR_HEIGHT 24
#define BORDER_WIDTH 2
#define CLOSE_WIDTH 20

evec_t client_frame_map;

void client__initialize_map(void)
{
    client_frame_map = evec__new(sizeof(client_t));
    return;
}

void client__free_map(void)
{
    evec__free(&client_frame_map);
    return;
}

void client__forget(Window client)
{
    for (u8 i = 0; i < client_frame_map.size; ++i)
    {
        if (((client_t*)evec__at(&client_frame_map, i))->client == client)
            memset(((client_t*)evec__at(&client_frame_map, i)), 0, sizeof(client_t));
        
    }
    return;
}

void client__store(Window window, Window frame)
{
    client_t client = {
        .client = window,
        .frame = frame,
        .moving = 0,
        .drag_x = 0,
        .drag_y = 0
    };
    evec__push(&client_frame_map, &client);
    return;
}

client_t* client__retrieve_from(Window window, bool frame)
{
    for (u8 i = 0; i < client_frame_map.size; ++i)
    {
        if (frame) 
        {
            if (((client_t*)evec__at(&client_frame_map, i))->frame == window)
                return ((client_t*)evec__at(&client_frame_map, i));
        }
        else
        {
            if (((client_t*)evec__at(&client_frame_map, i))->client == window)
                return ((client_t*)evec__at(&client_frame_map, i));
        }
    }
    return NULL;
}

void client__create(Window window, Display* dpy, Window root)
{
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, window, &attr);
    XMoveWindow(dpy, window, attr.x + BORDER_WIDTH, attr.y + BORDER_WIDTH + TITLEBAR_HEIGHT);
    XGetWindowAttributes(dpy, window, &attr);
    Window frame = XCreateSimpleWindow(dpy, root,
        attr.x - BORDER_WIDTH, 
        attr.y - BORDER_WIDTH - TITLEBAR_HEIGHT,
        attr.width + BORDER_WIDTH * 4,
        attr.height + TITLEBAR_HEIGHT + BORDER_WIDTH * 4,
        BORDER_WIDTH,
        BlackPixel(dpy, DefaultScreen(dpy)),
        WhitePixel(dpy, DefaultScreen(dpy))
    );
    client__store(window, frame);
    XAddToSaveSet(dpy, window);
    XReparentWindow(dpy, window, frame, BORDER_WIDTH, BORDER_WIDTH + TITLEBAR_HEIGHT);
    XSelectInput(dpy, frame,
        SubstructureRedirectMask | SubstructureNotifyMask |
        ButtonPressMask | ButtonReleaseMask | 
        PointerMotionMask | StructureNotifyMask
    );

    XMapWindow(dpy, window);
    XMapWindow(dpy, frame);
    return;
}
