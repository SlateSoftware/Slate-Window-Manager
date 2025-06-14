#include <X11/Xlib.h>
#include <X11/Xutil.h>
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
        {
            free(((client_t*)evec__at(&client_frame_map, i))->name);
            memset(((client_t*)evec__at(&client_frame_map, i)), 0, sizeof(client_t));
        }
    }
    return;
}

void client__store(Window window, Window frame, char* name)
{
    client_t client = {
        .client = window,
        .frame = frame,
        .moving = 0,
        .drag_x = 0,
        .drag_y = 0,
        .name = name
    };
    evec__push(&client_frame_map, &client);
    return /*(client_t*)evec__at(&client_frame_map, client_frame_map.size-1)*/;
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
        attr.width + BORDER_WIDTH * 3,
        attr.height + TITLEBAR_HEIGHT + BORDER_WIDTH * 3,
        BORDER_WIDTH,
        BlackPixel(dpy, DefaultScreen(dpy)),
        WhitePixel(dpy, DefaultScreen(dpy))
    );
    XTextProperty prop;
    char* name = NULL;
    int count = 0;
    char** list = NULL;
    if (XGetWMName(dpy, window, &prop) && prop.value)
    {
        if (prop.encoding == XA_STRING)
            name = strdup((char *)prop.value);
        
        else if (XmbTextPropertyToTextList(dpy, &prop, &list, &count) >= Success && count > 0)
        {
            name = strdup(list[0]);
            XFreeStringList(list);
            list = NULL;
        }

        XFree(prop.value);
    }
    client__store(window, frame, name);
    name = NULL;
    XAddToSaveSet(dpy, window);
    XReparentWindow(dpy, window, frame, BORDER_WIDTH, BORDER_WIDTH + TITLEBAR_HEIGHT);
    XSelectInput(dpy, frame,
        SubstructureRedirectMask | SubstructureNotifyMask |
        ButtonPressMask | ButtonReleaseMask | 
        PointerMotionMask | StructureNotifyMask | 
        ExposureMask
    );

    XMapWindow(dpy, window);
    XMapWindow(dpy, frame);
    return;
}

void client__redraw_all_decorations(Display* dpy)
{
    for (u16 i = 0; i < client_frame_map.size; ++i)
    {
        if (evec__at(&client_frame_map, i))
            client__draw_decor(
                ((client_t*) evec__at(&client_frame_map, i))->frame, 
                dpy, 
                ((client_t*) evec__at(&client_frame_map, i))->name
            );
    }
}

void client__draw_decor(Window frame, Display* dpy, char* title)
{
    GC gc = XCreateGC(dpy, frame, 0, NULL);
    XSetForeground(dpy, gc, 0x888888);
    XFillRectangle(dpy, frame, gc, 0, 0, 800, TITLEBAR_HEIGHT);

    XSetForeground(dpy, gc, 0xFF0000);
    XFillRectangle(dpy, frame, gc, 2, 2, CLOSE_WIDTH, TITLEBAR_HEIGHT - 4);

    XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));
    if (title)
        XDrawString(dpy, frame, gc, 30, 15, title, strlen(title));

    XFreeGC(dpy, gc);
    return;
}

bool client__can_close(XEvent* ev, client_t* client, Display* dpy)
{
    int click_x = ev->xbutton.x;
    int click_y = ev->xbutton.y;

    XWindowAttributes attr;
    XGetWindowAttributes(dpy, client->frame, &attr);
    int w = attr.width;

    // Check if click is in close button box
    if (click_y < TITLEBAR_HEIGHT && click_x <= TITLEBAR_HEIGHT)
    {
        // Try graceful close
        Atom *protocols;
        int n;
        Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);

        if (XGetWMProtocols(dpy, client->client, &protocols, &n))
        {
            for (int i = 0; i < n; ++i)
            {
                if (protocols[i] == wm_delete)
                {
                    XEvent msg = {0};
                    msg.xclient.type = ClientMessage;
                    msg.xclient.message_type = wm_protocols;
                    msg.xclient.display = dpy;
                    msg.xclient.window = client->client;
                    msg.xclient.format = 32;
                    msg.xclient.data.l[0] = wm_delete;
                    msg.xclient.data.l[1] = CurrentTime;

                    XSendEvent(dpy, client->client, False, NoEventMask, &msg);
                    XFree(protocols);
                    return true;
                }
            }
            XFree(protocols);
        }

        // Fallback: force kill
        XKillClient(dpy, client->client);
        return true;
    }
    else
        return false;
}