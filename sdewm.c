#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include <intdef.h>
#include <evec.h>

static bool wm_detecetd = false;
evec_t client_frame_map;

typedef struct _client_frame_pair
{
    Window window;
    Window frame;
} client_frame_pair_t;

void forget_client(Window client)
{
    for (u8 i = 0; i < client_frame_map.size; ++i)
    {
        if (((client_frame_pair_t*)evec__at(&client_frame_map, i))->window == client)
            memset(((client_frame_pair_t*)evec__at(&client_frame_map, i)), 0, sizeof(client_frame_pair_t));
        
    }
    return;
}

void store_client_frame_pair(Window client, Window frame)
{
    client_frame_pair_t pair = {
        .window = client,
        .frame = frame
    };
    evec__push(&client_frame_map, &pair);
}

Window retrieve_frame_from_client(Window client)
{
    for (u8 i = 0; i < client_frame_map.size; ++i)
    {
        if (((client_frame_pair_t*)evec__at(&client_frame_map, i))->window == client)
            return ((client_frame_pair_t*)evec__at(&client_frame_map, i))->frame;
    }
    return None;
}

int wm_error_handler(Display* dpy, XErrorEvent* ev)
{
    if (ev->error_code == BadAccess)
        wm_detecetd = true;

    return 0;
}

int main(void) 
{
    Display* display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "error: cannot open display\n");
        return EXIT_FAILURE;
    }

    Window root = DefaultRootWindow(display);
    Cursor default_cursor = XCreateFontCursor(display, XC_left_ptr);
    XDefineCursor(display, root, default_cursor);
    XSetErrorHandler(wm_error_handler);
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display, false);
    if (wm_detecetd)
    {
        fprintf(stderr, "error: another window manager is already running\n");
        return EXIT_FAILURE;
    }
    printf("sdewm: running\n");
    client_frame_map = evec__new(sizeof(client_frame_pair_t));
    XEvent ev;
    bool moving = false;
    Window moving_window = None;
    Window frame = None;
    int win_x = 0;
    int win_y = 0;
    int win_start_x = 0;
    int win_start_y = 0;
    int frame_x = 0;
    int frame_y = 0;
    int frame_start_x = 0;
    int frame_start_y = 0;

    while (true)
    {
        XNextEvent(display, &ev);
        switch (ev.type)
        {
            case MapRequest:
            {
                    Window client = ev.xmaprequest.window;
                    XWindowAttributes attr;
                    XGetWindowAttributes(display, client, &attr);

                    int border = 8;
                    int titlebar_height = 0;

                    Window frame = XCreateSimpleWindow(display, root,
                        attr.x, attr.y,
                        attr.width + border * 2,
                        attr.height + titlebar_height + border * 2,
                        border,
                        BlackPixel(display, DefaultScreen(display)),
                        WhitePixel(display, DefaultScreen(display))
                    );
                    store_client_frame_pair(client, frame);
                    XAddToSaveSet(display, client);
                    XReparentWindow(display, client, frame, border, border + titlebar_height);
                    XSelectInput(display, frame,
                        SubstructureRedirectMask | SubstructureNotifyMask |
                        ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
                    );

                    XMapWindow(display, client);
                    XMapWindow(display, frame);

                    break;
            }
            case ButtonPress:
            {
                if (ev.xbutton.subwindow != None && ev.xbutton.button == Button1 /*&& (ev.xbutton.state & Mod1Mask)*/)
                {
                    moving = true;
                    moving_window = ev.xbutton.subwindow;
                    frame = retrieve_frame_from_client(moving_window);
                    if (frame != None)
                    {
                        XWindowAttributes attr;
                        XGetWindowAttributes(display, moving_window, &attr);
                        win_x = attr.x;
                        win_y = attr.y;
                        win_start_x = ev.xbutton.x_root;
                        win_start_y = ev.xbutton.y_root;
                        XGetWindowAttributes(display, frame, &attr);
                        frame_x = attr.x;
                        frame_y = attr.y;
                        frame_start_x = ev.xbutton.x_root;
                        frame_start_y = ev.xbutton.y_root;
                    }
                }
                break;
            }
            case MotionNotify:
            {
                if (moving && moving_window != None)
                {
                    XMoveWindow(display, moving_window, win_x, win_y);
                    int dx = ev.xmotion.x_root - frame_start_x;
                    int dy = ev.xmotion.y_root - frame_start_y;
                    XMoveWindow(display, frame, frame_x + dx, frame_y + dy);
                }
                break;
            }
            case UnmapNotify:
            case DestroyNotify:
            {
                Window client = ev.xunmap.window;
                if (retrieve_frame_from_client(client)) 
                {
                    XDestroyWindow(display, retrieve_frame_from_client(client));
                    forget_client(client); 
                }
                break;
            }
            case ButtonRelease:
            {
                if (moving && ev.xbutton.button == Button1)
                {
                    moving = false;
                    moving_window = None;
                    frame = None;
                }
                break;
            }
            
            default:
                break;
        }
    }
    evec__free(&client_frame_map);
    XCloseDisplay(display);
    return 0;
}