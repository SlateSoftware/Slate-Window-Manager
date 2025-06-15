#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <evec.h>
#include <string.h>
#include "client.h"
#include "window.h"

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

void client__forget(client_t* client)
{
    cairo_destroy(client->cr);
    cairo_surface_destroy(client->surface);
    memset(client, 0, sizeof(client_t));
    return;
}

void client__store(client_t* client)
{
    client_t _client = {
        .client = client->client,
        .frame = client->frame,
        .moving = 0,
        .drag_x = 0,
        .drag_y = 0,
        .surface = client->surface,
        .cr = client->cr
    };
    evec__push(&client_frame_map, &_client);
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

Visual* get_argb_visual(Display* dpy)
{
    XVisualInfo vinfo_template = {0};
    vinfo_template.screen = DefaultScreen(dpy);
    int nitems;
    XVisualInfo* info = XGetVisualInfo(dpy, VisualScreenMask, &vinfo_template, &nitems);
    if (!XMatchVisualInfo(dpy, DefaultScreen(dpy), 32, TrueColor, info)) 
    {
        fprintf(stderr, "error: no ARGB visual found\n");
        exit(1);
    }
    for (int i = 0; i < nitems; ++i)
    {
        XRenderPictFormat* pict = XRenderFindVisualFormat(dpy, info[i].visual);
        if (pict && pict->type == PictTypeDirect && pict->direct.alphaMask)
        {
            Visual* v = info[i].visual;
            XFree(info);
            return v;
        }
    }
    XFree(info);
    return DefaultVisual(dpy, DefaultScreen(dpy));
}

void client__create(Window window, Display* dpy, Window root)
{
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, window, &attr);
    XMoveWindow(dpy, window, attr.x + CORNER_RADIUS, attr.y + CORNER_RADIUS);
    XGetWindowAttributes(dpy, window, &attr);
    Visual* visual = get_argb_visual(dpy);
    u8 depth = 32;
    Colormap colormap = XCreateColormap(dpy, root, visual, AllocNone);

    XSetWindowAttributes sattr;
    sattr.colormap = colormap;
    sattr.background_pixel = 0;
    sattr.border_pixel = 0;
    sattr.override_redirect = false;

    Window frame = XCreateWindow(
        dpy,
        root,
        attr.x - CORNER_RADIUS, 
        attr.y - CORNER_RADIUS,
        attr.width + CORNER_RADIUS * 2,
        attr.height + CORNER_RADIUS * 2 + TITLEBAR_HEIGHT,
        0,
        depth,
        InputOutput,
        visual,
        CWColormap | CWBackPixel | CWBorderPixel,
        &sattr
    );
    
    XAddToSaveSet(dpy, window);
    XSelectInput(dpy, frame,
        SubstructureRedirectMask | SubstructureNotifyMask |
        ButtonPressMask | ButtonReleaseMask | 
        PointerMotionMask | StructureNotifyMask | 
        ExposureMask
    );
    
    XReparentWindow(dpy, window, frame, CORNER_RADIUS, CORNER_RADIUS + TITLEBAR_HEIGHT);
    XMapWindow(dpy, window);
    XMapWindow(dpy, frame);

    cairo_surface_t* surface = cairo_xlib_surface_create(
        dpy, 
        frame, 
        visual, 
        attr.width + CORNER_RADIUS * 2,
        attr.height + CORNER_RADIUS * 2 + TITLEBAR_HEIGHT
    );

    cairo_t* cr = cairo_create(surface);

    client_t client = {
        .client = window,
        .frame = frame,
        .surface = surface,
        .cr = cr
    };

    client__store(&client);
    window__draw_decorations(&client, dpy);
    return;
}

void client__redraw_all_decorations(Display* dpy)
{
    for (u16 i = 0; i < client_frame_map.size; ++i)
    {
        if (evec__at(&client_frame_map, i) && ((client_t*) evec__at(&client_frame_map, i))->frame)
        {
            XClearArea(dpy, ((client_t*) evec__at(&client_frame_map, i))->frame, 0, 0, 0, 0, True);
            XClearWindow(dpy, ((client_t*) evec__at(&client_frame_map, i))->frame);
            XClearArea(dpy, ((client_t*) evec__at(&client_frame_map, i))->client, 0, 0, 0, 0, True);
            XClearWindow(dpy, ((client_t*) evec__at(&client_frame_map, i))->client);
            window__draw_decorations(
                ((client_t*) evec__at(&client_frame_map, i)), 
                dpy
            );
        }
    }
}

bool client__can_close(XEvent* ev, client_t* client, Display* dpy)
{
    int click_x = ev->xbutton.x;
    int click_y = ev->xbutton.y;

    XWindowAttributes attr;
    XGetWindowAttributes(dpy, client->frame, &attr);
    int w = attr.width;

    if (click_y < TITLEBAR_HEIGHT && click_x <= TITLEBAR_HEIGHT)
    { 
        Atom* protocols;
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
                    logf("Closing client");
                    XSendEvent(dpy, client->client, False, NoEventMask, &msg);
                    XFree(protocols);
                    return true;
                }
            }
            XFree(protocols);
        }

        XKillClient(dpy, client->client);
        return true;
    }
    else
        return false;
}