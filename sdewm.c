#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include <intdef.h>
#include "client.h"

static bool wm_detecetd = false;

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
    client__initialize_map();
    XEvent ev;
    client_t* moving_window = NULL;
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
                client__create(ev.xmaprequest.window, display, root);
                break;
            }
            case ButtonPress:
            {
                if (ev.xbutton.window != None && ev.xbutton.subwindow == None && ev.xbutton.button == Button1 /*&& (ev.xbutton.state & Mod1Mask)*/)
                {
                    moving_window = client__retrieve_from(ev.xbutton.window, true);
                    if (moving_window != NULL)
                    {
                        printf("win: %ld, frame: %ld\nevw: %ld, evsw: %ld, root: %ld\n", moving_window->client, moving_window->frame, ev.xbutton.window, ev.xbutton.subwindow, ev.xbutton.root);
                        
                        XWindowAttributes attr;
                        XGetWindowAttributes(display, moving_window->client, &attr);
                        win_x = attr.x;
                        win_y = attr.y;
                        win_start_x = ev.xbutton.x_root;
                        win_start_y = ev.xbutton.y_root;
                        XGetWindowAttributes(display, moving_window->frame, &attr);
                        frame_x = attr.x;
                        frame_y = attr.y;
                        frame_start_x = ev.xbutton.x_root;
                        frame_start_y = ev.xbutton.y_root;
                        moving_window->moving = 1;
                    }
                }
                break;
            }
            case MotionNotify:
            {
                if (moving_window != NULL && moving_window->moving)
                {
                    XMoveWindow(display, moving_window->client, win_x, win_y);
                    int dx = ev.xmotion.x_root - frame_start_x;
                    int dy = ev.xmotion.y_root - frame_start_y;
                    XMoveWindow(display, moving_window->frame, frame_x + dx, frame_y + dy);
                }
                break;
            }
            case UnmapNotify:
            case DestroyNotify:
            {
                Window client = ev.xunmap.window;
                if (client__retrieve_from(client, true)->frame) 
                {
                    XDestroyWindow(display, client__retrieve_from(client, true)->frame);
                    client__forget(client); 
                }
                break;
            }
            case ButtonRelease:
            {
                if (moving_window && moving_window->moving && ev.xbutton.button == Button1)
                {
                    moving_window->moving = false;
                    moving_window = NULL;
                }
                break;
            }
            
            default:
                break;
        }
    }
    client__free_map();
    XCloseDisplay(display);
    return 0;
}