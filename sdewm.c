#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xcomposite.h>
#include <stdio.h>
#include <stdlib.h>
#include <intdef.h>
#include "core.h"
#include "client.h"
#include "window.h"

static bool wm_detecetd = false;

int wm_error_handler(Display* dpy, XErrorEvent* ev)
{
    if (ev->error_code == BadAccess)
        wm_detecetd = true;

    char err[256];
    XGetErrorText(dpy, ev->error_code, err, sizeof(err));
    elogf("[X11 error %d] %s", ev->error_code, err);
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
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask | ExposureMask);
    XSync(display, false);
    if (wm_detecetd)
    {
        fprintf(stderr, "error: another window manager is already running\n");
        return EXIT_FAILURE;
    }
    printf("sdewm: running\n");
    if (!core__init_log_stream())
    {
        fprintf(stderr, "error: could not initialize log file, quitting\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    client__initialize_map();
    XEvent ev;
    client_t* current_window = NULL;
    bool is_resizing = false;
    while (true)
    {
        XNextEvent(display, &ev);
        //logf("[EVENT] Type: %d", ev.type);
        switch (ev.type)
        {
            case MapRequest:
            {
                client__create(ev.xmaprequest.window, display, root);
                break;
            }
            case Expose:
            {
                logf("Got Expose event");
                if (!is_resizing)
                    client__redraw_all_decorations(display);
                
                break;
            }
            case ConfigureRequest: 
            {
                XConfigureRequestEvent *e = &ev.xconfigurerequest;

                XWindowChanges changes = {
                    .x = e->x,
                    .y = e->y,
                    .width = e->width,
                    .height = e->height,
                    .border_width = e->border_width,
                    .sibling = e->above,
                    .stack_mode = e->detail
                };

                XConfigureWindow(display, e->window, e->value_mask, &changes);
                break;
            }
            case ButtonPress:
            {
                if (ev.xbutton.button == Button2 && ev.xbutton.state & Mod1Mask)
                    goto end;

                if (ev.xbutton.window != None && ev.xbutton.subwindow == None && ev.xbutton.button == Button1 /*&& (ev.xbutton.state & Mod1Mask)*/)
                {
                    logf("Retrieving at click");
                    current_window = client__retrieve_from(ev.xbutton.window, true);
                    if (current_window != NULL)
                    {
                        XRaiseWindow(display, current_window->frame);
                        XSetInputFocus(display, current_window->client, RevertToPointerRoot, CurrentTime);
                        if (!client__can_close(&ev, current_window, display) && !client__can_resize(&ev, current_window, display, &is_resizing))
                        {
                            //printf("win: %ld, frame: %ld\nevw: %ld, evsw: %ld, root: %ld\n", current_window->client, current_window->frame, ev.xbutton.window, ev.xbutton.subwindow, ev.xbutton.root);
                            //XWindowAttributes attr;
                            //XGetWindowAttributes(display, current_window->frame, &attr);
                            //current_window->frame_x = attr.x;
                            //current_window->frame_y = attr.y;
                            current_window->drag_x = ev.xbutton.x_root;
                            current_window->drag_y = ev.xbutton.y_root;
                            current_window->state |= MOVE_MASK;
                            client__redraw_all_decorations(display);
                        }
                    }
                }
                else if (ev.xbutton.window != None && ev.xbutton.subwindow != None && ev.xbutton.button == Button1)
                {
                    logf("Retrieving at click on child");
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
                if (current_window != NULL && (current_window->state & MOVE_MASK))
                {
                    //XMoveWindow(display, current_window->client, win_x, win_y);
                    int dx = ev.xmotion.x_root - current_window->drag_x;
                    int dy = ev.xmotion.y_root - current_window->drag_y;
                    XMoveWindow(display, current_window->frame, current_window->frame_x + dx, current_window->frame_y + dy);
                }
                else if (current_window != NULL && is_resizing)
                {
                    window__handle_resize_event(&ev, current_window, display);
                }
                break;
            }
            case UnmapNotify:
            {
                Window client = ev.xunmap.window;
                logf("Retrieving at unmapping");
                client_t* _client = client__retrieve_from(client, false);
                if (_client) 
                {
                    XDestroyWindow(display, _client->frame);
                    client__forget(_client); 
                    logf("Valid client for unmapping");
                }
                break;
            }
            case DestroyNotify:
            {
                Window client = ev.xdestroywindow.window;
                logf("Retrieving at destruction");
                client_t* _client = client__retrieve_from(client, false);
                if (_client) 
                {
                    XDestroyWindow(display, _client->frame);
                    client__forget(_client); 
                    logf("Valid client for destruction");
                }
                break;
            }
            case ButtonRelease:
            {
                if (current_window && (current_window->state & MOVE_MASK) && ev.xbutton.button == Button1)
                {
                    XWindowAttributes attr;
                    XGetWindowAttributes(display, current_window->frame, &attr);
                    current_window->state &= ~MOVE_MASK;
                    current_window->drag_x = 0;
                    current_window->drag_y = 0;
                    current_window->frame_x = attr.x;
                    current_window->frame_y = attr.y;
                    current_window = NULL;
                }
                else if (current_window && is_resizing && ev.xbutton.button == Button1)
                {
                    XWindowAttributes attr;
                    XGetWindowAttributes(display, current_window->frame, &attr);
                    current_window->state &= ~RESIZE_REGION_MASK;
                    current_window->drag_x = 0;
                    current_window->drag_y = 0;
                    current_window->frame_x = attr.x;
                    current_window->frame_y = attr.y;
                    current_window->frame_w = attr.width;
                    current_window->frame_h = attr.height;
                    XGetWindowAttributes(display, current_window->client, &attr);
                    current_window->client_w = attr.width;
                    current_window->client_h = attr.height;
                    cairo_surface_destroy(current_window->surface);
                    cairo_destroy(current_window->cr);
                    current_window->surface = client__get_cairo_surface(current_window->frame, display, current_window->frame_w, current_window->frame_h, NULL);
                    current_window->cr = cairo_create(current_window->surface);
                    window__draw_decorations(current_window, display, 0, 0, true);
                    is_resizing = false;
                    current_window = NULL;
                }
                else
                {
                    /*if (ev.xbutton.subwindow == None)
                    {
                        client_t* client = client__retrieve_from(ev.xbutton.window, true);
                        client__can_close(&ev, client, display);
                    }*/
                }
                break;
            }
            
            default:
                break;
        }
    }
    end:
    printf("quitting sdewm...");
    client__free_map();
    core__close_log_stream();
    XCloseDisplay(display);
    printf("done\n");
    return 0;
}