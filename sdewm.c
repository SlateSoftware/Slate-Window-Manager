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
    client_t* current_window = NULL;
    int win_x = 0;
    int win_y = 0;
    int frame_x = 0;
    int frame_y = 0;

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
                    current_window = client__retrieve_from(ev.xbutton.window, true);
                    if (current_window != NULL)
                    {
                        XRaiseWindow(display, current_window->frame);
                        XSetInputFocus(display, current_window->client, RevertToPointerRoot, CurrentTime);
                        if (!client__can_close(&ev, current_window, display))
                        {
                            //printf("win: %ld, frame: %ld\nevw: %ld, evsw: %ld, root: %ld\n", current_window->client, current_window->frame, ev.xbutton.window, ev.xbutton.subwindow, ev.xbutton.root);
                            XWindowAttributes attr;
                            XGetWindowAttributes(display, current_window->client, &attr);
                            win_x = attr.x;
                            win_y = attr.y;
                            XGetWindowAttributes(display, current_window->frame, &attr);
                            frame_x = attr.x;
                            frame_y = attr.y;
                            current_window->drag_x = ev.xbutton.x_root;
                            current_window->drag_y = ev.xbutton.y_root;
                            current_window->moving = 1;
                        }
                    }
                }
                else if (ev.xbutton.window != None && ev.xbutton.subwindow != None && ev.xbutton.button == Button1)
                {
                    current_window = client__retrieve_from(ev.xbutton.window, true);
                    if (current_window != NULL)
                    {
                        XRaiseWindow(display, current_window->frame);
                        XSetInputFocus(display, current_window->client, RevertToPointerRoot, CurrentTime);
                    }
                }
                break;
            }
            case MotionNotify:
            {
                if (current_window != NULL && current_window->moving)
                {
                    XMoveWindow(display, current_window->client, win_x, win_y);
                    int dx = ev.xmotion.x_root - current_window->drag_x;
                    int dy = ev.xmotion.y_root - current_window->drag_y;
                    XMoveWindow(display, current_window->frame, frame_x + dx, frame_y + dy);
                }
                break;
            }
            case UnmapNotify:
            case DestroyNotify:
            {
                Window client = ev.xunmap.window;
                if (client__retrieve_from(client, false)) 
                {
                    XDestroyWindow(display, client__retrieve_from(client, false)->frame);
                    client__forget(client); 
                }
                break;
            }
            case ButtonRelease:
            {
                if (current_window && current_window->moving && ev.xbutton.button == Button1)
                {
                    current_window->moving = false;
                    current_window->drag_x = 0;
                    current_window->drag_y = 0;
                    current_window = NULL;
                }
                else
                {
                    if (ev.xbutton.subwindow == None)
                    {
                        client_t* client = client__retrieve_from(ev.xbutton.window, true);
                        client__can_close(&ev, client, display);
                    }
                }
                break;
            }
            
            default:
                break;
        }
        client__redraw_all_decorations(display);
    }
    printf("quitting sdewm...");
    client__free_map();
    XCloseDisplay(display);
    printf("done\n");
    return 0;
}