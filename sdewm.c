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
    int win_x = 0;
    int win_y = 0;
    int start_x = 0;
    int start_y = 0;

    while (true)
    {
        XNextEvent(display, &ev);
        switch (ev.type)
        {
            case MapRequest:
            {
                    Window client = ev.xmaprequest.window;
                    // Create a frame window slightly bigger than the client
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
                    // Reparent client into frame
                    XAddToSaveSet(display, client);
                    XReparentWindow(display, client, frame, border, border + titlebar_height);

                    // Select input on the frame for future events
                    XSelectInput(display, frame,
                        SubstructureRedirectMask | SubstructureNotifyMask |
                        ButtonPressMask | ButtonReleaseMask | PointerMotionMask
                    );

                    // Map the frame and the client
                    XMapWindow(display, client);
                    XMapWindow(display, frame);

                    break;
            }
            case ButtonPress:
            {
                if (ev.xbutton.subwindow != None && ev.xbutton.button == Button1 && (ev.xbutton.state & Mod1Mask))
                {
                    moving = true;
                    moving_window = ev.xbutton.subwindow;
                    XWindowAttributes attr;
                    XGetWindowAttributes(display, moving_window, &attr);
                    win_x = attr.x;
                    win_y = attr.y;
                    start_x = ev.xbutton.x_root;
                    start_y = ev.xbutton.y_root;
                }
                break;
            }
            case MotionNotify:
            {
                if (moving && moving_window != None)
                {
                    int dx = ev.xmotion.x_root - start_x;
                    int dy = ev.xmotion.y_root - start_y;
                    XMoveWindow(display, moving_window, win_x + dx, win_y + dy);
                }
                break;
            }
            case ButtonRelease:
            {
                if (moving && ev.xbutton.button == Button1)
                {
                    moving = false;
                    moving_window = None;
                }
                break;
            }
            
            default:
                break;
        }
    }
    XCloseDisplay(display);
    return 0;
}